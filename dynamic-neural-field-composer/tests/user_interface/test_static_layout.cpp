// Headless tests for StaticLayoutWindow — comprehensive coverage of the main application layout.
//
// Strategy: StaticLayoutWindow is the container for all the other UI windows. Test it
// with various simulation states and visualization configurations.

#include <gtest/gtest.h>

#include <memory>

#include "ui_test_harness.h"

#include "simulation/simulation.h"
#include "visualization/visualization.h"
#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_stimulus_2d.h"
#include "elements/gauss_kernel.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/normal_noise.h"
#include "elements/memory_trace.h"
#include "elements/memory_trace_2d.h"
#include "user_interface/static_layout.h"

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
		const element::GaussStimulus2DParameters gsp{ 5.0, 15.0, 10.0, 10.0 };
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

class StaticLayoutWindowTest : public ::testing::Test
{
protected:
	test::HeadlessImGui gui;
};

TEST_F(StaticLayoutWindowTest, RenderEmptySimulation)
{
	const auto simulation = makeEmptySimulation();
	const auto visualization = std::make_shared<Visualization>(simulation);
	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderWith1DElements)
{
	const auto simulation = make1DSimulation();
	const auto visualization = std::make_shared<Visualization>(simulation);
	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderWith2DElements)
{
	const auto simulation = make2DSimulation();
	const auto visualization = std::make_shared<Visualization>(simulation);
	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderWithRichSimulation)
{
	const auto simulation = makeRichSimulation();
	const auto visualization = std::make_shared<Visualization>(simulation);
	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderAfterSimulationSteps)
{
	const auto simulation = makeRichSimulation();
	for (int i = 0; i < 3; ++i)
		simulation->step();

	const auto visualization = std::make_shared<Visualization>(simulation);
	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderBeforeSimulationInit)
{
	auto simulation = std::make_shared<Simulation>("test-before-init", 1.0, 0.0, 0.0);
	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters common{ std::string("field"), 50 };
	simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));

	const auto visualization = std::make_shared<Visualization>(simulation);
	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, MultipleFrames)
{
	const auto simulation = makeRichSimulation();
	const auto visualization = std::make_shared<Visualization>(simulation);
	user_interface::StaticLayoutWindow window{ simulation, visualization };

	// Multiple frames to catch state that changes after first render
	gui.frames(5, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderWithVisualization1D)
{
	const auto simulation = make1DSimulation();
	const auto visualization = std::make_shared<Visualization>(simulation);

	// Add some plots
	visualization->plot("field1d", "activation");
	visualization->plot("stimulus1d", "output");

	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderWithVisualization2D)
{
	const auto simulation = make2DSimulation();
	const auto visualization = std::make_shared<Visualization>(simulation);

	// Add some plots
	visualization->plot("field2d", "activation");
	visualization->plot("stimulus2d", "output");

	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderWithRichVisualization)
{
	const auto simulation = makeRichSimulation();
	const auto visualization = std::make_shared<Visualization>(simulation);

	// Add multiple plots
	visualization->plot("field1", "activation");
	visualization->plot("stimulus", "output");
	visualization->plot("kernel", "weights");
	visualization->plot("field2d", "activation");
	visualization->plot("stimulus2d", "output");

	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderAfterMultipleSteps)
{
	const auto simulation = makeRichSimulation();
	for (int i = 0; i < 10; ++i)
		simulation->step();

	const auto visualization = std::make_shared<Visualization>(simulation);
	visualization->plot("field1", "activation");
	visualization->plot("stimulus", "output");

	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderWithManyElements)
{
	auto simulation = std::make_shared<Simulation>("test-many", 1.0, 0.0, 0.0);

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };

	// Add multiple fields
	for (int i = 0; i < 3; ++i)
	{
		const element::ElementCommonParameters f{ "field" + std::to_string(i), 50 };
		simulation->addElement(std::make_shared<element::NeuralField>(f, nfp));
	}

	// Add multiple stimuli
	for (int i = 0; i < 3; ++i)
	{
		const element::ElementCommonParameters s{ "stim" + std::to_string(i), 50 };
		const element::GaussStimulusParameters gsp{ 5.0, 15.0, 25.0 };
		simulation->addElement(std::make_shared<element::GaussStimulus>(s, gsp));
	}

	simulation->init();

	const auto visualization = std::make_shared<Visualization>(simulation);
	for (int i = 0; i < 3; ++i)
		visualization->plot("field" + std::to_string(i), "activation");

	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderWithManyElements2D)
{
	auto simulation = std::make_shared<Simulation>("test-many-2d", 1.0, 0.0, 0.0);

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralField2DParameters nfp{ 25.0, -5.0, sigmoid };

	// Add multiple 2D fields
	for (int i = 0; i < 2; ++i)
	{
		const element::ElementCommonParameters f{ "field" + std::to_string(i), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
		simulation->addElement(std::make_shared<element::NeuralField2D>(f, nfp));
	}

	// Add multiple 2D stimuli
	for (int i = 0; i < 2; ++i)
	{
		const element::ElementCommonParameters s{ "stim" + std::to_string(i), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
		const element::GaussStimulus2DParameters gsp{ 5.0, 15.0, 10.0, 10.0 };
		simulation->addElement(std::make_shared<element::GaussStimulus2D>(s, gsp));
	}

	simulation->init();

	const auto visualization = std::make_shared<Visualization>(simulation);
	for (int i = 0; i < 2; ++i)
		visualization->plot("field" + std::to_string(i), "activation");

	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderEmptyVisualization)
{
	const auto simulation = make1DSimulation();
	const auto visualization = std::make_shared<Visualization>(simulation);
	// Don't add any plots

	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderAfterVisualizationChanges)
{
	const auto simulation = make1DSimulation();
	const auto visualization = std::make_shared<Visualization>(simulation);

	user_interface::StaticLayoutWindow window{ simulation, visualization };

	// First frame with empty visualization
	gui.frame([&] { window.render(); });

	// Add plots
	visualization->plot("field1d", "activation");

	// Render again
	gui.frame([&] { window.render(); });

	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderMultipleFramesWithUpdates)
{
	const auto simulation = makeRichSimulation();
	const auto visualization = std::make_shared<Visualization>(simulation);
	visualization->plot("field1", "activation");
	visualization->plot("stimulus", "output");

	user_interface::StaticLayoutWindow window{ simulation, visualization };

	// Render multiple frames
	for (int i = 0; i < 3; ++i)
	{
		simulation->step();
		gui.frame([&] { window.render(); });
	}

	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderWith1DAndNoise)
{
	auto simulation = std::make_shared<Simulation>("test-noise-1d", 1.0, 0.0, 0.0);

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters field{ std::string("field"), 50 };
	simulation->addElement(std::make_shared<element::NeuralField>(field, nfp));

	const element::ElementCommonParameters noise{ std::string("noise"), 50 };
	const element::NormalNoiseParameters nnp{ 1.0 };
	simulation->addElement(std::make_shared<element::NormalNoise>(noise, nnp));

	simulation->init();

	const auto visualization = std::make_shared<Visualization>(simulation);
	visualization->plot("field", "activation");
	visualization->plot("noise", "output");

	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(StaticLayoutWindowTest, RenderWith2DAndNoise)
{
	auto simulation = std::make_shared<Simulation>("test-noise-2d", 1.0, 0.0, 0.0);

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralField2DParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters field{ std::string("field"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
	simulation->addElement(std::make_shared<element::NeuralField2D>(field, nfp));

	const element::ElementCommonParameters noise{ std::string("noise"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
	const element::NormalNoise2DParameters nnp{ 1.0 };
	simulation->addElement(std::make_shared<element::NormalNoise2D>(noise, nnp));

	simulation->init();

	const auto visualization = std::make_shared<Visualization>(simulation);
	visualization->plot("field", "activation");
	visualization->plot("noise", "output");

	user_interface::StaticLayoutWindow window{ simulation, visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}
