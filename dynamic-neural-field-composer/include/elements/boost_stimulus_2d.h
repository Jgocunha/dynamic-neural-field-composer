#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>

#include "element.h"

namespace dnf_composer::element
{
	struct BoostStimulus2DParameters final : ElementSpecificParameters
	{
		double amplitude;
		bool isActive;

		explicit BoostStimulus2DParameters(double amplitude = 5.0, bool isActive = true)
			: amplitude(amplitude), isActive(isActive)
		{}

		bool operator==(const BoostStimulus2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(amplitude - other.amplitude) < epsilon &&
			       isActive == other.isActive;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "Amplitude: " << amplitude << ", "
				<< "IsActive: " << (isActive ? "true" : "false") << "]";
			return result.str();
		}
	};

	/**
	 * @brief Spatially uniform constant stimulus for 2D neural fields.
	 *
	 * Fills every position of the 2D output with a constant amplitude value.
	 * Useful as a global gain or resting-level offset for 2D fields.
	 *
	 * @param elementCommonParameters  Common parameters (id, 2D dimensions).
	 * @param parameters               Amplitude and active flag.
	 */
	class BoostStimulus2D final : public Element
	{
	private:
		BoostStimulus2DParameters parameters;
	public:
		BoostStimulus2D(const ElementCommonParameters& elementCommonParameters,
		                const BoostStimulus2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		[[nodiscard]] std::string toString() const override;
		[[nodiscard]] std::shared_ptr<Element> clone() const override;

		void setParameters(const BoostStimulus2DParameters& parameters);
		[[nodiscard]] BoostStimulus2DParameters getParameters() const;
	};
}
