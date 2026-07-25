#pragma once
// ----------------------------------------------------------------------------
//  Golden implementation reference — FieldCoupling, GaussFieldCoupling
//
//  Independent re-derivation of the maths in
//    src/elements/field_coupling.cpp        (updateOutput: output = scalar * W * input)
//    src/elements/gauss_field_coupling.cpp  (init: W built from a list of GaussCoupling
//                                             point-to-point projections; step: output = W * input)
//    + tools/math.h {gaussian_2d, gaussian_2d_periodic}.
//
//  This file does NOT include tools/math.h or the production element headers
//  for the maths — every formula is re-expressed from first principles.
//
//  Weight matrix layout (both classes, verified against production):
//    weights[j * outputSize + i]  where  j = input index, i = output index.
//    output[i] = sum_j  (scalar *) weights[j*outputSize+i] * input[j]
// ----------------------------------------------------------------------------
#include <vector>
#include <cmath>
#include <numbers>
#include <algorithm>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace dnf_composer::golden::ref
{
    // ==== FieldCoupling: output = scalar * W * input =======================
    // Given an explicit weight matrix (caller-supplied — this is what the
    // production test also writes directly into the element's "weights"
    // component, since learning is not exercised here), reproduce the matmul.
    inline std::vector<double> fieldCouplingOutput(const std::vector<double>& input,
        const std::vector<double>& weights, int outputSize, double scalar)
    {
        const int inputSize = static_cast<int>(input.size());
        std::vector<double> out(outputSize, 0.0);
        for (int i = 0; i < outputSize; ++i)
        {
            double acc = 0.0;
            for (int j = 0; j < inputSize; ++j)
                acc += scalar * weights[static_cast<std::size_t>(j) * outputSize + i] * input[j];
            out[i] = acc;
        }
        return out;
    }

    // ==== GaussFieldCoupling =================================================

    struct CouplingSpec
    {
        double x_i, x_j, amplitude, width;
    };

    // Mirrors tools::math::gaussian_2d.
    inline double gaussian2d(double x, double y, double mu_x, double mu_y, double sigma_x, double sigma_y, double A)
    {
        const double exponent = -((std::pow(x - mu_x, 2) / (2.0 * sigma_x * sigma_x)) +
                                   (std::pow(y - mu_y, 2) / (2.0 * sigma_y * sigma_y)));
        return A * std::exp(exponent);
    }

    // Mirrors tools::math::gaussian_2d_periodic.
    inline double gaussian2dPeriodic(double x, double y, double mu_x, double mu_y, double sigma, double A,
                                     double max_x, double max_y)
    {
        const double xStep = std::min(std::abs(x - mu_x), max_x - std::abs(x - mu_x));
        const double dy = std::min(std::abs(y - mu_y), max_y - std::abs(y - mu_y));
        const double exponent = -((xStep * xStep + dy * dy) / (2.0 * sigma * sigma));
        return A * std::exp(exponent);
    }

    // Mirrors GaussFieldCoupling::init(): builds the (inputSize x outputSize)
    // weight matrix, y-major-ish flattened as weights[j*cols + i] (j = input
    // row index 0..rows-1, i = output col index 0..cols-1), summing every
    // coupling's contribution. d_x_in / d_x_out are the input/output field
    // spatial resolutions (coupling coordinates are given in physical units
    // and divided by resolution before use, matching production).
    inline std::vector<double> gaussFieldCouplingWeights(int rows /*input size*/, int cols /*output size*/,
        double d_x_in, double d_x_out, bool normalized, bool circular,
        const std::vector<CouplingSpec>& couplings)
    {
        std::vector<double> weights(static_cast<std::size_t>(rows) * cols, 0.0);
        for (int i = 0; i < cols; ++i)
        {
            for (int j = 0; j < rows; ++j)
            {
                double value = 0.0;
                for (const auto& c : couplings)
                {
                    double amplitude = c.amplitude;
                    if (normalized) amplitude /= std::sqrt(2.0 * std::numbers::pi * c.width * c.width);

                    if (circular)
                    {
                        value += gaussian2dPeriodic(j, i, c.x_i / d_x_in, c.x_j / d_x_out, c.width, amplitude,
                                                    rows, cols);
                    }
                    else
                    {
                        value += gaussian2d(j, i, c.x_i / d_x_in, c.x_j / d_x_out, c.width, c.width, amplitude);
                    }
                }
                weights[static_cast<std::size_t>(j) * cols + i] = value;
            }
        }
        return weights;
    }

    // Mirrors GaussFieldCoupling::updateOutput(): output = W * input (no scalar).
    inline std::vector<double> gaussFieldCouplingOutput(const std::vector<double>& input,
        const std::vector<double>& weights, int outputSize)
    {
        return fieldCouplingOutput(input, weights, outputSize, 1.0);
    }
}
