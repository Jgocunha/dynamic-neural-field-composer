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
			// The 1D profile lies along the chosen axis of the 2D output, so its size
			// must equal that axis's size. A mismatch is a misconfiguration; fail loudly
			// here rather than silently clamping/repeating a partial profile in step().
			const bool alongX = parameters.broadcastProfileAxis == ProjectionAxis::X;
			const int profileAxisSize = alongX ? commonParameters.dimensionParameters.size_x
			                                   : commonParameters.dimensionParameters.size_y;
			if (parameters.inputDimensions.size != profileAxisSize)
			{
				log(tools::logger::LogLevel::ERROR, "Expand '" + this->getUniqueName()
					+ "': input size (" + std::to_string(parameters.inputDimensions.size)
					+ ") must equal the output's " + ProjectionAxisToString.at(parameters.broadcastProfileAxis)
					+ "-axis size (" + std::to_string(profileAxisSize) + ").");
				throw Exception(ErrorCode::ELEM_INVALID_SIZE, this->getUniqueName());
			}

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

			// Expand broadcasts a 1D profile; a 2D source has no single profile to repeat.
			// Reject anything that is not 1D so the semantics stay well-defined.
			const auto& srcDims = inputElement->getElementCommonParameters().dimensionParameters;
			if (srcDims.dimensionality != 1)
			{
				log(tools::logger::LogLevel::ERROR, "Expand '" + this->getUniqueName()
					+ "': input must be a 1D element (got dimensionality "
					+ std::to_string(srcDims.dimensionality) + ").");
				return;
			}

			// The source size must equal the profile axis of this element's 2D output,
			// otherwise the broadcast would clamp/repeat a partial profile. Reject it.
			const bool alongX = parameters.broadcastProfileAxis == ProjectionAxis::X;
			const int profileAxisSize = alongX ? commonParameters.dimensionParameters.size_x
			                                   : commonParameters.dimensionParameters.size_y;
			if (srcDims.size != profileAxisSize)
			{
				log(tools::logger::LogLevel::ERROR, "Expand '" + this->getUniqueName()
					+ "': cannot connect input; source size (" + std::to_string(srcDims.size)
					+ ") must equal the output's " + ProjectionAxisToString.at(parameters.broadcastProfileAxis)
					+ "-axis size (" + std::to_string(profileAxisSize) + ").");
				return;
			}

			// Size the input buffer to the source's output size before delegating, so the
			// base size check passes and the input cache is invalidated correctly.
			parameters.inputDimensions = srcDims;
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
			// Sever connections before resizing the input buffer. A still-connected
			// source would leave updateInput() with a stale/dangling input cache and a
			// source larger than the resized buffer (out-of-bounds write). removeInputs()
			// also resets the cache so it is rebuilt on the next step.
			removeInputs();
			removeOutputs();
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
