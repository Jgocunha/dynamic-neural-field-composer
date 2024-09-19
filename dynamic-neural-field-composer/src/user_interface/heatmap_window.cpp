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
            static bool open = true;

            if (ImGui::Begin("Inter-field synaptic connections", &open, ImGuiWindowFlags_NoCollapse))
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
            //std::shared_ptr<element::GaussFieldCoupling> coupling;
            std::shared_ptr<element::FieldCoupling> coupling;

            for (const auto& element : simulation->getElements())
            {
                if (element->getLabel() == element::FIELD_COUPLING)
                {
                    coupling = std::dynamic_pointer_cast<element::FieldCoupling>(element);
                }
            }

            //const std::vector<double> flattened_matrix = coupling->getComponent("kernel");
            const std::vector<double> flattened_matrix = dnf_composer::tools::math::flattenMatrix(coupling->getWeights());
            //static const int rows = coupling->getParameters().inputFieldDimensions.size;
            static const int rows = 720;
            static const int cols = coupling->getElementCommonParameters().dimensionParameters.size;
            static const int x_max = coupling->getElementCommonParameters().dimensionParameters.x_max;
            static const int y_max = 360;

            static constexpr ImPlotFlags hm_flags = ImPlotFlags_Crosshairs | ImPlotFlags_NoLegend;
            //ImPlot::SetNextAxesToFit();
            ImVec2 plotSize = ImGui::GetContentRegionAvail();  // Get available size in the ImGui window
            plotSize.x -= 90.0f; // Subtract some padding
            plotSize.y -= 5.0f; // Subtract some padding

            if (ImPlot::BeginPlot("##Heatmap1", plotSize, hm_flags))
            {
                static constexpr ImPlotAxisFlags flags = ImPlotAxisFlags_AutoFit;// | ImPlotAxisFlags_NoLabel;
                ImPlot::SetupAxes("Output field spatial dimension", "Perceptual field spatial dimension", flags, flags);
                ImPlot::PlotHeatmap("Heatmap Data", flattened_matrix.data(), rows, cols,
                    parameters.scaleMin, parameters.scaleMax, nullptr,
                    ImPlotPoint(0, 0), ImPlotPoint(x_max, y_max), hm_flags);
                ImPlot::EndPlot();
            }

            ImGui::SameLine();
            ImPlot::ColormapScale("##HeatScale", parameters.scaleMin, parameters.scaleMax, ImVec2(80, plotSize.y));

            ImPlot::PopColormap();
        }

	}
}
