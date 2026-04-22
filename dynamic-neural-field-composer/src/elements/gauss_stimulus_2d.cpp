// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/gauss_stimulus_2d.h"

namespace dnf_composer
{
	namespace element
	{
		GaussStimulus2D::GaussStimulus2D(const ElementCommonParameters& elementCommonParameters,
			const GaussStimulusParameters2D& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
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

			if (parameters.normalized && sum > 1e-12)
			{
				for (auto& v : components["output"])
					v = parameters.amplitude * v / sum;
			}
			else
			{
				for (auto& v : components["output"])
					v *= parameters.amplitude;
			}
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

		void GaussStimulus2D::setParameters(const GaussStimulusParameters2D& p)
		{
			parameters = p;
			init();
		}

		GaussStimulusParameters2D GaussStimulus2D::getParameters() const
		{
			return parameters;
		}
	}
}
