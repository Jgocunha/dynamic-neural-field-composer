
#pragma once

#include "tools/math.h"
#include "element.h"

namespace dnf_composer::element
{
	/// @brief Parameters for an additive normal-noise input.
	/// @ingroup elements
	struct NormalNoiseParameters final : ElementSpecificParameters
	{
		double amplitude; ///< Standard deviation of the zero-mean Gaussian noise drawn each step.

		/// @brief Construct NormalNoise parameters.
		/// @param amp  Noise amplitude / standard deviation (default 0.2).
		explicit NormalNoiseParameters(const double amp = 0.2) : amplitude(amp) {}

		bool operator==(const NormalNoiseParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(amplitude - other.amplitude) < epsilon;
		}
		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << "Parameters: [Amplitude: " << std::fixed << std::setprecision(2) << amplitude << "]";
			return result.str();
		}
	};

	/// @brief Additive Gaussian noise source for neural fields.
	///
	/// Each @c step() draws independent zero-mean normal samples with the configured
	/// amplitude (standard deviation) and stores them as the "output" component.
	/// Adding this element to a neural field breaks spatial symmetry and drives
	/// stochastic state transitions.
	///
	/// @ingroup elements
	class NormalNoise final : public Element
	{
	private:
		NormalNoiseParameters parameters;
	public:
		/// @brief Construct a NormalNoise element.
		/// @param elementCommonParameters  Name, label, and spatial dimensions.
		/// @param parameters               Noise-specific parameters.
		NormalNoise(const ElementCommonParameters& elementCommonParameters,
		            NormalNoiseParameters parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::shared_ptr<Element> clone() const override;
		std::string toString() const override;

		void setParameters(NormalNoiseParameters parameters);
		NormalNoiseParameters getParameters() const;
	};
}
