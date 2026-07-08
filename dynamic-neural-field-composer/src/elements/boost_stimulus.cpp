#include <utility>

#include "elements/boost_stimulus.h"


	namespace dnf_composer::element
	{
		BoostStimulus::BoostStimulus(const ElementCommonParameters& elementCommonParameters,
			BoostStimulusParameters  parameters)
			: Element(elementCommonParameters), parameters(std::move(parameters))
		{
			this->commonParameters.identifiers.label = ElementLabel::BOOST_STIMULUS;
		}

		void BoostStimulus::init()
		{
			const double value = parameters.isActive ? parameters.amplitude : 0.0;
			std::fill(components["output"].begin(), components["output"].end(), value);
		}

		void BoostStimulus::step(double t, double deltaT)
		{
			const double value = parameters.isActive ? parameters.amplitude : 0.0;
			std::fill(components["output"].begin(), components["output"].end(), value);
		}

		std::string BoostStimulus::toString() const
		{
			std::string result = "Boost stimulus element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> BoostStimulus::clone() const
		{
			auto cloned = std::make_shared<BoostStimulus>(*this);
			return cloned;
		}

		void BoostStimulus::setParameters(const BoostStimulusParameters& boostStimulusParameters)
		{
			parameters = boostStimulusParameters;
			init();
		}

		BoostStimulusParameters BoostStimulus::getParameters() const
		{
			return parameters;
		}
	}
