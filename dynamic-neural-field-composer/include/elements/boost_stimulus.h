#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>

#include "element.h"

namespace dnf_composer::element
{
	struct BoostStimulusParameters final : ElementSpecificParameters
	{
		double amplitude;
		bool isActive;

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

	class BoostStimulus final : public Element
	{
	private:
		BoostStimulusParameters parameters;
	public:
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
