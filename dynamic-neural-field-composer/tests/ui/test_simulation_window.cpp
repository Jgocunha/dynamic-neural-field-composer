// Headless tests for SimulationWindow — comprehensive coverage of a 2351-LOC UI component.
//
// Strategy: Render the window in multiple states to exercise add/remove/set-interaction
// card rendering paths. Use various simulation configurations.

#include <gtest/gtest.h>

#include <memory>

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
#include "elements/memory_trace.h"
#include "elements/memory_trace_2d.h"
#include "user_interface/simulation_window.h"

using namespace dnf_composer;

namespace
{
	std::shared_ptr<Simulation> makeEmptySimulation()
	{
		return std::make_shared<Simulation>("test-empty", 1.0, 0.0, 0.0);
	}

	std::shared_ptr<Simulation> make1DSimulation()
	{
		auto simulation = std::make_shared<Simulation>("test-1d", 1.0, 0.0, 0.0);

		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common{ std::string("field1d"), 50 };
		simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));

		const element::ElementCommonParameters common2{ std::string("stimulus1d"), 50 };
		const element::GaussStimulusParameters gsp{ 5.0, 15.0, 25.0 };
		simulation->addElement(std::make_shared<element::GaussStimulus>(common2, gsp));

		const element::ElementCommonParameters common3{ std::string("kernel1d"), 50 };
		const element::GaussKernelParameters gkp{ 3.0, 1.0 };
		simulation->addElement(std::make_shared<element::GaussKernel>(common3, gkp));

		simulation->init();
		return simulation;
	}

	std::shared_ptr<Simulation> make2DSimulation()
	{
		auto simulation = std::make_shared<Simulation>("test-2d", 1.0, 0.0, 0.0);

		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralField2DParameters nfp{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common{ std::string("field2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
		simulation->addElement(std::make_shared<element::NeuralField2D>(common, nfp));

		const element::ElementCommonParameters common2{ std::string("stimulus2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
		const element::GaussStimulus2DParameters gsp{ 10.0, 10.0, 5.0, 5.0 };
		simulation->addElement(std::make_shared<element::GaussStimulus2D>(common2, gsp));

		simulation->init();
		return simulation;
	}

	std::shared_ptr<Simulation> makeRichSimulation()
	{
		auto simulation = std::make_shared<Simulation>("test-rich", 1.0, 0.0, 0.0);

		// 1D elements
		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };

		const element::ElementCommonParameters f1{ std::string("field1"), 50 };
		simulation->addElement(std::make_shared<element::NeuralField>(f1, nfp));

		const element::ElementCommonParameters s1{ std::string("stimulus"), 50 };
		const element::GaussStimulusParameters gsp{ 5.0, 15.0, 25.0 };
		simulation->addElement(std::make_shared<element::GaussStimulus>(s1, gsp));

		const element::ElementCommonParameters k1{ std::string("kernel"), 50 };
		const element::GaussKernelParameters gkp{ 3.0, 1.0 };
		simulation->addElement(std::make_shared<element::GaussKernel>(k1, gkp));

		const element::ElementCommonParameters mh1{ std::string("mhat"), 50 };
		const element::MexicanHatKernelParameters mhkp{ 3.0, 1.0, 1.0, 0.25 };
		simulation->addElement(std::make_shared<element::MexicanHatKernel>(mh1, mhkp));

		const element::ElementCommonParameters m1{ std::string("memory"), 50 };
		const element::MemoryTraceParameters mtp{ 0.1, 0.01 };
		simulation->addElement(std::make_shared<element::MemoryTrace>(m1, mtp));

		// 2D elements
		const element::NeuralField2DParameters nfp2d{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters f2{ std::string("field2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
		simulation->addElement(std::make_shared<element::NeuralField2D>(f2, nfp2d));

		const element::ElementCommonParameters s2{ std::string("stimulus2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
		const element::GaussStimulus2DParameters gsp2d{ 5.0, 15.0, 10.0, 10.0 };
		simulation->addElement(std::make_shared<element::GaussStimulus2D>(s2, gsp2d));

		const element::ElementCommonParameters k2{ std::string("kernel2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
		const element::GaussKernel2DParameters gkp2d{ 3.0, 3.0, -0.01 };
		simulation->addElement(std::make_shared<element::GaussKernel2D>(k2, gkp2d));

		const element::ElementCommonParameters m2{ std::string("memory2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
		const element::MemoryTrace2DParameters mtp2d{ 0.1, 0.01 };
		simulation->addElement(std::make_shared<element::MemoryTrace2D>(m2, mtp2d));

		simulation->init();
		return simulation;
	}
}

class SimulationWindowTest : public ::testing::Test
{
protected:
	test::HeadlessImGui gui;
};

TEST_F(SimulationWindowTest, RenderEmptySimulation)
{
	const auto simulation = makeEmptySimulation();
	user_interface::SimulationWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderWith1DElements)
{
	const auto simulation = make1DSimulation();
	user_interface::SimulationWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderWith2DElements)
{
	const auto simulation = make2DSimulation();
	user_interface::SimulationWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderWithRichSimulation)
{
	const auto simulation = makeRichSimulation();
	user_interface::SimulationWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderAfterSimulationSteps)
{
	const auto simulation = makeRichSimulation();
	for (int i = 0; i < 3; ++i)
		simulation->step();

	user_interface::SimulationWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderSidebarContents)
{
	const auto simulation = make1DSimulation();
	user_interface::SimulationWindow window{ simulation };

	gui.frames(2, [&] {
		window.renderSidebarContents();
	});
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderAddElementCard)
{
	const auto simulation = makeEmptySimulation();
	user_interface::SimulationWindow window{ simulation };

	gui.frames(2, [&] {
		window.renderAddElementCard();
	});
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderRemoveElementCard)
{
	const auto simulation = make1DSimulation();
	user_interface::SimulationWindow window{ simulation };

	gui.frames(2, [&] {
		window.renderRemoveElementCard();
	});
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderSetInteractionCard)
{
	const auto simulation = make1DSimulation();
	user_interface::SimulationWindow window{ simulation };

	gui.frames(2, [&] {
		window.renderSetInteractionCard();
	});
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderDataCard)
{
	const auto simulation = make1DSimulation();
	user_interface::SimulationWindow window{ simulation };

	gui.frames(2, [&] {
		window.renderDataCard();
	});
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderLogElementParametersCard)
{
	const auto simulation = make1DSimulation();
	user_interface::SimulationWindow window{ simulation };

	gui.frames(2, [&] {
		window.renderLogElementParametersCard();
	});
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderMonitoringCard)
{
	const auto simulation = make1DSimulation();
	user_interface::SimulationWindow window{ simulation };

	gui.frames(2, [&] {
		window.renderMonitoringCard();
	});
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderBeforeSimulationInit)
{
	auto simulation = std::make_shared<Simulation>("test-before-init", 1.0, 0.0, 0.0);
	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters common{ std::string("field"), 50 };
	simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));

	user_interface::SimulationWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(SimulationWindowTest, MultipleFrames)
{
	const auto simulation = makeRichSimulation();
	user_interface::SimulationWindow window{ simulation };

	// Multiple frames to catch state-dependent rendering
	gui.frames(5, [&] { window.render(); });
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderAllCardsWithRichSimulation)
{
	const auto simulation = makeRichSimulation();
	user_interface::SimulationWindow window{ simulation };

	gui.frames(2, [&] {
		window.renderSidebarContents();
		window.renderAddElementCard();
		window.renderRemoveElementCard();
		window.renderSetInteractionCard();
		window.renderDataCard();
		window.renderLogElementParametersCard();
		window.renderMonitoringCard();
	});
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderWithManyElements1D)
{
	auto simulation = std::make_shared<Simulation>("test-many-1d", 1.0, 0.0, 0.0);

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters field{ std::string("field"), 50 };
	simulation->addElement(std::make_shared<element::NeuralField>(field, nfp));

	// Add multiple stimuli
	for (int i = 0; i < 3; ++i)
	{
		const element::ElementCommonParameters stim{ "stim" + std::to_string(i), 50 };
		const element::GaussStimulusParameters gsp{ 5.0, 15.0, 25.0 };
		simulation->addElement(std::make_shared<element::GaussStimulus>(stim, gsp));
	}

	// Add multiple kernels
	for (int i = 0; i < 3; ++i)
	{
		const element::ElementCommonParameters kern{ "kernel" + std::to_string(i), 50 };
		const element::GaussKernelParameters gkp{ 3.0, 1.0 };
		simulation->addElement(std::make_shared<element::GaussKernel>(kern, gkp));
	}

	simulation->init();

	user_interface::SimulationWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderWithManyElements2D)
{
	auto simulation = std::make_shared<Simulation>("test-many-2d", 1.0, 0.0, 0.0);

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralField2DParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters field{ std::string("field"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
	simulation->addElement(std::make_shared<element::NeuralField2D>(field, nfp));

	// Add multiple 2D stimuli
	for (int i = 0; i < 2; ++i)
	{
		const element::ElementCommonParameters stim{ "stim2d" + std::to_string(i), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
		const element::GaussStimulus2DParameters gsp{ 10.0, 10.0, 5.0, 5.0 };
		simulation->addElement(std::make_shared<element::GaussStimulus2D>(stim, gsp));
	}

	simulation->init();

	user_interface::SimulationWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderAfterMultipleSteps)
{
	const auto simulation = makeRichSimulation();
	for (int i = 0; i < 10; ++i)
		simulation->step();

	user_interface::SimulationWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderAllCardsEmptySimulation)
{
	const auto simulation = makeEmptySimulation();
	user_interface::SimulationWindow window{ simulation };

	gui.frames(2, [&] {
		window.renderSidebarContents();
		window.renderAddElementCard();
		window.renderRemoveElementCard();
		window.renderSetInteractionCard();
		window.renderDataCard();
		window.renderLogElementParametersCard();
		window.renderMonitoringCard();
	});
	SUCCEED();
}

TEST_F(SimulationWindowTest, RenderAllCardsBeforeInit)
{
	auto simulation = std::make_shared<Simulation>("test-before-init", 1.0, 0.0, 0.0);
	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters common{ std::string("field"), 50 };
	simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));

	user_interface::SimulationWindow window{ simulation };

	gui.frames(2, [&] {
		window.renderSidebarContents();
		window.renderAddElementCard();
		window.renderRemoveElementCard();
		window.renderSetInteractionCard();
		window.renderDataCard();
		window.renderLogElementParametersCard();
		window.renderMonitoringCard();
	});
	SUCCEED();
}
