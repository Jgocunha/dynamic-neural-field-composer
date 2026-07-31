#include <gtest/gtest.h>

#include "visualization/plot_parameters.h"

using namespace dnf_composer;

// ---------------------------------------------------------------------------
// PlotDimensions — default construction
// ---------------------------------------------------------------------------

TEST(PlotDimensionsDefault, SetsDocumentedDefaults)
{
    const PlotDimensions dims;
    EXPECT_DOUBLE_EQ(dims.xMin, 0.0);
    EXPECT_DOUBLE_EQ(dims.xMax, 100.0);
    EXPECT_DOUBLE_EQ(dims.yMin, -10.0);
    EXPECT_DOUBLE_EQ(dims.yMax, 10.0);
    EXPECT_DOUBLE_EQ(dims.xStep, 1.0);
    EXPECT_DOUBLE_EQ(dims.yStep, 1.0);
}

TEST(PlotDimensionsDefault, IsLegal)
{
    const PlotDimensions dims;
    EXPECT_TRUE(dims.isLegal());
}

// ---------------------------------------------------------------------------
// PlotDimensions — validating 6-arg constructor
//
// The constructor checks, in order: xMin>=xMax, yMin>=yMax, xStep<=0,
// yStep<=0. Each check that fires resets ONLY its own fields and then
// returns immediately, so any later violation in the same construction is
// left uncorrected. These tests intentionally exercise that early-return
// behavior, not an idealized "fix everything" behavior.
// ---------------------------------------------------------------------------

TEST(PlotDimensionsCtor, LegalValuesAreKept)
{
    const PlotDimensions dims{ 0.0, 50.0, -5.0, 5.0, 2.0, 3.0 };
    EXPECT_DOUBLE_EQ(dims.xMin, 0.0);
    EXPECT_DOUBLE_EQ(dims.xMax, 50.0);
    EXPECT_DOUBLE_EQ(dims.yMin, -5.0);
    EXPECT_DOUBLE_EQ(dims.yMax, 5.0);
    EXPECT_DOUBLE_EQ(dims.xStep, 2.0);
    EXPECT_DOUBLE_EQ(dims.yStep, 3.0);
    EXPECT_TRUE(dims.isLegal());
}

TEST(PlotDimensionsCtor, InvertedXRangeResetsOnlyXAndReturnsEarly)
{
    // xMin >= xMax fires first. yMin/yMax are also inverted here, but since
    // the constructor returns right after fixing xMin/xMax, that second
    // violation is never checked or corrected.
    const PlotDimensions dims{ 100.0, 0.0, 10.0, -10.0, 1.0, 1.0 };
    EXPECT_DOUBLE_EQ(dims.xMin, 0.0);
    EXPECT_DOUBLE_EQ(dims.xMax, 100.0);
    EXPECT_DOUBLE_EQ(dims.yMin, 10.0);
    EXPECT_DOUBLE_EQ(dims.yMax, -10.0);
    EXPECT_DOUBLE_EQ(dims.xStep, 1.0);
    EXPECT_DOUBLE_EQ(dims.yStep, 1.0);
    EXPECT_FALSE(dims.isLegal()); // yMin/yMax left illegal
}

TEST(PlotDimensionsCtor, InvertedYRangeResetsOnlyYAndReturnsEarly)
{
    // xMin < xMax so the first check passes. yMin >= yMax fires and resets
    // yMin/yMax, then returns before checking xStep, which is also illegal
    // here and therefore left uncorrected.
    const PlotDimensions dims{ 0.0, 100.0, 10.0, -10.0, -5.0, 1.0 };
    EXPECT_DOUBLE_EQ(dims.xMin, 0.0);
    EXPECT_DOUBLE_EQ(dims.xMax, 100.0);
    EXPECT_DOUBLE_EQ(dims.yMin, -10.0);
    EXPECT_DOUBLE_EQ(dims.yMax, 10.0);
    EXPECT_DOUBLE_EQ(dims.xStep, -5.0);
    EXPECT_DOUBLE_EQ(dims.yStep, 1.0);
    EXPECT_FALSE(dims.isLegal()); // xStep left illegal
}

