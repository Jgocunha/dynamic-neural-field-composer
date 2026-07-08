#pragma once

#include "element_parameters.h"

namespace dnf_composer::element
{
		struct NormalNoiseParameters : ElementSpecificParameters
		{
			double amplitude;
			NormalNoiseParameters(double amp = 1.0);
			bool operator==(const NormalNoiseParameters& other) const;
			[[nodiscard]] std::string toString() const override;
		};
}