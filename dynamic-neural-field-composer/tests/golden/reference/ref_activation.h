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
//  SigmoidFunction::apply() — the path actually used by NeuralField::calculateOutput()
//  every step — computes the logistic sigmoid in full float64 (previously float32;
//  the float32/float64 split with operator() was a production inconsistency, fixed
//  by moving apply() to float64 end-to-end), with the exponent clamped to [-88, 88]
//  to avoid a denormal-producing overflow path in std::exp — see the comment on
//  SigmoidFunction::apply() for why. HeavisideFunction::apply() and
//  AbsSigmoidFunction::apply() are plain float64 and match their respective
//  operator() exactly.
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
    // Mirrors SigmoidFunction::apply() — full float64, exponent clamped to [-88, 88].
    inline double sigmoidApply(double x, double x_shift, double steepness)
    {
        const double s  = steepness;
        const double xs = x_shift;
        double e = -s * (x - xs);
        e = e < -88.0 ? -88.0 : (e > 88.0 ? 88.0 : e);
        return 1.0 / (1.0 + std::exp(e));
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
