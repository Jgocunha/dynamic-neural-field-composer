#pragma once

#include "element_parameters.h"

namespace dnf_composer::element
{
		struct GaussKernelParameters : ElementSpecificParameters
		{
			double width;
			double amplitude;
			bool circular;
			bool normalized;

			GaussKernelParameters(double width = 5.0, double amp = 10.0,
				bool circular = true, bool normalized = true);
			bool operator==(const GaussKernelParameters& other) const;
			[[nodiscard]] std::string toString() const override;
		};
}