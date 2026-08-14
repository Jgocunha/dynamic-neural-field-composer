#pragma once

#include "plot.h"

namespace dnf_composer
{
	struct LinePlotParameters final : PlotSpecificParameters
	{
		double lineThickness;
		bool autoFit;

		LinePlotParameters();
		LinePlotParameters(double lineThickness, bool autoFit);
		[[nodiscard]] std::string toString() const override;
		bool operator==(const LinePlotParameters& other) const;
	};


	class LinePlot final : public Plot
	{
		LinePlotParameters linePlotParameters;
	public:
		/// @brief Construct a line plot.
		///
		/// @throws std::invalid_argument if `parameters.type != PlotType::LINE_PLOT`.
		/// This is a strict contract, unlike Heatmap's constructor, which
		/// normalizes a mismatched type instead of throwing (#143) -- LinePlot's
		/// throw predates that decision and is kept as-is here for backwards
		/// compatibility with existing callers that rely on it.
		///
		/// @param parameters        Common plot parameters (type, dimensions, annotations).
		/// @param linePlotParameters Line-plot-specific parameters (thickness, auto-fit).
		explicit LinePlot(const PlotCommonParameters& parameters =
		                  { PlotType::LINE_PLOT,
			                  {0.0, 100.0, -20.0, 20.0, 1.0, 1.0},
			                  PlotAnnotations{"Line plot", "Spatial dimension", "Amplitude"} },
		                  LinePlotParameters  linePlotParameters = LinePlotParameters());

		void setLineThickness(double lineThickness);
		void setAutoFit(bool autoFit);
		[[nodiscard]] double getLineThickness() const;
		[[nodiscard]] double getAutoFit() const;
		[[nodiscard]] std::string toString() const override;
		void render(const std::vector<std::vector<double>*>& data, const std::vector<std::string>& legends) override;
	};
}
