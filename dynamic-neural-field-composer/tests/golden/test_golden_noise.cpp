// ----------------------------------------------------------------------------
//  Golden tests — Noise family (Agent A)
//    NormalNoise, NormalNoise2D, CorrelatedNormalNoise, CorrelatedNormalNoise2D
//
//  MODE: STATISTICAL golden (not frozen-CSV analytic equivalence).
//  -----------------------------------------------------------------------
//  tools::math::generateNormalVector() (src/tools/math.cpp) seeds a
//  thread_local std::mt19937 from std::random_device{}() with no exposed seed
//  API — every run produces a genuinely different sample. Freezing a CSV of
//  raw samples would therefore be comparing against an arbitrary draw, not
//  the element's *dynamics*. Per the assignment brief, these are validated
//  statistically instead: draw a large pooled sample (many steps x large
//  field, so the standard error of the estimated moments is orders of
//  magnitude below golden::kStatTol) and assert the empirical mean / variance
//  / spatial correlation match the closed-form prediction for the
//  scaling + convolution the element performs. A change to the noise
//  generation or scaling law (e.g. dropping the 1/sqrt(dt) term, or breaking
//  the correlation-kernel convolution) will move these statistics far enough
//  to fail even though the exact bytes are never reproducible.
// ----------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <cmath>
#include <numeric>
#include <vector>
#include <string>

#include "elements/normal_noise.h"
#include "elements/normal_noise_2d.h"
#include "elements/correlated_normal_noise.h"
#include "elements/correlated_normal_noise_2d.h"
#include "../golden/golden_test_utils.h"

// Windows.h (pulled in transitively) defines min/max macros that break std::min.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

using namespace dnf_composer;
using namespace dnf_composer::element;
namespace g = dnf_composer::golden;

namespace
{
    struct Moments { double mean; double var; };

    Moments computeMoments(const std::vector<double>& samples)
    {
        const double n = static_cast<double>(samples.size());
        const double mean = std::accumulate(samples.begin(), samples.end(), 0.0) / n;
        double sq = 0.0;
        for (double v : samples) sq += (v - mean) * (v - mean);
        return { mean, sq / n };
    }

    // Independent re-derivation of CorrelatedNormalNoise's normalized Gaussian
    // correlation kernel (mirrors tools::math::gaussNorm + the half-width rule
    // in CorrelatedNormalNoise::init() — NOT calling production code).
    std::vector<double> correlationKernelRef(double width)
    {
        const double effectiveWidth = std::max(width, 1e-3);
        const int halfWidth = std::max(1, static_cast<int>(5.0 * effectiveWidth));
        const int kernelSize = 2 * halfWidth + 1;
        std::vector<double> k(kernelSize);
        double sum = 0.0;
        for (int i = 0; i < kernelSize; ++i)
        {
            const double x = static_cast<double>(i - halfWidth);
            k[i] = std::exp(-0.5 * x * x / (effectiveWidth * effectiveWidth));
            sum += k[i];
        }
        for (double& v : k) v /= sum;
        return k;
    }

    // sum(kernel^2) — determines Var(convolution of unit-variance white noise).
    double sumSquares(const std::vector<double>& k)
    {
        double s = 0.0;
        for (double v : k) s += v * v;
        return s;
    }

    // sum_i kernel[i]*kernel[i+lag] — determines the lag-N autocovariance of
    // the convolution output (Cov(y_i, y_{i+lag}) = scale^2 * this, for white
    // input of unit variance).
    double kernelAutocorr(const std::vector<double>& k, int lag)
    {
        double s = 0.0;
        for (std::size_t i = 0; i + lag < k.size(); ++i) s += k[i] * k[i + lag];
        return s;
    }
}

// ============================================================================
//  NormalNoise (1D) — mean ~ 0, std ~ amplitude / sqrt(deltaT)
// ============================================================================
namespace
{
    struct NoiseRegime { double amplitude; double deltaT; int size; int steps; };

    std::vector<NoiseRegime> noiseRegimes()
    {
        return {
            { 1.0, 1.0, 1000, 100 },
            { 0.5, 1.0, 1000, 100 },
            { 2.0, 0.5, 1000, 100 },
        };
    }
}

