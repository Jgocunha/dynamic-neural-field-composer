#include "elements/resize_2d.h"

namespace dnf_composer
{
	namespace element
	{
		namespace
		{
			// Resample a 1D buffer according to the selected interpolation method.
			void resample1d(const std::vector<double>& in, std::vector<double>& out,
				InterpolationMethod method)
			{
				switch (method)
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
		}

		Resize2D::Resize2D(const ElementCommonParameters& elementCommonParameters,
			const Resize2DParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::RESIZE_2D;
			components["input"] = std::vector<double>(parameters.inputDimensions.size);
			components["output"] = std::vector<double>(commonParameters.dimensionParameters.size);
		}

		void Resize2D::init()
		{
			components["input"].assign(parameters.inputDimensions.size, 0.0);
			components["output"].assign(commonParameters.dimensionParameters.size, 0.0);
		}

		void Resize2D::step(double t, double deltaT)
		{
			updateInput();

			const auto& in = components["input"];
			auto& out = components["output"];

			const int inSizeX = parameters.inputDimensions.size_x;
			const int inSizeY = parameters.inputDimensions.size_y;
			const int outSizeX = commonParameters.dimensionParameters.size_x;
			const int outSizeY = commonParameters.dimensionParameters.size_y;

			// Pass 1: resample each row along x (inSizeX -> outSizeX) into scratch (outSizeX x inSizeY).
			scratch.assign(static_cast<std::size_t>(outSizeX) * inSizeY, 0.0);
			std::vector<double> rowIn(inSizeX), rowOut(outSizeX);
			for (int y = 0; y < inSizeY; ++y)
			{
				for (int x = 0; x < inSizeX; ++x)
					rowIn[x] = in[static_cast<std::size_t>(y) * inSizeX + x];
				resample1d(rowIn, rowOut, parameters.method);
				for (int x = 0; x < outSizeX; ++x)
					scratch[static_cast<std::size_t>(y) * outSizeX + x] = rowOut[x];
			}

			// Pass 2: resample each column along y (inSizeY -> outSizeY) into out (outSizeX x outSizeY).
			std::vector<double> colIn(inSizeY), colOut(outSizeY);
			for (int x = 0; x < outSizeX; ++x)
			{
				for (int y = 0; y < inSizeY; ++y)
					colIn[y] = scratch[static_cast<std::size_t>(y) * outSizeX + x];
				resample1d(colIn, colOut, parameters.method);
				for (int y = 0; y < outSizeY; ++y)
					out[static_cast<std::size_t>(y) * outSizeX + x] = colOut[y];
			}
		}

		void Resize2D::addInput(const std::shared_ptr<Element>& inputElement,
			const std::string& inputComponent)
		{
			if (!inputElement)
			{
				log(tools::logger::LogLevel::ERROR, "Input is null.");
				return;
			}

			// Size the input buffer to the source's output size before delegating, so the
			// base size check passes and the input cache is invalidated correctly.
			parameters.inputDimensions = inputElement->getElementCommonParameters().dimensionParameters;
			components["input"].assign(inputElement->getComponentPtr("output")->size(), 0.0);

			Element::addInput(inputElement, inputComponent);
		}

		std::string Resize2D::toString() const
		{
			std::string result = "Resize 2D element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> Resize2D::clone() const
		{
			return std::make_shared<Resize2D>(*this);
		}

		void Resize2D::changeInputDimensions(const ElementDimensions& newInputDimensions)
		{
			parameters.inputDimensions = newInputDimensions;
			components["input"].assign(newInputDimensions.size, 0.0);
			init();
		}

		void Resize2D::setParameters(const Resize2DParameters& p)
		{
			parameters = p;
			init();
		}

		Resize2DParameters Resize2D::getParameters() const
		{
			return parameters;
		}
	}
}
