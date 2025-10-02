#include "visualization/lineplot.h"

namespace dnf_composer
{
	LinePlotParameters::LinePlotParameters()
		: lineThickness(3.0), autoFit(true)
	{}

	LinePlotParameters::LinePlotParameters(double lineThickness, bool autoFit)
		: lineThickness(lineThickness), autoFit(autoFit)
	{}

	std::string LinePlotParameters::toString() const
	{
		std::string result;
		result += "Line plot parameters: {";
		result += "Thickness: " + std::to_string(lineThickness) + ", ";
		result += "Auto-fit: " + std::string(autoFit ? "true" : "false") + "}";
		return result;
	}

	bool LinePlotParameters::operator==(const LinePlotParameters& other) const
	{
		static constexpr double epsilon = 1e-6;
		if (std::abs(lineThickness - other.lineThickness) > epsilon || autoFit != other.autoFit)
			return false;
		return true;
	}


	LinePlot::LinePlot(const PlotCommonParameters& parameters, const LinePlotParameters& linePlotParameters)
		: Plot(parameters), linePlotParameters(linePlotParameters)
	{
		if (commonParameters.type != PlotType::LINE_PLOT)
			throw std::invalid_argument("LinePlot: parameters.plotType must be PlotType::Line.");
	}

	void LinePlot::setLineThickness(double lineThickness)
	{
		linePlotParameters.lineThickness = lineThickness;
	}

	void LinePlot::setAutoFit(bool autoFit)
	{
		linePlotParameters.autoFit = autoFit;
	}

	double LinePlot::getLineThickness() const
	{
		return linePlotParameters.lineThickness;
	}

	double LinePlot::getAutoFit() const
	{
		return linePlotParameters.autoFit;
	}

	std::string LinePlot::toString() const
	{
		std::ostringstream result;
		result << "Plot: { ";
		result << "Unique identifier: " << uniqueIdentifier << ", ";
		result << commonParameters.toString() << ", ";
		result << linePlotParameters.toString();
		return result.str();
	}

	void LinePlot::render(const std::vector<std::vector<double>*>& data, const std::vector<std::string>& legends)
	{
        static constexpr double safeMargin = 0.01;
		bool whereDimensionsChangedByUser = false;

        const ImVec2 availableRegionSize = ImGui::GetContentRegionAvail();
        const ImVec2 plotSize = ImVec2(availableRegionSize.x - 5.0f, availableRegionSize.y - 5.0f);

        ImPlotFlags flags = ImPlotFlags_Crosshairs;
        const std::string uniquePlotID = commonParameters.annotations.title + "##" + std::to_string(uniqueIdentifier);

		auto x_max = static_cast<float>(commonParameters.dimensions.xMax);
		auto x_min = static_cast<float>(commonParameters.dimensions.xMin);
		auto y_max = static_cast<float>(commonParameters.dimensions.yMax);
		auto y_min = static_cast<float>(commonParameters.dimensions.yMin);
		auto x_step = static_cast<float>(commonParameters.dimensions.xStep);
		auto lineWeight = static_cast<float>(linePlotParameters.lineThickness);
		bool autoFit = linePlotParameters.autoFit;

		std::string title = commonParameters.annotations.title;
		std::string x_label = commonParameters.annotations.x_label;
		std::string y_label = commonParameters.annotations.y_label;
		char titleBuffer[128];
		char xLabelBuffer[128];
		char yLabelBuffer[128];

		snprintf(titleBuffer, sizeof(titleBuffer), "%s", title.c_str());
		snprintf(xLabelBuffer, sizeof(xLabelBuffer), "%s", x_label.c_str());
		snprintf(yLabelBuffer, sizeof(yLabelBuffer), "%s", y_label.c_str());

        if (ImGui::BeginMenuBar())
        {
			if (ImGui::BeginMenu("Dimensions"))
			{
				if (ImGui::DragFloat("X max", &x_max, 0.1f, x_min, 1000))
				{
					commonParameters.dimensions.xMax = x_max;
					whereDimensionsChangedByUser = true;
				}
				if (ImGui::DragFloat("Y max", &y_max, 0.1f, y_min, 1000))
				{
					commonParameters.dimensions.yMax = y_max;
					whereDimensionsChangedByUser = true;
				}
				if (ImGui::DragFloat("X min", &x_min, 0.1f, -1000, x_max))
				{
					commonParameters.dimensions.xMin = x_min;
					whereDimensionsChangedByUser = true;
				}
				if (ImGui::DragFloat("Y min", &y_min, 0.1f, -10000, y_max))
				{
					commonParameters.dimensions.yMin = y_min;
					whereDimensionsChangedByUser = true;
				}
				if (ImGui::DragFloat("X step", &x_step, 0.1f, 0.1f, 1000))
				{
					commonParameters.dimensions.xStep = x_step;
					whereDimensionsChangedByUser = true;
				}

				if (ImGui::Checkbox("Auto-fit", &autoFit))
					linePlotParameters.autoFit = autoFit;
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
                ImPlot::ShowColormapSelector("##ColormapSelector");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Line Thickness"))
            {
                ImGui::SliderFloat("##LineWeight", &lineWeight, 0.1f, 10.0f);
				linePlotParameters.lineThickness = lineWeight;
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

		ImPlotStyle& style = ImPlot::GetStyle();
		style.LineWeight = static_cast<float>(linePlotParameters.lineThickness);

        if (linePlotParameters.autoFit) {
            flags |= ImPlotFlags_Equal;
        }
        else {
            ImPlot::SetNextAxesLimits(
				commonParameters.dimensions.xMin - safeMargin,
				commonParameters.dimensions.xMax + safeMargin,
				commonParameters.dimensions.yMin - safeMargin,
				commonParameters.dimensions.yMax + safeMargin
            );
        }

        if (!ImPlot::BeginPlot(uniquePlotID.c_str(), plotSize, flags)) {
            return; 
        }

        if (linePlotParameters.autoFit) {
            ImPlot::SetupAxes(
				commonParameters.annotations.x_label.c_str(),
				commonParameters.annotations.y_label.c_str(),
                ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit
            );
        }
        else {
            ImPlot::SetupAxes(commonParameters.annotations.x_label.c_str(), commonParameters.annotations.y_label.c_str());
            if (whereDimensionsChangedByUser) {
                auto* currentPlot = ImPlot::GetCurrentPlot();
                if (currentPlot) {
                    currentPlot->Axes[0].Range.Min = commonParameters.dimensions.xMin - safeMargin;
                    currentPlot->Axes[0].Range.Max = commonParameters.dimensions.xMax + safeMargin;
                    currentPlot->Axes[3].Range.Min = commonParameters.dimensions.yMin - safeMargin;
                    currentPlot->Axes[3].Range.Max = commonParameters.dimensions.yMax + safeMargin;
                }
            }
        }

        ImPlot::SetupLegend(ImPlotLocation_SouthWest, ImPlotLegendFlags_None);

		for (size_t j = 0; j < data.size(); ++j) 
		{
			const std::string& label = legends[j];
            const std::vector<double>& line_data = *data[j];

            std::vector<double> shiftedXValues(line_data.size());
            for (size_t i = 0; i < line_data.size(); ++i) 
			{
                shiftedXValues[i] = (i + 1) * commonParameters.dimensions.xStep;
            }

            ImPlot::PlotLine(label.c_str(), shiftedXValues.data(), line_data.data(), static_cast<int>(line_data.size()));
        }

		ImPlot::EndPlot();
	}

}
