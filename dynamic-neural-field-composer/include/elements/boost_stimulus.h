#pragma once

#include <sstream>
#include <iomanip>

#include "element.h"

namespace dnf_composer
{
	namespace element
	{
		struct BoostStimulusParameters : ElementSpecificParameters
		{
			double amplitude;
			bool isActive;

			BoostStimulusParameters(double amplitude = 5.0, bool isActive = true)
				: amplitude(amplitude), isActive(isActive)
			{}

			bool operator==(const BoostStimulusParameters& other) const
			{
				constexpr double epsilon = 1e-6;
				return std::abs(amplitude - other.amplitude) < epsilon &&
					isActive == other.isActive;
			}

			std::string toString() const override
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

		class BoostStimulus : public Element
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
}
