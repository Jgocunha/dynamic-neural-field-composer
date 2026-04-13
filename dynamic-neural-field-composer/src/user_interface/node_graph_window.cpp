// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "user_interface/node_graph_window.h"

extern ImFont* g_BoldFont;
extern ImFont* g_MediumFont;
extern ImFont* g_BlackFont;

namespace dnf_composer::user_interface
{
	NodeGraphWindow::NodeGraphWindow(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{
		config.SettingsFile = "imnode-window.json";
		context = ImNodeEditor::CreateEditor(&config);
	}

	void NodeGraphWindow::render()
	{
		const ImGuiWindowFlags flags = imgui_kit::getGlobalWindowFlags()
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoScrollWithMouse;

		ImGui::PushFont(g_BlackFont);
		const bool open = ImGui::Begin("Node graph", nullptr, flags);
		ImGui::PopFont();

		if (open)
		{
			widgets::renderHelpMarker(
				"Visualize elements and their interactions.\n"
				"Drag from an Output pin to an Input pin to create a connection.\n"
				"Double-click a link to remove it.");

			ImNodeEditor::SetCurrentEditor(context);
			applyCanvasStyle();
			ImNodeEditor::Begin("dnf-composer-graph");
			renderElementNodes();
			handleInteractions();
			ImNodeEditor::End();
			restoreCanvasStyle();
			ImNodeEditor::SetCurrentEditor(nullptr);
		}
		ImGui::End();
	}

	void NodeGraphWindow::renderGraph() const
	{
		ImNodeEditor::SetCurrentEditor(context);
		applyCanvasStyle();
		ImNodeEditor::Begin("dnf-composer-graph");
		renderElementNodes();
		handleInteractions();
		ImNodeEditor::End();
		restoreCanvasStyle();
		ImNodeEditor::SetCurrentEditor(nullptr);
	}

	void NodeGraphWindow::applyCanvasStyle()
	{
		// Light background that matches the rest of the ImGui windows
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_Bg,
			ImVec4(0.94f, 0.95f, 0.96f, 1.00f));
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_Grid,
			ImVec4(0.80f, 0.82f, 0.85f, 0.60f));
	}

	void NodeGraphWindow::restoreCanvasStyle()
	{
		ImNodeEditor::PopStyleColor(2);
	}

	// -------------------------------------------------------------------------
	// Node rendering
	// -------------------------------------------------------------------------

	void NodeGraphWindow::renderElementNodes() const
	{
		for (const auto& element : simulation->getElements())
			renderElementNode(element);

		for (const auto& element : simulation->getElements())
			renderElementNodeConnections(element);
	}

	void NodeGraphWindow::renderElementNode(const std::shared_ptr<element::Element>& element)
	{
		namespace util = ax::NodeEditor::Utilities;
		using ax::Widgets::IconType;

		const ImNodeEditor::NodeId nodeId = getNodeId(element);
		const ImU32  headerU32  = getHeaderColorForElementType(element->getLabel());
		const ImVec4 headerVec4 = ImGui::ColorConvertU32ToFloat4(headerU32);

		// Uniform pastel: blend element colour 35 % toward white
		const ImVec4 bodyVec4 = {
			headerVec4.x * 0.35f + 0.65f,
			headerVec4.y * 0.35f + 0.65f,
			headerVec4.z * 0.35f + 0.65f,
			1.0f
		};

		// No border, uniform colour, tighter top padding, so the title sits higher
		ImNodeEditor::PushStyleVar(ImNodeEditor::StyleVar_NodeRounding,    10.0f);
		ImNodeEditor::PushStyleVar(ImNodeEditor::StyleVar_NodeBorderWidth, 0.0f);
		ImNodeEditor::PushStyleVar(ImNodeEditor::StyleVar_NodePadding,     ImVec4(8, 4, 8, 6));
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_NodeBg,     bodyVec4);
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_NodeBorder, bodyVec4);

		util::BlueprintNodeBuilder builder;
		builder.Begin(nodeId);

		// ----- "HEADER" section used for the full node body ------------------
		// Using the same pastel colour as NodeBg so the node is visually uniform.
		// Putting all content here forces Input/Output pins to appear below it.
		builder.Header(bodyVec4);
		{
			// Minimum width + title
			static constexpr float minNodeSize = 250.0f;
			ImGui::Dummy(ImVec2(minNodeSize, 0));
			ImGui::PushFont(g_BlackFont);
			ImGui::TextUnformatted(element->getUniqueName().c_str());
			ImGui::PopFont();

			// Info icon — hover shows parameters
			ImGui::Spacing();
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_CIRCLE_INFO);
			ImGui::PopFont();
			if (ImGui::IsItemHovered())
			{
				ImNodeEditor::Suspend();
				renderElementTooltip(element);
				ImNodeEditor::Resume();
			}
			ImGui::Spacing();

			// ---- Pin row: Input (flush left) | Output (flush right) ----
			constexpr ImVec4 pinColor = ImVec4(1.0f, 1.0f, 1.0f, 0.90f);
			constexpr ImVec2 iconSize = ImVec2(14, 14);

			if (ImGui::BeginTable("##pins", 2, ImGuiTableFlags_None, ImVec2(minNodeSize, 0.f)))
			{
				ImGui::TableSetupColumn("##in",  ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("##out", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableNextRow();

				// Left cell: Input pin
				ImGui::TableSetColumnIndex(0);
				ImNodeEditor::BeginPin(startingInputPinId + element->getUniqueIdentifier(),
				                       ImNodeEditor::PinKind::Input);
				ImNodeEditor::PinPivotAlignment(ImVec2(0.0f, 0.5f));
				ax::Widgets::Icon(iconSize, IconType::Circle, true, pinColor, ImVec4(0,0,0,0));
				ImGui::SameLine(0, 4);
				ImGui::TextUnformatted("Input");
				ImNodeEditor::EndPin();

				// Right cell: Output pin (right-aligned within cell)
				ImGui::TableSetColumnIndex(1);
				ImNodeEditor::BeginPin(startingOutputPinId + element->getUniqueIdentifier(),
				                       ImNodeEditor::PinKind::Output);
				ImNodeEditor::PinPivotAlignment(ImVec2(1.0f, 0.5f));
				{
					const float avail  = ImGui::GetContentRegionAvail().x;
					const float needed = ImGui::CalcTextSize("Output").x + 4.0f + iconSize.x;
					if (avail > needed)
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - needed);
				}
				ImGui::TextUnformatted("Output");
				ImGui::SameLine(0, 4);
				ax::Widgets::Icon(iconSize, IconType::Circle, true, pinColor, ImVec4(0,0,0,0));
				ImNodeEditor::EndPin();

				ImGui::EndTable();
			}
		}
		builder.EndHeader();

		builder.End();

		ImNodeEditor::PopStyleColor(2);
		ImNodeEditor::PopStyleVar(3);
	}

	void NodeGraphWindow::renderElementNodeConnections(const std::shared_ptr<element::Element>& element)
	{
		constexpr float    thickness = 2.0f;
		constexpr ImVec4   linkCol   = ImVec4(0.08f, 0.08f, 0.08f, 0.85f); // near-black

		for (const auto& input : element->getInputs())
		{
			const std::string idStr =
				std::to_string(element->getUniqueIdentifier()) +
				std::to_string(input->getUniqueIdentifier());
			const size_t linkId = std::stoull(idStr) + startingLinkId;

			ImNodeEditor::Link(
				linkId,
				input->getUniqueIdentifier()   + startingOutputPinId,
				element->getUniqueIdentifier() + startingInputPinId,
				linkCol, thickness);
		}
	}

	// -------------------------------------------------------------------------
	// Interaction handling
	// -------------------------------------------------------------------------

	void NodeGraphWindow::handleInteractions() const
	{
		handlePinInteractions();
		handleLinkInteractions();
	}

	void NodeGraphWindow::handlePinInteractions() const
	{
		static bool isAttemptingConnection = false;
		static ImNodeEditor::PinId outputPinId = 0;

		if (ImNodeEditor::GetHoveredPin() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			outputPinId = ImNodeEditor::GetHoveredPin();
			isAttemptingConnection = true;
			return;
		}

		if (ImNodeEditor::GetHoveredPin() && isAttemptingConnection)
		{
			const int srcId  = static_cast<int>(outputPinId.Get()) - startingOutputPinId;
			const int dstId  = static_cast<int>(ImNodeEditor::GetHoveredPin().Get()) - startingInputPinId;
			const int maxIdx = simulation->getHighestElementIndex();

			if (srcId < 0 || dstId < 0 || srcId > maxIdx || dstId > maxIdx)
			{
				isAttemptingConnection = false;
				return;
			}

			simulation->createInteraction(
				simulation->getElement(srcId)->getUniqueName(), "output",
				simulation->getElement(dstId)->getUniqueName());

			isAttemptingConnection = false;
		}
	}

	void NodeGraphWindow::handleLinkInteractions() const
	{
		const ImNodeEditor::LinkId clicked = ImNodeEditor::GetDoubleClickedLink();
		if (!clicked) return;

		ImNodeEditor::PinId startPin, endPin;
		GetLinkPins(clicked, &startPin, &endPin);

		const int srcId  = static_cast<int>(startPin.Get()) - startingOutputPinId;
		const int dstId  = static_cast<int>(endPin.Get())   - startingInputPinId;
		const int maxIdx = simulation->getHighestElementIndex();

		if (srcId < 0 || dstId < 0 || srcId > maxIdx || dstId > maxIdx) return;

		simulation->getElement(dstId)->removeInput(srcId);
	}

	// -------------------------------------------------------------------------
	// Helpers
	// -------------------------------------------------------------------------

	size_t NodeGraphWindow::getNodeId(const std::shared_ptr<element::Element>& element)
	{
		return std::hash<std::string>{}(element->getUniqueName());
	}

	void NodeGraphWindow::renderElementTooltip(const std::shared_ptr<element::Element>& element)
	{
		ImGui::BeginTooltip();

		const auto& cp = element->getElementCommonParameters();
		ImGui::Text("Name: %s", cp.identifiers.uniqueName.c_str());
		ImGui::Text("Type: %s", element::ElementLabelToString.at(cp.identifiers.label).c_str());
		ImGui::Text("Size: %d", cp.dimensionParameters.size);
		ImGui::Text("Step: %.2f", cp.dimensionParameters.d_x);
		ImGui::Separator();

		switch (element->getLabel())
		{
		case element::ElementLabel::NEURAL_FIELD:
		{
			const auto nf = std::dynamic_pointer_cast<element::NeuralField>(element);
			const auto& p = nf->getParameters();
			ImGui::Text("Tau: %.2f", p.tau);
			ImGui::Text("Resting level: %.2f", p.startingRestingLevel);
			ImGui::Text("Activation fn: %s", p.activationFunction->toString().c_str());
			break;
		}
		case element::ElementLabel::GAUSS_STIMULUS:
		{
			const auto gs = std::dynamic_pointer_cast<element::GaussStimulus>(element);
			const auto& p = gs->getParameters();
			ImGui::Text("Width: %.2f",     p.width);
			ImGui::Text("Amplitude: %.2f", p.amplitude);
			ImGui::Text("Position: %.2f",  p.position);
			ImGui::Text("Circular: %s",    p.circular    ? "true" : "false");
			ImGui::Text("Normalized: %s",  p.normalized  ? "true" : "false");
			break;
		}
		case element::ElementLabel::GAUSS_KERNEL:
		{
			const auto gk = std::dynamic_pointer_cast<element::GaussKernel>(element);
			const auto& p = gk->getParameters();
			ImGui::Text("Width: %.2f",        p.width);
			ImGui::Text("Amplitude: %.2f",    p.amplitude);
			ImGui::Text("Global amp: %.4f",   p.amplitudeGlobal);
			ImGui::Text("Circular: %s",       p.circular   ? "true" : "false");
			ImGui::Text("Normalized: %s",     p.normalized ? "true" : "false");
			break;
		}
		case element::ElementLabel::MEXICAN_HAT_KERNEL:
		{
			const auto mh = std::dynamic_pointer_cast<element::MexicanHatKernel>(element);
			const auto& p = mh->getParameters();
			ImGui::Text("Width exc: %.2f",    p.widthExc);
			ImGui::Text("Amplitude exc: %.2f",p.amplitudeExc);
			ImGui::Text("Width inh: %.2f",    p.widthInh);
			ImGui::Text("Amplitude inh: %.2f",p.amplitudeInh);
			ImGui::Text("Global amp: %.4f",   p.amplitudeGlobal);
			ImGui::Text("Circular: %s",       p.circular   ? "true" : "false");
			ImGui::Text("Normalized: %s",     p.normalized ? "true" : "false");
			break;
		}
		case element::ElementLabel::OSCILLATORY_KERNEL:
		{
			const auto ok = std::dynamic_pointer_cast<element::OscillatoryKernel>(element);
			const auto& p = ok->getParameters();
			ImGui::Text("Amplitude: %.2f",       p.amplitude);
			ImGui::Text("Decay: %.4f",           p.decay);
			ImGui::Text("Zero crossings: %.2f",  p.zeroCrossings);
			ImGui::Text("Global amp: %.4f",      p.amplitudeGlobal);
			ImGui::Text("Circular: %s",          p.circular   ? "true" : "false");
			ImGui::Text("Normalized: %s",        p.normalized ? "true" : "false");
			break;
		}
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:
		{
			const auto ak = std::dynamic_pointer_cast<element::AsymmetricGaussKernel>(element);
			const auto& p = ak->getParameters();
			ImGui::Text("Width: %.2f",        p.width);
			ImGui::Text("Amplitude: %.2f",    p.amplitude);
			ImGui::Text("Global amp: %.4f",   p.amplitudeGlobal);
			ImGui::Text("Time shift: %.2f",   p.timeShift);
			ImGui::Text("Circular: %s",       p.circular   ? "true" : "false");
			ImGui::Text("Normalized: %s",     p.normalized ? "true" : "false");
			break;
		}
		case element::ElementLabel::NORMAL_NOISE:
		{
			const auto nn = std::dynamic_pointer_cast<element::NormalNoise>(element);
			ImGui::Text("Amplitude: %.4f", nn->getParameters().amplitude);
			break;
		}
		case element::ElementLabel::FIELD_COUPLING:
		{
			const auto fc = std::dynamic_pointer_cast<element::FieldCoupling>(element);
			const auto& p = fc->getParameters();
			ImGui::Text("In dims: %d x %.2f",   p.inputFieldDimensions.x_max, p.inputFieldDimensions.d_x);
			ImGui::Text("Rule: %s",             LearningRuleToString.at(p.learningRule).c_str());
			ImGui::Text("Scalar: %.2f",         p.scalar);
			ImGui::Text("Learning rate: %.4f",  p.learningRate);
			ImGui::Text("Learning active: %s",  p.isLearningActive ? "true" : "false");
			break;
		}
		case element::ElementLabel::GAUSS_FIELD_COUPLING:
		{
			const auto gfc = std::dynamic_pointer_cast<element::GaussFieldCoupling>(element);
			const auto& p  = gfc->getParameters();
			ImGui::Text("In dims: %d x %.2f", p.inputFieldDimensions.x_max, p.inputFieldDimensions.d_x);
			ImGui::Text("Normalized: %s",     p.normalized ? "true" : "false");
			ImGui::Text("Circular: %s",       p.circular   ? "true" : "false");
			ImGui::Text("Couplings: %zu",     p.couplings.size());
			break;
		}
		default:
			ImGui::TextDisabled("No parameters available.");
			break;
		}

		ImGui::EndTooltip();
	}
}
