#include "user_interface/heatmap_window.h"


namespace dnf_composer
{
	namespace user_interface
	{
        HeatmapWindow::HeatmapWindow(const std::shared_ptr<Simulation>& simulation)
			: simulation(simulation)
		{
		}

		void HeatmapWindow::render()
		{
            bool open = true;

            if (ImGui::Begin("Heatmap", &open, ImGuiWindowFlags_NoCollapse))
            {
                renderColormap();
                renderPlot();
            }
            ImGui::End();

            if (!open)
                log(tools::logger::LogLevel::INFO, "Heatmap window closed.");
		}


        void HeatmapWindow::renderColormap()
        {
            static ImPlotColormap map = ImPlotColormap_Jet;
            if (ImPlot::ColormapButton(ImPlot::GetColormapName(map), ImVec2(225, 0), map)) 
            {
                map = (map + 1) % ImPlot::GetColormapCount();
                // We bust the color cache of our plots so that item colors will
                // resample the new colormap in the event that they have already
                // been created. See documentation in implot.h.
                ImPlot::BustColorCache("##Heatmap1");
            }

            ImGui::SameLine();
            ImGui::LabelText("##Colormap Index", "%s", "Change Colormap");
            ImGui::SetNextItemWidth(225);
            ImGui::DragFloatRange2("Min / Max", &parameters.scaleMin, &parameters.scaleMax, 0.01f, -20, 20);

            ImPlot::PushColormap(map);
        }

        void HeatmapWindow::renderPlot()
        {
            std::shared_ptr<element::GaussFieldCoupling> coupling;

            for (const auto& element : simulation->getElements())
            {
                if (element->getLabel() == element::GAUSS_FIELD_COUPLING)
                {
                    coupling = std::dynamic_pointer_cast<element::GaussFieldCoupling>(element);
                }
            }

            const std::vector<double> flattened_matrix = coupling->getComponent("kernel");
            static constexpr int rows = 50;
            static constexpr int cols = 50;

            static constexpr ImPlotFlags hm_flags = ImPlotFlags_Crosshairs | ImPlotFlags_NoLegend;
            ImPlot::SetNextAxesToFit();
            ImVec2 plotSize = ImGui::GetContentRegionAvail();  // Get available size in the ImGui window
            plotSize.x -= 60.0f; // Subtract some padding
            plotSize.y -= 5.0f; // Subtract some padding

            if (ImPlot::BeginPlot("##Heatmap1", plotSize, hm_flags))
            {
                ImPlot::PlotHeatmap("Heatmap Data", flattened_matrix.data(), rows, cols,
                    parameters.scaleMin, parameters.scaleMax, nullptr,
                    ImPlotPoint(0, 0), ImPlotPoint(rows, cols), hm_flags);
                ImPlot::EndPlot();
            }

            ImGui::SameLine();
            ImPlot::ColormapScale("##HeatScale", parameters.scaleMin, parameters.scaleMax, ImVec2(60, plotSize.y));

            ImPlot::PopColormap();
        }

	}
}
