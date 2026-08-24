#pragma once

#include "plot.h"

namespace dnf_composer
{
	/// @brief Line-plot-specific parameters: stroke thickness and axis auto-fit.
	struct LinePlotParameters final : PlotSpecificParameters
	{
		double lineThickness; ///< Line stroke width.
		bool autoFit; ///< Whether axis limits auto-fit the plotted data.

		/// @brief Construct default line plot parameters.
		LinePlotParameters();
		/// @brief Construct line plot parameters with explicit thickness and auto-fit.
		/// @param lineThickness Line stroke width.
		/// @param autoFit       Whether axis limits auto-fit the plotted data.
		LinePlotParameters(double lineThickness, bool autoFit);
		/// @brief Format the parameters as a human-readable string.
		/// @return A string describing the parameters.
		[[nodiscard]] std::string toString() const override;
		/// @brief Compare two parameter sets for equality.
		/// @param other Parameters to compare against.
		/// @return True if all fields are equal.
		bool operator==(const LinePlotParameters& other) const;
	};


	/// @brief 1D field/component plot rendered as a line.
	class LinePlot final : public Plot
	{
		LinePlotParameters linePlotParameters;
	public:
		/// @brief Construct a line plot.
		///
		/// @throws std::invalid_argument if `parameters.type != PlotType::LINE_PLOT`.
		/// @see Heatmap for the rationale behind this asymmetry (#143).
		/// @param parameters        Common plot parameters (type, dimensions, annotations).
		/// @param linePlotParameters Line-plot-specific parameters (thickness, auto-fit).
		explicit LinePlot(const PlotCommonParameters& parameters =
		                  { PlotType::LINE_PLOT,
			                  {0.0, 100.0, -20.0, 20.0, 1.0, 1.0},
			                  PlotAnnotations{"Line plot", "Spatial dimension", "Amplitude"} },
		                  LinePlotParameters  linePlotParameters = LinePlotParameters());

		/// @brief Set the line stroke width.
		/// @param lineThickness New stroke width.
		void setLineThickness(double lineThickness);
		/// @brief Set whether axis limits auto-fit the plotted data.
		/// @param autoFit True to auto-fit axis limits.
		void setAutoFit(bool autoFit);
		/// @brief Get the current line stroke width.
		/// @return The stroke width.
		[[nodiscard]] double getLineThickness() const;
		/// @brief Get whether axis limits auto-fit the plotted data.
		/// @return 1.0 if auto-fit is enabled, 0.0 otherwise.
		[[nodiscard]] double getAutoFit() const;
		/// @brief Format the line plot as a human-readable string.
		/// @return A string describing the line plot.
		[[nodiscard]] std::string toString() const override;
		void render(const std::vector<std::vector<double>*>& data, const std::vector<std::string>& legends) override;
	};
}
