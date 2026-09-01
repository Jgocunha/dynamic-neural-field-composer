#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "ui_test_harness.h"
#include "visualization/visualization.h"
#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Render-path buffer reuse - issue #53
//
// Visualization::render() used to build a fresh data-pointer vector and a fresh
// legend vector inside its per-plot loop on every frame. Those are now reusable
// members filled by gatherPlotSeries(), cleared (not reallocated) per plot.
//
// The risk that motivates these tests is silent staleness: a reused buffer that
// is not fully rebuilt would render a previous frame's series, and nothing about
// the visible output would announce it. gatherPlotSeries() is the pure,
// headlessly testable core of that fill (same extraction rationale as
// resolveManualHeatmapDimensions in heatmap.h), so the buffer contract can be
// asserted directly rather than inferred from a render() that returns nothing.
//
// The render() tests below drive the real thing through the headless ImGui
// harness to confirm repeated frames and mid-run data-source changes stay
// consistent with the simulation.
// ---------------------------------------------------------------------------

namespace
{
	std::shared_ptr<Simulation> makeSimWithField(const std::string& fieldName = "field",
	                                             const std::string& simId = "render-buffers-test")
	{
		auto simulation = std::make_shared<Simulation>(simId, 1.0, 0.0, 0.0);
		const AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
		const ElementCommonParameters common{ fieldName, 100 };
		simulation->addElement(std::make_shared<NeuralField>(common, nfp));
		simulation->init();
		return simulation;
	}

	using Sources = std::vector<std::pair<std::string, std::string>>;
}

// ---------------------------------------------------------------------------
// gatherPlotSeries - the buffer-fill contract
// ---------------------------------------------------------------------------

TEST(GatherPlotSeries, FillsPointersAndLegendsForEachSource)
{
	const auto sim = makeSimWithField();
	const Sources sources{ { "field", "activation" }, { "field", "output" } };

	std::vector<std::vector<double>*> data;
	std::vector<std::string> legends;
	gatherPlotSeries(*sim, sources, data, legends);

	ASSERT_EQ(data.size(), 2u);
	ASSERT_EQ(legends.size(), 2u);
	EXPECT_EQ(data[0], sim->getComponentPtr("field", "activation"));
	EXPECT_EQ(data[1], sim->getComponentPtr("field", "output"));
	EXPECT_EQ(legends[0], "field - activation");
	EXPECT_EQ(legends[1], "field - output");
}

TEST(GatherPlotSeries, EmptySourcesYieldEmptyBuffers)
{
	const auto sim = makeSimWithField();

	std::vector<std::vector<double>*> data;
	std::vector<std::string> legends;
	gatherPlotSeries(*sim, {}, data, legends);

	EXPECT_TRUE(data.empty());
	EXPECT_TRUE(legends.empty());
}

// The whole point of reusing a buffer: a second fill with FEWER series must not
// leave the extra entries from the first behind. This is the exact failure mode
// a `.clear()`-less reuse would produce, and it is invisible in the rendered
// output except as a phantom extra curve.
TEST(GatherPlotSeries, ReusedBufferShrinksWhenSeriesAreRemoved)
{
	const auto sim = makeSimWithField();

	std::vector<std::vector<double>*> data;
	std::vector<std::string> legends;

	gatherPlotSeries(*sim, Sources{ { "field", "activation" },
	                                { "field", "output" },
	                                { "field", "resting level" } }, data, legends);
	ASSERT_EQ(data.size(), 3u);
	ASSERT_EQ(legends.size(), 3u);

	gatherPlotSeries(*sim, Sources{ { "field", "output" } }, data, legends);
	ASSERT_EQ(data.size(), 1u);
	ASSERT_EQ(legends.size(), 1u);
	EXPECT_EQ(data[0], sim->getComponentPtr("field", "output"));
	EXPECT_EQ(legends[0], "field - output");
}

TEST(GatherPlotSeries, ReusedBufferGrowsWhenSeriesAreAdded)
{
	const auto sim = makeSimWithField();

	std::vector<std::vector<double>*> data;
	std::vector<std::string> legends;

	gatherPlotSeries(*sim, Sources{ { "field", "activation" } }, data, legends);
	ASSERT_EQ(legends.size(), 1u);

	gatherPlotSeries(*sim, Sources{ { "field", "activation" }, { "field", "output" } }, data, legends);
	ASSERT_EQ(data.size(), 2u);
	ASSERT_EQ(legends.size(), 2u);
	EXPECT_EQ(legends[1], "field - output");
}

