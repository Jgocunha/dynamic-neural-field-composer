#include "element_parameters/gauss_stimulus_parameters.h"

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
			std::string result = "Gaussian stimulus parameters\n";
			result += "Width = " + std::to_string(width) + ", ";
			result += "Amplitude = " + std::to_string(amplitude) + ", ";
			result += "Position = " + std::to_string(position) + ", ";
			result += "Circular = " + std::to_string(circular) + ", ";
			result += "Normalized = " + std::to_string(normalized) + ", ";
			return result;
		}
	}
}