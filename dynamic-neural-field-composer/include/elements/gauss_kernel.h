#pragma once

#include <array>
#include <optional>

#include "tools/math.h"
#include "kernel.h"

namespace dnf_composer::element
{
	/// @brief Parameters for a Gaussian lateral interaction kernel.
	/// @ingroup elements
	struct GaussKernelParameters final : ElementSpecificParameters
	{
		double width;           ///< Standard deviation (σ) of the Gaussian.
		double amplitude;       ///< Peak amplitude of the excitatory Gaussian.
		double amplitudeGlobal; ///< Spatially uniform inhibition added after convolution.
		bool circular;          ///< If true, convolution wraps around (toroidal boundary).
		bool normalized;        ///< If true, the Gaussian is area-normalised.
		std::optional<ElementDimensions> outputFieldDimensions; ///< Override output field size for cross-dimension kernels.

		/// @brief Construct a GaussKernel parameter set.
		/// @param width       Gaussian σ (default 3).
		/// @param amp         Peak amplitude (default 3).
		/// @param ampGlobal   Global inhibition constant (default -0.01).
		/// @param circular    Enable circular convolution (default true).
		/// @param normalized  Normalise the kernel (default true).
		/// @param outputDims  Override output dimensions for cross-field use (optional).
		explicit GaussKernelParameters(const double width = 3.0, const double amp = 3.0, const double ampGlobal = -0.01,
		                      const bool circular = true, const bool normalized = true,
		                      const std::optional<ElementDimensions>& outputDims = std::nullopt)
			: width(width), amplitude(amp), amplitudeGlobal(ampGlobal),
			  circular(circular), normalized(normalized),
			  outputFieldDimensions(outputDims)
		{}

		bool operator==(const GaussKernelParameters& other) const {
			constexpr double epsilon = 1e-6;

			return std::abs(width - other.width) < epsilon &&
				std::abs(amplitude - other.amplitude) < epsilon &&
				std::abs(amplitudeGlobal - other.amplitudeGlobal) < epsilon &&
				circular == other.circular &&
				normalized == other.normalized &&
				outputFieldDimensions == other.outputFieldDimensions;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2); // Ensures numbers have 2 decimal places
			result << "Parameters: ["
				<< "Width: " << width << ", "
				<< "Amplitude: " << amplitude << ", "
				<< "Amplitude global: " << amplitudeGlobal << ", "
				<< "Circular: " << (circular ? "true" : "false") << ", "
				<< "Normalized: " << (normalized ? "true" : "false");
			if (outputFieldDimensions.has_value())
				result << ", Output size: " << outputFieldDimensions->size;
			result << "]";
			return result.str();
		}
	};

	/// @brief Gaussian convolution kernel providing local excitation and optional global inhibition.
	///
	/// On each @c step(), the kernel convolves its input field's "output" component with
	/// a Gaussian of the given width and amplitude, then adds the global inhibition
	/// baseline (@c amplitudeGlobal * fullSum). The result is stored in the "output"
	/// component and fed forward to the receiving neural field.
	///
	/// @ingroup elements
	class GaussKernel final : public Kernel
	{
	private:
		GaussKernelParameters parameters;
		std::vector<double> scratchConvolution;
	public:
		/// @brief Construct a GaussKernel.
		/// @param elementCommonParameters  Name, label, and dimensions.
		/// @param parameters               Kernel-specific parameters.
		GaussKernel(const ElementCommonParameters& elementCommonParameters, GaussKernelParameters parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		/// @brief Replace the kernel parameters at runtime.
		void setParameters(const GaussKernelParameters& gk_parameters);

		/// @brief Return a copy of the current kernel parameters.
		GaussKernelParameters getParameters() const;
	};
}