// Reuse must preserve capacity - that is the allocation win the issue asks for.
// Without it the members are just as allocating as the old locals were.
TEST(GatherPlotSeries, ReuseDoesNotReallocateAtSteadyState)
{
	const auto sim = makeSimWithField();
	const Sources sources{ { "field", "activation" }, { "field", "output" } };

	std::vector<std::vector<double>*> data;
	std::vector<std::string> legends;

	gatherPlotSeries(*sim, sources, data, legends);
	const auto dataCapacity = data.capacity();
	const auto legendsCapacity = legends.capacity();
	const auto* const dataStorage = data.data();

	for (int frame = 0; frame < 10; ++frame)
		gatherPlotSeries(*sim, sources, data, legends);

	EXPECT_EQ(data.capacity(), dataCapacity);
	EXPECT_EQ(legends.capacity(), legendsCapacity);
	EXPECT_EQ(data.data(), dataStorage) << "steady-state frames must not reallocate the buffer";
}

// The vector-level capacity check above passes even if each individual
// std::string's own character storage is discarded and reallocated every
// call: legends.clear() destroys the std::string objects (only the vector's
// slot capacity survives), so a name/component pair long enough to defeat
// small-string optimization would still allocate on every frame despite
// "steady state". Long enough here means every SSO threshold in practice
// (15 on MSVC/libstdc++, 22 on libc++), well past what a legend like
// "field - activation" would ever need.
TEST(GatherPlotSeries, ReuseDoesNotReallocateLegendStorageAtSteadyState)
{
	const std::string longName(40, 'a');
	auto simulation = std::make_shared<Simulation>("long-legend-test", 1.0, 0.0, 0.0);
	const AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const ElementCommonParameters common{ longName, 100 };
	simulation->addElement(std::make_shared<NeuralField>(common, nfp));
	simulation->init();

	const Sources sources{ { longName, "activation" } };

	std::vector<std::vector<double>*> data;
	std::vector<std::string> legends;

	gatherPlotSeries(*simulation, sources, data, legends);
	ASSERT_EQ(legends.size(), 1u);
	ASSERT_GT(legends[0].size(), 22u) << "test setup: legend must exceed every libstdc++/libc++/MSVC SSO buffer";
	const auto* const legendStorage = legends[0].data();

	for (int frame = 0; frame < 10; ++frame)
		gatherPlotSeries(*simulation, sources, data, legends);

	EXPECT_EQ(legends[0].data(), legendStorage)
		<< "a long legend's character storage must be reused in place, not freed and reallocated each frame";
}

// Pointers are deliberately re-read from the simulation every call rather than
// cached across frames: a component vector can reallocate (e.g. on resize) and
// a cached pointer would dangle.
TEST(GatherPlotSeries, PointersAreRefetchedAfterComponentReallocation)
{
	const auto sim = makeSimWithField();
	const Sources sources{ { "field", "activation" } };

	std::vector<std::vector<double>*> data;
	std::vector<std::string> legends;
	gatherPlotSeries(*sim, sources, data, legends);
	ASSERT_EQ(data.size(), 1u);

	sim->changeDimensions("field", ElementDimensions(250, 1.0));
	sim->init();

	gatherPlotSeries(*sim, sources, data, legends);
	ASSERT_EQ(data.size(), 1u);
	EXPECT_EQ(data[0], sim->getComponentPtr("field", "activation"))
		<< "the buffer must hold the CURRENT component pointer, not a cached one";
	EXPECT_EQ(data[0]->size(), 250u);
}

// ---------------------------------------------------------------------------
// Visualization::render() - repeated frames and invalidation
// ---------------------------------------------------------------------------

TEST(VisualizationRenderBuffers, RepeatedFramesKeepPlotAndSourcesIdentical)
{
	test::HeadlessImGui gui;
	const auto sim = makeSimWithField();
	Visualization vis(sim);
	const Sources sources{ { "field", "activation" }, { "field", "output" } };
	vis.plot(sources);

	gui.frames(3, [&] { vis.render(); });

	ASSERT_EQ(vis.getPlots().size(), 1u);
	EXPECT_EQ(vis.getPlots().begin()->second, sources)
		<< "rendering must not mutate the plot's data sources";
}

