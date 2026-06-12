
#pragma once

#include <algorithm>
#include <cmath>
#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "elements/element_factory.h"
#include "application/application.h"
#include "user_interface/widgets.h"
#include "user_interface/fonts/IconsFontAwesome6.h"

extern ImFont* g_BlackLargeFont;
extern ImFont* g_BlackMediumFont;
extern ImFont* g_MediumIconsFont;

enum CharSize : size_t
{
	CHAR_SIZE = 50
};

namespace dnf_composer::user_interface
{
	class SimulationWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation> simulation;
		static int activePane;
	public:
		explicit SimulationWindow(const std::shared_ptr<Simulation>& simulation);

		SimulationWindow(const SimulationWindow&) = delete;
		SimulationWindow& operator=(const SimulationWindow&) = delete;
		SimulationWindow(SimulationWindow&&) = delete;
		SimulationWindow& operator=(SimulationWindow&&) = delete;

		void render() override;
		void renderSidebarContents() const;
		void renderAddElementCard() const;
		void renderRemoveElementCard() const;
		void renderSetInteractionCard() const;
		void renderDataCard() const;
		void renderLogElementParametersCard() const;
		void renderMonitoringCard() const;
		~SimulationWindow() override = default;
	private:
		static void drawIconStrip();
		static void renderContentPaneTitle();
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
		void addElementUnsupervisedFieldCoupling(char* id, bool addRequested) const;
		void addElementSupervisedFieldCoupling(char* id, bool addRequested) const;
	};
}
