// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/timed_gauss_stimulus.h"

namespace dnf_composer::element
{
	TimedGaussStimulus::TimedGaussStimulus(const ElementCommonParameters& elementCommonParameters,
	                                       const TimedGaussStimulusParameters& parameters)
		: Element(elementCommonParameters), parameters(parameters)
	{
		if (parameters.position < 0 || parameters.position >= elementCommonParameters.dimensionParameters.x_max)
			throw Exception(ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE,
				elementCommonParameters.identifiers.uniqueName);

		commonParameters.identifiers.label = ElementLabel::TIMED_GAUSS_STIMULUS;
	}

	void TimedGaussStimulus::init()
	{
		const int size = commonParameters.dimensionParameters.size;
		stimulusPattern.resize(size);

		std::vector<double> g(size);
		if (parameters.circular)
			g = tools::math::circularGauss(size, parameters.width,
				parameters.position / commonParameters.dimensionParameters.d_x);
		else
			g = tools::math::gauss(size, parameters.width,
				parameters.position / commonParameters.dimensionParameters.d_x);

		if (!parameters.normalized)
		{
			for (int i = 0; i < size; ++i)
				stimulusPattern[i] = parameters.amplitude * g[i];
		}
		else
		{
			const double sum = tools::math::calculateVectorSum(g);
			if (sum != 0.0)
				for (int i = 0; i < size; ++i)
					stimulusPattern[i] = parameters.amplitude * g[i] / sum;
			else
			{
				const std::string message = "Tried to normalize TimedGaussStimulus '"
					+ getUniqueName() + "' but sum of Gaussian is zero.";
				log(tools::logger::LogLevel::ERROR, message);
			}
		}

		std::ranges::fill(components["output"], 0.0);
		std::ranges::fill(components["input"], 0.0);
	}

	void TimedGaussStimulus::step(double t, double deltaT)
	{
		const bool isOn = std::ranges::any_of(parameters.onTimes, [t](const auto& interval) {
			return t >= interval.first && t <= interval.second;
		});

		if (isOn)
			components["output"] = stimulusPattern;
		else
			std::ranges::fill(components["output"], 0.0);
	}

	std::string TimedGaussStimulus::toString() const
	{
		std::string result = "Timed Gaussian stimulus element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> TimedGaussStimulus::clone() const
	{
		return std::make_shared<TimedGaussStimulus>(*this);
	}

	void TimedGaussStimulus::setParameters(const TimedGaussStimulusParameters& p)
	{
		if (p.position < 0 || p.position >= commonParameters.dimensionParameters.x_max)
			throw Exception(ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE,
				commonParameters.identifiers.uniqueName);
		parameters = p;
		init();
	}

	TimedGaussStimulusParameters TimedGaussStimulus::getParameters() const
	{
		return parameters;
	}
}