TEST(GoldenNormalNoise1D, StatisticalMeanAndStdAcrossRegimes)
{
    for (const auto& r : noiseRegimes())
    {
        ElementCommonParameters cp{ "normal_noise_1d_stat_probe", r.size };
        NormalNoiseParameters np{ r.amplitude };
        auto noise = std::make_shared<NormalNoise>(cp, np);
        noise->init();

        std::vector<double> samples;
        samples.reserve(static_cast<std::size_t>(r.size) * r.steps);
        for (int s = 0; s < r.steps; ++s)
        {
            noise->step(0.0, r.deltaT);
            const auto out = noise->getComponent("output");
            samples.insert(samples.end(), out.begin(), out.end());
        }

        const auto [mean, var] = computeMoments(samples);
        const double sampleStd = std::sqrt(var);
        const double expectedStd = r.amplitude / std::sqrt(r.deltaT);

        EXPECT_NEAR(mean, 0.0, g::kStatTol)
            << "amplitude=" << r.amplitude << " deltaT=" << r.deltaT;
        EXPECT_NEAR(sampleStd, expectedStd, g::kStatTol)
            << "amplitude=" << r.amplitude << " deltaT=" << r.deltaT;
    }
}

// ============================================================================
//  NormalNoise2D — same law, 2D field.
// ============================================================================
TEST(GoldenNormalNoise2D, StatisticalMeanAndStdAcrossRegimes)
{
    struct Regime { double amplitude; double deltaT; int size_x, size_y; int steps; };
    const std::vector<Regime> regimes = {
        { 1.0, 1.0, 60, 60, 60 },
        { 1.5, 0.5, 50, 50, 80 },
    };

    for (const auto& r : regimes)
    {
        ElementCommonParameters cp{ "normal_noise_2d_stat_probe", ElementDimensions(r.size_x, r.size_y, 1.0, 1.0) };
        NormalNoise2DParameters np{ r.amplitude };
        auto noise = std::make_shared<NormalNoise2D>(cp, np);
        noise->init();

        std::vector<double> samples;
        samples.reserve(static_cast<std::size_t>(r.size_x) * r.size_y * r.steps);
        for (int s = 0; s < r.steps; ++s)
        {
            noise->step(0.0, r.deltaT);
            const auto out = noise->getComponent("output");
            samples.insert(samples.end(), out.begin(), out.end());
        }

        const auto [mean, var] = computeMoments(samples);
        const double sampleStd = std::sqrt(var);
        const double expectedStd = r.amplitude / std::sqrt(r.deltaT);

        EXPECT_NEAR(mean, 0.0, g::kStatTol)
            << "amplitude=" << r.amplitude << " deltaT=" << r.deltaT;
        EXPECT_NEAR(sampleStd, expectedStd, g::kStatTol)
            << "amplitude=" << r.amplitude << " deltaT=" << r.deltaT;
    }
}

// ============================================================================
//  CorrelatedNormalNoise (1D) — variance and spatial (lag-1) correlation must
//  match the closed-form prediction for "white noise convolved with a
//  normalized Gaussian kernel, then scaled by amplitude/sqrt(dt)". circular=
//  true keeps the field statistically homogeneous (no boundary effects) so
//  pooling over every ring position is valid.
// ============================================================================
TEST(GoldenCorrelatedNormalNoise1D, StatisticalVarianceAndCorrelationAcrossRegimes)
{
    struct Regime { double amplitude; double width; double deltaT; int size; int steps; };
    const std::vector<Regime> regimes = {
        { 1.0, 2.0, 1.0, 200, 200 },
        { 1.5, 1.0, 1.0, 200, 200 },
    };

    for (const auto& r : regimes)
    {
        ElementCommonParameters cp{ "correlated_normal_noise_1d_stat_probe", r.size };
        CorrelatedNormalNoiseParameters np{ r.amplitude, r.width, /*circular=*/true };
        auto noise = std::make_shared<CorrelatedNormalNoise>(cp, np);
        noise->init();

        const auto kernel = correlationKernelRef(r.width);
        const double sumSq = sumSquares(kernel);
        const double lag1 = kernelAutocorr(kernel, 1);
        const double scale = r.amplitude / std::sqrt(r.deltaT);
        const double expectedVar = scale * scale * sumSq;
        const double expectedCorrLag1 = lag1 / sumSq;

        double sumX = 0.0, sumX2 = 0.0, sumXY = 0.0;
        long long count = 0, countPairs = 0;
        for (int s = 0; s < r.steps; ++s)
        {
            noise->step(0.0, r.deltaT);
            const auto out = noise->getComponent("output");
            for (int i = 0; i < r.size; ++i)
            {
                sumX += out[i];
                sumX2 += out[i] * out[i];
                ++count;
            }
            for (int i = 0; i < r.size; ++i) // circular ring: includes wrap pair (N-1,0)
            {
                const int j = (i + 1) % r.size;
                sumXY += out[i] * out[j];
                ++countPairs;
            }
        }

        const double mean = sumX / static_cast<double>(count);
        const double var = sumX2 / static_cast<double>(count) - mean * mean;
        const double covLag1 = sumXY / static_cast<double>(countPairs) - mean * mean;
        const double corrLag1 = covLag1 / var;

        EXPECT_NEAR(mean, 0.0, g::kStatTol) << "width=" << r.width;
        EXPECT_NEAR(var, expectedVar, std::max(g::kStatTol, 0.1 * expectedVar))
            << "width=" << r.width << " expectedVar=" << expectedVar;
        EXPECT_NEAR(corrLag1, expectedCorrLag1, std::max(g::kStatTol, 0.1 * std::abs(expectedCorrLag1)))
            << "width=" << r.width << " expectedCorrLag1=" << expectedCorrLag1;
    }
}

