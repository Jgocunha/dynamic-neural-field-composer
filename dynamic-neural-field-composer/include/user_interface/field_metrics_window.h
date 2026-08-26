#pragma once

#include <memory>
#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "application/application.h"

extern ImFont* g_BlackLargeFont;
extern ImFont* g_BoldLargeFont;
extern ImFont* g_BoldMediumFont;
extern ImFont* g_MediumIconsFont;

namespace dnf_composer::user_interface
{
	/// @brief Window showing per-field stability metrics (bump count, amplitude, etc.) as cards.
	class FieldMetricsWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation> simulation;

	public:
		/// @brief Construct the field metrics window for a simulation.
		/// @param simulation Simulation whose fields' metrics are displayed.
		explicit FieldMetricsWindow(const std::shared_ptr<Simulation>& simulation);

		FieldMetricsWindow(const FieldMetricsWindow&)            = delete;
		FieldMetricsWindow& operator=(const FieldMetricsWindow&) = delete;
		FieldMetricsWindow(FieldMetricsWindow&&)                 = delete;
		FieldMetricsWindow& operator=(FieldMetricsWindow&&)      = delete;

		/// @brief Draw the field metrics window for this frame.
		void render() override;
		/// @brief Draw a stability card for each neural field in @p simulation.
		/// @param simulation Simulation whose fields' metrics are drawn.
		static void renderContents(const std::shared_ptr<Simulation>& simulation);
		~FieldMetricsWindow() override = default;
	};
}
