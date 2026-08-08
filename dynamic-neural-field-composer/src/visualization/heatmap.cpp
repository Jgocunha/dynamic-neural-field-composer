#include "visualization/heatmap.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <utility>

// ImGui/ImPlot used directly below (BeginMenuBar, PlotHeatmap, ...). Previously
// reached transitively through tools/logger.h -> imgui-platform-kit/log_window.h;
// that path is gone now that tools/ no longer depends on the GUI stack (#123), so
// this file pulls in what it actually uses, same as lineplot.cpp already does.
#include "application/application.h"

namespace dnf_composer
{
	ManualHeatmapDimensions resolveManualHeatmapDimensions(int x_max, int y_max,
		float x_step, float y_step, std::size_t dataSize)
	{
		int rows = (y_step > 0.0F) ? static_cast<int>(static_cast<float>(y_max) / y_step) : 0;
		int cols = (x_step > 0.0F) ? static_cast<int>(static_cast<float>(x_max) / x_step) : 0;
		if (rows < 0) { rows = 0; }
		if (cols < 0) { cols = 0; }

		if (dataSize == 0)
		{
			return { 0, 0, false };
		}

		// Compare via division, not rows*cols, to avoid a multiplication overflow
		// for pathological inputs.
		if (rows > 0 && static_cast<std::size_t>(cols) > dataSize / static_cast<std::size_t>(rows))
		{
			return { rows, static_cast<int>(dataSize / static_cast<std::size_t>(rows)), true };
		}

		return { rows, cols, false };
	}

	const char* selectHeatmapTickFormat(double scaleMin, double scaleMax)
	{
		const double span = std::abs(scaleMax - scaleMin);

		if (!std::isfinite(span) || span == 0.0) {
			return "%.2f";
		}
		if (span >= 10.0) {
			return "%.0f";
		}
		if (span >= 1.0) {
			return "%.2f";
		}
		if (span >= 0.01) {
			return "%.4f";
		}
		if (span >= 1e-4) {
			return "%.5f";
		}
		return "%.1e";
	}

	namespace
	{
		// ImPlot::ColormapScale sizes its own frame to the fixed width the caller
		// passes in and clips tick labels to that frame rather than growing to fit
		// them. selectHeatmapTickFormat() picks precision from the displayed range,
		// so a narrow range now produces longer labels (e.g. "-0.0090") than
		// ImPlot's old "%g" default did (e.g. "-0.009"); a colorbar width tuned for
		// the old default clips the new, more precise labels right back down to
		// the unreadable text this fix exists to avoid. Size the frame from the
		// actual worst-case label instead of a constant.
		float colorbarWidthFor(double scaleMin, double scaleMax)
		{
			const char* fmt = selectHeatmapTickFormat(scaleMin, scaleMax);
			char buf[32];
			std::snprintf(buf, sizeof(buf), fmt, scaleMin);
			const float wMin = ImGui::CalcTextSize(buf).x;
			std::snprintf(buf, sizeof(buf), fmt, scaleMax);
			const float wMax = ImGui::CalcTextSize(buf).x;
			constexpr float barAndPadding = 34.0F; // color bar itself + tick marks + margins
			return barAndPadding + (wMin > wMax ? wMin : wMax);
		}
	}

	HeatmapParameters::HeatmapParameters()
		: scaleMin(0), scaleMax(1), autoScale(true), autoDimensions(true)
	{}

	HeatmapParameters::HeatmapParameters(double scaleMin, double scaleMax)
		: scaleMin(scaleMin), scaleMax(scaleMax), autoScale(false), autoDimensions(true)
	{}

	std::string HeatmapParameters::toString() const
	{
		std::string result;
		result += "Heatmap parameters: {";
		result += "Scale min: " + std::to_string(scaleMin) + ", ";
		result += "Scale max: " + std::to_string(scaleMax) + ", ";
		result += "Auto scale: " + std::string(autoScale ? "true" : "false") + "}";
		return result;
	}

	bool HeatmapParameters::operator==(const HeatmapParameters& other) const
	{
		static constexpr double epsilon = 1e-6;
		return std::abs(scaleMin - other.scaleMin) <= epsilon && std::abs(scaleMax - other.scaleMax) <= epsilon;
	}

	Heatmap::Heatmap(const PlotCommonParameters& parameters, HeatmapParameters  heatmapParameters)
		: Plot(parameters), heatmapParameters(std::move(heatmapParameters))
	{
	}