TEST(PlotDimensionsCtor, NonPositiveXStepResetsOnlyXStepAndReturnsEarly)
{
    // xMin<xMax and yMin<yMax both pass. xStep<=0 fires and resets xStep,
    // then returns before checking yStep, which is also illegal here and
    // therefore left uncorrected.
    const PlotDimensions dims{ 0.0, 100.0, -10.0, 10.0, -5.0, -3.0 };
    EXPECT_DOUBLE_EQ(dims.xMin, 0.0);
    EXPECT_DOUBLE_EQ(dims.xMax, 100.0);
    EXPECT_DOUBLE_EQ(dims.yMin, -10.0);
    EXPECT_DOUBLE_EQ(dims.yMax, 10.0);
    EXPECT_DOUBLE_EQ(dims.xStep, 1.0);
    EXPECT_DOUBLE_EQ(dims.yStep, -3.0);
    EXPECT_FALSE(dims.isLegal()); // yStep left illegal
}

TEST(PlotDimensionsCtor, NonPositiveYStepResetsYStepAndIsLegal)
{
    // Only yStep is illegal here, so it is the sole (and last) check that
    // fires; every other field is already legal, so the result is legal.
    const PlotDimensions dims{ 0.0, 100.0, -10.0, 10.0, 1.0, -3.0 };
    EXPECT_DOUBLE_EQ(dims.xMin, 0.0);
    EXPECT_DOUBLE_EQ(dims.xMax, 100.0);
    EXPECT_DOUBLE_EQ(dims.yMin, -10.0);
    EXPECT_DOUBLE_EQ(dims.yMax, 10.0);
    EXPECT_DOUBLE_EQ(dims.xStep, 1.0);
    EXPECT_DOUBLE_EQ(dims.yStep, 1.0);
    EXPECT_TRUE(dims.isLegal());
}

// ---------------------------------------------------------------------------
// PlotDimensions — explicit single x_step constructor
// ---------------------------------------------------------------------------

TEST(PlotDimensionsExplicit, LegalXStepIsKept)
{
    const PlotDimensions dims{ 2.5 };
    EXPECT_DOUBLE_EQ(dims.xMin, 0.0);
    EXPECT_DOUBLE_EQ(dims.xMax, 100.0);
    EXPECT_DOUBLE_EQ(dims.yMin, 0.0);
    EXPECT_DOUBLE_EQ(dims.yMax, 1.0);
    EXPECT_DOUBLE_EQ(dims.xStep, 2.5);
    EXPECT_DOUBLE_EQ(dims.yStep, 1.0);
}

TEST(PlotDimensionsExplicit, NegativeXStepResetsToOne)
{
    const PlotDimensions dims{ -1.0 };
    EXPECT_DOUBLE_EQ(dims.xStep, 1.0);
}

TEST(PlotDimensionsExplicit, ZeroXStepResetsToOne)
{
    const PlotDimensions dims{ 0.0 };
    EXPECT_DOUBLE_EQ(dims.xStep, 1.0);
}

// ---------------------------------------------------------------------------
// PlotDimensions — isLegal()
// ---------------------------------------------------------------------------

TEST(PlotDimensionsLegality, TrueForDefaultConstructed)
{
    const PlotDimensions dims;
    EXPECT_TRUE(dims.isLegal());
}

TEST(PlotDimensionsLegality, FalseWhenXRangeInverted)
{
    PlotDimensions dims;
    dims.xMin = 50.0;
    dims.xMax = 50.0; // xMin >= xMax
    EXPECT_FALSE(dims.isLegal());
}

TEST(PlotDimensionsLegality, FalseWhenYRangeInverted)
{
    PlotDimensions dims;
    dims.yMin = 10.0;
    dims.yMax = -10.0;
    EXPECT_FALSE(dims.isLegal());
}

TEST(PlotDimensionsLegality, FalseWhenXStepNonPositive)
{
    PlotDimensions dims;
    dims.xStep = 0.0;
    EXPECT_FALSE(dims.isLegal());
}

TEST(PlotDimensionsLegality, FalseWhenYStepNonPositive)
{
    PlotDimensions dims;
    dims.yStep = -1.0;
    EXPECT_FALSE(dims.isLegal());
}

// ---------------------------------------------------------------------------
// PlotDimensions — operator==
// ---------------------------------------------------------------------------

TEST(PlotDimensionsEquality, EqualWithinEpsilon)
{
    const PlotDimensions a{ 0.0, 100.0, -10.0, 10.0, 1.0, 1.0 };
    const PlotDimensions b{ 0.0, 100.0, -10.0, 10.0 + 1e-9, 1.0, 1.0 };
    EXPECT_TRUE(a == b);
}

