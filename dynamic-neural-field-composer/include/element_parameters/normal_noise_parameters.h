#pragma once

#include "element_parameters.h"

namespace dnf_composer
{
	namespace element
	{
		struct NormalNoiseParameters : ElementSpecificParameters
		{
			double amplitude;
			NormalNoiseParameters(double amp = 1.0);
			bool operator==(const NormalNoiseParameters& other) const;
			std::string toString() const override;
		};
	}
}