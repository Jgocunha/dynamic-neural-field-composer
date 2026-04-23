#pragma once

#include "plot_parameters.h"

namespace dnf_composer
{
	/// @brief Abstract base class for all renderable plots.
	///
	/// Each Plot has a unique integer ID, common parameters (type, dimensions,
	/// axis labels), and implements a @c render() method that draws the plot
	/// using the provided data pointers and legend strings.
	///
	/// @ingroup visualization
	class Plot
	{
	protected:
		static inline int uniqueIdentifierCounter = 0; ///< Global counter for plot ID assignment.
		int uniqueIdentifier;                          ///< Auto-assigned unique ID.
		PlotCommonParameters commonParameters;         ///< Type, axes ranges, and annotation strings.
	public:
		virtual ~Plot() = default;

		/// @brief Construct a plot with the given common parameters.
		/// @param parameters  Type, dimensions, and annotations (defaults to LINE_PLOT with default ranges).
		explicit Plot(PlotCommonParameters parameters = PlotCommonParameters());

		/// @brief Return the plot's unique numeric identifier.
		int getUniqueIdentifier() const;

		/// @brief Return the plot type (LINE_PLOT, HEATMAP, …).
		PlotType getType() const;

		/// @brief Return the current axis dimension settings.
		PlotDimensions getDimensions() const;

		/// @brief Return the current annotation (title, axis labels).
		PlotAnnotations getAnnotations() const;

		/// @brief Update the axis dimension settings.
		void setDimensions(const PlotDimensions& dimensions);

		/// @brief Update the annotation strings.
		void setAnnotations(const PlotAnnotations& annotations);

		/// @brief Return a human-readable description of the plot.
		virtual std::string toString() const = 0;

		/// @brief Render the plot using the provided data and legends.
		/// @param data     Pointers to the component vectors to display.
		/// @param legends  Legend label for each data series.
		virtual void render(const std::vector<std::vector<double>*>& data, const std::vector<std::string>& legends) = 0;
	};
}
