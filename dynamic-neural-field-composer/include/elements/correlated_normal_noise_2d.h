#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>
#include <numeric>

#include "tools/math.h"
#include "element.h"

namespace dnf_composer::element
{
	struct CorrelatedNormalNoise2DParameters final : ElementSpecificParameters
	{
		double amplitude;
		double width;
		bool circular;

		explicit CorrelatedNormalNoise2DParameters(double amplitude = 0.05,
		                                           double width = 1.0,
		                                           bool circular = true)
			: amplitude(amplitude), width(width), circular(circular)
		{}

		bool operator==(const CorrelatedNormalNoise2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(amplitude - other.amplitude) < epsilon &&
			       std::abs(width - other.width) < epsilon &&
			       circular == other.circular;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << "Parameters: ["
			       << "Amplitude: " << std::fixed << std::setprecision(4) << amplitude << ", "
			       << "Width: " << std::fixed << std::setprecision(2) << width << ", "
			       << "Circular: " << (circular ? "true" : "false") << "]";
			return result.str();
		}
	};

	class CorrelatedNormalNoise2D final : public Element
	{
	private:
		CorrelatedNormalNoise2DParameters parameters;
		std::vector<double> correlationKernel_x;
		std::vector<double> correlationKernel_y;
		std::vector<int>    extIndex_x;
		std::vector<int>    extIndex_y;
		std::vector<double> scratchTmp_;
		std::vector<double> scratchConv_;
	public:
		CorrelatedNormalNoise2D(const ElementCommonParameters& elementCommonParameters,
		                        const CorrelatedNormalNoise2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const CorrelatedNormalNoise2DParameters& parameters);
		CorrelatedNormalNoise2DParameters getParameters() const;
	};
}
