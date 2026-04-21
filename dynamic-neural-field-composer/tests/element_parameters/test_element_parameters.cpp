#include <gtest/gtest.h>
#include "element_parameters/element_parameters.h"

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
