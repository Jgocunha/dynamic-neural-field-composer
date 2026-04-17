#pragma once

#include <vector>
#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "application/application.h"

extern ImFont* g_BlackLargeFont;
extern ImFont* g_BoldLargeFont;
extern ImFont* g_BoldMediumFont;

namespace dnf_composer::user_interface
{
	class FieldMetricsWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation> simulation;

	public:
		explicit FieldMetricsWindow(const std::shared_ptr<Simulation>& simulation);

		FieldMetricsWindow(const FieldMetricsWindow&)            = delete;
		FieldMetricsWindow& operator=(const FieldMetricsWindow&) = delete;
		FieldMetricsWindow(FieldMetricsWindow&&)                 = delete;
		FieldMetricsWindow& operator=(FieldMetricsWindow&&)      = delete;

		void render() override;
		void renderCards() const;
		~FieldMetricsWindow() override = default;

	private:
		static void renderCardContent(const std::shared_ptr<element::NeuralField>& nf,
		                              const std::vector<element::NeuralFieldBump>&  bumps);
	};
}
