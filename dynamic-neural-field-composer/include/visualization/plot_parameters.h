#pragma once

#include <map>
#include <string>
#include <vector>

#include "tools/logger.h"

namespace dnf_composer
{
	struct PlotDimensions
	{
		double xMin, xMax, yMin, yMax, xStep, yStep;

		PlotDimensions();
		PlotDimensions(const double& x_min, const double& x_max, 
			const double& y_min, const double& y_max, 
			const double& x_step, const double& y_step);
		explicit PlotDimensions(double x_step);
		bool isLegal() const;
		std::string toString() const;
		bool operator==(const PlotDimensions& other) const;
	};

	struct PlotAnnotations
	{
		std::string title, x_label, y_label;

		PlotAnnotations();
		PlotAnnotations(std::string title, std::string x_label, std::string y_label);
		std::string toString() const;
		bool operator==(const PlotAnnotations& other) const;
	};

	enum class PlotType : int
	{
		LINE_PLOT = 0,
		HEATMAP = 1
	};

	inline const std::map<PlotType, std::string> PlotTypeToString = {
			{PlotType::LINE_PLOT, "line plot" },
			{PlotType::HEATMAP, "heatmap" }
		};

	struct PlotCommonParameters
	{
		PlotType type;
		PlotDimensions dimensions;
		PlotAnnotations annotations;

		PlotCommonParameters();
		PlotCommonParameters(const PlotType type, const PlotDimensions& dimensions, PlotAnnotations annotations);
		std::string toString() const;
		bool operator==(const PlotCommonParameters& other) const;
	};

	struct PlotSpecificParameters
	{
		virtual ~PlotSpecificParameters() = default;
		PlotSpecificParameters() = default;
		virtual std::string toString() const = 0;
	};
}