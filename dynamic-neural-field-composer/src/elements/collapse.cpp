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
			// The 1D output size is fully determined by the kept axis of the 2D input.
			// A mismatch is a misconfiguration (wrong output dims for the connected
			// source); fail loudly here rather than silently truncating/zero-padding
			// the marginal in step().
			const bool keepX = parameters.keepAxis == ProjectionAxis::X;
			const int keptAxisSize = keepX ? parameters.inputDimensions.size_x
			                                : parameters.inputDimensions.size_y;
			if (commonParameters.dimensionParameters.size != keptAxisSize)
			{
				log(tools::logger::LogLevel::ERROR, "Collapse '" + this->getUniqueName()
					+ "': output size (" + std::to_string(commonParameters.dimensionParameters.size)
					+ ") must equal the kept " + ProjectionAxisToString.at(parameters.keepAxis)
					+ "-axis size (" + std::to_string(keptAxisSize) + ").");
				throw Exception(ErrorCode::ELEM_INVALID_SIZE, this->getUniqueName());
			}

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
			// element's fixed-size "output" component without reallocating it — keeping
			// the output size (and any downstream cache) stable. init()/addInput()
			// guarantee out.size() == kept-axis size, so this copy is exact; the bounded
			// copy is defensive against any residual scratch-size discrepancy.
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

			// Collapse reduces a 2D field; a 1D source has no axis to collapse.
			// Reject anything that is not 2D so the semantics stay well-defined.
			const auto& srcDims = inputElement->getElementCommonParameters().dimensionParameters;
			if (srcDims.dimensionality != 2)
			{
				log(tools::logger::LogLevel::ERROR, "Collapse '" + this->getUniqueName()
					+ "': input must be a 2D element (got dimensionality "
					+ std::to_string(srcDims.dimensionality) + ").");
				return;
			}

			// The source's kept-axis size must equal this element's declared 1D output
			// size, otherwise the reduction would not fit the output. Reject the
			// connection rather than producing a truncated/zero-padded marginal.
			const bool keepX = parameters.keepAxis == ProjectionAxis::X;
			const int keptAxisSize = keepX ? srcDims.size_x : srcDims.size_y;
			if (commonParameters.dimensionParameters.size != keptAxisSize)
			{
				log(tools::logger::LogLevel::ERROR, "Collapse '" + this->getUniqueName()
					+ "': cannot connect input; output size ("
					+ std::to_string(commonParameters.dimensionParameters.size)
					+ ") must equal the source's kept " + ProjectionAxisToString.at(parameters.keepAxis)
					+ "-axis size (" + std::to_string(keptAxisSize) + ").");
				return;
			}

			// Size the input buffer to the source's output size before delegating, so the
			// base size check passes and the input cache is invalidated correctly.
			parameters.inputDimensions = srcDims;
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
