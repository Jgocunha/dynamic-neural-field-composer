#include <gtest/gtest.h>
#include <limits>
#include "element_parameters/element_parameters.h"
#include "exceptions/exception.h"

using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// ElementDimensions
// ---------------------------------------------------------------------------

TEST(ElementDimensions, DefaultConstruction)
{
    const ElementDimensions d;
    EXPECT_EQ(d.x_max, 100);
    EXPECT_DOUBLE_EQ(d.d_x, 1.0);
    // size = x_max / d_x
    EXPECT_EQ(d.size, 100);
}

TEST(ElementDimensions, CustomXMax)
{
    const ElementDimensions d{ 200, 1.0 };
    EXPECT_EQ(d.x_max, 200);
    EXPECT_EQ(d.size, 200);
}

TEST(ElementDimensions, CustomDX)
{
    const ElementDimensions d{ 100, 0.5 };
    EXPECT_DOUBLE_EQ(d.d_x, 0.5);
    EXPECT_EQ(d.size, 200);   // 100 / 0.5 = 200
}

TEST(ElementDimensions, EqualityOperatorSame)
{
    const ElementDimensions a{ 100, 1.0 };
    const ElementDimensions b{ 100, 1.0 };
    EXPECT_TRUE(a == b);
}

TEST(ElementDimensions, EqualityOperatorDifferentXMax)
{
    const ElementDimensions a{ 100, 1.0 };
    const ElementDimensions b{ 200, 1.0 };
    EXPECT_FALSE(a == b);
}

TEST(ElementDimensions, EqualityOperatorDifferentDX)
{
    const ElementDimensions a{ 100, 1.0 };
    const ElementDimensions b{ 100, 0.5 };
    EXPECT_FALSE(a == b);
}

TEST(ElementDimensions, ToStringIsNonEmpty)
{
    const ElementDimensions d;
    EXPECT_FALSE(d.toString().empty());
}

// ---------------------------------------------------------------------------
// Regression tests for issue #86: ElementDimensions{N} (single int) selects
// the dimensionality (1 or 2) constructor. Historically, an out-of-range
// value logged an ERROR but still returned a usable 100-cell object instead
// of failing -- silently mislabeling the requested size and, at larger N,
// tripping a stack-buffer overrun downstream. It must now throw instead of
// silently defaulting, while 1 and 2 keep working exactly as before.
// ---------------------------------------------------------------------------

TEST(ElementDimensions, SingleIntDimensionalityOneConstructsDefault1DField)
{
    const ElementDimensions d{ 1 };
    EXPECT_EQ(d.dimensionality, 1);
    EXPECT_EQ(d.x_max, 100);
    EXPECT_EQ(d.y_max, 1);
    EXPECT_EQ(d.size, 100);
}

TEST(ElementDimensions, SingleIntDimensionalityTwoConstructsDefault2DField)
{
    const ElementDimensions d{ 2 };
    EXPECT_EQ(d.dimensionality, 2);
    EXPECT_EQ(d.x_max, 100);
    EXPECT_EQ(d.y_max, 100);
    EXPECT_EQ(d.size, 10000);
}

TEST(ElementDimensions, SingleIntDefaultParameterIsDimensionalityOne)
{
    const ElementDimensions d;
    EXPECT_EQ(d.dimensionality, 1);
}

TEST(ElementDimensions, SingleIntInvalidDimensionalityThrowsInsteadOfDefaulting)
{
    EXPECT_THROW(ElementDimensions{ 150 }, dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions{ 0 }, dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions{ -1 }, dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions{ 200 }, dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions{ 3 }, dnf_composer::Exception);
}

TEST(ElementDimensions, SingleIntInvalidDimensionalityMessageNamesBadValueAndBothCtors)
{
    try
    {
        const ElementDimensions d{ 150 };
        FAIL() << "Expected dnf_composer::Exception to be thrown, but construction succeeded "
                  "with size " << d.size << '.';
    }
    catch (const dnf_composer::Exception& e)
    {
        const std::string message = e.what();
        EXPECT_NE(message.find("150"), std::string::npos);
        EXPECT_NE(message.find("ElementDimensions{N}"), std::string::npos);
        EXPECT_NE(message.find("ElementDimensions{N, d_x}"), std::string::npos);
    }
}

TEST(ElementDimensions, TwoArgConstructorBuildsFieldOfRequestedLength)
{
    // The disambiguated overload: ElementDimensions{N, d_x} always means a 1D
    // field of length N, for any N -- including values that would be invalid
    // dimensionalities under the single-int overload above.
    const ElementDimensions d{ 150, 1.0 };
    EXPECT_EQ(d.dimensionality, 1);
    EXPECT_EQ(d.x_max, 150);
    EXPECT_EQ(d.size, 150);

    const ElementDimensions d2{ 200, 1.0 };
    EXPECT_EQ(d2.x_max, 200);
    EXPECT_EQ(d2.size, 200);
}

TEST(ElementDimensions, TwoArgConstructorRejectsNonPositiveExtentOrStep)
{
    EXPECT_THROW(ElementDimensions(0, 1.0), dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions(-10, 1.0), dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions(100, 0.0), dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions(100, -1.0), dnf_composer::Exception);
}

