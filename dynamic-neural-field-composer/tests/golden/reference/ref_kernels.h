#pragma once
// ----------------------------------------------------------------------------
//  Golden implementation reference — lateral-interaction Kernels (1D + 2D)
//
//  Independent re-derivation of the maths in
//    src/elements/{gauss,mexican_hat,oscillatory,asymmetric_gauss}_kernel(_2d).cpp
//    + tools/math.h {computeKernelRange, createExtendedIndex, obtainCircularVector,
//                     conv_valid, conv_same, conv2d_separable, gauss(Norm),
//                     gaussDerivative(Norm), gaussian_2d(_periodic)}.
//
//  This file deliberately does NOT include tools/math.h or any production
//  element header for the maths — every formula and every piece of index
//  plumbing (kernel range, circular extension, convolution) is re-expressed
//  here from first principles, so a bug introduced in the shared library
//  cannot "hide" behind a helper this reference also calls into.
//
//  Every kernel produces two things we verify against production:
//    (a) the "kernel" component — the analytic weight profile (1D) or the
//        outer-product weight matrix (2D, y-major flattened).
//    (b) the "output" component — kernel convolved with a fixed probe input
//        (itself independently derived, mirroring GaussStimulus(2D)), plus
//        the amplitudeGlobal * fullSum(input) global-inhibition offset.
// ----------------------------------------------------------------------------
#include <vector>
#include <array>
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
    // ==== index / convolution plumbing (independently re-derived) =========

    // Mirrors tools::math::computeKernelRange.
    inline std::array<int, 2> computeKernelRange(double sigma, int cutOfFactor, int fieldSize, bool circular)
    {
        const int ceilSigmaCutOff = static_cast<int>(std::ceil(sigma * cutOfFactor));
        const int fieldSizeMinus1 = fieldSize - 1;
        if (circular)
        {
            const double half = (static_cast<double>(fieldSize) - 1) / 2.0;
            const int floorHalf = static_cast<int>(std::floor(half));
            const int ceilHalf  = static_cast<int>(std::ceil(half));
            return { std::min(ceilSigmaCutOff, floorHalf), std::min(ceilSigmaCutOff, ceilHalf) };
        }
        return { std::min(ceilSigmaCutOff, fieldSizeMinus1), std::min(ceilSigmaCutOff, fieldSizeMinus1) };
    }

    // Mirrors tools::math::createExtendedIndex.
    inline std::vector<int> createExtendedIndex(int fieldSize, const std::array<int, 2>& kernelRange)
    {
        const int startingValue = fieldSize - kernelRange[1] + 1;
        std::vector<int> ext;
        ext.reserve(static_cast<std::size_t>(fieldSize) + kernelRange[0] + kernelRange[1]);
        for (int v = startingValue; v <= fieldSize; ++v) ext.push_back(v);
        for (int v = 1; v <= fieldSize; ++v) ext.push_back(v);
        for (int v = 1; v <= kernelRange[0]; ++v) ext.push_back(v);
        return ext;
    }

    // Mirrors tools::math::obtainCircularVector.
    inline std::vector<double> obtainCircularVector(const std::vector<int>& idx, const std::vector<double>& contents)
    {
        std::vector<double> out(idx.size());
        for (std::size_t i = 0; i < idx.size(); ++i) out[i] = contents[idx[i] - 1];
        return out;
    }

    // Mirrors tools::math::conv_valid.
    inline std::vector<double> convValid(const std::vector<double>& f, const std::vector<double>& g)
    {
        const int nf = static_cast<int>(f.size()), ng = static_cast<int>(g.size());
        const std::vector<double>& minV = (nf < ng) ? f : g;
        const std::vector<double>& maxV = (nf < ng) ? g : f;
        const int n = std::max(nf, ng) - std::min(nf, ng) + 1;
        std::vector<double> out(n, 0.0);
        for (int i = 0; i < n; ++i)
        {
            double acc = 0.0;
            for (int j = static_cast<int>(minV.size()) - 1, k = i; j >= 0; --j, ++k) acc += minV[j] * maxV[k];
            out[i] = acc;
        }
        return out;
    }

    // Mirrors tools::math::conv_same.
    inline std::vector<double> convSame(const std::vector<double>& f, const std::vector<double>& g)
    {
        const int nf = static_cast<int>(f.size()), ng = static_cast<int>(g.size());
        std::vector<double> out(nf, 0.0);
        const int pad = (ng - 1) / 2;
        for (int i = 0; i < nf; ++i)
        {
            double acc = 0.0;
            for (int j = 0; j < ng; ++j)
            {
                const int fi = i + j - pad;
                if (fi >= 0 && fi < nf) acc += f[fi] * g[j];
            }
            out[i] = acc;
        }
        return out;
    }

    // Mirrors tools::math::conv2d_separable.
    inline std::vector<double> conv2dSeparable(const std::vector<double>& field,
        const std::vector<double>& kx, const std::vector<double>& ky,
        int size_x, int size_y, const std::vector<int>& extX, const std::vector<int>& extY)
    {
        const bool circX = !extX.empty();
        const bool circY = !extY.empty();

        std::vector<double> tmp(static_cast<std::size_t>(size_x) * size_y, 0.0);
        for (int y = 0; y < size_y; ++y)
        {
            std::vector<double> row(size_x);
            for (int x = 0; x < size_x; ++x) row[x] = field[y * size_x + x];
            std::vector<double> convRow = circX
                ? convValid(obtainCircularVector(extX, row), kx)
                : convSame(row, kx);
            for (int x = 0; x < size_x; ++x) tmp[y * size_x + x] = convRow[x];
        }

        std::vector<double> result(static_cast<std::size_t>(size_x) * size_y, 0.0);
        for (int x = 0; x < size_x; ++x)
        {
            std::vector<double> col(size_y);
            for (int y = 0; y < size_y; ++y) col[y] = tmp[y * size_x + x];
            std::vector<double> convCol = circY
                ? convValid(obtainCircularVector(extY, col), ky)
                : convSame(col, ky);
            for (int y = 0; y < size_y; ++y) result[y * size_x + x] = convCol[y];
        }
        return result;
    }

    // rangeVec = [-kernelRange[0] .. +kernelRange[1]] (inclusive).
    inline std::vector<int> makeRange(const std::array<int, 2>& kr)
    {
        const int n = kr[0] + kr[1] + 1;
        std::vector<int> r(n);
        for (int i = 0; i < n; ++i) r[i] = -kr[0] + i;
        return r;
    }

    // ==== Gaussian weight-profile primitives (mirrors tools::math::gauss*) =

    inline std::vector<double> gaussProfile(const std::vector<int>& rangeX, double sigma)
    {
        std::vector<double> g(rangeX.size());
        for (std::size_t i = 0; i < g.size(); ++i)
            g[i] = std::exp(-0.5 * std::pow(static_cast<double>(rangeX[i]), 2) / std::pow(sigma, 2));
        return g;
    }

    inline std::vector<double> gaussProfileNorm(const std::vector<int>& rangeX, double sigma)
    {
        auto g = gaussProfile(rangeX, sigma);
        const double s = std::accumulate(g.begin(), g.end(), 0.0);
        if (!g.empty()) for (auto& v : g) v /= s;
        return g;
    }

    inline std::vector<double> gaussDerivativeProfile(const std::vector<int>& rangeX, double sigma, double amplitude)
    {
        std::vector<double> d(rangeX.size());
        const double variance = sigma * sigma;
        for (std::size_t i = 0; i < d.size(); ++i)
        {
            const double x = rangeX[i];
            d[i] = -(x) / variance * amplitude * std::exp(-0.5 * std::pow(x, 2) / variance);
        }
        return d;
    }

    inline std::vector<double> gaussDerivativeProfileNorm(const std::vector<int>& rangeX, double sigma, double amplitude)
    {
        auto d = gaussDerivativeProfile(rangeX, sigma, amplitude);
        double sumAbs = 0.0;
        for (double v : d) sumAbs += std::abs(v);
        if (sumAbs > 0.0) for (auto& v : d) v /= sumAbs;
        return d;
    }

    // ==== probe stimuli (independent re-derivation of GaussStimulus(2D)) ===

    // Mirrors GaussStimulus: 1-indexed positions (x = i+1), d_x == 1.
    inline std::vector<double> gaussStimulus1D(int size, double sigma, double position, double amplitude,
                                               bool circular, bool normalized)
    {
        std::vector<double> g(size);
        if (circular)
        {
            const double l = static_cast<double>(size); // size - 2*1 + 2 == size
            const double r = position - 1.0;
            const double rem = std::fmod(r, l);
            const double posShift = rem + 1.0;
            for (int i = 0; i < size; ++i)
            {
                const double x = i + 1;
                const double d = std::abs(x - posShift);
                const double lMinusD = -(d - l);
                g[i] = std::exp(-0.5 * std::pow(std::min(d, lMinusD), 2) / std::pow(sigma, 2));
            }
        }
        else
        {
            for (int i = 0; i < size; ++i)
            {
                const double x = i + 1;
                g[i] = std::exp(-0.5 * std::pow(x - position, 2) / std::pow(sigma, 2));
            }
        }
        std::vector<double> out(size);
        if (!normalized)
        {
            for (int i = 0; i < size; ++i) out[i] = amplitude * g[i];
        }
        else
        {
            const double s = std::accumulate(g.begin(), g.end(), 0.0);
            for (int i = 0; i < size; ++i) out[i] = (s != 0.0) ? amplitude * g[i] / s : 0.0;
        }
        return out;
    }

    // Mirrors GaussStimulus2D: 1-indexed positions (x = (xi+1)*d_x), isotropic width.
    inline std::vector<double> gaussStimulus2D(int size_x, int size_y, double d_x, double d_y,
        double width, double position_x, double position_y, double amplitude, bool circular, bool normalized)
    {
        std::vector<double> out(static_cast<std::size_t>(size_x) * size_y);
        double sum = 0.0;
        const double x_max = size_x * d_x, y_max = size_y * d_y;
        for (int xi = 0; xi < size_x; ++xi)
        {
            const double x = (xi + 1) * d_x;
            for (int yi = 0; yi < size_y; ++yi)
            {
                const double y = (yi + 1) * d_y;
                double val;
                if (circular)
                {
                    const double dx = std::min(std::abs(x - position_x), x_max - std::abs(x - position_x));
                    const double dy = std::min(std::abs(y - position_y), y_max - std::abs(y - position_y));
                    val = std::exp(-((dx * dx) + (dy * dy)) / (2.0 * width * width));
                }
                else
                {
                    val = std::exp(-((std::pow(x - position_x, 2) / (2.0 * width * width)) +
                                      (std::pow(y - position_y, 2) / (2.0 * width * width))));
                }
                out[yi * size_x + xi] = val;
                sum += val;
            }
        }
        if (!normalized)
        {
            for (auto& v : out) v *= amplitude;
        }
        else if (sum > 1e-12)
        {
            for (auto& v : out) v = amplitude * v / sum;
        }
        return out;
    }

    // ==== per-kernel weight + convolution (the golden result) ==============

    inline constexpr int kCutOfFactor = 5; // matches Kernel::cutOfFactor (fixed by the base class)

    struct KernelResult
    {
        std::vector<double> weights; // "kernel" component
        std::vector<double> output;  // "output" component after one step on `probe`
    };

    // -- 1D --------------------------------------------------------------

    inline KernelResult gaussKernel1D(int fieldSize, double width, double amplitude, double amplitudeGlobal,
        bool circular, bool normalized, const std::vector<double>& probe)
    {
        const auto kr = computeKernelRange(width, kCutOfFactor, fieldSize, circular);
        const auto rangeX = makeRange(kr);
        const auto g = normalized ? gaussProfileNorm(rangeX, width) : gaussProfile(rangeX, width);

        std::vector<double> weights(g.size());
        for (std::size_t i = 0; i < g.size(); ++i) weights[i] = amplitude * g[i];

        const double fullSum = std::accumulate(probe.begin(), probe.begin() + fieldSize, 0.0);
        std::vector<double> conv = circular
            ? convValid(obtainCircularVector(createExtendedIndex(fieldSize, kr), probe), weights)
            : convSame(probe, weights);

        std::vector<double> out(fieldSize);
        for (int i = 0; i < fieldSize; ++i) out[i] = conv[i] + amplitudeGlobal * fullSum;
        return { weights, out };
    }

    inline KernelResult mexicanHatKernel1D(int fieldSize, double widthExc, double amplitudeExc,
        double widthInh, double amplitudeInh, double amplitudeGlobal,
        bool circular, bool normalized, const std::vector<double>& probe)
    {
        const double maxWidth = std::max((amplitudeExc != 0.0) ? widthExc : 0.0,
                                          (amplitudeInh != 0.0) ? widthInh : 0.0);
        const auto kr = computeKernelRange(maxWidth, kCutOfFactor, fieldSize, circular);
        const auto rangeX = makeRange(kr);

        const auto gExc = normalized ? gaussProfileNorm(rangeX, widthExc) : gaussProfile(rangeX, widthExc);
        const auto gInh = normalized ? gaussProfileNorm(rangeX, widthInh) : gaussProfile(rangeX, widthInh);

        std::vector<double> weights(rangeX.size());
        for (std::size_t i = 0; i < weights.size(); ++i)
            weights[i] = amplitudeExc * gExc[i] - amplitudeInh * gInh[i];

        const double fullSum = std::accumulate(probe.begin(), probe.begin() + fieldSize, 0.0);
        std::vector<double> conv = circular
            ? convValid(obtainCircularVector(createExtendedIndex(fieldSize, kr), probe), weights)
            : convSame(probe, weights);

        std::vector<double> out(fieldSize);
        for (int i = 0; i < fieldSize; ++i) out[i] = conv[i] + amplitudeGlobal * fullSum;
        return { weights, out };
    }

    inline KernelResult oscillatoryKernel1D(int fieldSize, double amplitude, double decay, double zeroCrossings,
        double amplitudeGlobal, bool circular, bool normalized, const std::vector<double>& probe)
    {
        const double effectiveRange = std::max(1.0 / decay, zeroCrossings * kCutOfFactor);
        const auto kr = computeKernelRange(effectiveRange, kCutOfFactor, fieldSize, circular);
        const auto rangeX = makeRange(kr);

        std::vector<double> weights(rangeX.size());
        for (std::size_t i = 0; i < weights.size(); ++i)
        {
            const double distance = rangeX[i];
            const double decayFactor = std::exp(-decay * std::abs(distance));
            const double oscillation = std::sin(decay * std::abs(zeroCrossings * distance)) + std::cos(zeroCrossings * distance);
            weights[i] = amplitude * decayFactor * oscillation;
        }
        if (normalized)
        {
            const double normFactor = std::accumulate(weights.begin(), weights.end(), 0.0);
            if (normFactor != 0.0) for (auto& v : weights) v /= normFactor;
        }

        const double fullSum = std::accumulate(probe.begin(), probe.begin() + fieldSize, 0.0);
        std::vector<double> conv = circular
            ? convValid(obtainCircularVector(createExtendedIndex(fieldSize, kr), probe), weights)
            : convSame(probe, weights);

        std::vector<double> out(fieldSize);
        for (int i = 0; i < fieldSize; ++i) out[i] = conv[i] + amplitudeGlobal * fullSum;
        return { weights, out };
    }

    inline KernelResult asymmetricGaussKernel1D(int fieldSize, double width, double amplitude, double amplitudeGlobal,
        double timeShift, bool circular, bool normalized, const std::vector<double>& probe)
    {
        const auto kr = computeKernelRange(width, kCutOfFactor, fieldSize, circular);
        const auto rangeX = makeRange(kr);

        const auto g  = normalized ? gaussProfileNorm(rangeX, width) : gaussProfile(rangeX, width);
        const auto gd = normalized ? gaussDerivativeProfileNorm(rangeX, width, amplitude)
                                    : gaussDerivativeProfile(rangeX, width, amplitude);

        std::vector<double> weights(rangeX.size());
        for (std::size_t i = 0; i < weights.size(); ++i)
            weights[i] = amplitude * g[i] + timeShift * gd[i];

        const double fullSum = std::accumulate(probe.begin(), probe.begin() + fieldSize, 0.0);
        std::vector<double> conv = circular
            ? convValid(obtainCircularVector(createExtendedIndex(fieldSize, kr), probe), weights)
            : convSame(probe, weights);

        std::vector<double> out(fieldSize);
        for (int i = 0; i < fieldSize; ++i) out[i] = conv[i] + amplitudeGlobal * fullSum;
        return { weights, out };
    }

    // -- 2D (separable) ----------------------------------------------------

    // Flatten two 1D profiles into the y-major outer-product weight matrix,
    // as production does: weights[j*kx + i] = kx_profile[i] * ky_profile[j].
    inline std::vector<double> outerProduct(const std::vector<double>& kx, const std::vector<double>& ky)
    {
        const int nx = static_cast<int>(kx.size()), ny = static_cast<int>(ky.size());
        std::vector<double> out(static_cast<std::size_t>(nx) * ny);
        for (int i = 0; i < nx; ++i)
            for (int j = 0; j < ny; ++j)
                out[j * nx + i] = kx[i] * ky[j];
        return out;
    }

    inline KernelResult gaussKernel2D(int size_x, int size_y, double width, double amplitude, double amplitudeGlobal,
        bool circular, bool normalized, const std::vector<double>& probe)
    {
        const auto krx = computeKernelRange(width, kCutOfFactor, size_x, circular);
        const auto kry = computeKernelRange(width, kCutOfFactor, size_y, circular);
        const auto rx = makeRange(krx), ry = makeRange(kry);

        auto kx = normalized ? gaussProfileNorm(rx, width) : gaussProfile(rx, width);
        auto ky = normalized ? gaussProfileNorm(ry, width) : gaussProfile(ry, width);
        for (auto& v : kx) v *= amplitude;

        const auto weights = outerProduct(kx, ky);

        const double fullSum = std::accumulate(probe.begin(), probe.end(), 0.0);
        const auto extX = circular ? createExtendedIndex(size_x, krx) : std::vector<int>{};
        const auto extY = circular ? createExtendedIndex(size_y, kry) : std::vector<int>{};
        auto conv = conv2dSeparable(probe, kx, ky, size_x, size_y, extX, extY);

        std::vector<double> out(conv.size());
        for (std::size_t i = 0; i < out.size(); ++i) out[i] = conv[i] + amplitudeGlobal * fullSum;
        return { weights, out };
    }

    inline KernelResult mexicanHatKernel2D(int size_x, int size_y, double widthExc, double amplitudeExc,
        double widthInh, double amplitudeInh, double amplitudeGlobal,
        bool circular, bool normalized, const std::vector<double>& probe)
    {
        auto buildAxis = [&](double width, int size)
        {
            const auto kr = computeKernelRange(width, kCutOfFactor, size, circular);
            const auto r = makeRange(kr);
            auto k = normalized ? gaussProfileNorm(r, width) : gaussProfile(r, width);
            const auto ext = circular ? createExtendedIndex(size, kr) : std::vector<int>{};
            return std::tuple{ k, ext };
        };

        auto [kxExc, extXExc] = buildAxis(widthExc, size_x);
        auto [kyExc, extYExc] = buildAxis(widthExc, size_y);
        auto [kxInh, extXInh] = buildAxis(widthInh, size_x);
        auto [kyInh, extYInh] = buildAxis(widthInh, size_y);

        for (auto& v : kxExc) v *= amplitudeExc;
        for (auto& v : kxInh) v *= amplitudeInh;

        // Net outer-product weights: exc - inh, centred on the larger support (matches production).
        const int kx = std::max(static_cast<int>(kxExc.size()), static_cast<int>(kxInh.size()));
        const int ky = std::max(static_cast<int>(kyExc.size()), static_cast<int>(kyInh.size()));
        std::vector<double> weights(static_cast<std::size_t>(kx) * ky, 0.0);
        auto addProduct = [&](const std::vector<double>& kxVec, const std::vector<double>& kyVec, double sign)
        {
            const int offX = (kx - static_cast<int>(kxVec.size())) / 2;
            const int offY = (ky - static_cast<int>(kyVec.size())) / 2;
            for (int i = 0; i < static_cast<int>(kxVec.size()); ++i)
                for (int j = 0; j < static_cast<int>(kyVec.size()); ++j)
                    weights[(j + offY) * kx + (i + offX)] += sign * kxVec[i] * kyVec[j];
        };
        addProduct(kxExc, kyExc, +1.0);
        addProduct(kxInh, kyInh, -1.0);

        const double fullSum = std::accumulate(probe.begin(), probe.end(), 0.0);
        auto convExc = conv2dSeparable(probe, kxExc, kyExc, size_x, size_y, extXExc, extYExc);
        auto convInh = conv2dSeparable(probe, kxInh, kyInh, size_x, size_y, extXInh, extYInh);

        std::vector<double> out(convExc.size());
        for (std::size_t i = 0; i < out.size(); ++i) out[i] = convExc[i] - convInh[i] + amplitudeGlobal * fullSum;
        return { weights, out };
    }

    inline KernelResult oscillatoryKernel2D(int size_x, int size_y, double amplitude, double decay,
        double zeroCrossings, double amplitudeGlobal, bool circular, bool normalized, const std::vector<double>& probe)
    {
        const double effectiveRange = std::max(1.0 / decay, zeroCrossings * kCutOfFactor);
        const auto krx = computeKernelRange(effectiveRange, kCutOfFactor, size_x, circular);
        const auto kry = computeKernelRange(effectiveRange, kCutOfFactor, size_y, circular);

        auto build1D = [&](const std::array<int, 2>& kr)
        {
            const auto r = makeRange(kr);
            std::vector<double> k(r.size());
            for (std::size_t i = 0; i < k.size(); ++i)
            {
                const double dist = r[i];
                const double decayFactor = std::exp(-decay * std::abs(dist));
                const double osc = std::sin(decay * std::abs(zeroCrossings * dist)) + std::cos(zeroCrossings * dist);
                k[i] = decayFactor * osc;
            }
            if (normalized)
            {
                const double s = std::accumulate(k.begin(), k.end(), 0.0);
                if (std::abs(s) > 1e-12) for (auto& v : k) v /= s;
            }
            return k;
        };

        auto kx = build1D(krx);
        auto ky = build1D(kry);
        for (auto& v : kx) v *= amplitude;

        const auto weights = outerProduct(kx, ky);

        const double fullSum = std::accumulate(probe.begin(), probe.end(), 0.0);
        const auto extX = circular ? createExtendedIndex(size_x, krx) : std::vector<int>{};
        const auto extY = circular ? createExtendedIndex(size_y, kry) : std::vector<int>{};
        auto conv = conv2dSeparable(probe, kx, ky, size_x, size_y, extX, extY);

        std::vector<double> out(conv.size());
        for (std::size_t i = 0; i < out.size(); ++i) out[i] = conv[i] + amplitudeGlobal * fullSum;
        return { weights, out };
    }

    inline KernelResult asymmetricGaussKernel2D(int size_x, int size_y, double width, double amplitude,
        double amplitudeGlobal, double timeShift_x, double timeShift_y,
        bool circular, bool normalized, const std::vector<double>& probe)
    {
        const auto krx = computeKernelRange(width, kCutOfFactor, size_x, circular);
        const auto kry = computeKernelRange(width, kCutOfFactor, size_y, circular);
        const auto rx = makeRange(krx), ry = makeRange(kry);

        const auto gx  = normalized ? gaussProfileNorm(rx, width) : gaussProfile(rx, width);
        const auto gdx = normalized ? gaussDerivativeProfileNorm(rx, width, 1.0) : gaussDerivativeProfile(rx, width, 1.0);
        const auto gy  = normalized ? gaussProfileNorm(ry, width) : gaussProfile(ry, width);
        const auto gdy = normalized ? gaussDerivativeProfileNorm(ry, width, 1.0) : gaussDerivativeProfile(ry, width, 1.0);

        std::vector<double> kx(rx.size()), ky(ry.size());
        for (std::size_t i = 0; i < kx.size(); ++i) kx[i] = gx[i] + timeShift_x * gdx[i];
        for (std::size_t i = 0; i < ky.size(); ++i) ky[i] = gy[i] + timeShift_y * gdy[i];
        for (auto& v : kx) v *= amplitude; // amplitude applied to x-axis only (matches production)

        const auto weights = outerProduct(kx, ky);

        const double fullSum = std::accumulate(probe.begin(), probe.end(), 0.0);
        const auto extX = circular ? createExtendedIndex(size_x, krx) : std::vector<int>{};
        const auto extY = circular ? createExtendedIndex(size_y, kry) : std::vector<int>{};
        auto conv = conv2dSeparable(probe, kx, ky, size_x, size_y, extX, extY);

        std::vector<double> out(conv.size());
        for (std::size_t i = 0; i < out.size(); ++i) out[i] = conv[i] + amplitudeGlobal * fullSum;
        return { weights, out };
    }
}
