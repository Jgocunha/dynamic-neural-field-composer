// ----------------------------------------------------------------------------
//  Golden test — Activation functions (Sigmoid / Heaviside / AbsSigmoid)
//
//  Analytic-equivalence: production ActivationFunction::apply() (the path used
//  by NeuralField::calculateOutput() every step) vs an independent reference
//  re-derivation in reference/ref_activation.h.
//
//  NOTE on SigmoidFunction: apply() computes internally in float32 (a real
//  precision inconsistency vs. its own operator(), which is float64 — see
//  ref_activation.h header comment and the report to god). We mirror that
//  float32 computation bit-for-bit so this stays a genuine 1e-9 analytic
//  equivalence test, not a loosened tolerance.
// ----------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "elements/activation_function.h"
#include "../golden/golden_test_utils.h"
#include "../golden/reference/ref_activation.h"

using namespace dnf_composer::element;
namespace g = dnf_composer::golden;

namespace
{
    std::vector<double> linspace(double lo, double hi, int n)
    {
        std::vector<double> v(static_cast<std::size_t>(n));
        if (n == 1) { v[0] = lo; return v; }
        for (int i = 0; i < n; ++i) {
            v[static_cast<std::size_t>(i)] = lo + (hi - lo) * static_cast<double>(i) / static_cast<double>(n - 1);
}
        return v;
    }
}

TEST(GoldenActivationFunction, SigmoidApplyAcrossRegimes)
{
    struct Regime { std::string slug; double x_shift; double steepness; };
    const std::vector<Regime> regimes = {
        { "activation_sigmoid_xs0_b10",   0.0,  10.0 },
        { "activation_sigmoid_xs5_b4",    5.0,   4.0 },
        { "activation_sigmoid_xsm3_b20", -3.0,  20.0 },
        { "activation_sigmoid_xs0_b100",  0.0, 100.0 }, // steep, near-Heaviside
        { "activation_sigmoid_xs0_b0_5",  0.0,   0.5 }, // very shallow
    };

    for (const auto& r : regimes)
    {
        auto input = linspace(-25.0, 25.0, 51);
        input.push_back(r.x_shift); // exact inflection point

        const SigmoidFunction fn(r.x_shift, r.steepness);
        std::vector<double> production(input.size());
        fn.apply(input, production);

        const std::vector<double> reference = g::ref::sigmoidApply(input, r.x_shift, r.steepness);
        g::checkAgainstReference(r.slug, production, reference);
    }
}

TEST(GoldenActivationFunction, HeavisideApplyAcrossRegimes)
{
    struct Regime { std::string slug; double x_shift; };
    const std::vector<Regime> regimes = {
        { "activation_heaviside_xs0",   0.0 },
        { "activation_heaviside_xs5",   5.0 },
        { "activation_heaviside_xsm2", -2.0 },
    };

    for (const auto& r : regimes)
    {
        auto input = linspace(-10.0, 10.0, 41);
        input.push_back(r.x_shift); // boundary: strict '>' means x_shift itself maps to 0

        const HeavisideFunction fn(r.x_shift);
        std::vector<double> production(input.size());
        fn.apply(input, production);

        const std::vector<double> reference = g::ref::heavisideApply(input, r.x_shift);
        g::checkAgainstReference(r.slug, production, reference);
    }
}

TEST(GoldenActivationFunction, AbsSigmoidApplyAcrossRegimes)
{
    struct Regime { std::string slug; double x_shift; double beta; };
    const std::vector<Regime> regimes = {
        { "activation_abssigmoid_xs0_b20",   0.0,  20.0 },
        { "activation_abssigmoid_xs5_b100",  5.0, 100.0 }, // cedar-default-like (near-Heaviside)
        { "activation_abssigmoid_xsm4_b2",  -4.0,   2.0 },
    };

    for (const auto& r : regimes)
    {
        auto input = linspace(-15.0, 15.0, 41);
        input.push_back(r.x_shift);

        const AbsSigmoidFunction fn(r.x_shift, r.beta);
        std::vector<double> production(input.size());
        fn.apply(input, production);

        const std::vector<double> reference = g::ref::absSigmoidApply(input, r.x_shift, r.beta);
        g::checkAgainstReference(r.slug, production, reference);
    }
}
