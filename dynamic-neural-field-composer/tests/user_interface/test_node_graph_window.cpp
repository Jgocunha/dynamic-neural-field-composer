// Headless tests for NodeGraphWindow — comprehensive coverage of a 1629-LOC UI component.
//
// Strategy: Render the node graph in multiple states. NodeGraphWindow uses the
// ImNodeEditor library which has more state to manage in headless mode.

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
#include "elements/field_coupling.h"
#include "elements/gauss_field_coupling.h"
#include "elements/memory_trace.h"
#include "elements/memory_trace_2d.h"
#include "user_interface/node_graph_window.h"

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
		auto field1 = std::make_shared<element::NeuralField>(f1, nfp);
		simulation->addElement(field1);

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
		const element::ElementCommonParameters f2{ std::string("field2d"), element::ElementDimensions{ 20, 20, 1.0, 1.0 } };
		const element::NeuralField2DParameters nfp2d{ 25.0, -5.0, sigmoid };
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

	std::shared_ptr<Simulation> makeCoupledSimulation()
	{
		auto simulation = std::make_shared<Simulation>("test-coupled", 1.0, 0.0, 0.0);

		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };

		const element::ElementCommonParameters f1{ std::string("field1"), 50 };
		simulation->addElement(std::make_shared<element::NeuralField>(f1, nfp));

		const element::ElementCommonParameters f2{ std::string("field2"), 50 };
		simulation->addElement(std::make_shared<element::NeuralField>(f2, nfp));

		// Add coupling
		const element::ElementCommonParameters coupling{ std::string("coupling"), 50 };
		const element::FieldCouplingParameters fcp{ element::ElementDimensions{ 50, 1.0 } };
		simulation->addElement(std::make_shared<element::FieldCoupling>(coupling, fcp));

		simulation->init();
		return simulation;
	}

	std::shared_ptr<Simulation> makeCoupled2DSimulation()
	{
		auto simulation = std::make_shared<Simulation>("test-coupled-2d", 1.0, 0.0, 0.0);

		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralField2DParameters nfp{ 25.0, -5.0, sigmoid };

		// Deliberately 10x10 rather than 20x20: the GaussFieldCoupling node draws a
		// weight matrix per cell pair, and at 20x20 a single ImDrawList exceeds ImGui's
		// 64k 16-bit index limit. 10x10 exercises the same coupled-2D render path.
		const element::ElementCommonParameters f1{ std::string("field1"), element::ElementDimensions{ 10, 10, 1.0, 1.0 } };
		simulation->addElement(std::make_shared<element::NeuralField2D>(f1, nfp));

		const element::ElementCommonParameters f2{ std::string("field2"), element::ElementDimensions{ 10, 10, 1.0, 1.0 } };
		simulation->addElement(std::make_shared<element::NeuralField2D>(f2, nfp));

		// Add 2D coupling
		const element::ElementCommonParameters coupling{ std::string("coupling"), element::ElementDimensions{ 10, 10, 1.0, 1.0 } };
		const element::GaussFieldCouplingParameters gfcp{ element::ElementDimensions{ 10, 10, 1.0, 1.0 } };
		simulation->addElement(std::make_shared<element::GaussFieldCoupling>(coupling, gfcp));

		simulation->init();
		return simulation;
	}
}

class NodeGraphWindowTest : public ::testing::Test
{
protected:
	// HeadlessImGui gives each test a fresh ImGui/ImPlot context, but the window's
	// own cross-frame caches (hover timers, EMA-smoothed colormap ranges, pending
	// pin) live outside it and are keyed by node id / element name -- and the
	// factories below deliberately reuse names like "field1" and "coupling". Clear
	// them too, or a test inherits smoothed state from whichever test ran first.
	void SetUp() override { user_interface::NodeGraphWindow::resetTransientStateForTesting(); }
	void TearDown() override { user_interface::NodeGraphWindow::resetTransientStateForTesting(); }

	test::HeadlessImGui gui;
};

