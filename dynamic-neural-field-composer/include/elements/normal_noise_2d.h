#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>

#include "tools/math.h"
#include "element.h"

namespace dnf_composer::element
{
	struct NormalNoise2DParameters final : ElementSpecificParameters
	{
		double amplitude;

		explicit NormalNoise2DParameters(double amplitude = 0.2)
			: amplitude(amplitude)
		{}

		bool operator==(const NormalNoise2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(amplitude - other.amplitude) < epsilon;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: [Amplitude: " << amplitude << "]";
			return result.str();
		}
	};

	class NormalNoise2D final : public Element
	{
	private:
		NormalNoise2DParameters parameters;
	public:
		NormalNoise2D(const ElementCommonParameters& elementCommonParameters,
		              const NormalNoise2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const NormalNoise2DParameters& parameters);
		NormalNoise2DParameters getParameters() const;
	};
}