TEST(PlotDimensionsEquality, NotEqualBeyondEpsilon)
{
    const PlotDimensions a{ 0.0, 100.0, -10.0, 10.0, 1.0, 1.0 };
    const PlotDimensions b{ 0.0, 100.0, -10.0, 10.0, 1.0, 1.0 };
    PlotDimensions c = b;
    c.yMax = 10.001; // well beyond the 1e-6 epsilon
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

// ---------------------------------------------------------------------------
// PlotDimensions — toString()
// ---------------------------------------------------------------------------

TEST(PlotDimensionsToString, ContainsAllFields)
{
    // distinct value per field so an omission of any one is caught
    const PlotDimensions dims{ 1.0, 50.0, -7.0, 8.0, 2.0, 3.0 };
    const auto str = dims.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("xMin: 1.00"), std::string::npos);
    EXPECT_NE(str.find("xMax: 50.00"), std::string::npos);
    EXPECT_NE(str.find("yMin: -7.00"), std::string::npos);
    EXPECT_NE(str.find("yMax: 8.00"), std::string::npos);
    EXPECT_NE(str.find("xStep: 2.00"), std::string::npos);
    EXPECT_NE(str.find("yStep: 3.00"), std::string::npos);
}

// ---------------------------------------------------------------------------
// PlotAnnotations — default construction
// ---------------------------------------------------------------------------

TEST(PlotAnnotationsDefault, SetsDocumentedDefaults)
{
    const PlotAnnotations ann;
    EXPECT_EQ(ann.title, "Element component(s)");
    EXPECT_EQ(ann.x_label, "Spatial dimension");
    EXPECT_EQ(ann.y_label, "Amplitude");
}

// ---------------------------------------------------------------------------
// PlotAnnotations — explicit constructor
// ---------------------------------------------------------------------------

TEST(PlotAnnotationsCtor, TitleOnlyUsesDefaultLabels)
{
    const PlotAnnotations ann{ std::string("My Title") };
    EXPECT_EQ(ann.title, "My Title");
    EXPECT_EQ(ann.x_label, "Spatial dimension");
    EXPECT_EQ(ann.y_label, "Amplitude");
}

TEST(PlotAnnotationsCtor, AllFieldsProvidedAreStored)
{
    const PlotAnnotations ann{ std::string("Title"), std::string("X"), std::string("Y") };
    EXPECT_EQ(ann.title, "Title");
    EXPECT_EQ(ann.x_label, "X");
    EXPECT_EQ(ann.y_label, "Y");
}

// ---------------------------------------------------------------------------
// PlotAnnotations — operator==
// ---------------------------------------------------------------------------

TEST(PlotAnnotationsEquality, EqualAndNotEqual)
{
    const PlotAnnotations a{ std::string("Title"), std::string("X"), std::string("Y") };
    const PlotAnnotations b{ std::string("Title"), std::string("X"), std::string("Y") };
    const PlotAnnotations c{ std::string("Other"), std::string("X"), std::string("Y") };
    const PlotAnnotations d{ std::string("Title"), std::string("Other"), std::string("Y") };
    const PlotAnnotations e{ std::string("Title"), std::string("X"), std::string("Other") };
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(a == d);
    EXPECT_FALSE(a == e);
}

// ---------------------------------------------------------------------------
// PlotAnnotations — toString()
// ---------------------------------------------------------------------------

TEST(PlotAnnotationsToString, ContainsAllFields)
{
    const PlotAnnotations ann{ std::string("MyTitle"), std::string("MyX"), std::string("MyY") };
    const auto str = ann.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("MyTitle"), std::string::npos);
    EXPECT_NE(str.find("MyX"), std::string::npos);
    EXPECT_NE(str.find("MyY"), std::string::npos);
}

// ---------------------------------------------------------------------------
// PlotCommonParameters — construction
// ---------------------------------------------------------------------------

TEST(PlotCommonParametersDefault, SetsLinePlotTypeAndDefaultMembers)
{
    const PlotCommonParameters common;
    EXPECT_EQ(common.type, PlotType::LINE_PLOT);
    EXPECT_TRUE(common.dimensions == PlotDimensions{});
    EXPECT_TRUE(common.annotations == PlotAnnotations{});
}

