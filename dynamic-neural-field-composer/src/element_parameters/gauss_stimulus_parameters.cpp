#include "element_parameters/gauss_stimulus_parameters.h"
#include <format>

namespace dnf_composer
{
	namespace element
	{

		GaussStimulusParameters::GaussStimulusParameters(double width, double amplitude,
			double position, bool circular, bool normalized)
			: width(width), amplitude(amplitude), position(position),
			circular(circular), normalized(normalized)
		{}


		bool GaussStimulusParameters::operator==(const GaussStimulusParameters& other) const
		{
			constexpr double epsilon = 1e-6;

			return std::abs(width - other.width) < epsilon &&
				std::abs(position - other.position) < epsilon &&
				std::abs(amplitude - other.amplitude) < epsilon &&
				circular == other.circular &&
				normalized == other.normalized;
		}

		std::string GaussStimulusParameters::toString() const override
		{
			return std::format(
        "Gaussian stimulus parameters\n"
        "Width = {:.2f}, Amplitude = {:.2f}, Position = {:.2f}, "
        "Circular = {}, Normalized = {}, ",
        width, amplitude, position, circular, normalized);
		}
	}
}