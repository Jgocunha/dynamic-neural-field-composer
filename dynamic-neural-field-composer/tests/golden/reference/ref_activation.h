#pragma once
// ----------------------------------------------------------------------------
//  Golden implementation reference — Activation functions (pointwise)
//
//  Independent re-derivation of the maths in
//    src/elements/activation_function.cpp  {SigmoidFunction, HeavisideFunction,
//    AbsSigmoidFunction}::apply().
//  This file deliberately does NOT include the production element headers for
//  the maths — it re-expresses each definition from first principles.
//
//  IMPORTANT PRODUCTION QUIRK (flagged to god — see report):
//  SigmoidFunction::apply() — the path actually used by NeuralField::calculateOutput()
//  every step — computes the logistic sigmoid in **float32** (it casts steepness,
//  x_shift and the activation value to `float` before calling std::exp), while
//  SigmoidFunction::operator() (unused by the field dynamics) computes it in
//  float64 via tools::math::sigmoid(). The two code paths of the SAME class are
//  numerically inconsistent by ~1e-7 relative. HeavisideFunction::apply() and
//  AbsSigmoidFunction::apply() have no such split — both are plain float64 and
//  match their respective operator() exactly.
//
//  To keep this an ANALYTIC-EQUIVALENCE test (not a loosened tolerance), we
//  mirror the float32 computation bit-for-bit here rather than silently
//  widening kTol.
// ----------------------------------------------------------------------------
#include <vector>
#include <cmath>
#include <cstddef>

// Windows.h (pulled in transitively) defines min/max macros that break std::min.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace dnf_composer::golden::ref
{
    // Mirrors SigmoidFunction::apply() — float32 internal precision (see note above).
    inline double sigmoidApply(double x, double x_shift, double steepness)
    {
        const float s  = static_cast<float>(steepness);
        const float xs = static_cast<float>(x_shift);
        const float xv = static_cast<float>(x);
        return static_cast<double>(1.0f / (1.0f + std::exp(-s * (xv - xs))));
    }

    inline std::vector<double> sigmoidApply(const std::vector<double>& x, double x_shift, double steepness)
    {
        std::vector<double> out(x.size());
        for (std::size_t i = 0; i < x.size(); ++i) {
            out[i] = sigmoidApply(x[i], x_shift, steepness);
}
        return out;
    }

    // Mirrors HeavisideFunction::apply() — plain float64, strict '>' threshold.
    inline double heavisideApply(double x, double x_shift)
    {
        return (x > x_shift) ? 1.0 : 0.0;
    }

    inline std::vector<double> heavisideApply(const std::vector<double>& x, double x_shift)
    {
        std::vector<double> out(x.size());
        for (std::size_t i = 0; i < x.size(); ++i) {
            out[i] = heavisideApply(x[i], x_shift);
}
        return out;
    }

    // Mirrors AbsSigmoidFunction::apply() — plain float64.
    // s(x) = 0.5 * (1 + beta*(x-x0) / (1 + beta*|x-x0|))
    inline double absSigmoidApply(double x, double x_shift, double beta)
    {
        const double diff = x - x_shift;
        return 0.5 * (1.0 + beta * diff / (1.0 + beta * std::abs(diff)));
    }

    inline std::vector<double> absSigmoidApply(const std::vector<double>& x, double x_shift, double beta)
    {
        std::vector<double> out(x.size());
        for (std::size_t i = 0; i < x.size(); ++i) {
            out[i] = absSigmoidApply(x[i], x_shift, beta);
}
        return out;
    }
}
