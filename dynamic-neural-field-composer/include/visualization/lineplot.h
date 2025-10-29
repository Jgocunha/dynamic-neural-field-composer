#pragma once

#include "plot.h"

namespace dnf_composer
{
	struct LinePlotParameters : PlotSpecificParameters
	{
		double lineThickness;
		bool autoFit;

		LinePlotParameters();
		LinePlotParameters(double lineThickness, bool autoFit);
		std::string toString() const override;
		bool operator==(const LinePlotParameters& other) const;
	};


	class LinePlot : public Plot
	{
		LinePlotParameters linePlotParameters;
	public:
		explicit LinePlot(const PlotCommonParameters& parameters =
		                  { PlotType::LINE_PLOT,
			                  {0.0, 100.0, -20.0, 20.0, 1.0, 1.0},
			                  {"Line plot", "Spatial dimension", "Amplitude"} },
		                  const LinePlotParameters& linePlotParameters = LinePlotParameters());

		void setLineThickness(double lineThickness);
		void setAutoFit(bool autoFit);
		double getLineThickness() const;
		double getAutoFit() const;
		std::string toString() const override;
		void render(const std::vector<std::vector<double>*>& data, const std::vector<std::string>& legends) override;
	};
}
