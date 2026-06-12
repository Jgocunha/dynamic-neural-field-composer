#include "element_parameters/gauss_kernel_parameters.h"
#include <format>

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

		std::string GaussKernelParameters::toString() const
		{
			return std::format(
        "Gauss kernel parameters\n"
        "Width: {:.2f}\n"
        "Amplitude: {:.2f}\n"
        "Circular: {}\n"
        "Normalized: {}\n",
        width, amplitude, circular, normalized);
		}
	}
}