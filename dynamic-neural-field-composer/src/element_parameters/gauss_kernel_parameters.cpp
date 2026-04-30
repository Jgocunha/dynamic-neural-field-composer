#include "element_parameters/gauss_kernel_parameters.h"

namespace dnf_composer
{
	namespace element
	{
		GaussKernelParameters::GaussKernelParameters(double width, double amp,
			bool circular, bool normalized)
			: width(width), amplitude(amp), circular(circular), normalized(normalized)
		{}

		bool GaussKernelParameters::operator==(const GaussKernelParameters& other) const
		{
			constexpr double epsilon = 1e-6;

			return std::abs(width - other.width) < epsilon &&
				std::abs(amplitude - other.amplitude) < epsilon &&
				circular == other.circular &&
				normalized == other.normalized;
		}

		std::string GaussKernelParameters::toString() const override
		{
			std::string result = "Gauss kernel parameters\n";
			result += "Width: " + std::to_string(width) + "\n";
			result += "Amplitude: " + std::to_string(amplitude) + "\n";
			result += "Circular: " + std::to_string(circular) + "\n";
			result += "Normalized: " + std::to_string(normalized) + "\n";
			return result;
		}
	}
}