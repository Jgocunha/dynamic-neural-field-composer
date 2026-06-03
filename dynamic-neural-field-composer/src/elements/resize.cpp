#include "elements/resize.h"

namespace dnf_composer
{
	namespace element
	{
		Resize::Resize(const ElementCommonParameters& elementCommonParameters,
			const ResizeParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::RESIZE;
			components["input"] = std::vector<double>(parameters.inputDimensions.size);
			components["output"] = std::vector<double>(commonParameters.dimensionParameters.size);
		}

		void Resize::init()
		{
			components["input"].assign(parameters.inputDimensions.size, 0.0);
			components["output"].assign(commonParameters.dimensionParameters.size, 0.0);
		}

		void Resize::step(double t, double deltaT)
		{
			updateInput();

			const auto& in = components["input"];
			auto& out = components["output"];

			switch (parameters.method)
			{
			case InterpolationMethod::LINEAR:
				tools::math::resampleInto(in, out);
				break;
			case InterpolationMethod::NEAREST:
				tools::math::resampleNearestInto(in, out);
				break;
			case InterpolationMethod::CUBIC:
				tools::math::resampleCubicInto(in, out);
				break;
			}
		}

		void Resize::addInput(const std::shared_ptr<Element>& inputElement,
			const std::string& inputComponent)
		{
			if (!inputElement)
			{
				log(tools::logger::LogLevel::ERROR, "Input is null.");
				return;
			}

			// Size the input buffer to the source's output size (N may differ from the
			// output size M) before delegating, so the base size check passes and the
			// input cache is invalidated correctly.
			parameters.inputDimensions = inputElement->getElementCommonParameters().dimensionParameters;
			components["input"].assign(inputElement->getComponentPtr("output")->size(), 0.0);

			Element::addInput(inputElement, inputComponent);
		}

		std::string Resize::toString() const
		{
			std::string result = "Resize element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> Resize::clone() const
		{
			return std::make_shared<Resize>(*this);
		}

		void Resize::changeInputDimensions(const ElementDimensions& newInputDimensions)
		{
			parameters.inputDimensions = newInputDimensions;
			components["input"].assign(newInputDimensions.size, 0.0);
			init();
		}

		void Resize::setParameters(const ResizeParameters& p)
		{
			parameters = p;
			init();
		}

		ResizeParameters Resize::getParameters() const
		{
			return parameters;
		}
	}
}
