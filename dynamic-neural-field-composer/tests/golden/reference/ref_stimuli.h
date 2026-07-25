#pragma once
// ----------------------------------------------------------------------------
//  Golden implementation reference — Stimuli family (Agent A)
//    GaussStimulus2D, BoostStimulus, BoostStimulus2D,
//    TimedGaussStimulus, TimedGaussStimulus2D
//
//  Independent re-derivation of the maths in
//    src/elements/gauss_stimulus_2d.cpp, boost_stimulus(.cpp|_2d.cpp),
//    timed_gauss_stimulus(.cpp|_2d.cpp)  +  tools/math.h
//    {gaussian_2d, gaussian_2d_periodic, gauss, circularGauss}.
//  This file deliberately does NOT include the production element headers for
//  the maths — it re-expresses each definition from first principles.
// ----------------------------------------------------------------------------
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <utility>

// Windows.h (pulled in transitively) defines min/max macros that break std::min.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace dnf_composer::golden::ref
{
    // ------------------------------------------------------------------
    // 1D building blocks (mirrors tools::math::gauss / circularGauss,
    // 1-indexed x = i+1, position already expressed in samples = position/d_x)
    // ------------------------------------------------------------------
    inline std::vector<double> gauss1d(int size, double sigma, double positionSamples)
    {
        std::vector<double> g(size);
        for (int i = 0; i < size; ++i)
        {
            const double x = static_cast<double>(i + 1);
            g[i] = std::exp(-0.5 * std::pow(x - positionSamples, 2) / std::pow(sigma, 2));
        }
        return g;
    }

    inline std::vector<double> circularGauss1d(int size, double sigma, double positionSamples)
    {
        const double l = static_cast<double>(size); // (size - 2*1 + 2) == size
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

    // ------------------------------------------------------------------
    // 2D Gaussian building blocks (mirrors tools::math::gaussian_2d /
    // gaussian_2d_periodic). Coordinates are ACTUAL spatial coordinates
    // x = (xi+1)*d_x, y = (yi+1)*d_y (NOT samples) — this is the convention
    // used by GaussStimulus2D / TimedGaussStimulus2D, distinct from the 1D
    // sample-indexed gauss() above.
    // ------------------------------------------------------------------
    inline double gaussian2d(double x, double y, double mu_x, double mu_y,
                              double sigma_x, double sigma_y, double A)
    {
        const double exponent = -((std::pow(x - mu_x, 2) / (2.0 * std::pow(sigma_x, 2))) +
                                   (std::pow(y - mu_y, 2) / (2.0 * std::pow(sigma_y, 2))));
        return A * std::exp(exponent);
    }

    inline double gaussian2dPeriodic(double x, double y, double mu_x, double mu_y,
                                      double sigma, double A, double max_x, double max_y)
    {
        const double dx = std::min(std::abs(x - mu_x), max_x - std::abs(x - mu_x));
        const double dy = std::min(std::abs(y - mu_y), max_y - std::abs(y - mu_y));
        const double exponent = -((std::pow(dx, 2) + std::pow(dy, 2)) / (2.0 * std::pow(sigma, 2)));
        return A * std::exp(exponent);
    }

    // Full GaussStimulus2D "output" component after init() (no external input).
    // field is y-major flattened: field[yi*size_x + xi].
    inline std::vector<double> gaussStimulus2DOutput(
        int size_x, int size_y, double d_x, double d_y, double x_max, double y_max,
        double width, double position_x, double position_y,
        double amplitude, bool circular, bool normalized)
    {
        std::vector<double> out(static_cast<std::size_t>(size_x) * size_y);
        double sum = 0.0;
        for (int xi = 0; xi < size_x; ++xi)
        {
            const double x = (xi + 1) * d_x;
            for (int yi = 0; yi < size_y; ++yi)
            {
                const double y = (yi + 1) * d_y;
                const double val = circular
                    ? gaussian2dPeriodic(x, y, position_x, position_y, width, 1.0, x_max, y_max)
                    : gaussian2d(x, y, position_x, position_y, width, width, 1.0);
                out[yi * size_x + xi] = val;
                sum += val;
            }
        }
        if (!normalized)
        {
            for (double& v : out) v *= amplitude;
        }
        else if (sum > 1e-12)
        {
            for (double& v : out) v = amplitude * v / sum;
        }
        return out;
    }

    // ------------------------------------------------------------------
    // BoostStimulus / BoostStimulus2D: spatially homogeneous fill.
    // ------------------------------------------------------------------
    inline std::vector<double> boostStimulusOutput(int size, double amplitude, bool isActive)
    {
        return std::vector<double>(size, isActive ? amplitude : 0.0);
    }

    // ------------------------------------------------------------------
    // TimedGaussStimulus: precomputed 1D Gaussian pattern, gated by onTimes.
    // ------------------------------------------------------------------
    inline std::vector<double> timedGaussStimulusPattern(
        int size, double sigma, double positionSamples, double amplitude,
        bool circular, bool normalized)
    {
        std::vector<double> g = circular ? circularGauss1d(size, sigma, positionSamples)
                                          : gauss1d(size, sigma, positionSamples);
        std::vector<double> pattern(size);
        if (!normalized)
        {
            for (int i = 0; i < size; ++i) pattern[i] = amplitude * g[i];
        }
        else
        {
            const double sum = std::accumulate(g.begin(), g.end(), 0.0);
            for (int i = 0; i < size; ++i)
                pattern[i] = (sum != 0.0) ? amplitude * g[i] / sum : 0.0;
        }
        return pattern;
    }

    inline bool isOnAt(double t, const std::vector<std::pair<double, double>>& onTimes)
    {
        for (const auto& interval : onTimes)
            if (t >= interval.first && t <= interval.second) return true;
        return false;
    }

    // output at time t: pattern if any window contains t, else zeros.
    inline std::vector<double> timedGaussStimulusOutputAt(
        double t, const std::vector<std::pair<double, double>>& onTimes,
        const std::vector<double>& pattern)
    {
        if (isOnAt(t, onTimes)) return pattern;
        return std::vector<double>(pattern.size(), 0.0);
    }

    // ------------------------------------------------------------------
    // TimedGaussStimulus2D: precomputed 2D Gaussian pattern, gated by onTimes.
    // ------------------------------------------------------------------
    inline std::vector<double> timedGaussStimulus2DPattern(
        int size_x, int size_y, double d_x, double d_y, double x_max, double y_max,
        double width, double position_x, double position_y,
        double amplitude, bool circular, bool normalized)
    {
        // Same maths as gaussStimulus2DOutput's raw pattern (amplitude applied the
        // same way — TimedGaussStimulus2D precomputes an identical pattern to
        // GaussStimulus2D's output, just gated in step() instead of returned in init()).
        return gaussStimulus2DOutput(size_x, size_y, d_x, d_y, x_max, y_max,
                                      width, position_x, position_y, amplitude, circular, normalized);
    }
}
