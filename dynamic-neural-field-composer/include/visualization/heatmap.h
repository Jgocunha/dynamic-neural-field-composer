#pragma once

#include "plot.h"

namespace dnf_composer
{
	struct HeatmapParameters final : PlotSpecificParameters
	{
		double scaleMin, scaleMax;
		bool autoScale;
		bool autoDimensions;  // infer rows/cols from data size each frame
		int hintRows = 0;     // set by Visualization when element sizes are known
		int hintCols = 0;

		HeatmapParameters();
		HeatmapParameters(double scaleMin, double scaleMax);
		std::string toString() const override;
		bool operator==(const HeatmapParameters& other) const;
	};

	class Heatmap : public Plot
	{
		HeatmapParameters heatmapParameters;
	public:
		explicit Heatmap(const PlotCommonParameters& parameters =
		                 { PlotType::HEATMAP,
			                 {0.0, 100.0, 0.0, 100.0, 1.0, 1.0},
			                 PlotAnnotations{"Heatmap plot", "Spatial dimension output", "Spatial dimension input"}},
		                 const HeatmapParameters& heatmapParameters = HeatmapParameters());

		void setScale(double min, double max);
		std::pair<double, double> getScale() const;
		void setDimensionHint(int rows, int cols);
		std::string toString() const override;
		void render(const std::vector<std::vector<double>*>& data, const std::vector<std::string>& legends) override;
	};
}