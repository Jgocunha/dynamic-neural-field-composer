#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "elements/gauss_stimulus.h"
#include "user_interface/widgets.h"
#include "application/application.h"

namespace dnf_composer::user_interface
{
	/// @brief Cached geometry and style for one "Add elements" grid panel.
	struct PanelScope
	{
		ImRect rect;          ///< Fixed rectangle of the panel (screen coords).
		float  ui{};           ///< UI scale factor the panel was laid out with.
		ImU32  fill{};          ///< Panel background fill colour.
		ImU32  border{};        ///< Panel border colour.
		float  rounding{};     ///< Corner rounding radius.
		ImVec2 pad;             ///< Inner padding between the border and the panel's content.
	};

	/// @brief Sidebar window for creating elements and editing the parameters of the focused one.
	class ElementWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation> simulation;
		static std::shared_ptr<element::Element> focusedElement;
	public:
		/// @brief Construct the element window for a simulation.
		/// @param simulation Simulation whose elements are listed, added to, and edited.
		explicit ElementWindow(const std::shared_ptr<Simulation>& simulation);

		ElementWindow(const ElementWindow&) = delete;
		ElementWindow& operator=(const ElementWindow&) = delete;
		ElementWindow(ElementWindow&&) = delete;
		ElementWindow& operator=(ElementWindow&&) = delete;

		/// @brief Draw the element window for this frame.
		void render() override;
		/// @brief Draw the "Add elements" card grid used to create new elements.
		void renderElementControlCard();
		/// @brief Draw the parameter-editing panel for the currently focused element.
		void renderModifyElementParameters();
		/// @brief Set the element whose parameters are shown for editing.
		/// @param element Element to switch the parameter panel to.
		/// @param simId   Id of the simulation @p element belongs to, when it differs from
		///                the window's own simulation (e.g. a coupled field in another sim).
		static void switchElementToModify(const std::shared_ptr<element::Element>& element, const std::string& simId = {});
		/// @brief Set the element highlighted as focused across the UI (e.g. from the node graph).
		/// @param element Element to focus.
		static void setFocusedElement(const std::shared_ptr<element::Element>& element);
		/// @brief Look up the display colour for an element type.
		/// @param label Element type to look up.
		/// @return The colour associated with @p label.
		static ImVec4 getColorForElementType(element::ElementLabel label);
		~ElementWindow() override = default;
	private:
		void renderIdentifiersSection(const std::shared_ptr<element::Element>& element) const;
		void renderDimensionControls(const std::shared_ptr<element::Element>& element) const;
		void renderDimensionControls2D(const std::shared_ptr<element::Element>& element) const;
		// Editable "Input dimensions" blocks. Read current input dims via the getter and
		// apply edits via the setter (which should sever connections + resize buffers).
		static void renderInputDimensionControls1D(const std::shared_ptr<element::Element>& element,
			const element::ElementDimensions& current,
			const std::function<void(const element::ElementDimensions&)>& apply) ;
		static void renderInputDimensionControls2D(const std::shared_ptr<element::Element>& element,
			const element::ElementDimensions& current,
			const std::function<void(const element::ElementDimensions&)>& apply) ;
		static void modifyElementNeuralField(const std::shared_ptr<element::Element>& element);
		static void modifyElementGaussStimulus(const std::shared_ptr<element::Element>& element);
		static void modifyElementFieldCoupling(const std::shared_ptr<element::Element>& element, const std::string& simId = {});
		static void modifyElementGaussKernel(const std::shared_ptr<element::Element>& element);
		static void modifyElementMexicanHatKernel(const std::shared_ptr<element::Element>& element);
		static void modifyElementNormalNoise(const std::shared_ptr<element::Element>& element);
		static void modifyElementCorrelatedNormalNoise(const std::shared_ptr<element::Element>& element);
		static void modifyElementGaussFieldCoupling(const std::shared_ptr<element::Element>& element);
		static void modifyElementOscillatoryKernel(const std::shared_ptr<element::Element>& element);
		static void modifyElementAsymmetricGaussKernel(const std::shared_ptr<element::Element>& element);
		static void modifyElementBoostStimulus(const std::shared_ptr<element::Element>& element);
		static void modifyElementMemoryTrace(const std::shared_ptr<element::Element>& element);
		static void modifyElementNeuralField2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementGaussStimulus2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementGaussKernel2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementMexicanHatKernel2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementNormalNoise2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementOscillatoryKernel2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementTimedGaussStimulus(const std::shared_ptr<element::Element>& element);
		static void modifyElementTimedGaussStimulus2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementBoostStimulus2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementCorrelatedNormalNoise2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementAsymmetricGaussKernel2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementMemoryTrace2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementResize(const std::shared_ptr<element::Element>& element);
		static void modifyElementResize2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementCollapse(const std::shared_ptr<element::Element>& element);
		static void modifyElementExpand(const std::shared_ptr<element::Element>& element);
		static std::string getElementTypeDisplayName(element::ElementLabel label);
		static PanelScope beginElementPanel(const ImVec4& baseColor, const ImVec2& size);
		static void endElementPanel(const PanelScope& p);
	};
}
