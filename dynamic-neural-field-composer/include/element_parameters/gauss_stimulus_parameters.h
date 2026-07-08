#pragma once

#include "element_parameters.h"

namespace dnf_composer::element
{
		struct GaussStimulusParameters : ElementSpecificParameters
		{
			double width;
			double amplitude;
			double position;
			bool circular;
			bool normalized;

			GaussStimulusParameters(double width = 5.0, double amplitude = 15.0,
				double position = 0.0, bool circular = true, bool normalized = false);
			bool operator==(const GaussStimulusParameters& other) const;
			[[nodiscard]] std::string toString() const override;
		};
}