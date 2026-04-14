#include "user_interface/plot_control_window.h"


namespace dnf_composer::user_interface
{
	PlotControlWindow::PlotControlWindow(const std::shared_ptr<Visualization>& visualization)
		:visualization(visualization), simulation(visualization->getSimulation())
	{}

	void PlotControlWindow::renderContent()
	{
		// Add a new plot button
		ImGui::Text("Add a new plot:"); ImGui::SameLine();
			static auto selectedPlotType = PlotType::LINE_PLOT;
			ImGui::SetNextItemWidth(150.0f);
			ImGui::Combo("##PlotType", reinterpret_cast<int*>(&selectedPlotType), "Line plot\0Heatmap\0\0");
			ImGui::SameLine();
			if (ImGui::Button("Add"))
			{
				visualization->plot(selectedPlotType);
			}

			const float availWidth = ImGui::GetContentRegionAvail().x;
			const float idColWidth       = availWidth * 0.050f;
			const float typeColWidth     = availWidth * 0.100f;
			const float dataColWidth     = availWidth * 0.500f;
			const float changeColWidth   = availWidth * 0.250f;

			if (ImGui::BeginTable("PlotControlTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				// Set column headers
				ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, idColWidth);
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, typeColWidth);
				ImGui::TableSetupColumn("Data", ImGuiTableColumnFlags_WidthFixed, dataColWidth);
				ImGui::TableSetupColumn("Change plotted data", ImGuiTableColumnFlags_WidthFixed, changeColWidth);
				ImGui::TableHeadersRow();

				// Get plots and sort them by ID
				const auto& plots = visualization->getPlots();
				std::vector<std::pair<std::shared_ptr<Plot>, std::vector<std::pair<std::string, std::string>>>>
					sortedPlots(plots.begin(), plots.end());

				std::ranges::sort(sortedPlots,
				                  [](const auto& a, const auto& b) {
					                  return a.first->getUniqueIdentifier() < b.first->getUniqueIdentifier();
				                  });

				for (const auto& plot : sortedPlots)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TextUnformatted(std::to_string(plot.first->getUniqueIdentifier()).c_str());
					ImGui::TableSetColumnIndex(1);
					ImGui::TextUnformatted(PlotTypeToString.at(plot.first->getType()).c_str());

					const std::string listbox_label = "##" + std::to_string(plot.first->getUniqueIdentifier());
					ImGui::TableSetColumnIndex(2);
					if (ImGui::BeginListBox(listbox_label.c_str(), ImVec2(300, 25)))
					{
						for (const auto& data : plot.second)
						{
							ImGui::TextUnformatted((data.first + " " + data.second).c_str());
						}
						ImGui::EndListBox();
					}

					// Add data button and popup
					ImGui::TableSetColumnIndex(3);
					const std::string popup_id = "AddDataPopup_" + std::to_string(plot.first->getUniqueIdentifier());
					ImGui::PushFont(g_MediumIconsFont);
					if (ImGui::Button((ICON_FA_CIRCLE_PLUS  "##add" + std::to_string(plot.first->getUniqueIdentifier())).c_str()))
					{
						ImGui::OpenPopup(popup_id.c_str());
					}
					ImGui::PopFont();

					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Add data");

					if (ImGui::BeginPopup(popup_id.c_str()))
					{
						ImGui::Text("Select data to add:");
						ImGui::Separator();
						for (const auto& element : simulation->getElements())
						{
							for (const auto* components = element->getComponents();
								const auto& name : *components | std::views::keys)
							{
								const std::string item_label = element->getUniqueName() + " " + name;
								if (ImGui::Selectable(item_label.c_str()))
								{
									visualization->plot(plot.first->getUniqueIdentifier(), element->getUniqueName(), name);
									ImGui::CloseCurrentPopup();
								}
							}
						}
						ImGui::EndPopup();
					}

					ImGui::SameLine();
					// Remove data button and popup
					const std::string remove_popup_id = "RemoveDataPopup_" + std::to_string(plot.first->getUniqueIdentifier());
					ImGui::PushFont(g_MediumIconsFont);
					if (ImGui::Button((ICON_FA_CIRCLE_MINUS  "##remove" + std::to_string(plot.first->getUniqueIdentifier())).c_str()))
					{
						ImGui::OpenPopup(remove_popup_id.c_str());
					}
					ImGui::PopFont();
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove data");

					if (ImGui::BeginPopup(remove_popup_id.c_str()))
					{
						ImGui::Text("Select data to remove:");
						ImGui::Separator();
						for (const auto& data : plot.second)
						{
							const std::string item_label = data.first + " " + data.second;
							if (ImGui::Selectable(item_label.c_str()))
							{
								visualization->removePlottingDataFromPlot(plot.first->getUniqueIdentifier(), data);
								ImGui::CloseCurrentPopup();
							}
						}
						ImGui::EndPopup();
					}

					ImGui::SameLine();
					// Remove the plot button
					ImGui::PushFont(g_MediumIconsFont);
					if (ImGui::Button((ICON_FA_TRASH "##removeplot" + std::to_string(plot.first->getUniqueIdentifier())).c_str()))
					{
						visualization->removePlot(plot.first->getUniqueIdentifier());
					}
					ImGui::PopFont();
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove plot");
				}

			}
			ImGui::EndTable();
	}

	void PlotControlWindow::render()
	{
		ImGui::PushFont(g_BlackLargeFont);
		const bool open = ImGui::Begin("Plot Control", nullptr, imgui_kit::getGlobalWindowFlags());
		ImGui::PopFont();
		if (open)
			renderContent();
		ImGui::End();
	}
}

