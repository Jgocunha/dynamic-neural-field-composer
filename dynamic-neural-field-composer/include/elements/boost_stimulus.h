#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>

#include "element.h"

namespace dnf_composer::element
{
	/// @brief Parameters for a spatially homogeneous boost stimulus.
	/// @ingroup elements
	struct BoostStimulusParameters final : ElementSpecificParameters
	{
		double amplitude; ///< Uniform input value applied across the entire field.
		bool isActive;    ///< If false, the stimulus outputs zero regardless of amplitude.

		/// @brief Construct a BoostStimulus parameter set.
		/// @param amplitude  Homogeneous input amplitude (default 5.0).
		/// @param isActive   Whether the stimulus is currently active (default true).
		explicit BoostStimulusParameters(const double amplitude = 5.0, const bool isActive = true)
			: amplitude(amplitude), isActive(isActive)
		{}

		bool operator==(const BoostStimulusParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(amplitude - other.amplitude) < epsilon &&
				isActive == other.isActive;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "Amplitude: " << amplitude << ", "
				<< "IsActive: " << (isActive ? "true" : "false")
				<< "]";
			return result.str();
		}
	};

	/// @brief Spatially uniform (homogeneous) input stimulus.
	///
	/// Adds a constant value to every point of the receiving neural field, effectively
	/// shifting the field's resting level. A non-zero boost can push a sub-threshold
	/// field into the detection regime, making it sensitive to weak localized inputs
	/// that would otherwise be ignored.
	///
	/// @ingroup elements
	class BoostStimulus final : public Element
	{
	private:
		BoostStimulusParameters parameters;
	public:
		/// @brief Construct a BoostStimulus.
		/// @param elementCommonParameters  Name, label, and spatial dimensions.
		/// @param parameters               Stimulus-specific parameters.
		BoostStimulus(const ElementCommonParameters& elementCommonParameters,
		              const BoostStimulusParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const BoostStimulusParameters& parameters);
		BoostStimulusParameters getParameters() const;
	};
}
