// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/boost_stimulus_2d.h"

namespace dnf_composer::element
{
	BoostStimulus2D::BoostStimulus2D(const ElementCommonParameters& elementCommonParameters,
	                                 const BoostStimulus2DParameters& parameters)
		: Element(elementCommonParameters), parameters(parameters)
	{
		commonParameters.identifiers.label = ElementLabel::BOOST_STIMULUS_2D;
	}

	void BoostStimulus2D::init()
	{
		const double value = parameters.isActive ? parameters.amplitude : 0.0;
		std::ranges::fill(components["output"], value);
	}

	void BoostStimulus2D::step(double t, double deltaT)
	{
		const double value = parameters.isActive ? parameters.amplitude : 0.0;
		std::ranges::fill(components["output"], value);
	}

	std::string BoostStimulus2D::toString() const
	{
		std::string result = "Boost stimulus 2D element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> BoostStimulus2D::clone() const
	{
		return std::make_shared<BoostStimulus2D>(*this);
	}

	void BoostStimulus2D::setParameters(const BoostStimulus2DParameters& p)
	{
		parameters = p;
		init();
	}

	BoostStimulus2DParameters BoostStimulus2D::getParameters() const
	{
		return parameters;
	}
}