TEST(ElementDimensions, FourArgConstructorRejectsNonPositiveExtentOrStep)
{
    EXPECT_THROW(ElementDimensions(0, 100, 1.0, 1.0), dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions(100, -5, 1.0, 1.0), dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions(100, 100, 0.0, 1.0), dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions(100, 100, 1.0, -2.0), dnf_composer::Exception);
}

TEST(ElementDimensions, RejectsTinyPositiveStepThatOverflowsSampleCount)
{
    // A finite, positive-but-tiny step makes extent/step non-finite or exceed the
    // safe sample range; the quotient must be rejected BEFORE std::llround (whose
    // out-of-range result is implementation-defined and could bypass the range check).
    EXPECT_THROW(ElementDimensions(200, 1e-310), dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions(200, std::numeric_limits<double>::denorm_min()), dnf_composer::Exception);
    EXPECT_THROW(ElementDimensions(200, 200, 1e-310, 1.0), dnf_composer::Exception);
}

// ---------------------------------------------------------------------------
// ElementIdentifiers
// ---------------------------------------------------------------------------

TEST(ElementIdentifiers, LabelConstructionStoresLabel)
{
    const ElementIdentifiers id{ ElementLabel::GAUSS_STIMULUS };
    EXPECT_EQ(id.label, ElementLabel::GAUSS_STIMULUS);
}

TEST(ElementIdentifiers, LabelConstructionGeneratesName)
{
    const ElementIdentifiers id{ ElementLabel::NEURAL_FIELD };
    EXPECT_FALSE(id.uniqueName.empty());
}

TEST(ElementIdentifiers, NameConstructionStoresName)
{
    const ElementIdentifiers id{ std::string("my-element") };
    EXPECT_EQ(id.uniqueName, "my-element");
}

TEST(ElementIdentifiers, UniqueIdentifierIsAutoIncremented)
{
    const ElementIdentifiers a{ ElementLabel::GAUSS_KERNEL };
    const ElementIdentifiers b{ ElementLabel::GAUSS_KERNEL };
    EXPECT_NE(a.uniqueIdentifier, b.uniqueIdentifier);
}

TEST(ElementIdentifiers, EqualityOperatorSame)
{
    const ElementIdentifiers a{ std::string("el") };
    const ElementIdentifiers b = a;
    EXPECT_TRUE(a == b);
}

TEST(ElementIdentifiers, ToStringIsNonEmpty)
{
    const ElementIdentifiers id{ ElementLabel::NEURAL_FIELD };
    EXPECT_FALSE(id.toString().empty());
}

// ---------------------------------------------------------------------------
// ElementCommonParameters
// ---------------------------------------------------------------------------

TEST(ElementCommonParameters, NameConstructionStoresName)
{
    const ElementCommonParameters cp{ std::string("field-1") };
    EXPECT_EQ(cp.identifiers.uniqueName, "field-1");
}

TEST(ElementCommonParameters, NameAndSizeConstruction)
{
    const ElementCommonParameters cp{ std::string("field-1"), 50 };
    EXPECT_EQ(cp.identifiers.uniqueName, "field-1");
    EXPECT_EQ(cp.dimensionParameters.x_max, 50);
    EXPECT_EQ(cp.dimensionParameters.size, 50);
}

TEST(ElementCommonParameters, NameAndDimensionsConstruction)
{
    const ElementDimensions dim{ 80, 0.5 };
    const ElementCommonParameters cp{ std::string("field-2"), dim };
    EXPECT_EQ(cp.dimensionParameters.x_max, 80);
    EXPECT_EQ(cp.dimensionParameters.size, 160);
}

TEST(ElementCommonParameters, EqualityOperatorSame)
{
    const ElementCommonParameters a{ std::string("x"), 100 };
    const ElementCommonParameters b = a;
    EXPECT_TRUE(a == b);
}

TEST(ElementCommonParameters, EqualityOperatorDifferentName)
{
    const ElementCommonParameters a{ std::string("a"), 100 };
    const ElementCommonParameters b{ std::string("b"), 100 };
    EXPECT_FALSE(a == b);
}

TEST(ElementCommonParameters, EqualityOperatorDifferentSize)
{
    const ElementCommonParameters a{ std::string("x"), 100 };
    const ElementCommonParameters b{ std::string("x"), 200 };
    EXPECT_FALSE(a == b);
}

TEST(ElementCommonParameters, ToStringIsNonEmpty)
{
    const ElementCommonParameters cp{ std::string("el"), 100 };
    EXPECT_FALSE(cp.toString().empty());
}

// ---------------------------------------------------------------------------
// ElementSpecificParameters (via concrete subclass — GaussStimulusParameters is
// defined in its own header file, so we use a minimal inline subclass here)
// ---------------------------------------------------------------------------

struct TestSpecificParams final : public ElementSpecificParameters
{
    int value;
    explicit TestSpecificParams(const int v) : value(v) {}
    std::string toString() const override { return "TestSpecificParams: " + std::to_string(value); }
};

TEST(ElementSpecificParameters, ToStringViaConcrete)
{
    TestSpecificParams p{ 42 };
    EXPECT_EQ(p.toString(), "TestSpecificParams: 42");
}
