// Headless tests for ElementWindow — comprehensive coverage of a 2515-LOC UI component.
//
// Strategy: Vary element types, simulation state, and focused elements to exercise
// all parameter-editing code paths. Each element type gets parameter branches.

#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "ui_test_harness.h"

#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_stimulus_2d.h"
#include "elements/gauss_kernel.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/mexican_hat_kernel_2d.h"
#include "elements/normal_noise.h"
#include "elements/normal_noise_2d.h"
#include "elements/correlated_normal_noise.h"
#include "elements/correlated_normal_noise_2d.h"
#include "elements/field_coupling.h"
#include "elements/gauss_field_coupling.h"
#include "elements/oscillatory_kernel.h"
#include "elements/oscillatory_kernel_2d.h"
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/asymmetric_gauss_kernel_2d.h"
#include "elements/boost_stimulus.h"
#include "elements/boost_stimulus_2d.h"
#include "elements/memory_trace.h"
#include "elements/memory_trace_2d.h"
#include "elements/timed_gauss_stimulus.h"
#include "elements/timed_gauss_stimulus_2d.h"
#include "user_interface/element_window.h"

using namespace dnf_composer;

namespace
{
	std::shared_ptr<Simulation> makeEmptySimulation()
	{
		auto simulation = std::make_shared<Simulation>("test-empty", 1.0, 0.0, 0.0);
		return simulation;
	}

