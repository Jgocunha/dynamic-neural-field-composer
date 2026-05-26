#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>

#include "tools/math.h"
#include "element.h"

namespace dnf_composer::element
{
	struct GaussStimulus2DParameters final : ElementSpecificParameters
	{
		double width;         // isotropic sigma
		double amplitude;
		double position_x;
		double position_y;
		bool circular;
		bool normalized;

		explicit GaussStimulus2DParameters(double width = 5.0, double amplitude = 15.0,
		                                   double position_x = 50.0, double position_y = 50.0,
		                                   bool circular = true, bool normalized = false)
			: width(width), amplitude(amplitude),
			  position_x(position_x), position_y(position_y),
			  circular(circular), normalized(normalized)
		{}

		bool operator==(const GaussStimulus2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(width - other.width) < epsilon &&
			       std::abs(amplitude - other.amplitude) < epsilon &&
			       std::abs(position_x - other.position_x) < epsilon &&
			       std::abs(position_y - other.position_y) < epsilon &&
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
				<< "Circular: " << (circular ? "true" : "false") << ", "
				<< "Normalized: " << (normalized ? "true" : "false") << "]";
			return result.str();
		}
	};

	class GaussStimulus2D final : public Element
	{
	private:
		GaussStimulus2DParameters parameters;
	public:
		GaussStimulus2D(const ElementCommonParameters& elementCommonParameters,
		                const GaussStimulus2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const GaussStimulus2DParameters& parameters);
		GaussStimulus2DParameters getParameters() const;
	};
}
