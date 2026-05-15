#include "elements/resize.h"
#include "tools/math.h"

namespace dnf_composer::element
{
	ResizeParameters::ResizeParameters(int outputSize, double outputStep, ResizeInterpolation interpolation)
		: outputSize(outputSize), outputStep(outputStep), interpolation(interpolation)
	{
	}

	std::string ResizeParameters::toString() const
	{
		const char* interpStr = interpolation == ResizeInterpolation::NEAREST ? "Nearest" :
		                        interpolation == ResizeInterpolation::CUBIC    ? "Cubic"   : "Linear";
		return "Output size: " + std::to_string(outputSize) +
		       "\nOutput step: " + std::to_string(outputStep) +
		       "\nInterpolation: " + interpStr;
	}

	Resize::Resize(const ElementCommonParameters& common, ResizeParameters params)
		: Element(common), parameters(std::move(params))
	{
		commonParameters.identifiers.label = ElementLabel::RESIZE;
	}

	void Resize::init()
	{
		components["output"].assign(parameters.outputSize, 0.0);
		std::ranges::fill(components["input"], 0.0);
	}

	void Resize::step(double t, double deltaT)
	{
		updateInput();
		switch (parameters.interpolation)
		{
		case ResizeInterpolation::NEAREST:
			tools::math::resampleNearestInto(components["input"], components["output"]);
			break;
		case ResizeInterpolation::CUBIC:
			tools::math::resampleCubicInto(components["input"], components["output"]);
			break;
		case ResizeInterpolation::LINEAR:
		default:
			tools::math::resampleInto(components["input"], components["output"]);
			break;
		}
	}

	void Resize::setParameters(const ResizeParameters& params)
	{
		parameters = params;
		init();
	}

	ResizeParameters Resize::getParameters() const
	{
		return parameters;
	}

	std::shared_ptr<Element> Resize::clone() const
	{
		return std::make_shared<Resize>(*this);
	}

	std::string Resize::toString() const
	{
		std::string result = "Resize element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}
}
