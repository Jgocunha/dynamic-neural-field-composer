#pragma once

#include <map>
#include <string>
#include <vector>

#include "tools/logger.h"

namespace dnf_composer
{
	/// @brief Axis range and sampling step of a plot, in field/data coordinates.
	struct PlotDimensions
	{
		double xMin;  ///< Lower x bound.
		double xMax;  ///< Upper x bound.
		double yMin;  ///< Lower y bound.
		double yMax;  ///< Upper y bound.
		double xStep; ///< Sampling step along x.
		double yStep; ///< Sampling step along y.

		/// @brief Construct default (unset) dimensions.
		PlotDimensions();
		/// @brief Construct explicit axis bounds and steps.
		/// @param x_min  Lower x bound.
		/// @param x_max  Upper x bound.
		/// @param y_min  Lower y bound.
		/// @param y_max  Upper y bound.
		/// @param x_step Sampling step along x.
		/// @param y_step Sampling step along y.
		PlotDimensions(const double& x_min, const double& x_max,
			const double& y_min, const double& y_max,
			const double& x_step, const double& y_step);
		/// @brief Construct dimensions with only the x sampling step set; bounds default to zero.
		/// @param x_step Sampling step along x.
		explicit PlotDimensions(double x_step);
		/// @brief Check that the bounds and steps form a valid, renderable plot range.
		/// @return True if the dimensions are legal.
		[[nodiscard]] bool isLegal() const;
		/// @brief Format the dimensions as a human-readable string.
		/// @return A string describing the bounds and steps.
		[[nodiscard]] std::string toString() const;
		/// @brief Compare two dimensions for equality.
		/// @param other Dimensions to compare against.
		/// @return True if all bounds and steps are equal.
		bool operator==(const PlotDimensions& other) const;
	};

	/// @brief Title and axis labels of a plot.
	struct PlotAnnotations
	{
		std::string title;   ///< Plot title.
		std::string x_label; ///< X-axis label.
		std::string y_label; ///< Y-axis label.

		/// @brief Construct default (empty) annotations.
		PlotAnnotations();
		/// @brief Construct annotations with an explicit title and axis labels.
		/// @param title   Plot title.
		/// @param x_label X-axis label.
		/// @param y_label Y-axis label.
		explicit PlotAnnotations(std::string title, std::string x_label = "Spatial dimension",
			std::string y_label = "Amplitude");
		/// @brief Format the annotations as a human-readable string.
		/// @return A string describing the title and axis labels.
		[[nodiscard]] std::string toString() const;
		/// @brief Compare two annotation sets for equality.
		/// @param other Annotations to compare against.
		/// @return True if the title and both axis labels are equal.
		bool operator==(const PlotAnnotations& other) const;
	};

	/// @brief Kind of plot a visualization entry renders as.
	enum class PlotType : int
	{
		LINE_PLOT = 0,
		HEATMAP = 1
	};

	/// @brief Human-readable name for each @c PlotType, used in the UI and logs.
	inline const std::map<PlotType, std::string> PlotTypeToString = {
			{PlotType::LINE_PLOT, "line plot" },
			{PlotType::HEATMAP, "heatmap" }
		};

	/// @brief Parameters shared by every plot type: kind, axis range, and annotations.
	struct PlotCommonParameters
	{
		PlotType type; ///< Kind of plot this describes.
		PlotDimensions dimensions; ///< Axis range and sampling step.
		PlotAnnotations annotations; ///< Title and axis labels.

		/// @brief Construct default common parameters (a line plot with default dimensions/annotations).
		PlotCommonParameters();
		/// @brief Construct common parameters for a plot type, with default dimensions and annotations.
		/// @param type Kind of plot.
		explicit PlotCommonParameters(PlotType type);
		/// @brief Construct common parameters for a plot type with explicit annotations.
		/// @param type        Kind of plot.
		/// @param annotations Title and axis labels.
		PlotCommonParameters(PlotType type, PlotAnnotations annotations);
		/// @brief Construct common parameters for a plot type with explicit dimensions and annotations.
		/// @param type        Kind of plot.
		/// @param dimensions  Axis range and sampling step.
		/// @param annotations Title and axis labels.
		PlotCommonParameters(PlotType type, const PlotDimensions& dimensions, PlotAnnotations annotations);
		/// @brief Format the parameters as a human-readable string.
		/// @return A string describing the type, dimensions, and annotations.
		[[nodiscard]] std::string toString() const;
		/// @brief Compare two parameter sets for equality.
		/// @param other Parameters to compare against.
		/// @return True if type, dimensions, and annotations are all equal.
		bool operator==(const PlotCommonParameters& other) const;
	};

	/// @brief Base for parameters specific to one plot type (e.g. line vs heatmap options).
	struct PlotSpecificParameters
	{
		virtual ~PlotSpecificParameters() = default;
		/// @brief Construct default (empty) plot-specific parameters.
		PlotSpecificParameters() = default;
		/// @brief Format the plot-type-specific parameters as a human-readable string.
		/// @return A string describing the parameters.
		[[nodiscard]] virtual std::string toString() const = 0;
	};
}