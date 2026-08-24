
#pragma once

#include <algorithm>
#include <cmath>
#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "elements/element_factory.h"
#include "application/application.h"
#include "user_interface/widgets.h"
#include "user_interface/fonts/IconsFontAwesome6.h"
#include "tools/utils.h"

extern ImFont* g_BlackMediumFont;

enum CharSize : size_t
{
	CHAR_SIZE = 50
};

namespace dnf_composer::user_interface
{
	/// @brief Sidebar window for adding, removing and configuring a simulation's elements.
	class SimulationWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation> simulation;
		static int activePane;

		// Set when the Add button is pressed; consumed on the next frame, because
		// the parameter forms are rendered before the button that triggers them.
		mutable bool addRequestedNextFrame = false;

		// Message from the last failed add attempt, shown under the Add button.
		// Empty when the last attempt succeeded. See issue #146.
		mutable std::string lastAddElementError;
	public:
		/// @brief Construct the simulation window for a simulation.
		/// @param simulation Simulation whose elements are added to, removed and configured.
		explicit SimulationWindow(const std::shared_ptr<Simulation>& simulation);

		SimulationWindow(const SimulationWindow&) = delete;
		SimulationWindow& operator=(const SimulationWindow&) = delete;
		SimulationWindow(SimulationWindow&&) = delete;
		SimulationWindow& operator=(SimulationWindow&&) = delete;

		/// @brief Draw the simulation window for this frame.
		void render() override;
		/// @brief Draw the sidebar's icon strip and the currently selected content pane.
		void renderSidebarContents() const;
		/// @brief Draw the "Add element" content pane.
		void renderAddElementCard() const;
		/// @brief Draw the "Remove element" content pane.
		void renderRemoveElementCard() const;
		/// @brief Draw the "Set interaction" content pane.
		void renderSetInteractionCard() const;
		/// @brief Draw the "Data" content pane.
		void renderDataCard() const;
		/// @brief Draw the "Log element parameters" content pane.
		void renderLogElementParametersCard() const;
		/// @brief Draw the "Monitoring" content pane.
		void renderMonitoringCard() const;
		~SimulationWindow() override = default;
	private:
		static void drawIconStrip();
		static void renderContentPaneTitle();

		/// @brief Render the parameter form for @p selected, and create the element
		/// when @p addRequested. Split out of renderAddElementCard() so the creation
		/// path can be funnelled through tools::utils::describeElementCreationFailure().
		void renderElementParameters(element::ElementLabel selected, char* id, bool addRequested) const;

		/// @brief Render the Add button and any validation message from the last attempt.
		void renderAddElementButton() const;

		void addElementNeuralField(const char* id, bool addRequested) const;
		void addElementGaussStimulus(char* id, bool addRequested) const;
		void addElementTimedGaussStimulus(char* id, bool addRequested) const;
		void addElementTimedGaussStimulus2D(char* id, bool addRequested) const;
		void addElementGaussKernel(char* id, bool addRequested) const;
		void addElementMexicanHatKernel(char* id, bool addRequested) const;
		void addElementOscillatoryKernel(char* id, bool addRequested) const;
		void addElementAsymmetricGaussKernel(char* id, bool addRequested) const;
		void addElementNormalNoise(char* id, bool addRequested) const;
		void addElementCorrelatedNormalNoise(char* id, bool addRequested) const;
		void addElementFieldCoupling(char* id, bool addRequested) const;
		void addElementGaussFieldCoupling(char* id, bool addRequested) const;
		void addElementBoostStimulus(char* id, bool addRequested) const;
		void addElementMemoryTrace(char* id, bool addRequested) const;
		void addElementNeuralField2D(char* id, bool addRequested) const;
		void addElementGaussStimulus2D(char* id, bool addRequested) const;
		void addElementGaussKernel2D(char* id, bool addRequested) const;
		void addElementMexicanHatKernel2D(char* id, bool addRequested) const;
		void addElementNormalNoise2D(char* id, bool addRequested) const;
		void addElementOscillatoryKernel2D(char* id, bool addRequested) const;
		void addElementAsymmetricGaussKernel2D(char* id, bool addRequested) const;
		void addElementBoostStimulus2D(char* id, bool addRequested) const;
		void addElementCorrelatedNormalNoise2D(char* id, bool addRequested) const;
		void addElementMemoryTrace2D(char* id, bool addRequested) const;
		void addElementResize(char* id, bool addRequested) const;
		void addElementResize2D(char* id, bool addRequested) const;
		void addElementCollapse(char* id, bool addRequested) const;
		void addElementExpand(char* id, bool addRequested) const;
	};
}
