// ----------------------------------------------------------------------------
//  Golden tests — Reshape family (Agent A)
//    Resize (1D), Resize2D, Collapse (2D->1D), Expand (1D->2D)
//
//  All ANALYTIC / deterministic: a fixed probe input is fed through the
//  production element (via addInput(), mirroring tests/elements/test_resize.cpp
//  et al.) and the output is checked against an independent first-principles
//  reference (tests/golden/reference/ref_reshape.h), frozen to CSV via
//  golden::checkAgainstReference(). Sweeps interpolation method, up/down/same
//  sizing, compression type, and kept/broadcast axis.
// ----------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <string>
#include <memory>

#include "elements/resize.h"
#include "elements/resize_2d.h"
#include "elements/collapse.h"
#include "elements/expand.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_stimulus_2d.h"
#include "../golden/golden_test_utils.h"
#include "../golden/reference/ref_reshape.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
namespace g = dnf_composer::golden;

namespace
{
    // Deterministic, non-trivial 1D probe (no symmetry, so interpolation bugs
    // at either edge or in the middle are both visible).
    std::vector<double> probe1D(int size)
    {
        std::vector<double> v(size);
        for (int i = 0; i < size; ++i)
            v[i] = std::sin(i * 0.37) * 5.0 + 0.1 * i;
        return v;
    }

    // Deterministic y-major 2D probe.
    std::vector<double> probe2D(int size_x, int size_y)
    {
        std::vector<double> v(static_cast<std::size_t>(size_x) * size_y);
        for (int y = 0; y < size_y; ++y)
            for (int x = 0; x < size_x; ++x)
                v[static_cast<std::size_t>(y) * size_x + x] =
                    std::sin(x * 0.31) * 3.0 + std::cos(y * 0.53) * 2.0 + 0.05 * (x - y);
        return v;
    }

    g::ref::Interp toRefInterp(InterpolationMethod m)
    {
        switch (m)
        {
        case InterpolationMethod::LINEAR:  return g::ref::Interp::LINEAR;
        case InterpolationMethod::NEAREST: return g::ref::Interp::NEAREST;
        case InterpolationMethod::CUBIC:   return g::ref::Interp::CUBIC;
        }
        return g::ref::Interp::LINEAR;
    }

    // ---- Resize (1D) plumbing (mirrors tests/elements/test_resize.cpp) ----
    std::shared_ptr<Resize> makeResize(const std::string& name, int inSize, int outSize,
        InterpolationMethod method)
    {
        const ElementDimensions inDim{ inSize, 1.0 };
        const ResizeParameters rp{ method, inDim };
        const ElementCommonParameters cp{ name, ElementDimensions{ outSize, 1.0 } };
        return std::make_shared<Resize>(cp, rp);
    }

    std::shared_ptr<GaussStimulus> makeSource1D(const std::string& name, int size)
    {
        const GaussStimulusParameters gp{ 1.0, 1.0, 0.0 };
        const ElementCommonParameters cp{ name, ElementDimensions{ size, 1.0 } };
        return std::make_shared<GaussStimulus>(cp, gp);
    }

    std::vector<double> resizeVia(const std::shared_ptr<Resize>& resize, const std::vector<double>& input)
    {
        const auto source = makeSource1D("rz_src", static_cast<int>(input.size()));
        source->init();
        resize->addInput(source);
        resize->init();
        std::copy(input.begin(), input.end(), source->getComponentPtr("output")->begin());
        resize->step(0.0, 1.0);
        return resize->getComponent("output");
    }

    // ---- Resize2D plumbing ----
    std::shared_ptr<Resize2D> makeResize2D(const std::string& name, int inX, int inY, int outX, int outY,
        InterpolationMethod method)
    {
        const ElementDimensions inDim{ inX, inY, 1.0, 1.0 };
        const Resize2DParameters rp{ method, inDim };
        const ElementCommonParameters cp{ name, ElementDimensions{ outX, outY, 1.0, 1.0 } };
        return std::make_shared<Resize2D>(cp, rp);
    }

    std::shared_ptr<GaussStimulus2D> makeSource2D(const std::string& name, int sizeX, int sizeY)
    {
        const GaussStimulus2DParameters gp{ 1.0, 1.0, 0.0, 0.0 };
        const ElementCommonParameters cp{ name, ElementDimensions{ sizeX, sizeY, 1.0, 1.0 } };
        return std::make_shared<GaussStimulus2D>(cp, gp);
    }

