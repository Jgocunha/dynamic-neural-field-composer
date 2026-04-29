#pragma once

#include <optional>

#include "kernel.h"
#include "tools/math.h"

namespace dnf_composer::element
{
	/// @brief Parameters for an asymmetric (shifted) Gaussian convolution kernel.
	/// @ingroup elements
	struct AsymmetricGaussKernelParameters final : ElementSpecificParameters
	{
		double width;           ///< Gaussian standard deviation σ.
		double amplitude;       ///< Peak amplitude of the Gaussian.
		double amplitudeGlobal; ///< Spatially uniform offset added after convolution.
		double timeShift;       ///< Spatial shift of the kernel centre (positive = rightward drift).
		bool circular;          ///< Enable circular (toroidal) convolution.
		bool normalized;        ///< Normalise the Gaussian before convolution.
		std::optional<ElementDimensions> outputFieldDimensions; ///< Override output field size for cross-dimension kernels.

		/// @brief Construct an AsymmetricGaussKernel parameter set.
		/// @param width         Gaussian σ (default 3).
		/// @param amp           Peak amplitude (default 3).
		/// @param ampGlobal     Global offset (default 0).
		/// @param timeShift     Spatial shift in positions (default 0 = symmetric).
		/// @param circular      Circular boundary (default true).
		/// @param normalized    Area normalisation (default true).
		/// @param outputDims    Override output dimensions for cross-field use (optional).
		explicit AsymmetricGaussKernelParameters(const double width = 3.0, const double amp = 3.0,
			const double ampGlobal = 0.00, const double timeShift = 0.00,
			const bool circular = true, const bool normalized = true,
			const std::optional<ElementDimensions>& outputDims = std::nullopt)
			: width(width), amplitude(amp), amplitudeGlobal(ampGlobal), timeShift(timeShift),
			  circular(circular), normalized(normalized), outputFieldDimensions(outputDims)
		{}

		bool operator==(const AsymmetricGaussKernelParameters& other) const {
			constexpr double epsilon = 1e-6;

			return std::abs(width - other.width) < epsilon &&
				std::abs(amplitude - other.amplitude) < epsilon &&
				std::abs(amplitudeGlobal - other.amplitudeGlobal) < epsilon &&
				std::abs(timeShift - other.timeShift) < epsilon &&
				circular == other.circular &&
				normalized == other.normalized &&
				outputFieldDimensions == other.outputFieldDimensions;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "Width: " << width << ", "
				<< "Amplitude: " << amplitude << ", "
				<< "Amplitude global: " << amplitudeGlobal << ", "
				<< "Time shift: " << timeShift << ", "
				<< "Circular: " << (circular ? "true" : "false") << ", "
				<< "Normalized: " << (normalized ? "true" : "false");
			if (outputFieldDimensions.has_value())
				result << ", Output size: " << outputFieldDimensions->size;
			result << "]";
			return result.str();
		}
	};

	/// @brief Spatially shifted Gaussian kernel that induces directional peak drift.
	///
	/// By offsetting the centre of the self-excitation Gaussian by @c timeShift
	/// positions, the kernel breaks spatial symmetry: a localized activation peak
	/// will be pulled toward higher (positive shift) or lower (negative shift)
	/// spatial positions each time step, producing smooth motion across the field.
	///
	/// When @c AsymmetricGaussKernelParameters::outputFieldDimensions is set, the
	/// convolution result is resampled to the specified output size, enabling
	/// connections between fields of different spatial dimensions.
	///
	/// @ingroup elements
	class AsymmetricGaussKernel final : public Kernel
	{
	private:
		AsymmetricGaussKernelParameters parameters;
		std::vector<double> gauss;
		std::vector<double> gaussDerivative;
		std::vector<double> scratchConvolution;
	public:
		/// @brief Construct an AsymmetricGaussKernel.
		/// @param elementCommonParameters  Name, label, and spatial dimensions.
		/// @param agk_parameters           Kernel-specific parameters.
		AsymmetricGaussKernel(const ElementCommonParameters& elementCommonParameters,
			AsymmetricGaussKernelParameters agk_parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const AsymmetricGaussKernelParameters& gk_parameters);
		AsymmetricGaussKernelParameters getParameters() const;
	};
}
