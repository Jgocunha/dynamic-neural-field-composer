#include "elements/collapse.h"

namespace dnf_composer
{
	namespace element
	{
		Collapse::Collapse(const ElementCommonParameters& elementCommonParameters,
			const CollapseParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::COLLAPSE;
			components["input"] = std::vector<double>(parameters.inputDimensions.size);
			components["output"] = std::vector<double>(commonParameters.dimensionParameters.size);
		}

		void Collapse::init()
		{
			components["input"].assign(parameters.inputDimensions.size, 0.0);
			components["output"].assign(commonParameters.dimensionParameters.size, 0.0);
		}

		void Collapse::step(double t, double deltaT)
		{
			updateInput();

			const auto& in = components["input"];
			auto& out = components["output"];

			const bool keepX = parameters.keepAxis == ProjectionAxis::X;
			// Reduce into a scratch buffer (sized to the kept axis), then copy into the
			// element's fixed-size "output" component without reallocating it — so the
			// declared output size (and any downstream cache) stays stable even if it
			// does not match the kept-axis size.
			tools::math::reduce2DAxis_into(scratch, in,
				parameters.inputDimensions.size_x, parameters.inputDimensions.size_y,
				keepX, toReduceOp(parameters.compression));

			const std::size_t n = std::min(out.size(), scratch.size());
			std::fill(out.begin(), out.end(), 0.0);
			std::copy_n(scratch.begin(), n, out.begin());
		}

		void Collapse::addInput(const std::shared_ptr<Element>& inputElement,
			const std::string& inputComponent)
		{
			if (!inputElement)
			{
				log(tools::logger::LogLevel::ERROR, "Input is null.");
				return;
			}

			// Collapse accepts exactly one input: it reduces a single 2D source field.
			// Allowing a second (possibly larger) input would let updateInput() write
			// past the resized "input" buffer. Reject any additional input.
			if (!inputs.empty())
			{
				log(tools::logger::LogLevel::ERROR, "Collapse '" + this->getUniqueName()
					+ "' already has an input; only one input is allowed.");
				return;
			}

			// Size the input buffer to the source's output size before delegating, so the
			// base size check passes and the input cache is invalidated correctly.
			parameters.inputDimensions = inputElement->getElementCommonParameters().dimensionParameters;
			components["input"].assign(inputElement->getComponentPtr("output")->size(), 0.0);

			Element::addInput(inputElement, inputComponent);
		}

		std::string Collapse::toString() const
		{
			std::string result = "Collapse element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> Collapse::clone() const
		{
			return std::make_shared<Collapse>(*this);
		}

		void Collapse::changeInputDimensions(const ElementDimensions& newInputDimensions)
		{
			parameters.inputDimensions = newInputDimensions;
			components["input"].assign(newInputDimensions.size, 0.0);
			init();
		}

		void Collapse::setParameters(const CollapseParameters& p)
		{
			parameters = p;
			init();
		}

		CollapseParameters Collapse::getParameters() const
		{
			return parameters;
		}
	}
}