	void Heatmap::setScale(double min, double max)
	{
		if (min >= max)
		{
			heatmapParameters.scaleMin = 0;
			heatmapParameters.scaleMax = 1;
			log(tools::logger::LogLevel::WARNING, "Scale min must be less than scale max.");
			return;
		}
		heatmapParameters.scaleMin = min;
		heatmapParameters.scaleMax = max;
	}

	std::pair<double, double> Heatmap::getScale() const
	{
		return { heatmapParameters.scaleMin, heatmapParameters.scaleMax };
	}

	void Heatmap::setDimensionHint(int rows, int cols)
	{
		heatmapParameters.hintRows = rows;
		heatmapParameters.hintCols = cols;
	}

	std::string Heatmap::toString() const
	{
		std::ostringstream result;
		result << "Plot: { ";
		result << "Unique identifier: " << uniqueIdentifier << ", ";
		result << commonParameters.toString() << ", ";
		result << heatmapParameters.toString();
		return result.str();
	}

	// NOLINTNEXTLINE(readability-function-cognitive-complexity) - linear ImPlot immediate-mode layout; splitting would fragment plot state across functions
	void Heatmap::render(const std::vector<std::vector<double>*>& data, const std::vector<std::string>& legends)
	{
		const ImVec2 availableRegionSize = ImGui::GetContentRegionAvail();
		const ImVec2 plotSize = ImVec2(availableRegionSize.x - 65.0F, availableRegionSize.y - 5.0F);

		const std::string uniquePlotID = commonParameters.annotations.title + "##" + std::to_string(uniqueIdentifier);

		auto x_max = static_cast<int>(commonParameters.dimensions.xMax);
		auto x_min = static_cast<int>(commonParameters.dimensions.xMin);
		auto y_max = static_cast<int>(commonParameters.dimensions.yMax);
		auto y_min = static_cast<int>(commonParameters.dimensions.yMin);
		auto x_step = static_cast<float>(commonParameters.dimensions.xStep);
		auto y_step = static_cast<float>(commonParameters.dimensions.yStep);
		auto scaleMin = static_cast<float>(heatmapParameters.scaleMin);
		auto scaleMax = static_cast<float>(heatmapParameters.scaleMax);
		bool autoScale = heatmapParameters.autoScale;

		std::string title = commonParameters.annotations.title;
		std::string x_label = commonParameters.annotations.x_label;
		std::string y_label = commonParameters.annotations.y_label;
		std::array<char, 128> titleBuffer{};
		std::array<char, 128> xLabelBuffer{};
		std::array<char, 128> yLabelBuffer{};
		snprintf(titleBuffer.data(), titleBuffer.size(), "%s", title.c_str());
		snprintf(xLabelBuffer.data(), xLabelBuffer.size(), "%s", x_label.c_str());
		snprintf(yLabelBuffer.data(), yLabelBuffer.size(), "%s", y_label.c_str());

		static ImPlotColormap map = ImPlotColormap_Deep;
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Dimensions"))
			{
				bool autoDim = heatmapParameters.autoDimensions;
				if (ImGui::Checkbox("Auto-fit from data", &autoDim)) {
					heatmapParameters.autoDimensions = autoDim;
}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip(
						"Automatically derive rows and columns from the\n"
						"data size. Finds the two integer factors closest\n"
						"to a square root (works for square and rectangular\n"
						"weight matrices). Manual settings are ignored.");
}
				if (!heatmapParameters.autoDimensions)
				{
					ImGui::Separator();
					if(ImGui::DragInt("X max", &x_max, 1, x_min, 1000)) {
						commonParameters.dimensions.xMax = x_max;
}
					if(ImGui::DragInt("Y max", &y_max, 1, y_min, 1000)) {
						commonParameters.dimensions.yMax = y_max;
}
					if(ImGui::DragInt("X min", &x_min, 1, 0, x_max)) {
						commonParameters.dimensions.xMin = x_min;
}
					if(ImGui::DragInt("Y min", &y_min, 1, 0, y_max)) {
						commonParameters.dimensions.yMin = y_min;
}
					if (ImGui::DragFloat("X step", &x_step, 0.1F, 0.1F, 1000)) {
						commonParameters.dimensions.xStep = x_step;
}
					if (ImGui::DragFloat("Y step", &y_step, 0.1F, 0.1F, 1000)) {
						commonParameters.dimensions.yStep = y_step;
}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Annotations"))
			{
				if (ImGui::InputText("Title", titleBuffer.data(), titleBuffer.size()))
				{
					title = titleBuffer.data();
					commonParameters.annotations.title = title;
				}
				if (ImGui::InputText("X label", xLabelBuffer.data(), xLabelBuffer.size()))
				{
					x_label = xLabelBuffer.data();
					commonParameters.annotations.x_label = x_label;
				}
				if (ImGui::InputText("Y label", yLabelBuffer.data(), yLabelBuffer.size()))
				{
					y_label = yLabelBuffer.data();
					commonParameters.annotations.y_label = y_label;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Colormap"))
			{
				if (ImPlot::ColormapButton(ImPlot::GetColormapName(map),
					ImVec2(availableRegionSize.x - 90.0F, 0.0F), map))
				{
					map = (map + 1) % ImPlot::GetColormapCount();
					ImPlot::BustColorCache(uniquePlotID.c_str());
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Scale"))
			{
				ImGui::DragFloatRange2("Min / Max", &scaleMin,
					&scaleMax, 0.01F, -20, 20);
				heatmapParameters.scaleMin = scaleMin;
				heatmapParameters.scaleMax = scaleMax;
				ImGui::Checkbox("Auto scale", &autoScale);
				heatmapParameters.autoScale = autoScale;
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (data.size() != 1) {
			return;
}

		auto *const flattened_matrix = data[0];

		if (autoScale)
		{
			auto [min_it, max_it] = std::minmax_element(flattened_matrix->begin(), flattened_matrix->end());
			heatmapParameters.scaleMin = *min_it;
			heatmapParameters.scaleMax = *max_it;
			scaleMin = static_cast<float>(heatmapParameters.scaleMin);
			scaleMax = static_cast<float>(heatmapParameters.scaleMax);
		}

		int rows;
		int cols;
		if (heatmapParameters.autoDimensions)
		{
			if (heatmapParameters.hintRows > 0 && heatmapParameters.hintCols > 0)
			{
				// Exact dimensions provided by the visualization layer (e.g. from
				// a field coupling's input/output component sizes).
				rows = heatmapParameters.hintRows;
				cols = heatmapParameters.hintCols;
			}
			else
			{
				// Fallback: find the two integer factors of data size closest to
				// each other (most square-like — works when no hint is available).
				const int total = static_cast<int>(flattened_matrix->size());
				rows = static_cast<int>(std::sqrt(static_cast<float>(total)));
				while (rows > 1 && total % rows != 0) {
					--rows;
}
				cols = (rows > 0) ? total / rows : total;
			}
			// Keep axis bounds in sync so the tick labels match the data
			commonParameters.dimensions.yMax = rows;
			commonParameters.dimensions.xMax = cols;
			x_max = cols;
			y_max = rows;
		}
		else
		{
			const auto manualDims = resolveManualHeatmapDimensions(
				x_max, y_max, x_step, y_step, flattened_matrix->size());
			rows = manualDims.rows;
			cols = manualDims.cols;

			// render() runs every frame, so warn only when the clamp changes.
			if (manualDims.clamped)
			{
				if (cols != lastReportedClampedCols)
				{
					log(tools::logger::LogLevel::WARNING,
						"Heatmap: manual dimensions exceed the available data (" +
						std::to_string(flattened_matrix->size()) + " elements); clamping columns to " +
						std::to_string(cols) + " to avoid an out-of-bounds read.");
					lastReportedClampedCols = cols;
				}
			}
			else
			{
				lastReportedClampedCols = -1;
			}
		}

		const float cbW = colorbarWidthFor(scaleMin, scaleMax);
		const ImVec2 hmSize(plotSize.x + 65.0F - cbW - ImGui::GetStyle().ItemSpacing.x, plotSize.y);

		static constexpr ImPlotFlags hm_flags = ImPlotFlags_Crosshairs | ImPlotFlags_NoLegend;
		if (ImPlot::BeginPlot(uniquePlotID.c_str(), hmSize, hm_flags)) {
			ImPlot::PushColormap(map);
			static constexpr ImPlotAxisFlags flags = ImPlotAxisFlags_AutoFit;
			ImPlot::SetupAxes(commonParameters.annotations.x_label.c_str(),
				commonParameters.annotations.y_label.c_str(), flags, flags);

			const std::string& label = legends[0];

			ImPlot::PlotHeatmap(
				label.c_str(),
				flattened_matrix->data(),
				rows, cols,
				scaleMin, scaleMax, nullptr,
				ImPlotPoint(x_min, y_max), ImPlotPoint(x_max, y_min)
			);
			ImPlot::EndPlot();
		}

		// // Add color scale next to the heatmap
		 ImGui::SameLine();
		 ImPlot::ColormapScale("##HeatScale", scaleMin, scaleMax, ImVec2(cbW, plotSize.y),
			 selectHeatmapTickFormat(scaleMin, scaleMax));
		//ImPlot::PopColormap();
	}

}