    std::vector<double> resize2DVia(const std::shared_ptr<Resize2D>& resize, const std::vector<double>& field,
        int sizeX, int sizeY)
    {
        const auto source = makeSource2D("rz2d_src", sizeX, sizeY);
        source->init();
        resize->addInput(source);
        resize->init();
        std::copy(field.begin(), field.end(), source->getComponentPtr("output")->begin());
        resize->step(0.0, 1.0);
        return resize->getComponent("output");
    }

    // ---- Collapse plumbing ----
    std::shared_ptr<Collapse> makeCollapse(const std::string& name, int inX, int inY, int outSize,
        CompressionType compression, ProjectionAxis keepAxis)
    {
        const ElementDimensions inDim{ inX, inY, 1.0, 1.0 };
        const CollapseParameters cp{ compression, keepAxis, inDim };
        const ElementCommonParameters common{ name, ElementDimensions{ outSize, 1.0 } };
        return std::make_shared<Collapse>(common, cp);
    }

    std::vector<double> collapseVia(const std::shared_ptr<Collapse>& collapse, const std::vector<double>& field,
        int sizeX, int sizeY)
    {
        const auto source = makeSource2D("cl_src", sizeX, sizeY);
        source->init();
        collapse->addInput(source);
        collapse->init();
        std::copy(field.begin(), field.end(), source->getComponentPtr("output")->begin());
        collapse->step(0.0, 1.0);
        return collapse->getComponent("output");
    }

    g::ref::Reduce toRefReduce(CompressionType c)
    {
        switch (c)
        {
        case CompressionType::SUM:     return g::ref::Reduce::SUM;
        case CompressionType::AVERAGE: return g::ref::Reduce::AVERAGE;
        case CompressionType::MAXIMUM: return g::ref::Reduce::MAXIMUM;
        case CompressionType::MINIMUM: return g::ref::Reduce::MINIMUM;
        }
        return g::ref::Reduce::SUM;
    }

    // ---- Expand plumbing ----
    std::shared_ptr<Expand> makeExpand(const std::string& name, int inSize, int outX, int outY,
        ProjectionAxis profileAxis)
    {
        const ElementDimensions inDim{ inSize, 1.0 };
        const ExpandParameters ep{ profileAxis, inDim };
        const ElementCommonParameters common{ name, ElementDimensions{ outX, outY, 1.0, 1.0 } };
        return std::make_shared<Expand>(common, ep);
    }

    std::vector<double> expandVia(const std::shared_ptr<Expand>& expand, const std::vector<double>& profile)
    {
        const auto source = makeSource1D("ex_src", static_cast<int>(profile.size()));
        source->init();
        expand->addInput(source);
        expand->init();
        std::copy(profile.begin(), profile.end(), source->getComponentPtr("output")->begin());
        expand->step(0.0, 1.0);
        return expand->getComponent("output");
    }
}

// ============================================================================
//  Resize (1D)
// ============================================================================
namespace
{
    struct ResizeRegime
    {
        std::string slug;
        int inSize, outSize;
        InterpolationMethod method;
    };

    std::vector<ResizeRegime> resizeRegimes()
    {
        return {
            { "resize_1d_linear_up_20to50",   20, 50, InterpolationMethod::LINEAR },
            { "resize_1d_linear_down_50to20", 50, 20, InterpolationMethod::LINEAR },
            { "resize_1d_nearest_up_15to40",  15, 40, InterpolationMethod::NEAREST },
            { "resize_1d_cubic_up_20to60",    20, 60, InterpolationMethod::CUBIC },
            { "resize_1d_cubic_down_60to20",  60, 20, InterpolationMethod::CUBIC },
            { "resize_1d_same_size_30",       30, 30, InterpolationMethod::LINEAR },
            { "resize_1d_to_single_point",    12, 1,  InterpolationMethod::LINEAR },
        };
    }
}

TEST(GoldenResize1D, AlgebraicEquivalenceAcrossRegimes)
{
    for (const auto& r : resizeRegimes())
    {
        const auto input = probe1D(r.inSize);
        auto resize = makeResize(r.slug, r.inSize, r.outSize, r.method);

        const g::Row production = resizeVia(resize, input);
        const g::Row reference = g::ref::resample1d(input, r.outSize, toRefInterp(r.method));

        g::checkAgainstReference(r.slug, production, reference);
    }
}

