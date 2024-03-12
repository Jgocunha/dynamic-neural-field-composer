// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/normal_noise.h"

namespace dnf_composer
{
	namespace element
	{
		NormalNoise::NormalNoise(const ElementCommonParameters& elementCommonParameters, NormalNoiseParameters parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			 commonParameters.identifiers.label = ElementLabel::NORMAL_NOISE;
		}

		void NormalNoise::init()
		{
			std::ranges::fill(components["output"], 0.0);
		}

		void NormalNoise::step(double t, double deltaT)
		{
			const std::vector<double> rand = tools::math::generateNormalVector(commonParameters.dimensionParameters.size);

			for (int i = 0; i < commonParameters.dimensionParameters.size; i++)
				components["output"][i] = parameters.amplitude / sqrt(deltaT) * rand[i];
		}

		void NormalNoise::printParameters()
		{
			printCommonParameters();

			std::ostringstream logStream;

			logStream << "Logging specific element parameters" << std::endl;
			logStream << "Amplitude: " << parameters.amplitude << std::endl;

			log(tools::logger::LogLevel::INFO, logStream.str());
		}

		std::shared_ptr<Element> NormalNoise::clone() const
		{
			auto cloned = std::make_shared<NormalNoise>(*this);
			// If there are deep copy specifics that the copy constructor doesn't handle, do them here.
			return cloned;
		}

		void NormalNoise::setParameters(NormalNoiseParameters normalNoiseParameters)
		{
			parameters = normalNoiseParameters;
		}

		NormalNoiseParameters NormalNoise::getParameters() const
		{
			return parameters;
		}
	}
}