// ============================================================================
//  CorrelatedNormalNoise2D — separable convolution: variance factorises as
//  scale^2 * sumSq(kernel_x) * sumSq(kernel_y), and each axis's lag-1
//  correlation reduces to the same 1D formula (see derivation in the PR
//  description / commit message: the y-pass mixes rows independently, so
//  the x-direction lag-1 correlation of the *result* equals the x-kernel's
//  own lag-1 autocorrelation ratio, independent of the y-kernel).
// ============================================================================
TEST(GoldenCorrelatedNormalNoise2D, StatisticalVarianceAndCorrelationAcrossRegimes)
{
    struct Regime { double amplitude; double width; double deltaT; int size_x, size_y; int steps; };
    const std::vector<Regime> regimes = {
        { 1.0, 2.0, 1.0, 80, 80, 80 },
    };

    for (const auto& r : regimes)
    {
        ElementCommonParameters cp{ "correlated_normal_noise_2d_stat_probe",
                                    ElementDimensions(r.size_x, r.size_y, 1.0, 1.0) };
        CorrelatedNormalNoise2DParameters np{ r.amplitude, r.width, /*circular=*/true };
        auto noise = std::make_shared<CorrelatedNormalNoise2D>(cp, np);
        noise->init();

        const auto kernel = correlationKernelRef(r.width); // kernel_x == kernel_y (isotropic)
        const double sumSq = sumSquares(kernel);
        const double lag1 = kernelAutocorr(kernel, 1);
        const double scale = r.amplitude / std::sqrt(r.deltaT);
        const double expectedVar = scale * scale * sumSq * sumSq;
        const double expectedCorrLag1x = lag1 / sumSq;

        double sumX = 0.0, sumX2 = 0.0, sumXYx = 0.0;
        long long count = 0, countPairsX = 0;
        for (int s = 0; s < r.steps; ++s)
        {
            noise->step(0.0, r.deltaT);
            const auto out = noise->getComponent("output");
            for (int y = 0; y < r.size_y; ++y)
            {
                for (int x = 0; x < r.size_x; ++x)
                {
                    const double v = out[static_cast<std::size_t>(y) * r.size_x + x];
                    sumX += v;
                    sumX2 += v * v;
                    ++count;
                }
                for (int x = 0; x < r.size_x; ++x) // lag-1 along x, circular wrap
                {
                    const int xj = (x + 1) % r.size_x;
                    sumXYx += out[static_cast<std::size_t>(y) * r.size_x + x] *
                              out[static_cast<std::size_t>(y) * r.size_x + xj];
                    ++countPairsX;
                }
            }
        }

        const double mean = sumX / static_cast<double>(count);
        const double var = sumX2 / static_cast<double>(count) - mean * mean;
        const double covLag1x = sumXYx / static_cast<double>(countPairsX) - mean * mean;
        const double corrLag1x = covLag1x / var;

        EXPECT_NEAR(mean, 0.0, g::kStatTol);
        EXPECT_NEAR(var, expectedVar, std::max(g::kStatTol, 0.15 * expectedVar))
            << "expectedVar=" << expectedVar;
        EXPECT_NEAR(corrLag1x, expectedCorrLag1x, std::max(g::kStatTol, 0.15 * std::abs(expectedCorrLag1x)))
            << "expectedCorrLag1x=" << expectedCorrLag1x;
    }
}
