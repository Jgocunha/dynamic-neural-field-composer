#include "application/application.h"
#include "user_interface/main_window.h"
#include "user_interface/field_metrics_window.h"
#include "user_interface/element_window.h"
#include "user_interface/plot_control_window.h"
#include "user_interface/simulation_window.h"
#include "user_interface/node_graph_window.h"
#include "elements/element_factory.h"
#include "user_interface/plots_window.h"

#include <memory>
#include <vector>
#include <string>

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation     = std::make_shared<Simulation>("40 nf architecture", 5.0, 0.0, 0.0);
		const auto visualization  = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainWindow>();
		app.addWindow<imgui_kit::LogWindow>();
		app.addWindow<user_interface::FieldMetricsWindow>();
		app.addWindow<user_interface::ElementWindow>();
		app.addWindow<user_interface::SimulationWindow>();
		app.addWindow<user_interface::PlotControlWindow>();
		app.addWindow<user_interface::PlotsWindow>();
		app.addWindow<user_interface::NodeGraphWindow>();

		// ------------------------------------------------------------
		// 40 neural fields: "nf 1" ... "nf 40"
		// each with:
		//   - self-excitation / local kernel
		//   - normal noise input
		// ------------------------------------------------------------

		constexpr int N = 100;

		std::vector<std::shared_ptr<element::NeuralField>> nfs;
		nfs.reserve(N);

		for (int i = 0; i < N; ++i)
		{
			const std::string idx      = std::to_string(i + 1);
			const std::string nf_name  = "nf " + idx;
			const std::string k_name   = "nf" + idx + " k";
			const std::string n_name   = "nf" + idx + " n";

			// field
			const auto nfcp = element::ElementCommonParameters{ nf_name };
			const auto nfp  = element::NeuralFieldParameters{
				25.0,
				-10.0,
				element::SigmoidFunction{ 0.0, 5.0 }
			};
			const auto nf = std::make_shared<element::NeuralField>(nfcp, nfp);
			simulation->addElement(nf);
			nfs.push_back(nf);

			// self kernel
			const auto kcp = element::ElementCommonParameters{ k_name };
			const auto kp  = element::GaussKernelParameters{
				20.0,   // amplitude
				2.0,    // width / sigma
				-0.01   // baseline
			};
			const auto k = std::make_shared<element::GaussKernel>(kcp, kp);
			simulation->addElement(k);
			nf->addInput(k);
			k->addInput(nf);

			// normal noise
			const auto nncp = element::ElementCommonParameters{ n_name };
			const auto nnp  = element::NormalNoiseParameters{ 0.30 };
			const auto nn   = std::make_shared<element::NormalNoise>(nncp, nnp);
			simulation->addElement(nn);
			nf->addInput(nn);
		}

		// ------------------------------------------------------------
		// interaction kernels between fields
		// directional couplings:
		//   nf i -> nf (i+1)
		//   nf i -> nf (i+5)
		// names kept short and readable, e.g. "nf1-nf2 k"
		// ------------------------------------------------------------

		auto make_coupling = [&](int src, int dst, double mu_shift)
		{
			const std::string src_idx = std::to_string(src + 1);
			const std::string dst_idx = std::to_string(dst + 1);
			const std::string name    = "nf" + src_idx + "-nf" + dst_idx + " k";

			const auto cp = element::ElementCommonParameters{ name };
			const auto p  = element::GaussKernelParameters{
				5.0,         // amplitude
				mu_shift,    // center / shift
				0.0          // baseline
			};

			const auto k = std::make_shared<element::GaussKernel>(cp, p);
			simulation->addElement(k);
			k->addInput(nfs[src]);
			nfs[dst]->addInput(k);
		};

		for (int i = 0; i < N; ++i)
		{
			const int next    = (i + 1)  % N;  // local neighbor
			const int farther = (i + 5)  % N;  // more "random-like" distant neighbor

			// short-range-ish coupling
			make_coupling(i, next,    8.0);   // nf i -> nf next

			// longer-range coupling
			make_coupling(i, farther, 14.0);  // nf i -> nf farther
		}

		// ------------------------------------------------------------
		// plots: for each neural field, plot activation & input
		// ------------------------------------------------------------
		for (const auto& nf : nfs)
		{
			visualization->plot({
				{ nf->getUniqueName(), "activation" },
				{ nf->getUniqueName(), "input" }
			});
		}

		app.init();

		while (!app.hasGUIBeenClosed())
		{
			app.step();
		}

		app.close();
	}
	catch (const dnf_composer::Exception& ex)
	{
		const std::string errorMessage =
			"Exception: " + std::string(ex.what()) +
			" ErrorCode: " + std::to_string(static_cast<int>(ex.getErrorCode())) + ". ";
		log(dnf_composer::tools::logger::LogLevel::FATAL,
			errorMessage,
			dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return static_cast<int>(ex.getErrorCode());
	}
	catch (const std::exception& ex)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL,
			"Exception caught: " + std::string(ex.what()) + ". ",
			dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
	catch (...)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL,
			"Unknown exception occurred. ",
			dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
}
