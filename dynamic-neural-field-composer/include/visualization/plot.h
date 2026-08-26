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

		/// @brief Get the plot's auto-assigned unique ID.
		/// @return The unique ID.
		[[nodiscard]] int getUniqueIdentifier() const;
		/// @brief Get the plot's type.
		/// @return The plot type.
		[[nodiscard]] PlotType getType() const;
		/// @brief Get the plot's axis dimensions.
		/// @return The current dimensions.
		[[nodiscard]] PlotDimensions getDimensions() const;
		/// @brief Get the plot's title and axis labels.
		/// @return The current annotations.
		[[nodiscard]] PlotAnnotations getAnnotations() const;
		/// @brief Set the plot's axis dimensions.
		/// @param dimensions New dimensions.
		void setDimensions(const PlotDimensions& dimensions);
		/// @brief Set the plot's title and axis labels.
		/// @param annotations New annotations.
		void setAnnotations(const PlotAnnotations& annotations);
		/// @brief Format the plot as a human-readable string.
		/// @return A string describing the plot.
		[[nodiscard]] virtual std::string toString() const = 0;

		/// @brief Render the plot using the provided data and legends.
		/// @param data     Pointers to the component vectors to display.
		/// @param legends  Legend label for each data series.
		virtual void render(const std::vector<std::vector<double>*>& data, const std::vector<std::string>& legends) = 0;
	};
}
