#include "application/application.h"
#include "user_interface/main_window.h"
#include "user_interface/field_metrics_window.h"
#include "user_interface/element_window.h"
#include "user_interface/plot_control_window.h"
#include "user_interface/simulation_window.h"
#include "user_interface/node_graph_window.h"
#include "elements/element_factory.h"
#include "user_interface/plots_window.h"


int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("example two robot team", 5.0, 0.0, 0.0);
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
		const auto hpscp = element::ElementCommonParameters{ "other robot motion stimulus" };
		const auto hpsp = element::GaussStimulusParameters{ 5, 15, 50 };
		const auto hps = std::make_shared < element::GaussStimulus > (hpscp, hpsp);
		simulation->addElement(hps);
		const auto so1scp = element::ElementCommonParameters{ "small object 1 stimulus" };
		const auto so1sp = element::GaussStimulusParameters{ 5, 15, 20 };
		const auto so1s = std::make_shared < element::GaussStimulus > (so1scp, so1sp);
		simulation->addElement(so1s);
		const auto so2scp = element::ElementCommonParameters{ "small object 2 stimulus" };
		const auto so2sp = element::GaussStimulusParameters{ 5, 15, 80 };
		const auto so2s = std::make_shared < element::GaussStimulus >(so2scp, so2sp);
		simulation->addElement(so2s);
		const auto loscp = element::ElementCommonParameters{ "large object stimulus" };
		const auto losp = element::GaussStimulusParameters{ 5, 15, 50 };
		const auto los = std::make_shared < element::GaussStimulus > (loscp, losp);
		simulation->addElement(los);

		// fields
		const auto hpfcp = element::ElementCommonParameters{ "sts other robot motion field" };
		const auto hpfp = element::NeuralFieldParameters{ 25.0, -10.0, element::SigmoidFunction{0.0, 5.0} };
		const auto hpf = std::make_shared < element::NeuralField > ( hpfcp, hpfp );
		simulation->addElement(hpf);
		hpf->addInput(hps);
		const auto sofcp = element::ElementCommonParameters{ "wm small object field" };
		const auto sofp = element::NeuralFieldParameters{ 25.0, -10.0, element::SigmoidFunction{0.0, 5.0} };
		const auto sof = std::make_shared < element::NeuralField > (sofcp, sofp );
		simulation->addElement(sof);
		sof->addInput(so1s);
		sof->addInput(so2s);
		const auto lofcp = element::ElementCommonParameters{ "wm large object field" };
		const auto lop = element::NeuralFieldParameters{ 25.0, -10.0, element::SigmoidFunction{0.0, 5.0} };
		const auto lof = std::make_shared < element::NeuralField >(lofcp, lop);
		simulation->addElement(lof);
		lof->addInput(los);
		const auto aefcp = element::ElementCommonParameters{ "small object action execution field" };
		const auto aefp = element::NeuralFieldParameters{ 25.0, -10.0, element::SigmoidFunction{0.0, 5.0} };
		const auto aef = std::make_shared < element::NeuralField >(aefcp, aefp);
		simulation->addElement(aef);
		const auto aefcp_ = element::ElementCommonParameters{ "large object action execution field" };
		const auto aefp_ = element::NeuralFieldParameters{ 25.0, -10.0, element::SigmoidFunction{0.0, 5.0} };
		const auto aef_ = std::make_shared < element::NeuralField >(aefcp_, aefp_);
		simulation->addElement(aef_);
		const auto sosfcp = element::ElementCommonParameters{ "small object goal field" };
		const auto sosp = element::NeuralFieldParameters{ 25.0, -10.0, element::SigmoidFunction{0.0, 5.0} };
		const auto sos = std::make_shared < element::NeuralField >(sosfcp, sosp);
		simulation->addElement(sos);
		const auto loifcp = element::ElementCommonParameters{ "large object goal field" };
		const auto loifp = element::NeuralFieldParameters{ 25.0, -10.0, element::SigmoidFunction{0.0, 5.0} };
		const auto loif = std::make_shared < element::NeuralField >(loifcp, loifp);
		simulation->addElement(loif);

		// self-excitation kernels
		const auto hpfkcp = element::ElementCommonParameters{ "sts.ormf kernel" };
		const auto hpfkp = element::GaussKernelParameters{ 20.0, 2.0, -0.01 };
		const auto hpfk = std::make_shared < element::GaussKernel >(hpfkcp, hpfkp);
		simulation->addElement(hpfk);
		hpf->addInput(hpfk);
		hpfk->addInput(hpf);
		const auto sofkcp = element::ElementCommonParameters{ "wm.sof kernel" };
		const auto sofkp = element::MexicanHatKernelParameters{ 5.0, 15.0, 10.0, 15.0, -0.01 };
		const auto sofk = std::make_shared < element::MexicanHatKernel >(sofkcp, sofkp);
		simulation->addElement(sofk);
		sof->addInput(sofk);
		sofk->addInput(sof);
		const auto lofkcp = element::ElementCommonParameters{ "wm.lof kernel" };
		const auto lofkp = element::GaussKernelParameters{ 20.0, 2.0, -0.01 };
		const auto lofk = std::make_shared < element::GaussKernel >(lofkcp, lofkp);
		simulation->addElement(lofk);
		lof->addInput(lofk);
		lofk->addInput(lof);
		const auto sosfkcp = element::ElementCommonParameters{ "sogf kernel" };
		const auto sosfkp = element::GaussKernelParameters{ 18.92, 23.22, -0.23 };
		const auto sosfk = std::make_shared < element::GaussKernel >(sosfkcp, sosfkp);
		simulation->addElement(sosfk);
		sos->addInput(sosfk);
		sosfk->addInput(sos);
		const auto loifkcp = element::ElementCommonParameters{ "logf kernel" };
		const auto loifkp = element::GaussKernelParameters{ 20.0, 2.0, -0.01 };
		const auto loifk = std::make_shared < element::GaussKernel >(loifkcp, loifkp);
		simulation->addElement(loifk);
		loif->addInput(loifk);
		loifk->addInput(loif);
		const auto aefkcp = element::ElementCommonParameters{ "so.aef kernel" };
		const auto aefkp = element::GaussKernelParameters{ 5.09, 7.85, -0.42 };
		const auto aefk = std::make_shared < element::GaussKernel >(aefkcp, aefkp);
		simulation->addElement(aefk);
		aef->addInput(aefk);
		aefk->addInput(aef);
		const auto aefkcp_ = element::ElementCommonParameters{ "lo.aef kernel" };
		const auto aefkp_ = element::GaussKernelParameters{ 5.09, 7.85, -0.42 };
		const auto aefk_ = std::make_shared < element::GaussKernel >(aefkcp_, aefkp_);
		simulation->addElement(aefk_);
		aef_->addInput(aefk_);
		aefk_->addInput(aef_);

		// couplings
		const auto hpf_loif_cp = element::ElementCommonParameters{ "ormf - logf coupling" };
		const auto hpf_loif_p = element::GaussKernelParameters{ 5.0, 10.47, 0.0};
		const auto hpf_loif = std::make_shared < element::GaussKernel >(hpf_loif_cp, hpf_loif_p);
		simulation->addElement(hpf_loif);
		hpf_loif->addInput(hpf);
		loif->addInput(hpf_loif);
		const auto hpf_sosf_cp = element::ElementCommonParameters{ "ormf - sogf coupling" };
		const auto hpf_sosf_p = element::GaussKernelParameters{ 5.0, -6.65, 0.0 };
		const auto hpf_sosf = std::make_shared < element::GaussKernel >( hpf_sosf_cp, hpf_sosf_p );
		simulation->addElement(hpf_sosf);
		hpf_sosf->addInput(hpf);
		sos->addInput(hpf_sosf);
		const auto sof_sosf_cp = element::ElementCommonParameters{ "wm.sof - sogf coupling" };
		const auto sof_sosf_p = element::GaussKernelParameters{ 2.96, 10.75, 0.0 };
		const auto sof_sosf = std::make_shared < element::GaussKernel >( sof_sosf_cp, sof_sosf_p );
		simulation->addElement(sof_sosf);
		sof_sosf->addInput(sof);
		sos->addInput(sof_sosf);
		const auto lof_loif_cp = element::ElementCommonParameters{ "wm.lof - logf coupling" };
		const auto lof_loif_p = element::GaussKernelParameters{ 5.0, 10.17, 0.0 };
		const auto lof_loif = std::make_shared < element::GaussKernel >( lof_loif_cp, lof_loif_p );
		simulation->addElement(lof_loif);
		lof_loif->addInput(lof);
		loif->addInput(lof_loif);
		const auto sosf_aef_cp = element::ElementCommonParameters{ "sogf - so.aef coupling" };
		const auto sosf_aef_p = element::GaussKernelParameters{ 5.0, 26.0, 0.0 };
		const auto sosf_aef = std::make_shared < element::GaussKernel >(sosf_aef_cp, sosf_aef_p);
		simulation->addElement(sosf_aef);
		sosf_aef->addInput(sos);
		aef->addInput(sosf_aef);
		const auto loif_aef_cp = element::ElementCommonParameters{ "logf - lo.aef coupling" };
		const auto loif_aef_p = element::GaussKernelParameters{ 5.0, 26.0, 0.0 };
		const auto loif_aef = std::make_shared < element::GaussKernel >(loif_aef_cp, loif_aef_p);
		simulation->addElement(loif_aef);
		loif_aef->addInput(loif);
		aef_->addInput(loif_aef);
		const auto logf_soaef_cp = element::ElementCommonParameters{ "logf - so.aef coupling" };
		const auto logf_soaef_p = element::GaussKernelParameters{ 20.0, -10.0, -0.8 };
		const auto logf_soaef = std::make_shared < element::GaussKernel >(logf_soaef_cp, logf_soaef_p);
		simulation->addElement(logf_soaef);
		logf_soaef->addInput(loif);
		aef->addInput(logf_soaef);

		// normal noise
		const auto nnsosfcp = element::ElementCommonParameters{ "sogf normal noise" };
		const auto nnsosfp = element::NormalNoiseParameters{0.32};
		const auto nnsos = std::make_shared < element::NormalNoise > (nnsosfcp, nnsosfp);
		simulation->addElement(nnsos);
		sos->addInput(nnsos);

		const auto nnaefcp = element::ElementCommonParameters{ "so-aef normal noise" };
		const auto nnaefp = element::NormalNoiseParameters{0.36};
		const auto nnaef = std::make_shared < element::NormalNoise > (nnaefcp, nnaefp);
		simulation->addElement(nnaef);
		aef->addInput(nnaef);
		const auto nnaefcp_ = element::ElementCommonParameters{ "lo-aef normal noise" };
		const auto nnaefp_ = element::NormalNoiseParameters{ 0.36 };
		const auto nnaef_ = std::make_shared < element::NormalNoise >(nnaefcp_, nnaefp_);
		simulation->addElement(nnaef_);
		aef_->addInput(nnaef_);

		visualization->plot({ {hpf->getUniqueName(), "activation"}, {hpf->getUniqueName(), "input"} });
		visualization->plot({ {sof->getUniqueName(), "activation"}, {sof->getUniqueName(), "input"} });
		visualization->plot({ {lof->getUniqueName(), "activation"}, {lof->getUniqueName(), "input"} });
		visualization->plot({ {sos->getUniqueName(), "activation"}, {sos->getUniqueName(), "input"} });
		visualization->plot({ {loif->getUniqueName(), "activation"}, {loif->getUniqueName(), "input"} });
		visualization->plot({ {aef->getUniqueName(), "activation"}, {aef->getUniqueName(), "input"} });
		visualization->plot({ {aef_->getUniqueName(), "activation"}, {aef_->getUniqueName(), "input"} });

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
