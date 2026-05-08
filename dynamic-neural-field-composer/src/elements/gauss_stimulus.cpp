#include "elements/gauss_stimulus.h"

namespace dnf_composer
{
	namespace element
	{
		GaussStimulus::GaussStimulus(const ElementCommonParameters& elementCommonParameters, 
			const GaussStimulusParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			if (parameters.position < 0 || parameters.position >= elementCommonParameters.dimensionParameters.x_max)
				throw Exception(ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE, 
					elementCommonParameters.identifiers.uniqueName);

			this->commonParameters.identifiers.label = ElementLabel::GAUSS_STIMULUS;
		}

		void GaussStimulus::init()
		{
			std::vector<double> g(commonParameters.dimensionParameters.size);

			if (parameters.circular)
				g = tools::math::circularGauss(commonParameters.dimensionParameters.size, 
												parameters.width, 
												parameters.position / commonParameters.dimensionParameters.d_x);
			else
				g = tools::math::gauss(commonParameters.dimensionParameters.size, 
					parameters.width, parameters.position / commonParameters.dimensionParameters.d_x);

			if (!parameters.normalized)
				for (int i = 0; i < commonParameters.dimensionParameters.size; i++)
					components["output"][i] = parameters.amplitude * g[i];
			else
			{
				const double sum = tools::math::calculateVectorSum(g);
				if(sum != 0.0)
					for (int i = 0; i < commonParameters.dimensionParameters.size; i++)
						components["output"][i] = parameters.amplitude * g[i] / sum;
				else
				{
					const std::string message = "Tried to initialize a normalized Gaussian stimulus '"
						+ this->getUniqueName() + "'. With the sum of the output vector equal "
								"to zero that is impossible! ";
					log(tools::logger::LogLevel::ERROR, message);
				}
			}

			std::ranges::fill(components["input"], 0.0);
			updateInput();
			for (int i = 0; i < commonParameters.dimensionParameters.size; i++)
				components["output"][i] += components["input"][i];
		}

		void GaussStimulus::step(double t, double deltaT)
		{
		}

		std::string GaussStimulus::toString() const
		{
			std::string result = "Gaussian stimulus element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> GaussStimulus::clone() const
		{
			auto cloned = std::make_shared<GaussStimulus>(*this);
			return cloned;
		}

		void GaussStimulus::setParameters(const GaussStimulusParameters& gaussStimulusParameters)
		{
			parameters = gaussStimulusParameters;
			init();
		}

		GaussStimulusParameters GaussStimulus::getParameters() const
		{
			return parameters;
		}
	}
}