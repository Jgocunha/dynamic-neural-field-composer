#include <gtest/gtest.h>
#include <memory>

#include "user_interface/plot_control_window.h"
#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
#include "elements/activation_function.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// quickPopulatePlotTypeFor() is the pure decision logic behind the Plot
// Control window's quick-populate button (see issue #57): it never touches
// ImGui, so it can be exercised headlessly, unlike PlotControlWindow::render()/
// renderContent(), which require a live ImGui/OpenGL context and are not
// unit-testable in this suite.

static std::shared_ptr<NeuralField> make1DField(const std::string& name = "nf-1d")
{
    const ElementCommonParameters cp{ name, 50 };
    const NeuralFieldParameters nfp{ 25.0, -5.0, SigmoidFunction(0.0, 10.0) };
    return std::make_shared<NeuralField>(cp, nfp);
}

static std::shared_ptr<NeuralField2D> make2DField(const std::string& name = "nf-2d")
{
    const ElementCommonParameters cp{ name, ElementDimensions(10, 10, 1.0, 1.0) };
    const NeuralField2DParameters nfp{ 25.0, -5.0, SigmoidFunction(0.0, 10.0) };
    return std::make_shared<NeuralField2D>(cp, nfp);
}

TEST(QuickPopulatePlotTypeFor, OneDimensionalFieldGetsLinePlot)
{
    const auto field = make1DField();
    EXPECT_EQ(user_interface::quickPopulatePlotTypeFor(field), PlotType::LINE_PLOT);
}

TEST(QuickPopulatePlotTypeFor, TwoDimensionalFieldGetsHeatmap)
{
    const auto field = make2DField();
    EXPECT_EQ(user_interface::quickPopulatePlotTypeFor(field), PlotType::HEATMAP);
}

TEST(QuickPopulatePlotTypeFor, LabelMatchesElementDimensionality)
{
    const auto field1D = make1DField();
    const auto field2D = make2DField();
    EXPECT_EQ(field1D->getLabel(), ElementLabel::NEURAL_FIELD);
    EXPECT_EQ(field2D->getLabel(), ElementLabel::NEURAL_FIELD_2D);
    EXPECT_NE(user_interface::quickPopulatePlotTypeFor(field1D),
              user_interface::quickPopulatePlotTypeFor(field2D));
}
