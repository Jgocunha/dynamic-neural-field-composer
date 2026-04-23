#pragma once

#include <iostream>

#include "tools/math.h"
#include "element.h"


namespace dnf_composer::element
{
	/// @brief Parameters for a Gaussian-shaped external stimulus.
	/// @ingroup elements
	struct GaussStimulusParameters final : ElementSpecificParameters
	{
		double width;      ///< Standard deviation (σ) of the Gaussian bump.
		double amplitude;  ///< Peak amplitude.
		double position;   ///< Spatial position (centre) of the Gaussian.
		bool circular;     ///< If true, the stimulus wraps at the field boundary.
		bool normalized;   ///< If true, the Gaussian is area-normalised.

		/// @brief Construct a GaussStimulus parameter set.
		/// @param width       Gaussian σ (default 5).
		/// @param amplitude   Peak value (default 15).
		/// @param position    Centre in spatial coordinates (default 50).
		/// @param circular    Circular/toroidal boundary (default true).
		/// @param normalized  Area normalization (default false).
		explicit GaussStimulusParameters(const double width = 5.0, const double amplitude = 15.0,
		                        const double position = 50.0, const bool circular = true, const bool normalized = false)
			: width(width), amplitude(amplitude), position(position),
			  circular(circular), normalized(normalized)
		{}

		bool operator==(const GaussStimulusParameters& other) const
		{
			constexpr double epsilon = 1e-6;

			return std::abs(width - other.width) < epsilon &&
				std::abs(position - other.position) < epsilon &&
				std::abs(amplitude - other.amplitude) < epsilon &&
				circular == other.circular &&
				normalized == other.normalized;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2); // Ensures numbers have 2 decimal places
			result << "Parameters: ["
				<< "Width: " << width << ", "
				<< "Amplitude: " << amplitude << ", "
				<< "Position: " << position << ", "
				<< "Circular: " << (circular ? "true" : "false") << ", "
				<< "Normalized: " << (normalized ? "true" : "false")
				<< "]";
			return result.str();
		}
	};

	/// @brief Localized Gaussian input stimulus applied to a neural field.
	///
	/// Provides a spatially localised, time-invariant (constant) input that models
	/// an external sensory cue or manually specified target location. The amplitude
	/// can be changed at runtime via @c setParameters() to simulate stimulus onset
	/// and offset.
	///
	/// @ingroup elements
	class GaussStimulus final : public Element
	{
	private:
		GaussStimulusParameters parameters;
	public:
		/// @brief Construct a GaussStimulus.
		/// @param elementCommonParameters  Name, label, and spatial dimensions.
		/// @param parameters               Stimulus-specific parameters.
		GaussStimulus(const ElementCommonParameters& elementCommonParameters,
		              const GaussStimulusParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		/// @brief Replace the stimulus parameters at runtime (e.g. to change amplitude or position).
		void setParameters(const GaussStimulusParameters& parameters);

		/// @brief Return a copy of the current stimulus parameters.
		GaussStimulusParameters getParameters() const;
	};
}
