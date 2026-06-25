#pragma once

#include <atomic>

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
		static inline std::atomic<int> uniqueIdentifierCounter{0}; ///< Global counter for plot ID assignment (atomic: thread-safe construction).
		int uniqueIdentifier;                          ///< Auto-assigned unique ID.
		PlotCommonParameters commonParameters;         ///< Type, axes ranges, and annotation strings.
	public:
		virtual ~Plot() = default;

		/// @brief Construct a plot with the given common parameters.
		/// @param parameters  Type, dimensions, and annotations (defaults to LINE_PLOT with default ranges).
		explicit Plot(PlotCommonParameters parameters = PlotCommonParameters());

		int getUniqueIdentifier() const;
		PlotType getType() const;
		PlotDimensions getDimensions() const;
		PlotAnnotations getAnnotations() const;
		void setDimensions(const PlotDimensions& dimensions);
		void setAnnotations(const PlotAnnotations& annotations);
		virtual std::string toString() const = 0;

		/// @brief Render the plot using the provided data and legends.
		/// @param data     Pointers to the component vectors to display.
		/// @param legends  Legend label for each data series.
		virtual void render(const std::vector<std::vector<double>*>& data, const std::vector<std::string>& legends) = 0;
	};
}