TEST(VisualizationRenderBuffers, RenderReflectsSeriesAddedBetweenFrames)
{
	test::HeadlessImGui gui;
	const auto sim = makeSimWithField();
	Visualization vis(sim);
	vis.plot("field", "activation");
	const int id = vis.getPlots().begin()->first->getUniqueIdentifier();

	gui.frame([&] { vis.render(); });

	vis.plot(id, "field", "output");
	gui.frame([&] { vis.render(); });

	ASSERT_EQ(vis.getPlots().size(), 1u);
	const auto sources = vis.getPlots().begin()->second;
	ASSERT_EQ(sources.size(), 2u);
	EXPECT_EQ(sources[1], (std::pair<std::string, std::string>{ "field", "output" }));

	// The gathered buffers for those sources must show both series, not the
	// single-series shape the first frame saw.
	std::vector<std::vector<double>*> data;
	std::vector<std::string> legends;
	gatherPlotSeries(*sim, sources, data, legends);
	ASSERT_EQ(legends.size(), 2u);
	EXPECT_EQ(legends[0], "field - activation");
	EXPECT_EQ(legends[1], "field - output");
}

TEST(VisualizationRenderBuffers, RenderReflectsSeriesRemovedBetweenFrames)
{
	test::HeadlessImGui gui;
	const auto sim = makeSimWithField();
	Visualization vis(sim);
	vis.plot(Sources{ { "field", "activation" }, { "field", "output" } });
	const int id = vis.getPlots().begin()->first->getUniqueIdentifier();

	gui.frames(2, [&] { vis.render(); });

	vis.removePlottingDataFromPlot(id, { "field", "activation" });
	gui.frame([&] { vis.render(); });

	ASSERT_EQ(vis.getPlots().size(), 1u);
	const auto sources = vis.getPlots().begin()->second;
	ASSERT_EQ(sources.size(), 1u);

	std::vector<std::vector<double>*> data;
	std::vector<std::string> legends;
	gatherPlotSeries(*sim, sources, data, legends);
	ASSERT_EQ(legends.size(), 1u);
	EXPECT_EQ(legends[0], "field - output")
		<< "the removed series must not survive into the next frame";
}

// The invalidation case that matters most: the element behind a plot goes away.
// render() drops the whole plot, and no buffer may keep a pointer into the
// destroyed element's components.
TEST(VisualizationRenderBuffers, RenderDropsPlotWhenUnderlyingElementIsRemoved)
{
	test::HeadlessImGui gui;
	const auto sim = makeSimWithField();
	Visualization vis(sim);
	vis.plot("field", "activation");

	gui.frame([&] { vis.render(); });
	ASSERT_EQ(vis.getPlots().size(), 1u);

	sim->removeElement("field");
	gui.frame([&] { vis.render(); });

	EXPECT_TRUE(vis.getPlots().empty())
		<< "a plot whose element is gone must be removed, not rendered from a stale buffer";
}

TEST(VisualizationRenderBuffers, RenderSurvivesPlotsWithDifferingSeriesCounts)
{
	// Two plots in one frame, with different series counts: the shared buffer is
	// refilled between them, so the smaller plot must not inherit the larger
	// plot's trailing entries.
	test::HeadlessImGui gui;
	const auto sim = makeSimWithField();
	Visualization vis(sim);
	vis.plot(Sources{ { "field", "activation" }, { "field", "output" }, { "field", "resting level" } });
	vis.plot(Sources{ { "field", "output" } });

	gui.frames(2, [&] { vis.render(); });

	EXPECT_EQ(vis.getPlots().size(), 2u);
	for (const auto& [plot, sources] : vis.getPlots())
	{
		std::vector<std::vector<double>*> data;
		std::vector<std::string> legends;
		gatherPlotSeries(*sim, sources, data, legends);
		EXPECT_EQ(data.size(), sources.size());
		EXPECT_EQ(legends.size(), sources.size());
	}
}

TEST(VisualizationRenderBuffers, RenderWithNoPlotsIsANoOp)
{
	test::HeadlessImGui gui;
	const auto sim = makeSimWithField();
	Visualization vis(sim);

	gui.frames(2, [&] { vis.render(); });

	EXPECT_TRUE(vis.getPlots().empty());
}

TEST(VisualizationRenderBuffers, RenderStaysConsistentWhileSimulationSteps)
{
	test::HeadlessImGui gui;
	const auto sim = makeSimWithField();
	Visualization vis(sim);
	const Sources sources{ { "field", "activation" } };
	vis.plot(sources);

	for (int frame = 0; frame < 5; ++frame)
	{
		sim->step();
		gui.frame([&] { vis.render(); });
	}

	ASSERT_EQ(vis.getPlots().size(), 1u);
	EXPECT_EQ(vis.getPlots().begin()->second, sources);
}