TEST(PlotCommonParametersCtor, TypeOnlyLeavesDimensionsAndAnnotationsDefault)
{
    const PlotCommonParameters common{ PlotType::HEATMAP };
    EXPECT_EQ(common.type, PlotType::HEATMAP);
    EXPECT_TRUE(common.dimensions == PlotDimensions{});
    EXPECT_TRUE(common.annotations == PlotAnnotations{});
}

TEST(PlotCommonParametersCtor, TypeAndAnnotationsLeavesDimensionsDefault)
{
    const PlotAnnotations ann{ std::string("Custom") };
    const PlotCommonParameters common{ PlotType::HEATMAP, ann };
    EXPECT_EQ(common.type, PlotType::HEATMAP);
    EXPECT_TRUE(common.dimensions == PlotDimensions{});
    EXPECT_TRUE(common.annotations == ann);
}

TEST(PlotCommonParametersCtor, TypeDimensionsAndAnnotationsAllStored)
{
    const PlotDimensions dims{ 0.0, 50.0, -5.0, 5.0, 2.0, 2.0 };
    const PlotAnnotations ann{ std::string("Custom") };
    const PlotCommonParameters common{ PlotType::HEATMAP, dims, ann };
    EXPECT_EQ(common.type, PlotType::HEATMAP);
    EXPECT_TRUE(common.dimensions == dims);
    EXPECT_TRUE(common.annotations == ann);
}

// ---------------------------------------------------------------------------
// PlotCommonParameters — operator==
//
// operator== only compares dimensions and annotations; it does NOT compare
// type. Verified directly against the .cpp implementation.
// ---------------------------------------------------------------------------

TEST(PlotCommonParametersEquality, IgnoresTypeWhenDimensionsAndAnnotationsMatch)
{
    const PlotCommonParameters a{ PlotType::LINE_PLOT };
    const PlotCommonParameters b{ PlotType::HEATMAP };
    EXPECT_TRUE(a == b);
}

TEST(PlotCommonParametersEquality, NotEqualWhenDimensionsDiffer)
{
    const PlotDimensions dimsA{ 0.0, 50.0, -5.0, 5.0, 1.0, 1.0 };
    const PlotDimensions dimsB{ 0.0, 100.0, -5.0, 5.0, 1.0, 1.0 };
    const PlotCommonParameters a{ PlotType::LINE_PLOT, dimsA, PlotAnnotations{} };
    const PlotCommonParameters b{ PlotType::LINE_PLOT, dimsB, PlotAnnotations{} };
    EXPECT_FALSE(a == b);
}

TEST(PlotCommonParametersEquality, NotEqualWhenAnnotationsDiffer)
{
    const PlotAnnotations annA{ std::string("A") };
    const PlotAnnotations annB{ std::string("B") };
    const PlotCommonParameters a{ PlotType::LINE_PLOT, PlotDimensions{}, annA };
    const PlotCommonParameters b{ PlotType::LINE_PLOT, PlotDimensions{}, annB };
    EXPECT_FALSE(a == b);
}

// ---------------------------------------------------------------------------
// PlotCommonParameters — toString()
// ---------------------------------------------------------------------------

TEST(PlotCommonParametersToString, ContainsTypeDimensionsAndAnnotations)
{
    const PlotCommonParameters common{ PlotType::HEATMAP };
    const auto str = common.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("heatmap"), std::string::npos);
    EXPECT_NE(str.find("Plot dimensions"), std::string::npos);
    EXPECT_NE(str.find("Plot annotations"), std::string::npos);
}

TEST(PlotCommonParametersToString, LinePlotContainsLinePlotName)
{
    const PlotCommonParameters common{ PlotType::LINE_PLOT };
    const auto str = common.toString();
    EXPECT_NE(str.find("line plot"), std::string::npos);
}

// ---------------------------------------------------------------------------
// PlotType / PlotTypeToString
// ---------------------------------------------------------------------------

TEST(PlotType, ToStringMapsLinePlot)
{
    EXPECT_EQ(PlotTypeToString.at(PlotType::LINE_PLOT), "line plot");
}

TEST(PlotType, ToStringMapsHeatmap)
{
    EXPECT_EQ(PlotTypeToString.at(PlotType::HEATMAP), "heatmap");
}

TEST(PlotType, ToStringMapHasExactlyTwoEntries)
{
    EXPECT_EQ(PlotTypeToString.size(), 2u);
}