// ============================================================================
//  Resize2D
// ============================================================================
namespace
{
    struct Resize2DRegime
    {
        std::string slug;
        int inX, inY, outX, outY;
        InterpolationMethod method;
    };

    std::vector<Resize2DRegime> resize2dRegimes()
    {
        return {
            { "resize_2d_linear_up_10x8to20x16",   10, 8, 20, 16, InterpolationMethod::LINEAR },
            { "resize_2d_linear_down_20x16to10x8", 20, 16, 10, 8, InterpolationMethod::LINEAR },
            { "resize_2d_nearest_rect_12x6to18x9", 12, 6, 18, 9,  InterpolationMethod::NEAREST },
            { "resize_2d_cubic_up_10x10to25x25",   10, 10, 25, 25, InterpolationMethod::CUBIC },
        };
    }
}

TEST(GoldenResize2D, AlgebraicEquivalenceAcrossRegimes)
{
    for (const auto& r : resize2dRegimes())
    {
        const auto field = probe2D(r.inX, r.inY);
        auto resize = makeResize2D(r.slug, r.inX, r.inY, r.outX, r.outY, r.method);

        const g::Row production = resize2DVia(resize, field, r.inX, r.inY);
        const g::Row reference = g::ref::resize2D(field, r.inX, r.inY, r.outX, r.outY, toRefInterp(r.method));

        g::checkAgainstReference(r.slug, production, reference);
    }
}

// ============================================================================
//  Collapse (2D -> 1D)
// ============================================================================
namespace
{
    struct CollapseRegime
    {
        std::string slug;
        int inX, inY;
        CompressionType compression;
        ProjectionAxis keepAxis;
    };

    std::vector<CollapseRegime> collapseRegimes()
    {
        return {
            { "collapse_sum_keepX_12x8",     12, 8, CompressionType::SUM,     ProjectionAxis::X },
            { "collapse_sum_keepY_12x8",     12, 8, CompressionType::SUM,     ProjectionAxis::Y },
            { "collapse_average_keepX_10x6", 10, 6, CompressionType::AVERAGE, ProjectionAxis::X },
            { "collapse_maximum_keepY_9x14", 9, 14, CompressionType::MAXIMUM, ProjectionAxis::Y },
            { "collapse_minimum_keepX_7x11", 7, 11, CompressionType::MINIMUM, ProjectionAxis::X },
        };
    }
}

TEST(GoldenCollapse, AlgebraicEquivalenceAcrossRegimes)
{
    for (const auto& r : collapseRegimes())
    {
        const auto field = probe2D(r.inX, r.inY);
        const bool keepX = r.keepAxis == ProjectionAxis::X;
        const int outSize = keepX ? r.inX : r.inY;
        auto collapse = makeCollapse(r.slug, r.inX, r.inY, outSize, r.compression, r.keepAxis);

        const g::Row production = collapseVia(collapse, field, r.inX, r.inY);
        const g::Row reference = g::ref::reduce2DAxis(field, r.inX, r.inY, keepX, toRefReduce(r.compression));

        g::checkAgainstReference(r.slug, production, reference);
    }
}

// ============================================================================
//  Expand (1D -> 2D)
// ============================================================================
namespace
{
    struct ExpandRegime
    {
        std::string slug;
        int inSize, outX, outY;
        ProjectionAxis profileAxis;
    };

    std::vector<ExpandRegime> expandRegimes()
    {
        return {
            { "expand_alongX_8to8x6",  8, 8, 6, ProjectionAxis::X },
            { "expand_alongY_6to8x6",  6, 8, 6, ProjectionAxis::Y },
            { "expand_alongX_15to15x4", 15, 15, 4, ProjectionAxis::X },
        };
    }
}

TEST(GoldenExpand, AlgebraicEquivalenceAcrossRegimes)
{
    for (const auto& r : expandRegimes())
    {
        const auto profile = probe1D(r.inSize);
        auto expand = makeExpand(r.slug, r.inSize, r.outX, r.outY, r.profileAxis);

        const g::Row production = expandVia(expand, profile);
        const g::Row reference = g::ref::broadcast1DTo2D(profile, r.outX, r.outY,
            r.profileAxis == ProjectionAxis::X);

        g::checkAgainstReference(r.slug, production, reference);
    }
}
