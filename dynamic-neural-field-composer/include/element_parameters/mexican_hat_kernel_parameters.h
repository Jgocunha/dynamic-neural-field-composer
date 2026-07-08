#pragma once

#include "element_parameters.h"

namespace dnf_composer::element
{
		struct MexicanHatKernelParameters : ElementSpecificParameters
		{
			double widthExc;
			double amplitudeExc;
			double widthInh;
			double amplitudeInh;
			bool circular;
			bool normalized;

			MexicanHatKernelParameters(double widthExc, double amplitudeExc, double widthInh,
				double amplitudeInh, bool circular = true, bool normalized = true);
			bool operator==(const MexicanHatKernelParameters& other) const;
			[[nodiscard]] std::string toString() const override;
		};
}