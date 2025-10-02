#include "visualization/heatmap.h"

namespace dnf_composer
{
	HeatmapParameters::HeatmapParameters()
		: scaleMin(0), scaleMax(1), autoScale(true)
	{}

	HeatmapParameters::HeatmapParameters(double scaleMin, double scaleMax)
		: scaleMin(scaleMin), scaleMax(scaleMax), autoScale(false)
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
		if (std::abs(scaleMin - other.scaleMin) > epsilon || std::abs(scaleMax - other.scaleMax) > epsilon)
			return false;
		return true;
	}

	Heatmap::Heatmap(const PlotCommonParameters& parameters, const HeatmapParameters& heatmapParameters)
		: Plot(parameters), heatmapParameters(heatmapParameters)
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

	std::string Heatmap::toString() const
	{
		std::ostringstream result;
		result << "Plot: { ";
		result << "Unique identifier: " << uniqueIdentifier << ", ";
		result << commonParameters.toString() << ", ";
		result << heatmapParameters.toString();
		return result.str();
	}

	void Heatmap::render(const std::vector<std::vector<double>*>& data, const std::vector<std::string>& legends)
	{
		if (data.size() != 1)
		{
			return;
		}

		const ImVec2 availableRegionSize = ImGui::GetContentRegionAvail();
		const ImVec2 plotSize = ImVec2(availableRegionSize.x - 90.0f, availableRegionSize.y - 5.0f);

		const auto flattened_matrix = data[0];
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
		char titleBuffer[128];
		char xLabelBuffer[128];
		char yLabelBuffer[128];
		snprintf(titleBuffer, sizeof(titleBuffer), "%s", title.c_str());
		snprintf(xLabelBuffer, sizeof(xLabelBuffer), "%s", x_label.c_str());
		snprintf(yLabelBuffer, sizeof(yLabelBuffer), "%s", y_label.c_str());

		static ImPlotColormap map = ImPlotColormap_Deep;
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Dimensions"))
			{
				if(ImGui::DragInt("X max", &x_max, 1, x_min, 1000))
					commonParameters.dimensions.xMax = x_max;
				if(ImGui::DragInt("Y max", &y_max, 1, y_min, 1000))
					commonParameters.dimensions.yMax = y_max;
				if(ImGui::DragInt("X min", &x_min, 1, 0, x_max))
					commonParameters.dimensions.xMin = x_min;
				if(ImGui::DragInt("Y min", &y_min, 1, 0, y_max))
					commonParameters.dimensions.yMin = y_min;
				if (ImGui::DragFloat("X step", &x_step, 0.1f, 0.1f, 1000))
					commonParameters.dimensions.xStep = x_step;
				if (ImGui::DragFloat("Y step", &y_step, 0.1f, 0.1f, 1000))
					commonParameters.dimensions.yStep = y_step;
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Annotations"))
			{
				if (ImGui::InputText("Title", titleBuffer, sizeof(titleBuffer)))
				{
					title = titleBuffer;
					commonParameters.annotations.title = title;
				}
				if (ImGui::InputText("X label", xLabelBuffer, sizeof(xLabelBuffer)))
				{
					x_label = xLabelBuffer;
					commonParameters.annotations.x_label = x_label;
				}
				if (ImGui::InputText("Y label", yLabelBuffer, sizeof(yLabelBuffer)))
				{
					y_label = yLabelBuffer;
					commonParameters.annotations.y_label = y_label;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Colormap"))
			{
				if (ImPlot::ColormapButton(ImPlot::GetColormapName(map), ImVec2(availableRegionSize.x - 90.0f, 0.0f), map))
				{
					map = (map + 1) % ImPlot::GetColormapCount();
					ImPlot::BustColorCache(uniquePlotID.c_str());
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Scale"))
			{
				ImGui::DragFloatRange2("Min / Max", &scaleMin, &scaleMax, 0.01f, -20, 20);
				heatmapParameters.scaleMin = scaleMin;
				heatmapParameters.scaleMax = scaleMax;
				ImGui::Checkbox("Auto scale", &autoScale);
				heatmapParameters.autoScale = autoScale;
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (autoScale)
		{
			auto [min_it, max_it] = std::minmax_element(flattened_matrix->begin(), flattened_matrix->end());
			heatmapParameters.scaleMin = *min_it;
			heatmapParameters.scaleMax = *max_it;
			scaleMin = static_cast<float>(heatmapParameters.scaleMin);
			scaleMax = static_cast<float>(heatmapParameters.scaleMax);
		}

		const int rows = static_cast<int>(static_cast<float>(y_max) / y_step);
		const int cols = static_cast<int>(static_cast<float>(x_max) / x_step);

		static constexpr ImPlotFlags hm_flags = ImPlotFlags_Crosshairs | ImPlotFlags_NoLegend;
		if (ImPlot::BeginPlot(uniquePlotID.c_str(), plotSize, hm_flags)) {
			ImPlot::PushColormap(map);
			static constexpr ImPlotAxisFlags flags = ImPlotAxisFlags_AutoFit;
			ImPlot::SetupAxes(commonParameters.annotations.x_label.c_str(), commonParameters.annotations.y_label.c_str(), flags, flags);

			const std::string& label = legends[0];

			ImPlot::PlotHeatmap(
				label.c_str(),
				flattened_matrix->data(),
				rows, cols,
				scaleMin, scaleMax, nullptr,
				ImPlotPoint(y_min, y_max), ImPlotPoint(x_max, x_min),
				hm_flags
			);
			ImPlot::EndPlot();
		}

		// Add color scale next to the heatmap
		ImGui::SameLine();
		ImPlot::ColormapScale("##HeatScale", scaleMin, scaleMax, ImVec2(60, plotSize.y));
		ImPlot::PopColormap();
	}

}
