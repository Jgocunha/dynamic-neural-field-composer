#pragma once
// ----------------------------------------------------------------------------
//  Golden implementation reference — NeuralField / NeuralField2D / MemoryTrace
//
//  Independent re-derivation of the maths in
//    src/elements/neural_field.cpp    NeuralField::calculateActivation/calculateOutput
//    src/elements/neural_field_2d.cpp NeuralField2D::calculateActivation/calculateOutput
//    src/elements/memory_trace.cpp    MemoryTrace::step
//    src/elements/memory_trace_2d.cpp MemoryTrace2D::step
//  Does NOT call production maths — re-expresses the Amari update and the
//  leaky-integrator memory trace from first principles.
//
//  Amari update reproduced (matches NeuralField::step() ordering EXACTLY):
//    step(t, dt):
//      1. input[i]      = (sum of externally supplied input this step)
//      2. act[i]       += (dt/tau) * ( -act[i] + h + input[i] )   // Euler, OLD act
//      3. output[i]      = f(act[i])                              // f of the NEW act
//  The external input for a step is supplied by the caller via a callback
//  (externalInputAtStep) — for a standalone field driven only by a
//  time-invariant GaussStimulus, that is just a constant vector (GaussStimulus::
//  step() is a no-op, its "output" never changes after init()).
//
//  MemoryTrace update reproduced (matches MemoryTrace::step() EXACTLY):
//    if input[i] > threshold: out[i] += dt * (1/tauBuild) * (-out[i] + input[i])
//    else:                    out[i] += dt * (1/tauDecay) * (-out[i])
// ----------------------------------------------------------------------------
#include <vector>
#include <functional>
#include <cstddef>

#include "ref_activation.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace dnf_composer::golden::ref
{
    // Pointwise activation-function selector mirroring NeuralField's
    // parameters.activationFunction->apply() call.
    using ActivationFn = std::function<std::vector<double>(const std::vector<double>&)>;

    inline ActivationFn makeSigmoidFn(double x_shift, double steepness)
    {
        return [x_shift, steepness](const std::vector<double>& v) { return sigmoidApply(v, x_shift, steepness); };
    }

    inline ActivationFn makeHeavisideFn(double x_shift)
    {
        return [x_shift](const std::vector<double>& v) { return heavisideApply(v, x_shift); };
    }

    inline ActivationFn makeAbsSigmoidFn(double x_shift, double beta)
    {
        return [x_shift, beta](const std::vector<double>& v) { return absSigmoidApply(v, x_shift, beta); };
    }

    struct FieldTrajectory
    {
        std::vector<std::vector<double>> activation; // one row per step (post-update)
        std::vector<std::vector<double>> output;      // one row per step (f of that step's activation)
    };

    // Independent multi-step Euler integration of the Amari field equation.
    //   size               field sample count (1D size, or size_x*size_y flattened y-major for 2D)
    //   tau, restingLevel  field parameters (h == restingLevel, homogeneous)
    //   dt                 integration step size
    //   f                  activation function (post-update)
    //   steps              number of simulation steps to integrate
    //   externalInputAtStep(s) -> vector of size `size`: the summed external input
    //                       (from stimuli/kernels/etc.) supplied during step s (0-indexed).
    inline FieldTrajectory amariFieldTrajectory(
        int size, double tau, double restingLevel, double dt,
        const ActivationFn& f, int steps,
        const std::function<std::vector<double>(int)>& externalInputAtStep)
    {
        std::vector<double> act(static_cast<std::size_t>(size), restingLevel);
        const double h = restingLevel;
        const double dtOverTau = dt / tau;

        FieldTrajectory traj;
        traj.activation.reserve(static_cast<std::size_t>(steps));
        traj.output.reserve(static_cast<std::size_t>(steps));

        for (int s = 0; s < steps; ++s)
        {
            const std::vector<double> input = externalInputAtStep(s);
            for (int i = 0; i < size; ++i) {
                act[static_cast<std::size_t>(i)] += dtOverTau *
                    (-act[static_cast<std::size_t>(i)] + h + input[static_cast<std::size_t>(i)]);
}
            std::vector<double> out = f(act);
            traj.activation.push_back(act);
            traj.output.push_back(std::move(out));
        }
        return traj;
    }

    // Independent leaky-integrator memory trace (mirrors MemoryTrace/MemoryTrace2D
    // step() — identical formula, `size` is 1D size or flattened 2D size).
    inline std::vector<std::vector<double>> memoryTraceTrajectory(
        int size, double tauBuild, double tauDecay, double threshold, double dt, int steps,
        const std::function<std::vector<double>(int)>& inputAtStep)
    {
        std::vector<double> out(static_cast<std::size_t>(size), 0.0);
        std::vector<std::vector<double>> traj;
        traj.reserve(static_cast<std::size_t>(steps));

        for (int s = 0; s < steps; ++s)
        {
            const std::vector<double> in = inputAtStep(s);
            for (int i = 0; i < size; ++i)
            {
                const std::size_t ui = static_cast<std::size_t>(i);
                if (in[ui] > threshold) {
                    out[ui] += dt * (1.0 / tauBuild) * (-out[ui] + in[ui]);
                } else {
                    out[ui] += dt * (1.0 / tauDecay) * (-out[ui]);
}
            }
            traj.push_back(out);
        }
        return traj;
    }
}
