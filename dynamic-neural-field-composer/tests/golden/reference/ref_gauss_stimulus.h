#pragma once
// ----------------------------------------------------------------------------
//  Golden implementation reference — GaussStimulus (1D)   [EXEMPLAR]
//
//  Independent re-derivation of the maths in
//    src/elements/gauss_stimulus.cpp  +  tools/math.h {gauss, circularGauss}.
//  This file deliberately does NOT include the production element headers for
//  the maths — it re-expresses the definition from first principles so that a
//  bug introduced in the library cannot "hide" behind a shared helper.
//
//  Definition reproduced (position is in *samples* = position / d_x):
//    non-circular:  g[i] = exp(-0.5 * (x - p)^2 / sigma^2),  x = i+1  (1-indexed)
//    circular    :  wraps the distance on a ring of length l = size
//    output      :  amplitude * g            (or amplitude * g / sum(g) if normalized)
//    (+ summed external input, which is 0 for a standalone stimulus)
// ----------------------------------------------------------------------------
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>

// Windows.h (pulled in transitively) defines min/max macros that break std::min.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace dnf_composer::golden::ref
{
    // Mirrors tools::math::gauss(size, sigma, position) — 1-indexed x = i+1.
    inline std::vector<double> gauss(int size, double sigma, double positionSamples)
    {
        std::vector<double> g(size);
        for (int i = 0; i < size; ++i)
        {
            const double x = static_cast<double>(i + 1);
            g[i] = std::exp(-0.5 * std::pow(x - positionSamples, 2) / std::pow(sigma, 2));
        }
        return g;
    }

    // Mirrors tools::math::circularGauss(size, sigma, position).
    inline std::vector<double> circularGauss(int size, double sigma, double positionSamples)
    {
        const double l = static_cast<double>(size - 2 * 1 + 2); // == size
        const double m = 1.0;
        const double r = positionSamples - m;
        const double rem = std::fmod(r, l);
        const double positionShifted = rem + m;

        std::vector<double> g(size);
        for (int i = 0; i < size; ++i)
        {
            const double x = static_cast<double>(i + 1);
            const double d = std::abs(x - positionShifted);
            const double lMinusD = -1.0 * (d - l);
            g[i] = std::exp(-0.5 * std::pow(std::min(d, lMinusD), 2) / std::pow(sigma, 2));
        }
        return g;
    }

    // Full GaussStimulus "output" component after init() (no external input).
    inline std::vector<double> gaussStimulusOutput(int size, double sigma, double positionSamples,
                                                   double amplitude, bool circular, bool normalized)
    {
        std::vector<double> g = circular ? circularGauss(size, sigma, positionSamples)
                                         : gauss(size, sigma, positionSamples);
        std::vector<double> out(size);
        if (!normalized)
        {
            for (int i = 0; i < size; ++i) out[i] = amplitude * g[i];
        }
        else
        {
            const double sum = std::accumulate(g.begin(), g.end(), 0.0);
            for (int i = 0; i < size; ++i) out[i] = (sum != 0.0) ? amplitude * g[i] / sum : 0.0;
        }
        return out;
    }
}
