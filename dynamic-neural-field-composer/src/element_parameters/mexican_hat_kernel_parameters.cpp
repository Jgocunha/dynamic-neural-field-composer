#include "element_parameters/mexican_hat_kernel_parameters.h"
#include <format>

namespace dnf_composer
{
	namespace element
	{
		MexicanHatKernelParameters::MexicanHatKernelParameters(double widthExc, double amplitudeExc, double widthInh,
			double amplitudeInh, bool circular, bool normalized)
			: widthExc(widthExc), amplitudeExc(amplitudeExc), widthInh(widthInh),
			amplitudeInh(amplitudeInh), circular(circular), normalized(normalized)
		{}

		bool MexicanHatKernelParameters::operator==(const MexicanHatKernelParameters& other) const
		{
			constexpr double epsilon = 1e-6;

			return std::abs(widthExc - other.widthExc) < epsilon &&
				std::abs(amplitudeExc - other.amplitudeExc) < epsilon &&
				std::abs(widthInh - other.widthInh) < epsilon &&
				std::abs(amplitudeInh - other.amplitudeInh) < epsilon &&
				circular == other.circular &&
				normalized == other.normalized;
		}

		std::string MexicanHatKernelParameters::toString() const
		{
			return std::format(
        "Mexican-hat kernel parameters\n"
        "Width Exc: {:.2f}\n"
        "Amplitude Exc: {:.2f}\n"
        "Width Inh: {:.2f}\n"
        "Amplitude Inh: {:.2f}\n"
        "Circular: {}\n"
        "Normalized: {}\n",
        widthExc, amplitudeExc, widthInh, amplitudeInh, circular, normalized);
		}
	}
}