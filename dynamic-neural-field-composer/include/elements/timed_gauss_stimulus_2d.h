#pragma once

#include <vector>
#include <utility>
#include <sstream>
#include <iomanip>

#include "tools/math.h"
#include "element.h"

namespace dnf_composer::element
{
	struct TimedGaussStimulus2DParameters final : ElementSpecificParameters
	{
		double width;
		double amplitude;
		double position_x;
		double position_y;
		std::vector<std::pair<double, double>> onTimes;
		bool circular;
		bool normalized;

		explicit TimedGaussStimulus2DParameters(double width = 5.0, double amplitude = 15.0,
		                                        double position_x = 25.0, double position_y = 25.0,
		                                        std::vector<std::pair<double, double>> onTimes = {},
		                                        bool circular = true, bool normalized = false)
			: width(width), amplitude(amplitude),
			  position_x(position_x), position_y(position_y),
			  onTimes(std::move(onTimes)), circular(circular), normalized(normalized)
		{}

		bool operator==(const TimedGaussStimulus2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(width - other.width) < epsilon &&
			       std::abs(amplitude - other.amplitude) < epsilon &&
			       std::abs(position_x - other.position_x) < epsilon &&
			       std::abs(position_y - other.position_y) < epsilon &&
			       onTimes == other.onTimes &&
			       circular == other.circular &&
			       normalized == other.normalized;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "Width: " << width << ", "
				<< "Amplitude: " << amplitude << ", "
				<< "Position x: " << position_x << ", "
				<< "Position y: " << position_y << ", "
				<< "OnTimes: [";
			for (size_t i = 0; i < onTimes.size(); ++i)
			{
				if (i > 0) result << "; ";
				result << onTimes[i].first << "-" << onTimes[i].second;
			}
			result << "], "
				<< "Circular: " << (circular ? "true" : "false") << ", "
				<< "Normalized: " << (normalized ? "true" : "false") << "]";
			return result.str();
		}
	};

	/**
	 * @brief 2D Gaussian stimulus that is active only during specified time intervals.
	 *
	 * During active intervals the output equals the pre-computed 2D Gaussian pattern.
	 * Outside intervals the output is zero. Multiple non-overlapping [tStart, tEnd]
	 * windows are supported.
	 *
	 * @param elementCommonParameters  Common parameters (id, 2D dimensions).
	 * @param parameters               Stimulus parameters including on-time windows.
	 */
	class TimedGaussStimulus2D final : public Element
	{
	private:
		TimedGaussStimulus2DParameters parameters;
		std::vector<double> stimulusPattern;
	public:
		TimedGaussStimulus2D(const ElementCommonParameters& elementCommonParameters,
		                     const TimedGaussStimulus2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		[[nodiscard]] std::string toString() const override;
		[[nodiscard]] std::shared_ptr<Element> clone() const override;

		void setParameters(const TimedGaussStimulus2DParameters& parameters);
		[[nodiscard]] TimedGaussStimulus2DParameters getParameters() const;
	};
}