TEST_F(NodeGraphWindowTest, RenderEmptySimulation)
{
	const auto simulation = makeEmptySimulation();
	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderWith1DElements)
{
	const auto simulation = make1DSimulation();
	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderWith2DElements)
{
	const auto simulation = make2DSimulation();
	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderWithRichSimulation)
{
	const auto simulation = makeRichSimulation();
	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderWithCoupledElements)
{
	const auto simulation = makeCoupledSimulation();
	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderWithNarrowNonzeroWeightRangeDoesNotCrash)
{
	// Regression: a DELTA FieldCoupling's learned weights are legitimately a
	// narrow, nonzero, non-symmetric range (e.g. [-0.0074, 0.0090] -- see
	// FieldCoupling::updateWeights()). The inline node preview's colorbar used
	// to reserve a fixed-width margin for its tick labels; sizing that margin
	// from the actual label text (see NodeGraphWindow::inlineColorbarWidth)
	// must not corrupt layout or crash across multiple render frames.
	const auto simulation = makeCoupledSimulation();
	const auto coupling = std::dynamic_pointer_cast<element::FieldCoupling>(simulation->getElement("coupling"));
	ASSERT_NE(coupling, nullptr);
	auto* weights = coupling->getComponentPtr("weights");
	ASSERT_NE(weights, nullptr);
	for (std::size_t i = 0; i < weights->size(); ++i) {
		(*weights)[i] = -0.0074 + (0.0164 * static_cast<double>(i) / static_cast<double>(weights->size()));
	}

	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(3, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderWithCoupled2DElements)
{
	const auto simulation = makeCoupled2DSimulation();
	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderEmbedded)
{
	const auto simulation = make1DSimulation();
	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.renderEmbedded(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderEmbeddedRich)
{
	const auto simulation = makeRichSimulation();
	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.renderEmbedded(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderEmbeddedWithCouplings)
{
	const auto simulation = makeCoupledSimulation();
	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.renderEmbedded(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderAfterSimulationSteps)
{
	const auto simulation = makeRichSimulation();
	for (int i = 0; i < 3; ++i)
		simulation->step();

	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderBeforeSimulationInit)
{
	auto simulation = std::make_shared<Simulation>("test-before-init", 1.0, 0.0, 0.0);
	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters common{ std::string("field"), 50 };
	simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));

	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, MultipleFrames)
{
	const auto simulation = makeRichSimulation();
	user_interface::NodeGraphWindow window{ simulation };

	// Multiple frames to catch node layout state changes
	gui.frames(5, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, MultipleFramesWithCouplings)
{
	const auto simulation = makeCoupledSimulation();
	user_interface::NodeGraphWindow window{ simulation };

	gui.frames(5, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, MultipleFrames2D)
{
	const auto simulation = make2DSimulation();
	user_interface::NodeGraphWindow window{ simulation };

	gui.frames(5, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderWithManyElements)
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

	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderWithManyElements2D)
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
		const element::GaussStimulus2DParameters gsp{ 10.0, 10.0, 5.0, 5.0 };
		simulation->addElement(std::make_shared<element::GaussStimulus2D>(s, gsp));
	}

	simulation->init();

	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderAfterMultipleSteps)
{
	const auto simulation = makeRichSimulation();
	for (int i = 0; i < 10; ++i)
		simulation->step();

	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderEmbeddedAfterSteps)
{
	const auto simulation = makeRichSimulation();
	for (int i = 0; i < 5; ++i)
		simulation->step();

	user_interface::NodeGraphWindow window{ simulation };
	gui.frames(2, [&] { window.renderEmbedded(); });
	SUCCEED();
}

TEST_F(NodeGraphWindowTest, RenderAndEmbeddedInSameWindow)
{
	const auto simulation = makeRichSimulation();
	user_interface::NodeGraphWindow window{ simulation };

	gui.frames(3, [&] {
		window.render();
		window.renderEmbedded();
	});
	SUCCEED();
}
