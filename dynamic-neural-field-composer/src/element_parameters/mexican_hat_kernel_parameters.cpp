#include "element_parameters/mexican_hat_kernel_parameters.h"

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
			std::string result = "Mexican-hat kernel parameters\n";
			result += "Width exc: " + std::to_string(widthExc) + "\n";
			result += "Amplitude exc: " + std::to_string(amplitudeExc) + "\n";
			result += "Width inh: " + std::to_string(widthInh) + "\n";
			result += "Amplitude inh: " + std::to_string(amplitudeInh) + "\n";
			result += "Circular: " + std::to_string(circular) + "\n";
			result += "Normalized: " + std::to_string(normalized) + "\n";
			return result;
		}
	}
}