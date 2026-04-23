#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>

#include "kernel.h"
#include "tools/math.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif


namespace dnf_composer::element
{
	/// @brief Parameters for a Mexican-hat lateral interaction kernel.
	///
	/// A Mexican-hat kernel is the difference of two Gaussians: a narrow excitatory
	/// Gaussian minus a wider inhibitory Gaussian, plus a global inhibition constant.
	/// This combination produces local self-excitation with surrounding lateral inhibition,
	/// the standard interaction profile for winner-takes-all selection fields.
	///
	/// @ingroup elements
	struct MexicanHatKernelParameters final : ElementSpecificParameters
	{
		double widthExc;          ///< σ of the excitatory Gaussian.
		double amplitudeExc;      ///< Peak amplitude of the excitatory Gaussian.
		double widthInh;          ///< σ of the inhibitory Gaussian.
		double amplitudeInh;      ///< Peak amplitude of the inhibitory Gaussian.
		double amplitudeGlobal;   ///< Spatially uniform inhibition added after convolution.
		bool circular;            ///< Enable circular (toroidal) convolution.
		bool normalized;          ///< Normalise both Gaussians before differencing.
		std::optional<ElementDimensions> outputFieldDimensions; ///< Override output size for cross-dimension use.

		/// @brief Construct a MexicanHatKernel parameter set with sensible defaults.
		/// @param widthExc       Excitatory σ (default 2.5).
		/// @param amplitudeExc   Excitatory peak (default 11).
		/// @param widthInh       Inhibitory σ (default 5).
		/// @param amplitudeInh   Inhibitory peak (default 15).
		/// @param amplitudeGlobal  Global inhibition (default -0.1).
		/// @param circular       Circular convolution (default true).
		/// @param normalized     Normalise kernels (default true).
		/// @param outputDims     Optional output dimension override.
		explicit MexicanHatKernelParameters(const double widthExc = 2.5, const double amplitudeExc = 11.0,
		                           const double widthInh = 5.0, const double amplitudeInh = 15.0,
		                           const double amplitudeGlobal = -0.1,
		                           const bool circular = true, const bool normalized = true,
		                           const std::optional<ElementDimensions>& outputDims = std::nullopt)
			: widthExc(widthExc), amplitudeExc(amplitudeExc),
			  widthInh(widthInh), amplitudeInh(amplitudeInh),
			  amplitudeGlobal(amplitudeGlobal),
			  circular(circular), normalized(normalized),
			  outputFieldDimensions(outputDims)
		{}

		bool operator==(const MexicanHatKernelParameters& other) const
		{
			constexpr double epsilon = 1e-6;

			return std::abs(widthExc - other.widthExc) < epsilon &&
				std::abs(amplitudeExc - other.amplitudeExc) < epsilon &&
				std::abs(widthInh - other.widthInh) < epsilon &&
				std::abs(amplitudeInh - other.amplitudeInh) < epsilon &&
				std::abs(amplitudeGlobal - other.amplitudeGlobal) < epsilon &&
				circular == other.circular &&
				normalized == other.normalized &&
				outputFieldDimensions == other.outputFieldDimensions;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "Width exc.: " << widthExc << ", "
				<< "Amplitude exc.: " << amplitudeExc << ", "
				<< "Width inh.: " << widthInh << ", "
				<< "Amplitude inh.: " << amplitudeInh << ", "
				<< "Amplitude glob.: " << amplitudeGlobal << ", "
				<< "Circular: " << (circular ? "true" : "false") << ", "
				<< "Normalized: " << (normalized ? "true" : "false");
			if (outputFieldDimensions.has_value())
				result << ", Output size: " << outputFieldDimensions->size;
			result << "]";
			return result.str();
		}
	};

	/// @brief Difference-of-Gaussians convolution kernel for DFT lateral interactions.
	///
	/// Produces local excitation and surround inhibition in one pass, making it the
	/// natural choice for selection and working-memory fields.
	///
	/// @ingroup elements
	class MexicanHatKernel final : public Kernel
	{
	private:
		MexicanHatKernelParameters parameters;
		std::vector<double> scratchConvolution;
	public:
		/// @brief Construct a MexicanHatKernel.
		/// @param elementCommonParameters  Name, label, and dimensions.
		/// @param mhk_parameters           Kernel-specific parameters.
		MexicanHatKernel(const ElementCommonParameters& elementCommonParameters,
		                 MexicanHatKernelParameters mhk_parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		/// @brief Replace the kernel parameters at runtime.
		void setParameters(const MexicanHatKernelParameters& mhk_parameters);

		/// @brief Return a copy of the current kernel parameters.
		MexicanHatKernelParameters getParameters() const;
	};
}
