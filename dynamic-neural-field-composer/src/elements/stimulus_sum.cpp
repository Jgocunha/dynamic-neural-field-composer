#include <utility>

#include "elements/stimulus_sum.h"


namespace dnf_composer::element
{
	StimulusSum::StimulusSum(const ElementCommonParameters& elementCommonParameters,
		StimulusSumParameters parameters)
		: Element(elementCommonParameters), parameters(std::move(parameters))
	{
		this->commonParameters.identifiers.label = ElementLabel::STIMULUS_SUM;
	}

	void StimulusSum::init()
	{
		std::fill(components["input"].begin(),  components["input"].end(),  0.0);
		std::fill(components["output"].begin(), components["output"].end(), 0.0);
	}

	void StimulusSum::step(double t, double deltaT)
	{
		if (!hasInput())
		{
			const std::string logMessage = "StimulusSum '" + getUniqueName() +
				"' has no connected inputs; output will be zero.";
			log(tools::logger::LogLevel::WARNING, logMessage);
		}

		// Element::updateInput() already sums every connected input source
		// element-wise into components["input"] (arbitrary N, zero-fills when
		// there are none), so StimulusSum's own step() only needs to publish
		// that sum as its "output".
		updateInput();
		components["output"] = components["input"];
	}

	std::string StimulusSum::toString() const
	{
		std::string result = "Stimulus sum element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> StimulusSum::clone() const
	{
		auto cloned = std::make_shared<StimulusSum>(*this);
		return cloned;
	}

	void StimulusSum::setParameters(const StimulusSumParameters& stimulusSumParameters)
	{
		parameters = stimulusSumParameters;
	}

	StimulusSumParameters StimulusSum::getParameters() const
	{
		return parameters;
	}
}
