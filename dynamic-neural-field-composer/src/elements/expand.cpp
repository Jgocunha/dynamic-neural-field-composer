#include "elements/expand.h"

namespace dnf_composer
{
	namespace element
	{
		Expand::Expand(const ElementCommonParameters& elementCommonParameters,
			const ExpandParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::EXPAND;
			components["input"] = std::vector<double>(parameters.inputDimensions.size);
			components["output"] = std::vector<double>(commonParameters.dimensionParameters.size);
		}

		void Expand::init()
		{
			components["input"].assign(parameters.inputDimensions.size, 0.0);
			components["output"].assign(commonParameters.dimensionParameters.size, 0.0);
		}

		void Expand::step(double t, double deltaT)
		{
			updateInput();

			const auto& in = components["input"];
			auto& out = components["output"];

			const bool alongX = parameters.broadcastProfileAxis == ProjectionAxis::X;
			tools::math::broadcast1DTo2D_into(out, in,
				commonParameters.dimensionParameters.size_x,
				commonParameters.dimensionParameters.size_y,
				alongX);
		}

		void Expand::addInput(const std::shared_ptr<Element>& inputElement,
			const std::string& inputComponent)
		{
			if (!inputElement)
			{
				log(tools::logger::LogLevel::ERROR, "Input is null.");
				return;
			}

			// Expand accepts exactly one input: it broadcasts a single 1D source field.
			// Allowing a second (possibly larger) input would let updateInput() write
			// past the resized "input" buffer. Reject any additional input.
			if (!inputs.empty())
			{
				log(tools::logger::LogLevel::ERROR, "Expand '" + this->getUniqueName()
					+ "' already has an input; only one input is allowed.");
				return;
			}

			// Size the input buffer to the source's output size before delegating, so the
			// base size check passes and the input cache is invalidated correctly.
			parameters.inputDimensions = inputElement->getElementCommonParameters().dimensionParameters;
			components["input"].assign(inputElement->getComponentPtr("output")->size(), 0.0);

			Element::addInput(inputElement, inputComponent);
		}

		std::string Expand::toString() const
		{
			std::string result = "Expand element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> Expand::clone() const
		{
			return std::make_shared<Expand>(*this);
		}

		void Expand::changeInputDimensions(const ElementDimensions& newInputDimensions)
		{
			parameters.inputDimensions = newInputDimensions;
			components["input"].assign(newInputDimensions.size, 0.0);
			init();
		}

		void Expand::setParameters(const ExpandParameters& p)
		{
			parameters = p;
			init();
		}

		ExpandParameters Expand::getParameters() const
		{
			return parameters;
		}
	}
}
