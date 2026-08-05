#pragma once

#include <cstddef>
#include <utility>

#include "plot.h"

namespace dnf_composer
{
	/// @brief Compute the row/column grid for manual (non-auto) heatmap dimensions.
	///
	/// In manual mode, Heatmap::render() derives the grid from user-editable axis
	/// extents and step sizes: `rows = y_max / y_step`, `cols = x_max / x_step`.
	/// Those fields have no built-in relationship to the actual data buffer
	/// size, so a mismatched combination (e.g. a step too small, or maxes too
	/// large for the connected element) previously asked
	/// `ImPlot::PlotHeatmap(..., rows, cols, ...)` to read `rows * cols`
	/// elements from a buffer that holds fewer — an out-of-bounds read (#121).
	///
	/// This computes the same naive rows/cols and then clamps @c cols down
	/// (preserving @p rows, and therefore the aspect ratio the user configured)
	/// until `rows * cols <= dataSize`. A non-positive step yields 0 for that
	/// axis, which is degenerate but never out of bounds.
	///
	/// It is deliberately pure and silent: it is called from a render function
	/// that runs every frame, so reporting a clamp is the caller's decision
	/// (see @c clamped) rather than a log line emitted 60 times a second.
	struct ManualHeatmapDimensions
	{
		int rows;     ///< Rows to pass to ImPlot::PlotHeatmap.
		int cols;     ///< Columns to pass, already clamped to fit @c dataSize.
		bool clamped; ///< True if @c cols was reduced to fit the available data.
	};

	/// @param x_max,y_max  Axis extents (the heatmap "max" dimension fields).
	/// @param x_step,y_step Axis step sizes.
	/// @param dataSize     Number of elements actually available in the flattened data.
	/// @return rows/cols such that `rows * cols <= dataSize` (both >= 0).
	[[nodiscard]] ManualHeatmapDimensions resolveManualHeatmapDimensions(int x_max, int y_max,
		float x_step, float y_step, std::size_t dataSize);

	struct HeatmapParameters final : PlotSpecificParameters
	{
		double scaleMin, scaleMax;
		bool autoScale;
		bool autoDimensions;  // infer rows/cols from data size each frame
		int hintRows = 0;     // set by Visualization when element sizes are known
		int hintCols = 0;

		HeatmapParameters();
		HeatmapParameters(double scaleMin, double scaleMax);
		[[nodiscard]] std::string toString() const override;
		bool operator==(const HeatmapParameters& other) const;
	};

	class Heatmap : public Plot
	{
		HeatmapParameters heatmapParameters;
		/// Last column count reported as clamped, so render() warns once per change
		/// instead of once per frame. -1 means "nothing reported yet".
		int lastReportedClampedCols = -1;
	public:
		explicit Heatmap(const PlotCommonParameters& parameters =
		                 { PlotType::HEATMAP,
			                 {0.0, 100.0, 0.0, 100.0, 1.0, 1.0},
			                 PlotAnnotations{"Heatmap plot", "Spatial dimension output", "Spatial dimension input"}},
		                 HeatmapParameters  heatmapParameters = HeatmapParameters());

		void setScale(double min, double max);
		[[nodiscard]] std::pair<double, double> getScale() const;
		void setDimensionHint(int rows, int cols);
		[[nodiscard]] std::string toString() const override;
		void render(const std::vector<std::vector<double>*>& data, const std::vector<std::string>& legends) override;
	};
}