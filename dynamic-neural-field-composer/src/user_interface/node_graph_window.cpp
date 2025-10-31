#include "user_interface/node_graph_window.h"




namespace dnf_composer::user_interface
{
	NodeGraphWindow::NodeGraphWindow(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{
		config.SettingsFile = "imnode-window.json";
		context = ImNodeEditor::CreateEditor();

		// ImNodeEditor::SetCurrentEditor(context);          // make it active
		// {
		// 	auto& st = ImNodeEditor::GetStyle();
		// 	st.NodePadding            = ImVec4(8, 6, 8, 8);
		// 	st.NodeRounding           = 6.0f;
		// 	st.NodeBorderWidth        = 1.5f;
		// 	st.HoveredNodeBorderWidth = 2.0f;
		// 	st.SelectedNodeBorderWidth= 2.0f;
		// 	st.PinRounding            = 4.0f;
		// 	st.PinBorderWidth         = 1.0f;
		// 	st.LinkStrength           = 120.0f;
		//
		// 	auto& col = st.Colors;
		// 	col[ImNodeEditor::StyleColor_Bg]           = ImColor(240,244,247,255);
		// 	col[ImNodeEditor::StyleColor_Grid]         = ImColor(210,217,220,255);
		// 	col[ImNodeEditor::StyleColor_NodeBg]       = ImColor(238,243,245,255);
		// 	col[ImNodeEditor::StyleColor_NodeBorder]   = ImColor(210,217,220,255);
		// 	col[ImNodeEditor::StyleColor_HovNodeBorder]= ImColor(90,185,150,255);
		// 	col[ImNodeEditor::StyleColor_SelNodeBorder]= ImColor(64,163,130,255);
		//
		// 	// These three control the actual link color:
		// 	// col[ImNodeEditor::StyleColor_Link]         = ImColor(52,135,108,200);
		// 	// col[ImNodeEditor::StyleColor_LinkHovered]  = ImColor(64,163,130,255);
		// 	// col[ImNodeEditor::StyleColor_LinkSelected] = ImColor(90,185,150,255);
		//
		// 	col[ImNodeEditor::StyleColor_PinRect]      = ImColor(255,255,255,255);
		// 	col[ImNodeEditor::StyleColor_PinRectBorder]= ImColor(210,217,220,255);
		// }
		// ImNodeEditor::SetCurrentEditor(nullptr);
	}

	void NodeGraphWindow::render()
	{
		// auto& st = ImNodeEditor::GetStyle();
		// st.NodePadding            = ImVec4(8, 6, 8, 8);
		// st.NodeRounding           = 6.0f;
		// st.NodeBorderWidth        = 1.5f;
		// st.HoveredNodeBorderWidth = 2.0f;
		// st.SelectedNodeBorderWidth= 2.0f;
		// st.PinRounding            = 4.0f;
		// st.PinBorderWidth         = 1.0f;
		// st.LinkStrength           = 120.0f;            // slightly stiffer curves
		// st.ScrollDuration         = 0.25f;
		// st.GroupRounding          = 6.0f;
		// st.GroupBorderWidth       = 1.0f;
		//
		// // palette
		// st.Colors[ImNodeEditor::StyleColor_NodeBg]         = ImColor(36, 36, 40, 255);
		// st.Colors[ImNodeEditor::StyleColor_NodeBorder]     = ImColor(68, 68, 78, 255);
		// st.Colors[ImNodeEditor::StyleColor_HovNodeBorder]  = ImColor(112, 165, 255, 255);
		// st.Colors[ImNodeEditor::StyleColor_SelNodeBorder]  = ImColor(165, 205, 255, 255);
		// st.Colors[ImNodeEditor::StyleColor_PinRect]        = ImColor(50, 50, 55, 255);
		// st.Colors[ImNodeEditor::StyleColor_PinRectBorder]  = ImColor(90, 90, 95, 255);
		// //st.Colors[ImNodeEditor::StyleColor_Link]           = ImColor(150, 150, 160, 200);
		// st.Colors[ImNodeEditor::StyleColor_HovLinkBorder]    = ImColor(200, 200, 220, 255);
		// st.Colors[ImNodeEditor::StyleColor_LinkSelRectBorder]   = ImColor(210, 210, 230, 255);
		//
		// // st.Colors[ImNodeEditor::StyleColor_LinkHovered]    = ImColor(200, 200, 220, 255);
		// // st.Colors[ImNodeEditor::StyleColor_LinkSelected]   = ImColor(210, 210, 230, 255);

		// auto& st = ImNodeEditor::GetStyle();
		// st.NodePadding            = ImVec4(8, 6, 8, 8);
		// st.NodeRounding           = 6.0f;
		// st.NodeBorderWidth        = 1.5f;
		// st.HoveredNodeBorderWidth = 2.0f;
		// st.SelectedNodeBorderWidth= 2.0f;
		// st.PinRounding            = 4.0f;
		// st.PinBorderWidth         = 1.0f;
		// st.LinkStrength           = 120.0f;
		//
		// auto& col = st.Colors;
		// col[ImNodeEditor::StyleColor_Bg]                = ImColor(240,244,247,255);
		// col[ImNodeEditor::StyleColor_Grid]              = ImColor(210,217,220,255);
		// col[ImNodeEditor::StyleColor_NodeBg]            = ImColor(238,243,245,255);
		// col[ImNodeEditor::StyleColor_NodeBorder]        = ImColor(210,217,220,255);
		// col[ImNodeEditor::StyleColor_HovNodeBorder]     = ImColor(90,185,150,255);
		// col[ImNodeEditor::StyleColor_SelNodeBorder]     = ImColor(64,163,130,255);
		// // col[ImNodeEditor::StyleColor_Link]              = ImColor(52,135,108,200);
		// // col[ImNodeEditor::StyleColor_LinkHovered]       = ImColor(64,163,130,255);
		// // col[ImNodeEditor::StyleColor_LinkSelected]      = ImColor(90,185,150,255);
		// col[ImNodeEditor::StyleColor_PinRect]           = ImColor(255,255,255,255);
		// col[ImNodeEditor::StyleColor_PinRectBorder]     = ImColor(210,217,220,255);

		const ImGuiWindowFlags flags =  imgui_kit::getGlobalWindowFlags()
									| ImGuiWindowFlags_NoScrollbar
									| ImGuiWindowFlags_NoScrollWithMouse;

		if (ImGui::Begin("Node Graph Window", nullptr, flags))
		{
			ImNodeEditor::SetCurrentEditor(context);
			//const auto& io = ImGui::GetIO();
			widgets::renderHelpMarker("Visualize the elements and their interactions in the simulation.\n"
						  "Create interactions by clicking on output pins and dragging to input pins.\n"
						  "Delete interactions by double clicking on links.");
			//ImGui::SameLine();
			//ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);
			ImNodeEditor::Begin("dnf-composer node graph");
			renderElementNodes();
			handleInteractions();
			ImNodeEditor::End();
			ImNodeEditor::SetCurrentEditor(nullptr);
		}
		ImGui::End();
	}

	void NodeGraphWindow::renderGraph() const
	{
		ImNodeEditor::SetCurrentEditor(context);
		//const auto& io = ImGui::GetIO();
		widgets::renderHelpMarker("Visualize the elements and their interactions in the simulation.\n"
					  "Create interactions by clicking on output pins and dragging to input pins.\n"
					  "Delete interactions by double clicking on links.");
		//ImGui::SameLine();
		//ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);
		ImNodeEditor::Begin("dnf-composer node graph");
		renderElementNodes();
		handleInteractions();
		ImNodeEditor::End();
		ImNodeEditor::SetCurrentEditor(nullptr);
	}


	void NodeGraphWindow::renderElementNodes() const
	{
		// for (const auto& element : simulation->getElements())
		// {
		// 	ImGui::PushStyleColor(ImGuiCol_Text, imgui_kit::colours::White);
		// 	const size_t nodeId = std::hash<std::string>{}(element->getUniqueName());
		// 	ImNodeEditor::BeginNode(nodeId);
		// 	setNodeStyle(element);
		// 	renderElementNode(element);
		// 	ImGui::PopStyleColor();
		// 	ImNodeEditor::EndNode();
		// }
		//  for (const auto& element : simulation->getElements())
		//  {
		// 	renderElementNodeConnections(element);
		//  }
		for (const auto& element : simulation->getElements())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, imgui_kit::colours::White);

			const size_t nodeId = std::hash<std::string>{}(element->getUniqueName());
			ImNodeEditor::BeginNode(nodeId);
			setNodeStyle(element);                // Push x1 var + x2 colors
			renderElementNode(element);
			ImNodeEditor::PopStyleColor(2);       // <<< pop NodeBg & NodeBorder
			ImNodeEditor::PopStyleVar(1);         // <<< pop NodeRounding

			ImGui::PopStyleColor();
			ImNodeEditor::EndNode();
		}

