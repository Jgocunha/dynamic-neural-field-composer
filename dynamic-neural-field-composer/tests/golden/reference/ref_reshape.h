#pragma once
// ----------------------------------------------------------------------------
//  Golden implementation reference — Reshape family (Agent A)
//    Resize (1D), Resize2D, Collapse (2D->1D), Expand (1D->2D)
//
//  Independent re-derivation of the maths in tools/math.h
//    {resample*/resampleNearest*/resampleCubic*, reduce2DAxis_into,
//     broadcast1DTo2D_into}. This file deliberately does NOT call the
//     production tools::math functions — it re-expresses each algorithm
//     from first principles.
// ----------------------------------------------------------------------------
#include <vector>
#include <cmath>
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
    // ------------------------------------------------------------------
    // 1D resampling, input.size() N -> outputSize M.
    // ------------------------------------------------------------------
    inline std::vector<double> resampleLinear(const std::vector<double>& input, int outputSize)
    {
        const int N = static_cast<int>(input.size());
        if (input.empty() || outputSize <= 0) return {};
        if (N == outputSize) return input;
        if (outputSize == 1) return { input[N / 2] };

        std::vector<double> out(outputSize);
        for (int i = 0; i < outputSize; ++i)
        {
            const double pos = static_cast<double>(i) * (N - 1) / (outputSize - 1);
            const int lo = static_cast<int>(pos);
            const int hi = std::min(lo + 1, N - 1);
            const double t = pos - lo;
            out[i] = input[lo] * (1.0 - t) + input[hi] * t;
        }
        return out;
    }

    inline std::vector<double> resampleNearest(const std::vector<double>& input, int outputSize)
    {
        const int N = static_cast<int>(input.size());
        if (input.empty() || outputSize <= 0) return {};
        if (N == outputSize) return input;
        if (outputSize == 1) return { input[N / 2] };

        std::vector<double> out(outputSize);
        for (int i = 0; i < outputSize; ++i)
        {
            const double pos = static_cast<double>(i) * (N - 1) / (outputSize - 1);
            out[i] = input[static_cast<int>(std::round(pos))];
        }
        return out;
    }

    // Catmull-Rom cubic spline resampling.
    inline std::vector<double> resampleCubic(const std::vector<double>& input, int outputSize)
    {
        const int N = static_cast<int>(input.size());
        if (input.empty() || outputSize <= 0) return {};
        if (N == outputSize) return input;
        if (outputSize == 1) return { input[N / 2] };

        const auto clamp = [N](int idx) { return std::max(0, std::min(idx, N - 1)); };
        std::vector<double> out(outputSize);
        for (int i = 0; i < outputSize; ++i)
        {
            const double pos = static_cast<double>(i) * (N - 1) / (outputSize - 1);
            const int lo = static_cast<int>(pos);
            const double t = pos - lo;
            const double p0 = input[clamp(lo - 1)];
            const double p1 = input[clamp(lo)];
            const double p2 = input[clamp(lo + 1)];
            const double p3 = input[clamp(lo + 2)];
            out[i] = 0.5 * (2.0 * p1 + (-p0 + p2) * t +
                (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t * t +
                (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * t * t * t);
        }
        return out;
    }

    // ------------------------------------------------------------------
    // Resize2D: separable pass — resample each row along x (Nx->Mx), then
    // each column of the result along y (Ny->My). field is y-major.
    // ------------------------------------------------------------------
    enum class Interp { LINEAR, NEAREST, CUBIC };

    inline std::vector<double> resample1d(const std::vector<double>& in, int outSize, Interp method)
    {
        switch (method)
        {
        case Interp::LINEAR:  return resampleLinear(in, outSize);
        case Interp::NEAREST: return resampleNearest(in, outSize);
        case Interp::CUBIC:   return resampleCubic(in, outSize);
        }
        return {};
    }

    inline std::vector<double> resize2D(const std::vector<double>& field,
        int inSizeX, int inSizeY, int outSizeX, int outSizeY, Interp method)
    {
        // Pass 1: resample each row along x -> scratch is (outSizeX x inSizeY), y-major.
        std::vector<double> scratch(static_cast<std::size_t>(outSizeX) * inSizeY);
        for (int y = 0; y < inSizeY; ++y)
        {
            std::vector<double> row(inSizeX);
            for (int x = 0; x < inSizeX; ++x) row[x] = field[static_cast<std::size_t>(y) * inSizeX + x];
            const auto rowOut = resample1d(row, outSizeX, method);
            for (int x = 0; x < outSizeX; ++x) scratch[static_cast<std::size_t>(y) * outSizeX + x] = rowOut[x];
        }
        // Pass 2: resample each column along y -> out is (outSizeX x outSizeY), y-major.
        std::vector<double> out(static_cast<std::size_t>(outSizeX) * outSizeY);
        for (int x = 0; x < outSizeX; ++x)
        {
            std::vector<double> col(inSizeY);
            for (int y = 0; y < inSizeY; ++y) col[y] = scratch[static_cast<std::size_t>(y) * outSizeX + x];
            const auto colOut = resample1d(col, outSizeY, method);
            for (int y = 0; y < outSizeY; ++y) out[static_cast<std::size_t>(y) * outSizeX + x] = colOut[y];
        }
        return out;
    }

    // ------------------------------------------------------------------
    // Collapse: reduce a y-major 2D field along one axis.
    // keepX == true:  output has size_x entries, out[x] reduces over all y.
    // keepX == false: output has size_y entries, out[y] reduces over all x.
    // ------------------------------------------------------------------
    enum class Reduce { SUM, AVERAGE, MAXIMUM, MINIMUM };

    inline std::vector<double> reduce2DAxis(const std::vector<double>& field,
        int size_x, int size_y, bool keepX, Reduce op)
    {
        const int outSize = keepX ? size_x : size_y;
        const int reduceCount = keepX ? size_y : size_x;
        std::vector<double> out(outSize);
        for (int o = 0; o < outSize; ++o)
        {
            auto sampleAt = [&](int k) -> double
            {
                const int x = keepX ? o : k;
                const int y = keepX ? k : o;
                return field[static_cast<std::size_t>(y) * size_x + x];
            };
            double acc = sampleAt(0);
            for (int k = 1; k < reduceCount; ++k)
            {
                const double v = sampleAt(k);
                switch (op)
                {
                case Reduce::SUM:
                case Reduce::AVERAGE: acc += v; break;
                case Reduce::MAXIMUM: acc = std::max(acc, v); break;
                case Reduce::MINIMUM: acc = std::min(acc, v); break;
                }
            }
            if (op == Reduce::AVERAGE) acc /= static_cast<double>(reduceCount);
            out[o] = acc;
        }
        return out;
    }

    // ------------------------------------------------------------------
    // Expand: broadcast a 1D profile into a y-major 2D buffer.
    // alongX == true:  profile indexes x (size == size_x), repeated for every y.
    // alongX == false: profile indexes y (size == size_y), repeated for every x.
    // ------------------------------------------------------------------
    inline std::vector<double> broadcast1DTo2D(const std::vector<double>& profile,
        int size_x, int size_y, bool alongX)
    {
        std::vector<double> out(static_cast<std::size_t>(size_x) * size_y);
        for (int y = 0; y < size_y; ++y)
            for (int x = 0; x < size_x; ++x)
                out[static_cast<std::size_t>(y) * size_x + x] = profile[alongX ? x : y];
        return out;
    }
}
