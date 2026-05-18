#pragma once

#include "kernel.h"
#include "tools/math.h"


namespace dnf_composer::element
{
	/// @brief Parameters for an oscillatory (damped-cosine) convolution kernel.
	/// @ingroup elements
	struct OscillatoryKernelParameters final : ElementSpecificParameters
	{
		double amplitude;        ///< Overall kernel amplitude.
		double decay;            ///< Exponential envelope decay rate (must be > 0).
		double zeroCrossings;    ///< Spatial frequency controlling oscillation period (clamped to [0, 1]).
		double amplitudeGlobal;  ///< Spatially uniform inhibition added after convolution.
		bool circular;           ///< Enable circular (toroidal) convolution.
		bool normalized;         ///< Normalise the kernel before convolution.

		explicit OscillatoryKernelParameters(const double amplitude = 1.0, const double decay = 0.08,
			const double zeroCrossings = 0.3, const double amplitudeGlobal = -0.01,
			const bool circular = true, const bool normalized = false)
			: amplitude(amplitude), decay(decay),
			zeroCrossings(zeroCrossings), amplitudeGlobal(amplitudeGlobal),
			circular(circular), normalized(normalized)
		{
			// zero crossings must be in the range [0, 1]
			if (zeroCrossings < 0.0)
				this->zeroCrossings = 0.0;
			else if (zeroCrossings > 1.0)
				this->zeroCrossings = 1.0;
			// decay cannot be negative or zero
			if (decay <= 0.0)
				this->decay = 0.01;
		}

		bool operator==(const OscillatoryKernelParameters& other) const
		{
			constexpr double epsilon = 1e-6;

			return std::abs(amplitude - other.amplitude) < epsilon &&
				std::abs(decay - other.decay) < epsilon &&
				std::abs(zeroCrossings - other.zeroCrossings) < epsilon &&
				std::abs(amplitudeGlobal - other.amplitudeGlobal) < epsilon &&
				circular == other.circular &&
				normalized == other.normalized;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "Amplitude: " << amplitude << ", "
				<< "Decay: " << decay << ", "
				<< "Zero crossings: " << zeroCrossings << ", "
				<< "Amplitude global: " << amplitudeGlobal << ", "
				<< "Circular: " << (circular ? "true" : "false") << ", "
				<< "Normalized: " << (normalized ? "true" : "false") << "]";
			return result.str();
		}
	};

	/// @brief Damped-cosine convolution kernel for oscillatory field dynamics.
	///
	/// The kernel shape is a decaying cosine: @c amplitude * exp(-decay * |x|) * cos(2π * zeroCrossings * x).
	/// This produces alternating excitatory and inhibitory bands at increasing distances,
	/// which can generate wave-like or rhythmic activity patterns in a neural field.
	///
	/// @ingroup elements
	class OscillatoryKernel final : public Kernel
	{
	private:
		OscillatoryKernelParameters parameters;
		std::vector<double> scratchExtended;
		std::vector<double> scratchConvolution;
	public:
		/// @brief Construct an OscillatoryKernel.
		/// @param elementCommonParameters  Name, label, and spatial dimensions.
		/// @param ok_parameters            Kernel-specific parameters.
		OscillatoryKernel(const ElementCommonParameters& elementCommonParameters,
			OscillatoryKernelParameters ok_parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const OscillatoryKernelParameters& ok_parameters);
		OscillatoryKernelParameters getParameters() const;
	};
}
