#pragma once




#include "tools/math.h"
#include "element.h"

namespace dnf_composer::element
{
	/// @brief Parameters for a spatially correlated Gaussian noise source.
	///
	/// Implements the spatially correlated noise available in cedar's NeuralField.
	/// White Gaussian noise is convolved with a Gaussian kernel of the given @c width
	/// to produce spatially smooth noise, then scaled by @f$ \text{amplitude} / \sqrt{\Delta t} @f$.
	///
	/// **Cross-framework equivalence:**
	/// | Framework    | Equivalent feature                                              |
	/// |--------------|------------------------------------------------------------------|
	/// | dnf-composer | `CorrelatedNormalNoise(amplitude, width)` element               |
	/// | cedar        | NeuralField `NoiseCorrelationKernel` (amplitude + sigma)        |
	/// | cosivina     | no built-in equivalent                                          |
	///
	/// @note For uncorrelated noise (width → 0), use NormalNoise instead.
	/// @ingroup elements
	struct CorrelatedNormalNoiseParameters final : ElementSpecificParameters
	{
		double amplitude; ///< Noise amplitude (standard deviation before convolution). Default 0.05.
		double width;     ///< Standard deviation of the Gaussian correlation kernel (in samples/indices). Default 1.0.
		bool circular;    ///< Use circular (toroidal) boundary for convolution. Default true.

		/// @brief Construct CorrelatedNormalNoise parameters.
		/// @param amplitude  Noise amplitude. Default 0.05.
		/// @param width      Spatial correlation width (sigma of the Gaussian kernel). Default 1.0.
		/// @param circular   Circular boundary for convolution. Default true.
		explicit CorrelatedNormalNoiseParameters(double amplitude = 0.05,
		                                         double width = 1.0,
		                                         bool circular = true)
			: amplitude(amplitude), width(width), circular(circular)
		{}

		bool operator==(const CorrelatedNormalNoiseParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(amplitude - other.amplitude) < epsilon &&
			       std::abs(width - other.width) < epsilon &&
			       circular == other.circular;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << "Parameters: ["
			       << "Amplitude: " << std::fixed << std::setprecision(4) << amplitude << ", "
			       << "Width: " << std::fixed << std::setprecision(2) << width << ", "
			       << "Circular: " << (circular ? "true" : "false") << "]";
			return result.str();
		}
	};

	/// @brief Spatially correlated Gaussian noise source — cedar-compatible.
	///
	/// Each @c step() draws white Gaussian noise, convolves it with a normalised
	/// Gaussian of the configured @c width, and scales by
	/// @f$ \text{amplitude} / \sqrt{\Delta t} @f$ to maintain correct stochastic
	/// scaling regardless of integration step size.
	///
	/// Connect the output of this element to a NeuralField as you would NormalNoise.
	///
	/// @ingroup elements
	class CorrelatedNormalNoise final : public Element
	{
	private:
		CorrelatedNormalNoiseParameters parameters;
		std::vector<double> correlationKernel; ///< Precomputed normalised Gaussian kernel.
		std::vector<int> extIndex;             ///< Extended index for circular convolution.
		std::vector<double> whiteNoise_;       ///< Reusable white-noise buffer (avoids per-step alloc).
	public:
		/// @brief Construct a CorrelatedNormalNoise element.
		/// @param elementCommonParameters  Name, label, and spatial dimensions.
		/// @param parameters               Noise-specific parameters.
		CorrelatedNormalNoise(const ElementCommonParameters& elementCommonParameters,
		                      CorrelatedNormalNoiseParameters  parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::shared_ptr<Element> clone() const override;
		std::string toString() const override;

		void setParameters(const CorrelatedNormalNoiseParameters& parameters);
		CorrelatedNormalNoiseParameters getParameters() const;
	};
}
