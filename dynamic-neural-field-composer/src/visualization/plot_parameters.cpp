#include "visualization/plot_parameters.h"
#include <format> 

namespace dnf_composer
{
	PlotDimensions::PlotDimensions()
		: xMin(0), xMax(100), yMin(-10.0), yMax(10.0), xStep(1.0), yStep(1.0)
	{}

	PlotDimensions::PlotDimensions(const double& x_min, const double& x_max, 
		const double& y_min, const double& y_max, 
		const double& x_step, const double& y_step)
		: xMin(x_min), xMax(x_max), yMin(y_min), yMax(y_max), xStep(x_step), yStep(y_step)
	{
		if (xMin >= xMax)
		{
			xMin = 0;
			xMax = 100;
			log(tools::logger::LogLevel::WARNING, "xMin must be less than xMax.");
			return;
		}
		if (yMin >= yMax)
		{
			yMin = -10.0;
			yMax = 10.0;
			log(tools::logger::LogLevel::WARNING, "yMin must be less than yMax.");
			return;
		}
		if (xStep <= 0)
		{
			this->xStep = 1.0;
			log(tools::logger::LogLevel::WARNING, "xStep must be positive.");
			return;
		}
		if (yStep <= 0)
		{
			this->yStep = 1.0;
			log(tools::logger::LogLevel::WARNING, "yStep must be positive.");
			return;
		}
	}

	PlotDimensions::PlotDimensions(double xStep)
		: xMin(0), xMax(100), yMin(0), yMax(1), xStep(xStep), yStep(1.0)
	{
		if (xStep <= 0)
		{
			this->xStep = 1.0;
			log(tools::logger::LogLevel::WARNING, "xStep must be positive.");
		}
	}

	bool PlotDimensions::isLegal() const
	{
		return xMin < xMax && yMin < yMax && xStep > 0 && yStep > 0;
	}

	std::string PlotDimensions::toString() const
	{
		return std::format(
        "Plot dimensions: {{xMin: {:.2f}, xMax: {:.2f}, "
        "yMin: {:.2f}, yMax: {:.2f}, xStep: {:.2f}, yStep: {:.2f}}}",
        xMin, xMax, yMin, yMax, xStep, yStep);
	}

	bool PlotDimensions::operator==(const PlotDimensions& other) const
	{
		constexpr double epsilon = 1e-6;

		return std::fabs(xMin - other.xMin) <= epsilon &&
			std::fabs(xMax - other.xMax) <= epsilon &&
			std::fabs(yMin - other.yMin) <= epsilon &&
			std::fabs(yMax - other.yMax) <= epsilon &&
			std::fabs(xStep - other.xStep) <= epsilon &&
			std::fabs(yStep - other.yStep) <= epsilon;
	}

	PlotAnnotations::PlotAnnotations()
		:title("Element component(s)"), x_label("Spatial dimension"), y_label("Amplitude")
	{}

	PlotAnnotations::PlotAnnotations(std::string title, std::string x_label, std::string y_label)
		:title(std::move(title)), x_label(std::move(x_label)), y_label(std::move(y_label))
	{}

	std::string PlotAnnotations::toString() const
	{
		return std::format(
        "Plot annotations: {{title: {}, x_label: {}, y_label: {}}}",
        title, x_label, y_label);
	}

	bool PlotAnnotations::operator==(const PlotAnnotations& other) const
	{
		return !(title != other.title || x_label != other.x_label || y_label != other.y_label);
	}

	PlotCommonParameters::PlotCommonParameters()
		: type(PlotType::LINE_PLOT)
	{}

	PlotCommonParameters::PlotCommonParameters(const PlotType type)
		: type(type)
	{}

	PlotCommonParameters::PlotCommonParameters(const PlotType type , PlotAnnotations annotations)
		: type(type), annotations(std::move(annotations))
	{}

	PlotCommonParameters::PlotCommonParameters(const PlotType type, const PlotDimensions& dimensions, PlotAnnotations annotations)
		: type(type), dimensions(dimensions), annotations(std::move(annotations))
	{}

	std::string PlotCommonParameters::toString() const
	{
		return std::format(
			"Plot parameters: [ type: {}, {}, {} ]",
			PlotTypeToString.at(type), dimensions.toString(), annotations.toString());
	}
	

	bool PlotCommonParameters::operator==(const PlotCommonParameters& other) const
	{
		if (dimensions != other.dimensions || annotations != other.annotations)
			return false;
		return true;
	}

}
