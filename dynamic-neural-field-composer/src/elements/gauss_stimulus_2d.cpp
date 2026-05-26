// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/gauss_stimulus_2d.h"

namespace dnf_composer
{
	namespace element
	{
		GaussStimulus2D::GaussStimulus2D(const ElementCommonParameters& elementCommonParameters,
			const GaussStimulus2DParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			if (parameters.position_x < 0 || parameters.position_x >= elementCommonParameters.dimensionParameters.x_max)
				throw Exception(ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE,
					elementCommonParameters.identifiers.uniqueName);
			if (parameters.position_y < 0 || parameters.position_y >= elementCommonParameters.dimensionParameters.y_max)
				throw Exception(ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE,
					elementCommonParameters.identifiers.uniqueName);

			commonParameters.identifiers.label = ElementLabel::GAUSS_STIMULUS_2D;
		}

		void GaussStimulus2D::init()
		{
			const int size_x = commonParameters.dimensionParameters.size_x;
			const int size_y = commonParameters.dimensionParameters.size_y;
			const double x_max = commonParameters.dimensionParameters.x_max;
			const double y_max = commonParameters.dimensionParameters.y_max;

			double sum = 0.0;
			for (int xi = 0; xi < size_x; ++xi)
			{
				const double x = (xi + 1) * commonParameters.dimensionParameters.d_x;
				for (int yi = 0; yi < size_y; ++yi)
				{
					const double y = (yi + 1) * commonParameters.dimensionParameters.d_y;
					double val;
					if (parameters.circular)
						val = tools::math::gaussian_2d_periodic(x, y,
							parameters.position_x, parameters.position_y,
							parameters.width, 1.0, x_max, y_max);
					else
						val = tools::math::gaussian_2d(x, y,
							parameters.position_x, parameters.position_y,
							parameters.width, parameters.width, 1.0);
					components["output"][xi * size_y + yi] = val;
					sum += val;
				}
			}

			if (!parameters.normalized)
			{
				for (auto& v : components["output"])
					v *= parameters.amplitude;
			}
			else
			{
				if (sum > 1e-12)
				{
					for (auto& v : components["output"])
						v = parameters.amplitude * v / sum;
				}
				else
				{
					const std::string message = "Tried to initialize a normalized Gaussian stimulus 2D '"
						+ getUniqueName() + "'. With the sum of the output vector equal "
						"to zero that is impossible! ";
					log(tools::logger::LogLevel::ERROR, message);
				}
			}

			std::ranges::fill(components["input"], 0.0);
			updateInput();
			const int totalSize = size_x * size_y;
			for (int i = 0; i < totalSize; ++i)
				components["output"][i] += components["input"][i];
		}

		void GaussStimulus2D::step(double t, double deltaT)
		{
		}

		std::string GaussStimulus2D::toString() const
		{
			std::string result = "Gauss stimulus 2D element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> GaussStimulus2D::clone() const
		{
			return std::make_shared<GaussStimulus2D>(*this);
		}

		void GaussStimulus2D::setParameters(const GaussStimulus2DParameters& p)
		{
			parameters = p;
			init();
		}

		GaussStimulus2DParameters GaussStimulus2D::getParameters() const
		{
			return parameters;
		}
	}
}
