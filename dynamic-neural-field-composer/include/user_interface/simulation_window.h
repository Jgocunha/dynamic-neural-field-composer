
#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "elements/element_factory.h"

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
	public:
		explicit SimulationWindow(const std::shared_ptr<Simulation>& simulation);

		SimulationWindow(const SimulationWindow&) = delete;
		SimulationWindow& operator=(const SimulationWindow&) = delete;
		SimulationWindow(SimulationWindow&&) = delete;
		SimulationWindow& operator=(SimulationWindow&&) = delete;

		void render() override;
		void render(const ImRect& bounds, bool* p_open = nullptr) const;
		void renderAddElementCard() const;
		void renderRemoveElementCard() const;
		void renderSetInteractionCard() const;
		void renderExportElementComponentCard() const;
		void renderLogElementParametersCard() const;
		~SimulationWindow() override = default;
	private:
		void renderPanelContents() const;
		void renderSimulationControlButtons() const;
		void renderSimulationProperties() const;
		void renderAddElement() const;
		void renderSetInteraction() const;
		void renderRemoveElement() const;
		void renderElementProperties(const std::pair<int, std::string>& elementId) const;
		void renderLogElementProperties() const;
		void renderExportElementComponents() const;

		// unfortunately, these functions need to be here like this
		//                .=-.-.   _ __
		//    .-.,.---.  /==/_ /.-`.' ,`.
		//   /==/  `   \|==|, |/==/, -   \
		//  |==|-, .=., |==|  |==| _ .=. |
		//  |==|   '='  /==|- |==| , '=',|
		//  |==|- ,   .'|==| ,|==|-  '..'
		//  |==|_  . ,'.|==|- |==|,  |
		//  /==/  /\ ,  )==/. /==/ - |
		//  `--`-`--`--'`--`-``--`---'
		void addElementNeuralField() const;
		void addElementGaussStimulus() const;
		void addElementFieldCoupling() const;
		void addElementGaussKernel() const;
		void addElementMexicanHatKernel() const;
		void addElementNormalNoise() const;
		void addElementGaussFieldCoupling() const;
	};
}

