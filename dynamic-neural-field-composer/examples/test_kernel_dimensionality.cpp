#include "application/application.h"
#include "user_interface/main_window.h"
#include "user_interface/field_metrics_window.h"
#include "user_interface/element_window.h"
#include "user_interface/plot_control_window.h"
#include "user_interface/simulation_window.h"
#include "user_interface/node_graph_window.h"
#include "elements/element_factory.h"
#include "user_interface/plots_window.h"
#include "elements/kernel_coupling.h"


int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("test kernel dimensionality", 5.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainWindow>();
		app.addWindow<imgui_kit::LogWindow>();
		app.addWindow<user_interface::FieldMetricsWindow>();
		app.addWindow<user_interface::ElementWindow>();
		app.addWindow<user_interface::SimulationWindow>();
		app.addWindow<user_interface::PlotControlWindow>();
		app.addWindow<user_interface::PlotsWindow>();
		app.addWindow<user_interface::NodeGraphWindow>();

		// external stimuli
		const auto scp = element::ElementCommonParameters{ "stimulus" };
		const auto sp = element::GaussStimulusParameters{ 5, 15, 50 };
		const auto s = std::make_shared < element::GaussStimulus > (scp, sp);
		simulation->addElement(s);

		// fields
		const auto field_ucp = element::ElementCommonParameters{ "field u" };
		const auto field_up = element::NeuralFieldParameters{ 25.0, -10.0, element::SigmoidFunction{0.0, 5.0} };
		const auto field_u = std::make_shared < element::NeuralField > ( field_ucp, field_up );
		simulation->addElement(field_u);
		field_u->addInput(s);

		const auto field_vcp = element::ElementCommonParameters{ "field v", 200};
		const auto field_vp = element::NeuralFieldParameters{ 25.0, -10.0, element::SigmoidFunction{0.0, 5.0} };
		const auto field_v = std::make_shared < element::NeuralField > (field_vcp, field_vp );
		simulation->addElement(field_v);

		// self-excitation kernels
		const auto kernel_ucp = element::ElementCommonParameters{ "kernel u-u" };
		const auto kernel_up = element::GaussKernelParameters{ 20.0, 2.0, -0.01 };
		const auto kernel_u = std::make_shared < element::GaussKernel >(kernel_ucp, kernel_up);
		simulation->addElement(kernel_u);
		field_u->addInput(kernel_u);
		kernel_u->addInput(field_u);

		const auto kernel_vcp = element::ElementCommonParameters{ "kernel v-v", 200};
		const auto kernel_vp = element::MexicanHatKernelParameters{ 5.0, 15.0, 10.0, 15.0, -0.01 };
		const auto kernel_v = std::make_shared < element::MexicanHatKernel >(kernel_vcp, kernel_vp);
		simulation->addElement(kernel_v);
		field_v->addInput(kernel_v);
		kernel_v->addInput(field_v);

		// couplings
		// const auto u_v_cp = element::ElementCommonParameters{ "coupling u-v" };
		// const auto u_v_p = element::GaussKernelParameters{ 5.0, 18.00, 0.0};
		// const auto u_v = std::make_shared < element::GaussKernel >(u_v_cp, u_v_p);
		// simulation->addElement(u_v);
		// u_v->addInput(field_u);
		// field_v->addInput(u_v);
		const auto u_v_cp = element::ElementCommonParameters{ "coupling u-v"};
		const auto u_v_p = element::KernelCouplingParameters{ 5.0, 18.00, 0.0, true, true, 100, 200};
		const auto u_v = std::make_shared < element::KernelCoupling >(u_v_cp, u_v_p);
		simulation->addElement(u_v);
		u_v->addInput(field_u);
		field_v->addInput(u_v);

		// normal noise
		const auto nn_ucp = element::ElementCommonParameters{ "normal noise u" };
		const auto nn_up = element::NormalNoiseParameters{0.015};
		const auto nn_u = std::make_shared < element::NormalNoise > (nn_ucp, nn_up);
		simulation->addElement(nn_u);
		field_u->addInput(nn_u);

		const auto nn_vcp = element::ElementCommonParameters{ "normal noise v", 200 };
		const auto nn_vp = element::NormalNoiseParameters{0.015};
		const auto nn_v = std::make_shared < element::NormalNoise > (nn_vcp, nn_vp);
		simulation->addElement(nn_v);
		field_v->addInput(nn_v);

		visualization->plot({ {field_u->getUniqueName(), "activation"}, {field_u->getUniqueName(), "input"} });
		visualization->plot({ {field_v->getUniqueName(), "activation"}, {field_v->getUniqueName(), "input"} });

		app.init();

		while (!app.hasGUIBeenClosed())
		{
			app.step();
		}

		app.close();
	}
	catch (const dnf_composer::Exception& ex)
	{
		const std::string errorMessage = "Exception: " + std::string(ex.what()) + " ErrorCode: " + std::to_string(static_cast<int>(ex.getErrorCode())) + ". ";
		log(dnf_composer::tools::logger::LogLevel::FATAL, errorMessage, dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return static_cast<int>(ex.getErrorCode());
	}
	catch (const std::exception& ex)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL, "Exception caught: " + std::string(ex.what()) + ". ", dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
	catch (...)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL, "Unknown exception occurred. ", dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
}
