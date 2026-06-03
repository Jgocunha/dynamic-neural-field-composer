#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Dimensionality (collapse / expand) (example)",
			5.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// Shared dimensions reused across the models.
		const element::ElementDimensions dim1D(50, 0.5);             // size 50, step 0.5
		const element::ElementDimensions dim2D(50, 50, 0.5, 0.5);    // 50x50, step 0.5

		const element::NeuralFieldParameters nfp{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } };
		const element::NeuralField2DParameters nfp2D{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } };
		const element::GaussKernelParameters gkp{ 3.0, 4.0, -0.01, true, true };
		const element::GaussKernel2DParameters gkp2D{ 3.0, 4.0, -0.01, true, true };

		// ──────────────────────────────────────────────────────────────────
		// Expansion (direct): 1D stim → 1D field (+kernel) → Expand → 2D field (+kernel)
		// ──────────────────────────────────────────────────────────────────
		const auto a_stim = std::make_shared<element::GaussStimulus>(
			element::ElementCommonParameters{ "a: stimulus (1d)", dim1D },
			element::GaussStimulusParameters{ 2.5, 10.0, 12.5 });
		const auto a_u = std::make_shared<element::NeuralField>(
			element::ElementCommonParameters{ "a: field u (1d)", dim1D }, nfp);
		const auto a_kUU = std::make_shared<element::GaussKernel>(
			element::ElementCommonParameters{ "a: kernel u-u (1d)", dim1D }, gkp);
		const auto a_expand = std::make_shared<element::Expand>(
			element::ElementCommonParameters{ "a: expand u-v", dim2D },
			element::ExpandParameters{ element::ProjectionAxis::X, dim1D });
		const auto a_v = std::make_shared<element::NeuralField2D>(
			element::ElementCommonParameters{ "a: field v (2d)", dim2D }, nfp2D);
		const auto a_kVV = std::make_shared<element::GaussKernel2D>(
			element::ElementCommonParameters{ "a: kernel v-v (2d)", dim2D }, gkp2D);

		simulation->addElement(a_stim);
		simulation->addElement(a_u);
		simulation->addElement(a_kUU);
		simulation->addElement(a_expand);
		simulation->addElement(a_v);
		simulation->addElement(a_kVV);

		a_u->addInput(a_stim);
		a_u->addInput(a_kUU);
		a_kUU->addInput(a_u);
		a_expand->addInput(a_u);
		a_v->addInput(a_expand);
		a_v->addInput(a_kVV);
		a_kVV->addInput(a_v);

		// ──────────────────────────────────────────────────────────────────
		// Collapse (direct): 2D stim → 2D field (+kernel) → Collapse → 1D field (+kernel)
		// ──────────────────────────────────────────────────────────────────
		const auto c_stim = std::make_shared<element::GaussStimulus2D>(
			element::ElementCommonParameters{ "b: stimulus (2d)", dim2D },
			element::GaussStimulus2DParameters{ 2.5, 10.0, 12.5, 12.5 });
		const auto c_u = std::make_shared<element::NeuralField2D>(
			element::ElementCommonParameters{ "b: field u (2d)", dim2D }, nfp2D);
		const auto c_kUU = std::make_shared<element::GaussKernel2D>(
			element::ElementCommonParameters{ "b: kernel u-u (2d)", dim2D }, gkp2D);
		const auto c_collapse = std::make_shared<element::Collapse>(
			element::ElementCommonParameters{ "b: collapse u-v", dim1D },
			element::CollapseParameters{ element::CompressionType::SUM, element::ProjectionAxis::X, dim2D });
		const auto c_v = std::make_shared<element::NeuralField>(
			element::ElementCommonParameters{ "b: field v (1d)", dim1D }, nfp);
		const auto c_kVV = std::make_shared<element::GaussKernel>(
			element::ElementCommonParameters{ "b: kernel v-v (1d)", dim1D }, gkp);

		simulation->addElement(c_stim);
		simulation->addElement(c_u);
		simulation->addElement(c_kUU);
		simulation->addElement(c_collapse);
		simulation->addElement(c_v);
		simulation->addElement(c_kVV);

		c_u->addInput(c_stim);
		c_u->addInput(c_kUU);
		c_kUU->addInput(c_u);
		c_collapse->addInput(c_u);
		c_v->addInput(c_collapse);
		c_v->addInput(c_kVV);
		c_kVV->addInput(c_v);

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
