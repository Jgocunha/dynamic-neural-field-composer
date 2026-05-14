#pragma once

#include <vector>
#include <utility>
#include <sstream>
#include <iomanip>

#include "tools/math.h"
#include "element.h"

namespace dnf_composer::element
{
	struct TimedGaussStimulusParameters final : ElementSpecificParameters
	{
		double width;
		double amplitude;
		double position;
		std::vector<std::pair<double, double>> onTimes;
		bool circular;
		bool normalized;

		explicit TimedGaussStimulusParameters(double width = 5.0, double amplitude = 15.0,
		                                      double position = 50.0,
		                                      std::vector<std::pair<double, double>> onTimes = {},
		                                      bool circular = true, bool normalized = false)
			: width(width), amplitude(amplitude), position(position),
			  onTimes(std::move(onTimes)), circular(circular), normalized(normalized)
		{}

		bool operator==(const TimedGaussStimulusParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(width - other.width) < epsilon &&
			       std::abs(amplitude - other.amplitude) < epsilon &&
			       std::abs(position - other.position) < epsilon &&
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
				<< "Position: " << position << ", "
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
	 * @brief Gaussian stimulus that is active only during specified time intervals.
	 *
	 * During active intervals the output equals the pre-computed Gaussian pattern.
	 * Outside intervals the output is zero. Multiple non-overlapping [tStart, tEnd]
	 * windows are supported.
	 *
	 * @param elementCommonParameters  Common parameters (id, dimensions).
	 * @param parameters               Stimulus parameters including on-time windows.
	 */
	class TimedGaussStimulus final : public Element
	{
	private:
		TimedGaussStimulusParameters parameters;
		std::vector<double> stimulusPattern;
	public:
		TimedGaussStimulus(const ElementCommonParameters& elementCommonParameters,
		                   const TimedGaussStimulusParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		[[nodiscard]] std::string toString() const override;
		[[nodiscard]] std::shared_ptr<Element> clone() const override;

		void setParameters(const TimedGaussStimulusParameters& parameters);
		[[nodiscard]] TimedGaussStimulusParameters getParameters() const;
	};
}
