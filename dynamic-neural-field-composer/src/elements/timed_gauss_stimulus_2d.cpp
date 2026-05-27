// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/timed_gauss_stimulus_2d.h"

namespace dnf_composer::element
{
	TimedGaussStimulus2D::TimedGaussStimulus2D(const ElementCommonParameters& elementCommonParameters,
	                                           const TimedGaussStimulus2DParameters& parameters)
		: Element(elementCommonParameters), parameters(parameters)
	{
		const double x_max = elementCommonParameters.dimensionParameters.x_max;
		const double y_max = elementCommonParameters.dimensionParameters.y_max;

		if (parameters.position_x < 0 || parameters.position_x >= x_max)
			throw Exception(ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE,
				elementCommonParameters.identifiers.uniqueName);
		if (parameters.position_y < 0 || parameters.position_y >= y_max)
			throw Exception(ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE,
				elementCommonParameters.identifiers.uniqueName);

		commonParameters.identifiers.label = ElementLabel::TIMED_GAUSS_STIMULUS_2D;
	}

	void TimedGaussStimulus2D::init()
	{
		const int    size_x = commonParameters.dimensionParameters.size_x;
		const int    size_y = commonParameters.dimensionParameters.size_y;
		const double x_max  = commonParameters.dimensionParameters.x_max;
		const double y_max  = commonParameters.dimensionParameters.y_max;

		stimulusPattern.resize(size_x * size_y);

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
				stimulusPattern[yi * size_x + xi] = val;
				sum += val;
			}
		}

		if (!parameters.normalized)
		{
			for (auto& v : stimulusPattern)
				v *= parameters.amplitude;
		}
		else
		{
			if (sum > 1e-12)
			{
				for (auto& v : stimulusPattern)
					v = parameters.amplitude * v / sum;
			}
			else
			{
				const std::string message = "Tried to normalize TimedGaussStimulus2D '"
					+ getUniqueName() + "' but sum of Gaussian is zero.";
				log(tools::logger::LogLevel::ERROR, message);
			}
		}

		std::ranges::fill(components["output"], 0.0);
		std::ranges::fill(components["input"],  0.0);
	}

	void TimedGaussStimulus2D::step(double t, double deltaT)
	{
		const bool isOn = std::ranges::any_of(parameters.onTimes, [t](const auto& interval) {
			return t >= interval.first && t <= interval.second;
		});

		if (isOn)
			components["output"] = stimulusPattern;
		else
			std::ranges::fill(components["output"], 0.0);
	}

	std::string TimedGaussStimulus2D::toString() const
	{
		std::string result = "Timed Gaussian stimulus 2D element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> TimedGaussStimulus2D::clone() const
	{
		return std::make_shared<TimedGaussStimulus2D>(*this);
	}

	void TimedGaussStimulus2D::setParameters(const TimedGaussStimulus2DParameters& p)
	{
		const double x_max = commonParameters.dimensionParameters.x_max;
		const double y_max = commonParameters.dimensionParameters.y_max;
		if (p.position_x < 0 || p.position_x >= x_max)
			throw Exception(ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE,
				commonParameters.identifiers.uniqueName);
		if (p.position_y < 0 || p.position_y >= y_max)
			throw Exception(ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE,
				commonParameters.identifiers.uniqueName);
		parameters = p;
		init();
	}

	TimedGaussStimulus2DParameters TimedGaussStimulus2D::getParameters() const
	{
		return parameters;
	}
}