		for (const auto& element : simulation->getElements())
			renderElementNodeConnections(element);
	}

	void NodeGraphWindow::setNodeStyle(const std::shared_ptr<element::Element>& element)
	{
		// static constexpr float rounding = 5.0f;
		// ImNodeEditor::PushStyleVar(ImNodeEditor::StyleVar_NodeRounding, rounding);
		//
		// ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_NodeBg, imgui_kit::colours::DarkGray);
		// ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_NodeBorder, imgui_kit::colours::White);
		// //ImNodeEditor::PopStyleColor(2); // apparently this is not necessary
		ImNodeEditor::PushStyleVar(ImNodeEditor::StyleVar_NodeRounding, 6.0f);
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_NodeBg,     ImColor(32, 34, 38, 255));
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_NodeBorder, ImColor(70, 76, 88, 255));
	}

	void NodeGraphWindow::renderElementNode(const std::shared_ptr<element::Element>& element)
	{
		renderElementNodeHeader(element);
		renderElementCommonParameters(element);
		renderElementSpecificParameters(element);
		renderElementPins(element);
	}


	void NodeGraphWindow::renderElementNodeHeader(const std::shared_ptr<element::Element>& element)
	{
		// const size_t nodeId = getNodeId(element);
		// const ImVec2 nodePos = ImNodeEditor::GetNodePosition(nodeId);
		// const ImVec2 nodeSize = ImNodeEditor::GetNodeSize(nodeId);
		//
		// ImDrawList* draw_list = ImGui::GetWindowDrawList();
		// const ImVec2 titleBarPos = ImVec2(nodePos.x + 1.5f, nodePos.y + 1.0f);
		// const ImVec2 titleBarSize = ImVec2(nodeSize.x - 2.5f, ImGui::GetTextLineHeightWithSpacing());
		//
		// // Draw title bar background
		// draw_list->AddRectFilled(
		// 	titleBarPos,
		// 	ImVec2(titleBarPos.x + titleBarSize.x, titleBarPos.y + titleBarSize.y),
		// 	getHeaderColorForElementType(element->getLabel()), 5.0f
		// );
		//
		// // Draw title text
		// ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);// bold font
		// const element::ElementCommonParameters parameters = element->getElementCommonParameters();
		// const std::string name = parameters.identifiers.uniqueName;
		// draw_list->AddText(ImVec2(titleBarPos.x + 3.0f, titleBarPos.y),
		// 	IM_COL32(255, 255, 255, 255),
		// 	name.c_str());
		// ImGui::PopFont();
		//
		// // Offset the position of the remaining node content to avoid overlapping with the title bar
		// ImGui::Dummy(ImVec2(0, 15));
		const size_t nodeId = getNodeId(element);
		const ImVec2 nodePos  = ImNodeEditor::GetNodePosition(nodeId);
		const ImVec2 nodeSize = ImNodeEditor::GetNodeSize(nodeId);

		ImDrawList* dl = ImGui::GetWindowDrawList();

		// base header color -> top/bottom gradient
		const ImU32 baseU32 = getHeaderColorForElementType(element->getLabel());
		ImVec4 base = ImGui::ColorConvertU32ToFloat4(baseU32);
		ImU32 top = ImGui::ColorConvertFloat4ToU32(ImVec4(base.x, base.y, base.z, 1.0f));
		ImU32 bot = ImGui::ColorConvertFloat4ToU32(ImVec4(base.x * 0.85f, base.y * 0.85f, base.z * 0.85f, 1.0f));

		const float h = ImGui::GetTextLineHeightWithSpacing() + 4.0f;

		const ImVec2 p0(nodePos.x + 1.5f, nodePos.y + 1.0f);
		const ImVec2 p1(p0.x + (ImNodeEditor::GetNodeSize(nodeId).x - 2.5f), p0.y + h);

		// gradient header
		dl->AddRectFilledMultiColor(p0, p1, top, top, bot, bot);

		// bottom separator
		dl->AddLine(ImVec2(p0.x, p1.y), ImVec2(p1.x, p1.y), IM_COL32(0, 0, 0, 80));

		// title text
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]); // bold
		const auto& name = element->getElementCommonParameters().identifiers.uniqueName;
		dl->AddText(ImVec2(p0.x + 6.0f, p0.y + 2.0f), IM_COL32(255, 255, 255, 255), name.c_str());
		ImGui::PopFont();

		// push content below the header
		ImGui::Dummy(ImVec2(0, h - ImGui::GetTextLineHeight()));
	}


	void NodeGraphWindow::renderElementCommonParameters(const std::shared_ptr<element::Element>& element)
	{
		const element::ElementCommonParameters parameters = element->getElementCommonParameters();
		const std::string name = "Name: " + parameters.identifiers.uniqueName;
		ImGui::TextUnformatted(name.c_str());

		const std::string type = "Type: " + element::ElementLabelToString.find(parameters.identifiers.label)->second;
		ImGui::Text(type.c_str());
		ImGui::Text("Size: %d", parameters.dimensionParameters.size);
		ImGui::Text("Step size: %.2f", parameters.dimensionParameters.d_x);
	}

	void NodeGraphWindow::renderElementSpecificParameters(const std::shared_ptr<element::Element>& element)
	{
		static bool showSpecificParameters = false;

		ImGui::PushID(element.get());

		// Toggle button
		if (ImGui::Button(showSpecificParameters ? "Hide Specific Parameters" : "Show Specific Parameters"))
		{
			showSpecificParameters = !showSpecificParameters;
		}

		if (showSpecificParameters)
		{

			switch (element->getElementCommonParameters().identifiers.label)
			{
			case element::ElementLabel::NEURAL_FIELD:
			{
				const auto neuralField = std::dynamic_pointer_cast<element::NeuralField>(element);
				const element::NeuralFieldParameters parameters = neuralField->getParameters();
				ImGui::Text("Resting level: %.2f", parameters.startingRestingLevel);
				ImGui::Text("Tau: %.2f", parameters.tau);
				ImGui::Text("Activation function: %s", parameters.activationFunction->toString().c_str());
			}
			break;
			case element::ElementLabel::NORMAL_NOISE:
			{
				const auto normalNoise = std::dynamic_pointer_cast<element::NormalNoise>(element);
				const element::NormalNoiseParameters parameters = normalNoise->getParameters();
				ImGui::Text("Amplitude: %.2f", parameters.amplitude);
			}
			break;
			case element::ElementLabel::GAUSS_KERNEL:
			{
				const auto gaussKernel = std::dynamic_pointer_cast<element::GaussKernel>(element);
				const element::GaussKernelParameters parameters = gaussKernel->getParameters();
				ImGui::Text("Width: %.2f", parameters.width);
				ImGui::Text("Amplitude: %.2f", parameters.amplitude);
				ImGui::Text("Amplitude global: %.2f", parameters.amplitudeGlobal);
				ImGui::Text("Circular: %s", parameters.circular ? "true" : "false");
				ImGui::Text("Normalized: %s", parameters.normalized ? "true" : "false");
			}
			break;
			case element::ElementLabel::GAUSS_STIMULUS:
			{
				const auto gaussStimulus = std::dynamic_pointer_cast<element::GaussStimulus>(element);
				const element::GaussStimulusParameters parameters = gaussStimulus->getParameters();
				ImGui::Text("Amplitude: %.2f", parameters.amplitude);
				ImGui::Text("Center: %.2f", parameters.position);
				ImGui::Text("Width: %.2f", parameters.width);
				ImGui::Text("Circular: %s", parameters.circular ? "true" : "false");
				ImGui::Text("Normalized: %s", parameters.normalized ? "true" : "false");
			}
			break;
			case element::ElementLabel::MEXICAN_HAT_KERNEL:
			{
				const auto mexicanHatKernel = std::dynamic_pointer_cast<element::MexicanHatKernel>(element);
				const element::MexicanHatKernelParameters parameters = mexicanHatKernel->getParameters();
				ImGui::Text("Amplitude exc: %.2f", parameters.amplitudeExc);
				ImGui::Text("Amplitude inh: %.2f", parameters.amplitudeInh);
				ImGui::Text("Width exc: %.2f", parameters.widthExc);
				ImGui::Text("Width inh: %.2f", parameters.widthInh);
				ImGui::Text("Amplitude global: %.2f", parameters.amplitudeGlobal);
				ImGui::Text("Circular: %s", parameters.circular ? "true" : "false");
				ImGui::Text("Normalized: %s", parameters.normalized ? "true" : "false");
			}
			break;
			case element::ElementLabel::GAUSS_FIELD_COUPLING:
			{
				const auto gfc = std::dynamic_pointer_cast<element::GaussFieldCoupling>(element);
				const element::GaussFieldCouplingParameters parameters = gfc->getParameters();
				ImGui::Text("Input field dimensions: x_max %d, d_x %.2f", parameters.inputFieldDimensions.x_max, parameters.inputFieldDimensions.d_x);
				ImGui::Text("Normalized: %s", parameters.normalized ? "true" : "false");
				ImGui::Text("Circular: %s", parameters.circular ? "true" : "false");

				ImGui::Text("Couplings:");
				for (const auto& coupling : parameters.couplings)
				{
					ImGui::Text("x_i: %.2f", coupling.x_i); ImGui::SameLine();
					ImGui::Text("x_j: %.2f", coupling.x_j); ImGui::SameLine();
					ImGui::Text("A: %.2f", coupling.amplitude); ImGui::SameLine();
					ImGui::Text("W: %.2f", coupling.width);
				}
			}
			break;
			case element::ElementLabel::FIELD_COUPLING:
			{
				const auto fc = std::dynamic_pointer_cast<element::FieldCoupling>(element);
				const element::FieldCouplingParameters parameters = fc->getParameters();
				ImGui::Text("Input field dimensions: x_max %d, d_x %.2f", parameters.inputFieldDimensions.x_max, parameters.inputFieldDimensions.d_x);
				ImGui::Text("Learning rule: %s", LearningRuleToString.at(parameters.learningRule).c_str());
				ImGui::Text("Scalar: %.2f", parameters.scalar);
				ImGui::Text("Learning rate: %.2f", parameters.learningRate);
				ImGui::Text("Learning active: %s", parameters.isLearningActive ? "true" : "false");
			}
			break;
			case element::ElementLabel::OSCILLATORY_KERNEL:
			{
				const auto oscillatoryKernel = std::dynamic_pointer_cast<element::OscillatoryKernel>(element);
				const element::OscillatoryKernelParameters parameters = oscillatoryKernel->getParameters();
				ImGui::Text("Amplitude: %.2f", parameters.amplitude);
				ImGui::Text("Decay: %.2f", parameters.decay);
				ImGui::Text("Zero crossings: %.2f", parameters.zeroCrossings);
				ImGui::Text("Amplitude global: %.2f", parameters.amplitudeGlobal);
				ImGui::Text("Circular: %s", parameters.circular ? "true" : "false");
				ImGui::Text("Normalized: %s", parameters.normalized ? "true" : "false");
			}
			break;
			default:
				tools::logger::log(tools::logger::LogLevel::ERROR, "Element label not recognized at node graph.");
				break;
			}
		}
		ImGui::PopID();
	}

	void NodeGraphWindow::renderElementPins(const std::shared_ptr<element::Element>& element)
	{
		// // Begin an input pin for the node with a unique identifier
		// ImNodeEditor::BeginPin(startingInputPinId + element->getUniqueIdentifier(), ImNodeEditor::PinKind::Input);
		// // Align the input pin to the left
		// ImNodeEditor::PinPivotAlignment(ImVec2(0.0f, 0.5f));
		// ImGui::Text("Input");
		// ImNodeEditor::EndPin();
		// ImGui::SameLine();
		//
		// // Add some space to separate the input from the output visually
		// ImGui::Dummy(ImVec2(100.0f, 0.0f)); ImGui::SameLine();  // Adjust for spacing
		//
		// // Begin an output pin for the node with a unique identifier
		// ImNodeEditor::BeginPin(startingOutputPinId + element->getUniqueIdentifier(), ImNodeEditor::PinKind::Output);
		// // Align the output pin to the right
		// ImNodeEditor::PinPivotAlignment(ImVec2(1.0f, 0.5f));
		// ImGui::Text("Output");
		// ImNodeEditor::EndPin();
		constexpr float bubbleR = 6.0f;

		// INPUT
		ImNodeEditor::BeginPin(startingInputPinId + element->getUniqueIdentifier(),
							   ImNodeEditor::PinKind::Input);
		ImNodeEditor::PinPivotAlignment(ImVec2(0.0f, 0.5f));
		ImGui::BeginGroup();
		{
			ImVec2 min = ImGui::GetCursorScreenPos();

			// bubble center = (min.x + bubbleR, min.y + lineHeight*0.5)
			ImVec2 bubbleCenter(min.x + bubbleR, min.y + ImGui::GetTextLineHeight() * 0.5f);
			drawPinBubble(bubbleCenter, bubbleR,
						  IM_COL32(70,180,255,255), IM_COL32(25,60,90,255));

			ImGui::Dummy(ImVec2(bubbleR * 2.0f + 6.0f, 0.0f));
			ImGui::SameLine();
			ImGui::TextUnformatted("Input");
		}
		ImGui::EndGroup();
		ImNodeEditor::EndPin();

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(60.0f, 0.0f));   // spacing like the example
		ImGui::SameLine();

		// OUTPUT
		ImNodeEditor::BeginPin(startingOutputPinId + element->getUniqueIdentifier(),
							   ImNodeEditor::PinKind::Output);
		ImNodeEditor::PinPivotAlignment(ImVec2(1.0f, 0.5f));
		ImGui::BeginGroup();
		{
			ImGui::TextUnformatted("Output");
			ImGui::SameLine();

			const ImVec2 min = ImGui::GetCursorScreenPos();
			const float  w   = ImGui::CalcTextSize("Output").x;

			const ImVec2 bubbleCenter(min.x + w + bubbleR + 6.0f,
								min.y + ImGui::GetTextLineHeight() * 0.5f);
			drawPinBubble(bubbleCenter, bubbleR,
						  IM_COL32(255,120,120,255), IM_COL32(90,40,40,255));

			ImGui::Dummy(ImVec2(w + bubbleR * 2.0f + 8.0f, 0.0f));
		}
		ImGui::EndGroup();
		ImNodeEditor::EndPin();
	}

	void NodeGraphWindow::renderElementNodeConnections(const std::shared_ptr<element::Element>& element)
	{
		// static constexpr float thickness = 3.0f;
		// for (const auto& connection : element->getInputs())
		// {
		// 	const std::string linkId = std::to_string(element->getUniqueIdentifier())
		// 		+ std::to_string(connection->getUniqueIdentifier());
		// 	const size_t link = stoull(linkId) + startingLinkId;
		// 	ImNodeEditor::Link(link,
		// 		connection->getUniqueIdentifier() + startingOutputPinId,
		// 		element->getUniqueIdentifier() + startingInputPinId,
		// 		imgui_kit::colours::White, thickness);
		// }
		static constexpr float thickness = 3.0f;

		for (const auto& in : element->getInputs())
		{
			const ImU32 c_u32 = getHeaderColorForElementType(in->getLabel());   // ImU32
			const ImVec4 c     = ImGui::ColorConvertU32ToFloat4(c_u32);         // -> ImVec4

			const std::string linkId = std::to_string(element->getUniqueIdentifier()) +
									   std::to_string(in->getUniqueIdentifier());
			const size_t id = std::stoull(linkId) + startingLinkId;             // your id scheme

			ImNodeEditor::Link(id,
				in->getUniqueIdentifier() + startingOutputPinId,
				element->getUniqueIdentifier() + startingInputPinId,
				c, thickness);
		}
	}

	void NodeGraphWindow::handleInteractions() const
	{
		handlePinInteractions();
		handleLinkInteractions();
	}

	void NodeGraphWindow::handlePinInteractions() const
	{
		static bool isUsrAttemptingAConnection = false;
		static ImNodeEditor::PinId outputPinId = 0;

		// Set interactions by clicking on output pins and dragging to input pins
		if(ImNodeEditor::GetHoveredPin() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			outputPinId = ImNodeEditor::GetHoveredPin();
			isUsrAttemptingAConnection = true;
			return;
		}

		if (ImNodeEditor::GetHoveredPin() && isUsrAttemptingAConnection )
		{
			const int outputElementId = static_cast<int>(outputPinId.Get()) - startingOutputPinId;
			const int inputElementId = static_cast<int>(ImNodeEditor::GetHoveredPin().Get()) - startingInputPinId;

			const int highestElementIndex = simulation->getHighestElementIndex();
			if (outputElementId > highestElementIndex || inputElementId > highestElementIndex
				|| outputElementId < 0 || inputElementId < 0)
			{
				isUsrAttemptingAConnection = false;
				return;
			}

			const std::string outputElementName = simulation->getElement(outputElementId)->getUniqueName();
			const std::string inputElementName = simulation->getElement(inputElementId)->getUniqueName();
			simulation->createInteraction(outputElementName, "output", inputElementName);
			isUsrAttemptingAConnection = false; // Reset the connection state
		}
	}

	void NodeGraphWindow::handleLinkInteractions() const
	{
		// Delete interactions by double-clicking on links
		if (const ImNodeEditor::LinkId doubleClickedLink = ImNodeEditor::GetDoubleClickedLink())
		{
			ImNodeEditor::PinId startPin;
			ImNodeEditor::PinId endPin;
			GetLinkPins(doubleClickedLink, &startPin, &endPin);

			const int inputElementId = static_cast<int>(startPin.Get()) - startingOutputPinId;
			const int outputElementId = static_cast<int>(endPin.Get()) - startingInputPinId;

			const int highestElementIndex = simulation->getHighestElementIndex();
			if (outputElementId > highestElementIndex || inputElementId > highestElementIndex
				|| outputElementId < 0 || inputElementId < 0)
			{
				return;
			}

			simulation->getElement(outputElementId)->removeInput(inputElementId);
		}
	}

	size_t NodeGraphWindow::getNodeId(const std::shared_ptr<element::Element>& element)
	{
		return std::hash<std::string>{}(element->getUniqueName());
	}

	void NodeGraphWindow::drawPinBubble(const ImVec2& center, float r, ImU32 fill, ImU32 border)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddCircleFilled(center, r, fill, 16);
		dl->AddCircle(center, r, border, 16, 1.0f);
	}

}

