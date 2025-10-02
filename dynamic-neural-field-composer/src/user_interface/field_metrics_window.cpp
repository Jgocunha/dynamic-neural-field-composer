#include "./user_interface/field_metrics_window.h"

namespace dnf_composer
{
	namespace user_interface
	{
		FieldMetricsWindow::FieldMetricsWindow(const std::shared_ptr<Simulation>& simulation)
			: simulation(simulation)
		{
		}

		void FieldMetricsWindow::render()
		{
			if (ImGui::Begin("Neural Field Monitoring", nullptr, imgui_kit::getGlobalWindowFlags()))
			{
				ImGui::Text("Overview of Neural Fields:");
				getNeuralFieldsAndRenderCentroids();
			}
			ImGui::End();
		}

		void FieldMetricsWindow::getNeuralFieldsAndRenderCentroids() const
		{
			ImGui::BeginTabBar("NeuralFieldTabs");

			for(const auto& element : simulation->getElements())
			{
				if (element->getLabel() == element::NEURAL_FIELD)
				{
					const auto neuralField = std::dynamic_pointer_cast<element::NeuralField>(element);
					const std::string elementName = neuralField->getUniqueName();

					if (ImGui::BeginTabItem(elementName.c_str()))
					{
						renderNeuralFieldDetails(neuralField);
						ImGui::EndTabItem();
					}
				}
			}


			ImGui::EndTabBar();
		}

		void FieldMetricsWindow::renderNeuralFieldDetails(const std::shared_ptr<element::NeuralField>& neuralField)
		{
			//const double centroid = neuralField->getCentroid();
			const bool stable = neuralField->isStable();
			const double lowestActivation = neuralField->getLowestActivation();
			const double highestActivation = neuralField->getHighestActivation();
			const std::vector<element::NeuralFieldBump> bumps = neuralField->getBumps();
			//const double selfStabilized = neuralField->isSelfStabilized();
			//const double selfSustained = neuralField->isSelfSustained();

			//ImGui::Text("Cumulative centroid: %.2f", centroid);
			ImGui::TextColored(stable ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1), "Stability: %s", stable ? "Stable" : "Unstable");
			ImGui::Text("Lowest Activation: %.2f", lowestActivation);
			ImGui::Text("Highest Activation: %.2f", highestActivation);
			ImGui::Text("Number of Bumps: %d", static_cast<int>(bumps.size()));
			//ImGui::Text(selfStabilized ? "Self-stabilized" : "Not self-stabilized");
			//ImGui::Text(selfSustained ? "Self-sustained" : "Not self-sustained");
			ImGui::Separator();

			for (size_t j = 0; j < bumps.size(); j++)
			{
				const element::NeuralFieldBump& bump = bumps[j];
				ImGui::Text("Bump %d: Start %.2f, End %.2f, Amplitude %.2f, Width %.2f, Position %.2f, Velocity %.2f, Acceleration %.2f", 
					j, bump.startPosition, bump.endPosition, bump.amplitude, bump.width, bump.centroid, bump.velocity, bump.acceleration);
			}
		}
	}
}