	std::shared_ptr<Simulation> make1DSimulation()
	{
		auto simulation = std::make_shared<Simulation>("test-1d", 1.0, 0.0, 0.0);

		// NeuralField
		{
			const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
			const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
			const element::ElementCommonParameters common{ std::string("field1d"), 50 };
			simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));
		}

		// GaussStimulus
		{
			const element::ElementCommonParameters common{ std::string("stimulus"), 50 };
			const element::GaussStimulusParameters gsp{ 5.0, 15.0, 25.0 };
			simulation->addElement(std::make_shared<element::GaussStimulus>(common, gsp));
		}

		// GaussKernel
		{
			const element::ElementCommonParameters common{ std::string("kernel"), 50 };
			const element::GaussKernelParameters gkp{ 3.0, 1.0 };
			simulation->addElement(std::make_shared<element::GaussKernel>(common, gkp));
		}

		// NormalNoise
		{
			const element::ElementCommonParameters common{ std::string("noise"), 50 };
			const element::NormalNoiseParameters nnp{ 1.0 };
			simulation->addElement(std::make_shared<element::NormalNoise>(common, nnp));
		}

		simulation->init();
		return simulation;
	}

	std::shared_ptr<Simulation> make2DSimulation()
	{
		auto simulation = std::make_shared<Simulation>("test-2d", 1.0, 0.0, 0.0);

		// NeuralField2D
		{
			const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
			const element::NeuralField2DParameters nfp{ 25.0, -5.0, sigmoid };
			const element::ElementCommonParameters common{ std::string("field2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			simulation->addElement(std::make_shared<element::NeuralField2D>(common, nfp));
		}

		// GaussStimulus2D
		{
			const element::ElementCommonParameters common{ std::string("stimulus2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			const element::GaussStimulus2DParameters gsp{ 5.0, 10.0, 10.0, 10.0 };
			simulation->addElement(std::make_shared<element::GaussStimulus2D>(common, gsp));
		}

		// NormalNoise2D
		{
			const element::ElementCommonParameters common{ std::string("noise2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			const element::NormalNoise2DParameters nnp{ 1.0 };
			simulation->addElement(std::make_shared<element::NormalNoise2D>(common, nnp));
		}

		simulation->init();
		return simulation;
	}

	std::shared_ptr<Simulation> makeRichSimulation()
	{
		auto simulation = std::make_shared<Simulation>("test-rich", 1.0, 0.0, 0.0);

		// 1D Elements
		{
			const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
			const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
			const element::ElementCommonParameters common{ std::string("field1d"), 50 };
			simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));
		}

		{
			const element::ElementCommonParameters common{ std::string("stimulus1d"), 50 };
			const element::GaussStimulusParameters gsp{ 5.0, 15.0, 25.0 };
			simulation->addElement(std::make_shared<element::GaussStimulus>(common, gsp));
		}

		{
			const element::ElementCommonParameters common{ std::string("kernel1d"), 50 };
			const element::GaussKernelParameters gkp{ 3.0, 1.0 };
			simulation->addElement(std::make_shared<element::GaussKernel>(common, gkp));
		}

		{
			const element::ElementCommonParameters common{ std::string("mhat"), 50 };
			const element::MexicanHatKernelParameters mhkp{ 3.0, 1.0, 1.0, 0.25 };
			simulation->addElement(std::make_shared<element::MexicanHatKernel>(common, mhkp));
		}

		{
			const element::ElementCommonParameters common{ std::string("noise1d"), 50 };
			const element::NormalNoiseParameters nnp{ 1.0 };
			simulation->addElement(std::make_shared<element::NormalNoise>(common, nnp));
		}

		{
			const element::ElementCommonParameters common{ std::string("cor_noise"), 50 };
			const element::CorrelatedNormalNoiseParameters cnnp{ 1.0, 2.0 };
			simulation->addElement(std::make_shared<element::CorrelatedNormalNoise>(common, cnnp));
		}

		{
			const element::ElementCommonParameters common{ std::string("osc_kernel"), 50 };
			const element::OscillatoryKernelParameters okp{ 3.0, 1.0, 1.0 };
			simulation->addElement(std::make_shared<element::OscillatoryKernel>(common, okp));
		}

		{
			const element::ElementCommonParameters common{ std::string("asym_gauss"), 50 };
			const element::AsymmetricGaussKernelParameters agkp{ 3.0, 1.0, 2.0, 1.0 };
			simulation->addElement(std::make_shared<element::AsymmetricGaussKernel>(common, agkp));
		}

		{
			const element::ElementCommonParameters common{ std::string("boost"), 50 };
			const element::BoostStimulusParameters bsp{ 10.0, true };
			simulation->addElement(std::make_shared<element::BoostStimulus>(common, bsp));
		}

		{
			const element::ElementCommonParameters common{ std::string("memory"), 50 };
			const element::MemoryTraceParameters mtp{ 0.1, 0.01 };
			simulation->addElement(std::make_shared<element::MemoryTrace>(common, mtp));
		}

		{
			const element::ElementCommonParameters common{ std::string("timed_stim"), 50 };
			const element::TimedGaussStimulusParameters tgsp{ 5.0, 15.0, 25.0, { { 10.0, 5.0 } } };
			simulation->addElement(std::make_shared<element::TimedGaussStimulus>(common, tgsp));
		}

		// 2D Elements
		{
			const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
			const element::NeuralField2DParameters nfp{ 25.0, -5.0, sigmoid };
			const element::ElementCommonParameters common{ std::string("field2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			simulation->addElement(std::make_shared<element::NeuralField2D>(common, nfp));
		}

		{
			const element::ElementCommonParameters common{ std::string("stimulus2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			const element::GaussStimulus2DParameters gsp{ 5.0, 15.0, 10.0, 10.0 };
			simulation->addElement(std::make_shared<element::GaussStimulus2D>(common, gsp));
		}

		{
			const element::ElementCommonParameters common{ std::string("kernel2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			const element::GaussKernel2DParameters gkp{ 3.0, 3.0, -0.01 };
			simulation->addElement(std::make_shared<element::GaussKernel2D>(common, gkp));
		}

		{
			const element::ElementCommonParameters common{ std::string("mhat2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			const element::MexicanHatKernel2DParameters mhkp{ 2.5, 11.0, 5.0, 15.0, -0.1 };
			simulation->addElement(std::make_shared<element::MexicanHatKernel2D>(common, mhkp));
		}

		{
			const element::ElementCommonParameters common{ std::string("noise2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			const element::NormalNoise2DParameters nnp{ 1.0 };
			simulation->addElement(std::make_shared<element::NormalNoise2D>(common, nnp));
		}

		{
			const element::ElementCommonParameters common{ std::string("cor_noise2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			const element::CorrelatedNormalNoise2DParameters cnnp{ 1.0, 2.0, true };
			simulation->addElement(std::make_shared<element::CorrelatedNormalNoise2D>(common, cnnp));
		}

		{
			const element::ElementCommonParameters common{ std::string("osc2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			// The last two parameters are bools (circular, normalized) -- passing 1.0
			// here is a narrowing conversion that MSVC accepts but clang/gcc reject.
			const element::OscillatoryKernel2DParameters okp{ 3.0, 1.0, 1.0, 3.0, true, true };
			simulation->addElement(std::make_shared<element::OscillatoryKernel2D>(common, okp));
		}

		{
			const element::ElementCommonParameters common{ std::string("boost2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			const element::BoostStimulus2DParameters bsp{ 10.0, true };
			simulation->addElement(std::make_shared<element::BoostStimulus2D>(common, bsp));
		}

		{
			const element::ElementCommonParameters common{ std::string("memory2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			const element::MemoryTrace2DParameters mtp{ 0.1, 0.01 };
			simulation->addElement(std::make_shared<element::MemoryTrace2D>(common, mtp));
		}

		{
			const element::ElementCommonParameters common{ std::string("timed2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			const element::TimedGaussStimulus2DParameters tgsp{ 10.0, 15.0, 5.0, 5.0, { { 10.0, 5.0 } } };
			simulation->addElement(std::make_shared<element::TimedGaussStimulus2D>(common, tgsp));
		}

		{
			const element::ElementCommonParameters common{ std::string("asym2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
			const element::AsymmetricGaussKernel2DParameters agkp{ 3.0, 3.0, 0.0, 1.0, 1.0 };
			simulation->addElement(std::make_shared<element::AsymmetricGaussKernel2D>(common, agkp));
		}

		simulation->init();
		return simulation;
	}
}

class ElementWindowTest : public ::testing::Test
{
protected:
	test::HeadlessImGui gui;
};

TEST_F(ElementWindowTest, RenderEmptySimulation)
{
	const auto simulation = makeEmptySimulation();
	user_interface::ElementWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(ElementWindowTest, RenderWith1DElements)
{
	const auto simulation = make1DSimulation();
	user_interface::ElementWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(ElementWindowTest, RenderWith2DElements)
{
	const auto simulation = make2DSimulation();
	user_interface::ElementWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(ElementWindowTest, RenderWithRichSimulation)
{
	const auto simulation = makeRichSimulation();
	user_interface::ElementWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(ElementWindowTest, RenderAfterSimulationSteps)
{
	const auto simulation = makeRichSimulation();
	for (int i = 0; i < 3; ++i)
		simulation->step();

	user_interface::ElementWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(ElementWindowTest, RenderElementControlCard)
{
	const auto simulation = make1DSimulation();
	user_interface::ElementWindow window{ simulation };

	auto elements = simulation->getElements();
	ASSERT_FALSE(elements.empty());
	auto element = elements[0];

	user_interface::ElementWindow::setFocusedElement(element);

	gui.frames(2, [&] {
		window.renderElementControlCard();
	});
	SUCCEED();
}

TEST_F(ElementWindowTest, RenderModifyElementParameters)
{
	const auto simulation = makeRichSimulation();
	user_interface::ElementWindow window{ simulation };

	auto elements = simulation->getElements();
	ASSERT_FALSE(elements.empty());
	auto element = elements[0];

	user_interface::ElementWindow::setFocusedElement(element);

	gui.frames(2, [&] {
		window.renderModifyElementParameters();
	});
	SUCCEED();
}

TEST_F(ElementWindowTest, FocusDifferentElementTypes1D)
{
	const auto simulation = make1DSimulation();
	user_interface::ElementWindow window{ simulation };

	auto elements = simulation->getElements();
	for (const auto& element : elements)
	{
		user_interface::ElementWindow::setFocusedElement(element);
		gui.frames(1, [&] {
			window.renderModifyElementParameters();
		});
	}
	SUCCEED();
}

TEST_F(ElementWindowTest, FocusDifferentElementTypes2D)
{
	const auto simulation = make2DSimulation();
	user_interface::ElementWindow window{ simulation };

	auto elements = simulation->getElements();
	for (const auto& element : elements)
	{
		user_interface::ElementWindow::setFocusedElement(element);
		gui.frames(1, [&] {
			window.renderModifyElementParameters();
		});
	}
	SUCCEED();
}

TEST_F(ElementWindowTest, FocusAllElementTypesInRichSimulation)
{
	const auto simulation = makeRichSimulation();
	user_interface::ElementWindow window{ simulation };

	auto elements = simulation->getElements();
	ASSERT_GT(elements.size(), 0u);

	for (const auto& element : elements)
	{
		user_interface::ElementWindow::setFocusedElement(element);
		gui.frames(1, [&] {
			window.render();
			window.renderElementControlCard();
			window.renderModifyElementParameters();
		});
	}
	SUCCEED();
}

TEST_F(ElementWindowTest, RenderBeforeSimulationInit)
{
	auto simulation = std::make_shared<Simulation>("test-before-init", 1.0, 0.0, 0.0);
	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters common{ std::string("field"), 50 };
	simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));
	// Don't call init()

	user_interface::ElementWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(ElementWindowTest, MultipleFramesAfterInit)
{
	const auto simulation = makeRichSimulation();
	user_interface::ElementWindow window{ simulation };

	// Multiple frames to catch state that changes after first render
	gui.frames(5, [&] { window.render(); });
	SUCCEED();
}

TEST_F(ElementWindowTest, SwitchFocusedElementMultipleTimes)
{
	const auto simulation = makeRichSimulation();
	user_interface::ElementWindow window{ simulation };

	auto elements = simulation->getElements();
	ASSERT_GT(elements.size(), 2u);

	for (int i = 0; i < static_cast<int>(elements.size()); ++i)
	{
		user_interface::ElementWindow::setFocusedElement(elements[i]);
		gui.frame([&] { window.render(); });
	}
	SUCCEED();
}

TEST_F(ElementWindowTest, RenderWith1DFieldAndCouplings)
{
	auto simulation = std::make_shared<Simulation>("test-couplings-1d", 1.0, 0.0, 0.0);

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };

	const element::ElementCommonParameters field1{ std::string("field1"), 50 };
	const element::ElementCommonParameters field2{ std::string("field2"), 50 };

	auto f1 = std::make_shared<element::NeuralField>(field1, nfp);
	auto f2 = std::make_shared<element::NeuralField>(field2, nfp);

	simulation->addElement(f1);
	simulation->addElement(f2);

	// Add coupling
	const element::ElementCommonParameters coupling{ std::string("coupling"), 50 };
	const element::FieldCouplingParameters fcp{ element::ElementDimensions{ 50, 1.0 } };
	simulation->addElement(std::make_shared<element::FieldCoupling>(coupling, fcp));

	simulation->init();

	user_interface::ElementWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(ElementWindowTest, RenderWith2DFieldAndCouplings)
{
	auto simulation = std::make_shared<Simulation>("test-couplings-2d", 1.0, 0.0, 0.0);

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralField2DParameters nfp{ 25.0, -5.0, sigmoid };

	const element::ElementCommonParameters field1{ std::string("field1"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
	const element::ElementCommonParameters field2{ std::string("field2"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };

	auto f1 = std::make_shared<element::NeuralField2D>(field1, nfp);
	auto f2 = std::make_shared<element::NeuralField2D>(field2, nfp);

	simulation->addElement(f1);
	simulation->addElement(f2);

	// Add 2D coupling
	const element::ElementCommonParameters coupling{ std::string("coupling"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
	const element::GaussFieldCouplingParameters gfcp{ element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
	simulation->addElement(std::make_shared<element::GaussFieldCoupling>(coupling, gfcp));

	simulation->init();

	user_interface::ElementWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(ElementWindowTest, SwitchElementToModify)
{
	const auto simulation = make1DSimulation();
	user_interface::ElementWindow window{ simulation };

	auto elements = simulation->getElements();
	ASSERT_FALSE(elements.empty());

	// switchElementToModify emits ImGui draw calls, so it must run inside a frame —
	// in production it is only ever reached from within ElementWindow::render().
	for (const auto& element : elements)
	{
		gui.frame([&] {
			window.render();
			user_interface::ElementWindow::switchElementToModify(element, "test-sim");
		});
	}
	SUCCEED();
}
