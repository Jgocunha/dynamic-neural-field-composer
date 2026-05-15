#include "user_interface/node_graph_window.h"

extern ImFont* g_BlackSmallFont;

namespace dnf_composer::user_interface
{
	NodeGraphWindow::NodeGraphWindow(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{
		config.SettingsFile = "imnode-window.json";
		context = ImNodeEditor::CreateEditor(&config);
	}

	void NodeGraphWindow::renderGraphContent() const
	{
		widgets::renderHelpMarker(
			"Visualize elements and their interactions.\n"
			"Drag from an Output pin to an Input pin to create a connection.\n"
			"Double-click a link to remove it.\n"
			"Double-click a node to open/close its plot card.");

		ImNodeEditor::SetCurrentEditor(context);
		applyCanvasStyle();
		ImNodeEditor::Begin("dnf-composer-graph");
		renderElementNodes();
		handleInteractions();
		ImNodeEditor::End();
		restoreCanvasStyle();
		ImNodeEditor::SetCurrentEditor(nullptr);

		const ImVec2 ngPos  = ImGui::GetWindowPos();
		const ImVec2 ngSize = ImGui::GetWindowSize();
		ngBoundsMin = ngPos;
		ngBoundsMax = ImVec2(ngPos.x + ngSize.x, ngPos.y + ngSize.y);
	}

	void NodeGraphWindow::render()
	{
		const ImGuiWindowFlags flags = imgui_kit::getGlobalWindowFlags()
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoScrollWithMouse;

		ImGui::PushFont(g_BlackLargeFont);
		const bool open = ImGui::Begin("Node Graph", nullptr, flags);
		ImGui::PopFont();

		if (open)
		{
			renderGraphContent();
		}

		ImGui::End();

		// Plot cards are separate top-level windows, must be rendered outside Begin/End.
		renderNodePlotCards();
	}

	void NodeGraphWindow::renderEmbedded() const
	{
		renderGraphContent();
		renderNodePlotCards();
	}

	void NodeGraphWindow::applyCanvasStyle()
	{
		// Light background that matches the rest of the ImGui windows
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_Bg,
			ImVec4(0.94F, 0.95F, 0.96F, 1.00F));
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_Grid,
			ImVec4(0.80F, 0.82F, 0.85F, 0.60F));
	}

	void NodeGraphWindow::restoreCanvasStyle()
	{
		ImNodeEditor::PopStyleColor(2);
	}

	void NodeGraphWindow::renderElementNodes() const
	{
		// Apply initial positions queued in the previous frame.
		// SetNodePosition must be called before BeginNode to take effect on this frame.
		for (const auto& [nodeId, pos] : pendingInitialPositions)
		{
			ImNodeEditor::SetNodePosition(nodeId, pos);
		}
		pendingInitialPositions.clear();

		for (const auto& element : simulation->getElements())
		{
			renderElementNode(element);
		}

		// After nodes have been submitted, we can read their actual canvas positions.
		// Any node still sitting at the origin (0,0) has no saved layout
		// queue a grid position, so it spreads out on the next frame.
		constexpr float colSpacing = 300.0F;
		constexpr float rowSpacing = 250.0F;
		constexpr float baseX     =  50.0F;
		constexpr float baseY     =  50.0F;
		constexpr int   maxRows   =   5;

		// Compute how many physical columns each type-group needs (ceil(count / maxRows)),
		// then assign cumulative x offsets so overflow columns never collide with the
		// next group's base column.
		//
		// columnCounts is derived from the *current* simulation's elements only, so it
		// resets naturally whenever a different simulation is loaded - no stale state
		// from a previously loaded (possibly larger) simulation bleeds through.
		const auto& elements = simulation->getElements();
		std::array<int, 4> groupCount    = {};
		std::array<int, 4> columnCounts  = {};   // already-positioned nodes per group
		for (const auto& el : elements)
		{
			const int g = getColumnForElement(el->getLabel());
			++groupCount[g];
			if (positionedNodeIds.contains(getNodeId(el)))
			{
				++columnCounts[g];
			}
		}

		std::array<float, 4> groupBaseX = {};
		float curX = baseX;
		for (int g = 0; g < 4; ++g)
		{
			groupBaseX[g] = curX;
			const int cols = std::max(1, (groupCount[g] + maxRows - 1) / maxRows);
			curX += static_cast<float>(cols) * colSpacing;
		}

		for (const auto& element : elements)
		{
			if (const size_t nodeId = getNodeId(element); !positionedNodeIds.contains(nodeId))
			{
				if (const ImVec2 pos = ImNodeEditor::GetNodePosition(nodeId);
					std::abs(pos.x) < 1.0F && std::abs(pos.y) < 1.0F)
				{
					const int g      = getColumnForElement(element->getLabel());
					const int rowIdx = columnCounts[g]++;
					const int col    = rowIdx / maxRows;
					const int row    = rowIdx % maxRows;
					pendingInitialPositions[nodeId] =
						ImVec2(groupBaseX[g] + (static_cast<float>(col) * colSpacing),
							baseY + (static_cast<float>(row) * rowSpacing));
				}
				positionedNodeIds.insert(nodeId);
			}
		}

		for (const auto& element : simulation->getElements())
		{
			renderElementNodeConnections(element);
		}
	}

	void NodeGraphWindow::renderElementNode(const std::shared_ptr<element::Element>& element)
	{
		namespace util = ax::NodeEditor::Utilities;

		const ImNodeEditor::NodeId nodeId = getNodeId(element);
		const ImU32  headerU32  = getHeaderColorForElementType(element->getLabel());
		const ImVec4 headerVec4 = ImGui::ColorConvertU32ToFloat4(headerU32);

		// Uniform pastel: blend element color 35 % toward white
		const ImVec4 bodyVec4 = {
			(headerVec4.x * 0.35F) + 0.65F,
			(headerVec4.y * 0.35F) + 0.65F,
			(headerVec4.z * 0.35F) + 0.65F,
			1.0F
		};

		ImNodeEditor::PushStyleVar(ImNodeEditor::StyleVar_NodeRounding,    10.0F);
		ImNodeEditor::PushStyleVar(ImNodeEditor::StyleVar_NodeBorderWidth, 0.0F);
		ImNodeEditor::PushStyleVar(ImNodeEditor::StyleVar_NodePadding,     ImVec4(8, 4, 8, 6));
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_NodeBg,     bodyVec4);
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_NodeBorder, bodyVec4);

		util::BlueprintNodeBuilder builder;
		builder.Begin(nodeId);

		bool showTooltip = false;

		builder.Header(bodyVec4);
		{
			static constexpr float minNodeSize = 280.0F;
			ImGui::Dummy(ImVec2(minNodeSize, 0));

			renderNodeScrollingName(element, minNodeSize);

			ImGui::Spacing();
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_CIRCLE_INFO);
			ImGui::PopFont();
			showTooltip = ImGui::IsItemHovered();
			ImGui::Spacing();

			renderNodeInlinePreview(element, minNodeSize);
			ImGui::Spacing();

			renderNodePins(element, minNodeSize);
		}
		builder.EndHeader();
		builder.End();

		ImNodeEditor::PopStyleColor(2);
		ImNodeEditor::PopStyleVar(3);

		// Suspend/Resume outside the BeginHorizontal (builder.Header) context to
		// avoid corrupting the draw list clip-rect stack on long-name nodes.
		if (showTooltip)
		{
			ImNodeEditor::Suspend();
			renderElementTooltip(element);
			ImNodeEditor::Resume();
		}
	}

	void NodeGraphWindow::renderNodeScrollingName(const std::shared_ptr<element::Element>& element, const float minNodeSize)
	{
		static constexpr float scrollSpeed = 50.0F;
		static constexpr float scrollDelay = 0.5F;
		static constexpr float scrollPause = 1.0F;
		static std::unordered_map<size_t, double> s_hoverStart;

		ImGui::PushFont(g_BlackLargeFont);
		const std::string& name  = element->getUniqueName();
		const float        lineH = ImGui::GetTextLineHeight();
		const float        textW = ImGui::CalcTextSize(name.c_str()).x;
		constexpr float    nodePad = 16.0F;
		const float        availW  = minNodeSize - nodePad;

		if (textW <= availW)
		{
			ImGui::TextUnformatted(name.c_str());
		}
		else
		{
			const ImVec2 origin  = ImGui::GetCursorScreenPos();
			ImGui::Dummy(ImVec2(availW, lineH));

			const size_t id       = getNodeId(element);
			const double now      = ImGui::GetTime();
			const float  overflow = textW - availW;
			float        offsetX  = 0.0F;

			if (ImGui::IsItemHovered())
			{
				if (!s_hoverStart.contains(id))
					s_hoverStart[id] = now;
				if (const auto elapsed = static_cast<float>(now - s_hoverStart.at(id)); elapsed > scrollDelay)
				{
					const float scrollTime = elapsed - scrollDelay;
					const float cycleDur   = (overflow / scrollSpeed) + scrollPause;
					const float phase      = std::fmod(scrollTime, cycleDur);
					offsetX = -std::min(phase * scrollSpeed, overflow);
				}
			}
			else
			{
				s_hoverStart.erase(id);
			}

			ImGui::PushClipRect(origin, ImVec2(origin.x + availW, origin.y + lineH), true);
			ImGui::GetWindowDrawList()->AddText(
				ImVec2(origin.x + offsetX, origin.y),
				ImGui::GetColorU32(ImGuiCol_Text),
				name.c_str());
			ImGui::PopClipRect();
		}
		ImGui::PopFont();
	}

	void NodeGraphWindow::renderNodeInlinePreview(const std::shared_ptr<element::Element>& element, const float minNodeSize)
	{
		constexpr float pad       = 0.0f;
		//constexpr float axisLeft  = 24.0f;  // reserved for y-axis labels
		//constexpr float axisBot   = 13.0f;  // reserved for x-axis labels
		constexpr float axisRight = 26.0f;  // reserved for amplitude colorbar

		const auto  label       = element->getLabel();
		const bool  isWeightMap = isWeightMapElement(label);
		const bool  is2D        = element->getElementCommonParameters().dimensionParameters.dimensionality == 2;
		const float plotH       = (isWeightMap || is2D) ? minNodeSize : 60.0f;

		const ImVec2 origin = ImGui::GetCursorScreenPos();
		const ImRect rect(origin, ImVec2(origin.x + minNodeSize, origin.y + plotH));

		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddRectFilled(rect.Min, rect.Max, IM_COL32(255, 255, 255, 40), 4.0f);
		dl->AddRect      (rect.Min, rect.Max, IM_COL32(0,   0,   0,   30), 4.0f);

		const auto* comps = element->getComponents();
		bool drewContent  = false;

		if (isWeightMap && comps && comps->contains("weights"))
		{
			const auto& weights = comps->at("weights");
			const int cols = comps->contains("output") ? static_cast<int>(comps->at("output").size()) : 0;
			const int rows = comps->contains("input")  ? static_cast<int>(comps->at("input").size())  : 0;
			if (cols > 0 && rows > 0 && cols * rows == static_cast<int>(weights.size()))
			{
				drawWeightHeatmap(dl, rect, weights, rows, cols);
				drewContent = true;
			}
		}
		else if (is2D && comps)
		{
			const std::string compName =
				(label == element::ElementLabel::NEURAL_FIELD_2D) ? "activation" : "output";
			if (comps->contains(compName))
			{
				const auto& dp   = element->getElementCommonParameters().dimensionParameters;
				const int   rows = dp.size_x;
				const int   cols = dp.size_y;
				if (const auto& data = comps->at(compName); rows > 0 && cols > 0
					&& static_cast<int>(data.size()) == rows * cols)
				{
					// EMA-smoothed range: adapts to data changes while dampening
					// per-frame jitter that causes colormap band flashing.
					static std::unordered_map<std::string, std::pair<double,double>> s_rangeCache;
					const double frameMin = *std::ranges::min_element(data);
					const double frameMax = *std::ranges::max_element(data);
					if (!std::isfinite(frameMin) || !std::isfinite(frameMax))
						return;
					const std::string key = element->getUniqueName();
					if (const auto it = s_rangeCache.find(key); it == s_rangeCache.end())
						s_rangeCache[key] = { frameMin, frameMax };
					else
					{
						constexpr double alpha = 0.05;
						it->second.first  = it->second.first  * (1.0 - alpha) + frameMin * alpha;
						it->second.second = it->second.second * (1.0 - alpha) + frameMax * alpha;
					}
					auto& [stableMin, stableMax] = s_rangeCache[key];
					if (stableMax - stableMin < 1e-9) stableMax = stableMin + 1.0;

					const ImRect hmRect(
						ImVec2(rect.Min.x,        rect.Min.y + pad),
						ImVec2(rect.Max.x - pad - axisRight, rect.Max.y ));
					draw2DFieldHeatmap(dl, hmRect, data, rows, cols, stableMin, stableMax);
					drawInlineHeatmapAxes(dl, hmRect, rows, cols, stableMin, stableMax);
					drewContent = true;
				}
			}
		}

		if (!drewContent && !isWeightMap && !is2D && comps)
		{
			double globalMin =  1e300, globalMax = -1e300;
			for (const auto& data : *comps | std::views::values)
				for (const double v : data) { globalMin = std::min(globalMin, v); globalMax = std::max(globalMax, v); }
			const double range = (globalMax - globalMin) < 1e-9 ? 1.0 : (globalMax - globalMin);

			int colorIdx = 0;
			for (const auto& data : *comps | std::views::values)
			{
				if (data.size() < 2) { ++colorIdx; continue; }
				const ImVec4 colF = ImPlot::GetColormapColor(colorIdx++, ImPlotColormap_Deep);
				const ImU32  col  = ImGui::ColorConvertFloat4ToU32(ImVec4(colF.x, colF.y, colF.z, 0.86f));
				const int    n    = static_cast<int>(data.size());

				auto toScreen = [&](const int i) -> ImVec2 {
					const float x = rect.Min.x + pad + (rect.GetWidth()  - 2*pad) *
						(static_cast<float>(i) / static_cast<float>(n - 1));
					const float y = rect.Max.y - pad - (rect.GetHeight() - 2*pad) *
						static_cast<float>((data[i] - globalMin) / range);
					return { x, y };
				};

				for (int i = 0; i < n - 1; ++i)
					dl->AddLine(toScreen(i), toScreen(i + 1), col, 1.8f);
			}
		}

		ImGui::Dummy(ImVec2(minNodeSize, plotH));
	}

	void NodeGraphWindow::renderNodePins(const std::shared_ptr<element::Element>& element, const float minNodeSize)
	{
		using ax::Widgets::IconType;
		constexpr auto pinColor = ImVec4(1.0f, 1.0f, 1.0f, 0.90f);
		constexpr auto iconSize = ImVec2(14, 14);

		const auto lbl = element->getLabel();
		const bool hasInputPin =
			lbl != element::ElementLabel::GAUSS_STIMULUS &&
			lbl != element::ElementLabel::GAUSS_STIMULUS_2D &&
			lbl != element::ElementLabel::TIMED_GAUSS_STIMULUS &&
			lbl != element::ElementLabel::TIMED_GAUSS_STIMULUS_2D &&
			lbl != element::ElementLabel::NORMAL_NOISE &&
			lbl != element::ElementLabel::NORMAL_NOISE_2D &&
			lbl != element::ElementLabel::CORRELATED_NORMAL_NOISE &&
			lbl != element::ElementLabel::BOOST_STIMULUS &&
			lbl != element::ElementLabel::BOOST_STIMULUS_2D;

		if (ImGui::BeginTable("##pins", 2, ImGuiTableFlags_None, ImVec2(minNodeSize, 0.f)))
		{
			ImGui::TableSetupColumn("##in",  ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("##out", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			if (hasInputPin)
			{
				ImNodeEditor::BeginPin(startingInputPinId + element->getUniqueIdentifier(),
				                       ImNodeEditor::PinKind::Input);
				ImNodeEditor::PinPivotAlignment(ImVec2(0.0f, 0.5f));
				ax::Widgets::Icon(iconSize, IconType::Circle, true, pinColor, ImVec4(0,0,0,0));
				ImGui::SameLine(0, 4);
				ImGui::TextUnformatted("Input");
				ImNodeEditor::EndPin();
			}

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

	void NodeGraphWindow::renderElementNodeConnections(const std::shared_ptr<element::Element>& element)
	{
		constexpr float thickness = 2.0f;
		constexpr auto linkCol   = ImVec4(0.08f, 0.08f, 0.08f, 0.85f); // near-black

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

	void NodeGraphWindow::handleInteractions() const
	{
		handlePinInteractions();
		handleLinkInteractions();
		handleNodeSelection();
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

	void NodeGraphWindow::handleNodeSelection() const
	{
		const ImNodeEditor::NodeId hovered = ImNodeEditor::GetHoveredNode();
		if (!hovered) return;

		const size_t id = hovered.Get();

		// Find the element associated with this node.
		std::shared_ptr<element::Element> element;
		for (const auto& el : simulation->getElements())
			if (getNodeId(el) == id) { element = el; break; }
		if (!element) return;

		// Single-click to select an element in the control panel.
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			ElementWindow::setFocusedElement(element);

		// Double-click to open a plot card.
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			if (!plotCards.contains(id))
			{
				PlotCardState state;
				const float midX = ngBoundsMin.x + (ngBoundsMax.x - ngBoundsMin.x) * 0.5f;
				const float midY = ngBoundsMin.y + (ngBoundsMax.y - ngBoundsMin.y) * 0.5f;
				state.initialPos = ImVec2(midX - state.size.x * 0.5f, midY - state.size.y * 0.5f);
				plotCards[id] = state;
			}
		}
	}

	void NodeGraphWindow::renderNodePlotCards() const
	{
		for (auto it = plotCards.begin(); it != plotCards.end(); )
		{
			const size_t nodeId = it->first;
			PlotCardState& state = it->second;

			std::shared_ptr<element::Element> element;
			for (const auto& el : simulation->getElements())
				if (getNodeId(el) == nodeId) { element = el; break; }
			if (!element) { it = plotCards.erase(it); continue; }

			constexpr ImGuiWindowFlags cardFlags =
				ImGuiWindowFlags_NoCollapse      |
				ImGuiWindowFlags_NoDocking       |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_MenuBar;

			const auto  lbl      = element->getLabel();
			const bool  isWM     = isWeightMapElement(lbl);
			const bool  is2D     = element->getElementCommonParameters().dimensionParameters.dimensionality == 2;
			if (state.isFirstFrame)
			{
				ImGui::SetNextWindowPos (state.initialPos, ImGuiCond_Always);
				ImGui::SetNextWindowSize(state.size,       ImGuiCond_Always);
				state.isFirstFrame = false;
			}

			bool open = true;
			const float ui = ImGui::GetIO().FontGlobalScale;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 2.0f * ui));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.95f, 0.97f, 0.98f, 1.0f));
			ImGui::PushFont(g_BlackLargeFont);
			const bool visible = ImGui::Begin(element->getUniqueName().c_str(), &open, cardFlags);
			ImGui::PopFont();
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();

			if (visible)
			{
				renderPlotCardMenuBar(state, is2D, element);
				renderPlotCardContent(element, state, isWM, is2D);
			}

			if (!open) { it = plotCards.erase(it); ImGui::End(); continue; }
			ImGui::End();
			++it;
		}
	}

	void NodeGraphWindow::renderPlotCardMenuBar(PlotCardState& state, const bool is2DField,
		const std::shared_ptr<element::Element>& element)
	{
		if (!ImGui::BeginMenuBar()) return;

		if (ImGui::BeginMenu("Dimensions"))
		{
			ImGui::DragFloat("X max",  &state.xMax,  0.1f, state.xMin, 1000.f,    "%.1f");
			ImGui::DragFloat("Y max",  &state.yMax,  0.1f, state.yMin, 1000.f,    "%.2f");
			ImGui::DragFloat("X min",  &state.xMin,  0.1f, -1000.f,   state.xMax, "%.1f");
			ImGui::DragFloat("Y min",  &state.yMin,  0.1f, -10000.f,  state.yMax, "%.2f");
			ImGui::DragFloat("X step", &state.xStep, 0.1f, 0.1f,      1000.f,    "%.1f");
			ImGui::Checkbox("Auto-fit", &state.autoFit);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Annotations"))
		{
			ImGui::InputText("Title",   state.title,  sizeof(state.title));
			ImGui::InputText("X label", state.xLabel, sizeof(state.xLabel));
			ImGui::InputText("Y label", state.yLabel, sizeof(state.yLabel));
			ImGui::EndMenu();
		}
		if (!is2DField)
		{
			if (ImGui::BeginMenu("Line Thickness"))
			{
				ImGui::SliderFloat("##lt", &state.lineThickness, 0.1f, 10.0f, "%.1f");
				ImGui::EndMenu();
			}
		}
		else
		{
			if (ImGui::BeginMenu("Colormap"))
			{
				if (ImPlot::ColormapButton(ImPlot::GetColormapName(state.colormap),
					ImVec2(120.0f, 0.0f), state.colormap))
				{
					state.colormap = (state.colormap + 1) % ImPlot::GetColormapCount();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Scale"))
			{
				ImGui::DragFloatRange2("Min / Max", &state.scaleMin, &state.scaleMax,
					0.01f, -1000.f, 1000.f, "%.2f");
				ImGui::Checkbox("Auto scale", &state.autoScale);
				ImGui::EndMenu();
			}
			if (const auto* comps = element->getComponents(); comps && ImGui::BeginMenu("Component"))
			{
				const std::string defaultComp =
					(element->getLabel() == element::ElementLabel::NEURAL_FIELD_2D) ? "activation" : "output";
				const std::string activeComp =
					(state.selectedComponent[0] != '\0') ? state.selectedComponent : defaultComp;
				for (const auto& name : *comps | std::views::keys)
				{
					const bool selected = (activeComp == name);
					if (ImGui::MenuItem(name.c_str(), nullptr, selected))
						std::snprintf(state.selectedComponent, sizeof(state.selectedComponent), "%s", name.c_str());
				}
				ImGui::EndMenu();
			}
		}

		ImGui::EndMenuBar();
	}

	void NodeGraphWindow::renderPlotCardContent(const std::shared_ptr<element::Element>& element,
		PlotCardState& state, const bool isWM, const bool is2DField)
	{
		const auto* comps = element->getComponents();
		const float plotW = ImGui::GetContentRegionAvail().x;
		const float plotH = ImGui::GetContentRegionAvail().y;

		if (isWM && comps && comps->contains("weights"))
		{
			const auto& weights = comps->at("weights");
			const int rows = comps->contains("input")  ? static_cast<int>(comps->at("input").size())  : 0;
			const int cols = comps->contains("output") ? static_cast<int>(comps->at("output").size()) : 0;
			if (rows > 0 && cols > 0 && rows * cols == static_cast<int>(weights.size()))
			{
				const ImVec2 origin = ImGui::GetCursorScreenPos();
				ImGui::Dummy(ImVec2(plotW, plotH));
				const ImRect rect(origin, ImVec2(origin.x + plotW, origin.y + plotH));
				drawWeightHeatmap(ImGui::GetWindowDrawList(), rect, weights, rows, cols);
			}
		}
		else if (is2DField && comps)
		{
			const std::string defaultComp =
				(element->getLabel() == element::ElementLabel::NEURAL_FIELD_2D) ? "activation" : "output";
			const std::string compName =
				(state.selectedComponent[0] != '\0') ? state.selectedComponent : defaultComp;
			if (!comps->contains(compName)) return;
			const auto& dp   = element->getElementCommonParameters().dimensionParameters;
			const int   rows = dp.size_x;
			const int   cols = dp.size_y;
			const auto& data = comps->at(compName);
			if (rows > 0 && cols > 0 && static_cast<int>(data.size()) == rows * cols)
			{
				double scMin, scMax;
				if (state.autoScale)
				{
					scMin = *std::ranges::min_element(data);
					scMax = *std::ranges::max_element(data);
					if (!std::isfinite(scMin) || !std::isfinite(scMax)) return;
					if (scMax - scMin < 1e-9) scMax = scMin + 1.0;
				}
				else
				{
					scMin = state.scaleMin;
					scMax = state.scaleMax;
					if (scMax <= scMin) scMax = scMin + 1.0;
				}

				if (state.title[0] == '\0' || std::strcmp(state.autoTitleComponent, compName.c_str()) != 0)
				{
					const std::string defaultTitle = element->getUniqueName() + " " + compName;
					std::snprintf(state.title, sizeof(state.title), "%s", defaultTitle.c_str());
					std::snprintf(state.autoTitleComponent, sizeof(state.autoTitleComponent), "%s", compName.c_str());
					std::snprintf(state.xLabel, sizeof(state.xLabel), "%s", "Spatial location x");
					std::snprintf(state.yLabel, sizeof(state.yLabel), "%s", "Spatial location y");
				}

				const float cbW    = 60.0f;
				const float hmW    = plotW - cbW - ImGui::GetStyle().ItemSpacing.x;
				const ImPlotAxisFlags axF = state.autoFit ? ImPlotAxisFlags_AutoFit : ImPlotAxisFlags_None;
				if (!state.autoFit)
					ImPlot::SetNextAxesLimits(state.xMin, state.xMax, state.yMin, state.yMax, ImPlotCond_Always);

				const std::string uniquePlotId = std::string(state.title) + "##node_" + element->getUniqueName();
				ImPlot::PushColormap(state.colormap);
				if (ImPlot::BeginPlot(uniquePlotId.c_str(), ImVec2(hmW, plotH), ImPlotFlags_Crosshairs))
				{
					ImPlot::SetupAxes(state.xLabel, state.yLabel, axF, axF);
					ImPlot::PlotHeatmap("##data", data.data(), rows, cols, scMin, scMax, nullptr,
						ImPlotPoint(0, 0), ImPlotPoint(cols, rows));
					ImPlot::EndPlot();
				}
				ImGui::SameLine(0, 4.0f);
				ImPlot::ColormapScale("##cb", scMin, scMax, ImVec2(cbW, plotH));
				ImPlot::PopColormap();
			}
		}
		else if (!isWM && comps)
		{
			if (state.title[0] == '\0')
			{
				const std::string defaultTitle = element->getUniqueName() + " components";
				std::snprintf(state.title, sizeof(state.title), "%s", defaultTitle.c_str());
			}

			constexpr ImPlotFlags    plotFlags = ImPlotFlags_Crosshairs;
			const ImPlotAxisFlags axF      = state.autoFit ? ImPlotAxisFlags_AutoFit : ImPlotAxisFlags_None;

			if (!state.autoFit)
			{
				ImPlot::SetNextAxesLimits(
					state.xMin, state.xMax,
					state.yMin, state.yMax,
					ImPlotCond_Always);
			}

			const std::string uniquePlotId = std::string(state.title) + "##node_" + element->getUniqueName();
			const ImPlotSpec lineSpec = { ImPlotProp_LineWeight, state.lineThickness };
			ImPlot::PushColormap(state.colormap);
			if (ImPlot::BeginPlot(uniquePlotId.c_str(), ImVec2(plotW, plotH), plotFlags))
			{
				ImPlot::SetupAxes(state.xLabel, state.yLabel, axF, axF);
				ImPlot::SetupLegend(ImPlotLocation_SouthWest, ImPlotLegendFlags_None);

				for (const auto& [name, seriesData] : *comps)
				{
					if (seriesData.size() < 2) continue;
					std::vector<float> xs(seriesData.size()), ys(seriesData.size());
					for (int i = 0; i < static_cast<int>(seriesData.size()); ++i)
					{
						xs[i] = static_cast<float>(i + 1) * state.xStep;
						ys[i] = static_cast<float>(seriesData[i]);
					}
					ImPlot::PlotLine(name.c_str(), xs.data(), ys.data(), static_cast<int>(xs.size()), lineSpec);
				}

				ImPlot::EndPlot();
			}
			ImPlot::PopColormap();
		}
	}

	size_t NodeGraphWindow::getNodeId(const std::shared_ptr<element::Element>& element)
	{
		return std::hash<std::string>{}(element->getUniqueName());
	}

	// Returns a column index for the topology-aware initial layout.
	// Signal flow: sources (0) - kernels (1) - couplings (2) - fields (3)
	int NodeGraphWindow::getColumnForElement(const element::ElementLabel label)
	{
		switch (label)
		{
		case element::ElementLabel::GAUSS_STIMULUS:
		case element::ElementLabel::NORMAL_NOISE:
		case element::ElementLabel::CORRELATED_NORMAL_NOISE:
		case element::ElementLabel::BOOST_STIMULUS:
		case element::ElementLabel::GAUSS_STIMULUS_2D:
		case element::ElementLabel::NORMAL_NOISE_2D:
		case element::ElementLabel::TIMED_GAUSS_STIMULUS:
		case element::ElementLabel::TIMED_GAUSS_STIMULUS_2D:
		case element::ElementLabel::BOOST_STIMULUS_2D:
			return 0;
		case element::ElementLabel::GAUSS_KERNEL:
		case element::ElementLabel::MEXICAN_HAT_KERNEL:
		case element::ElementLabel::OSCILLATORY_KERNEL:
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:
		case element::ElementLabel::MEMORY_TRACE:
		case element::ElementLabel::GAUSS_KERNEL_2D:
		case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:
		case element::ElementLabel::OSCILLATORY_KERNEL_2D:
			return 1;
		case element::ElementLabel::FIELD_COUPLING:
		case element::ElementLabel::GAUSS_FIELD_COUPLING:
			return 2;
		case element::ElementLabel::NEURAL_FIELD:
		case element::ElementLabel::NEURAL_FIELD_2D:
			return 3;
		default:
			return 1;
		}
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
			if (!nf) { ImGui::TextDisabled("(type mismatch)"); break; }
			const auto& p = nf->getParameters();
			ImGui::Text("Tau: %.2f", p.tau);
			ImGui::Text("Resting level: %.2f", p.startingRestingLevel);
			if (p.activationFunction)
				ImGui::Text("Activation fn: %s", p.activationFunction->toString().c_str());
			break;
		}
		case element::ElementLabel::GAUSS_STIMULUS:
		{
			const auto gs = std::dynamic_pointer_cast<element::GaussStimulus>(element);
			if (!gs) { ImGui::TextDisabled("(type mismatch)"); break; }
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
			if (!gk) { ImGui::TextDisabled("(type mismatch)"); break; }
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
			if (!mh) { ImGui::TextDisabled("(type mismatch)"); break; }
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
			if (!ok) { ImGui::TextDisabled("(type mismatch)"); break; }
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
			if (!ak) { ImGui::TextDisabled("(type mismatch)"); break; }
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
			if (!nn) { ImGui::TextDisabled("(type mismatch)"); break; }
			ImGui::Text("Amplitude: %.4f", nn->getParameters().amplitude);
			break;
		}
		case element::ElementLabel::CORRELATED_NORMAL_NOISE:
		{
			const auto cnn = std::dynamic_pointer_cast<element::CorrelatedNormalNoise>(element);
			if (!cnn) { ImGui::TextDisabled("(type mismatch)"); break; }
			const auto& p = cnn->getParameters();
			ImGui::Text("Amplitude: %.4f", p.amplitude);
			ImGui::Text("Width: %.2f",     p.width);
			ImGui::Text("Circular: %s",    p.circular ? "true" : "false");
			break;
		}
		case element::ElementLabel::BOOST_STIMULUS:
		{
			const auto bs = std::dynamic_pointer_cast<element::BoostStimulus>(element);
			if (!bs) { ImGui::TextDisabled("(type mismatch)"); break; }
			const auto& p = bs->getParameters();
			ImGui::Text("Amplitude: %.2f", p.amplitude);
			ImGui::Text("Active: %s",      p.isActive ? "true" : "false");
			break;
		}
		case element::ElementLabel::MEMORY_TRACE:
		{
			const auto mt = std::dynamic_pointer_cast<element::MemoryTrace>(element);
			if (!mt) { ImGui::TextDisabled("(type mismatch)"); break; }
			const auto& p = mt->getParameters();
			ImGui::Text("Tau build: %.2f",  p.tauBuild);
			ImGui::Text("Tau decay: %.2f",  p.tauDecay);
			ImGui::Text("Threshold: %.2f",  p.threshold);
			break;
		}
		case element::ElementLabel::FIELD_COUPLING:
		{
			const auto fc = std::dynamic_pointer_cast<element::FieldCoupling>(element);
			if (!fc) { ImGui::TextDisabled("(type mismatch)"); break; }
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
			if (!gfc) { ImGui::TextDisabled("(type mismatch)"); break; }
			const auto& p  = gfc->getParameters();
			ImGui::Text("In dims: %d x %.2f", p.inputFieldDimensions.x_max, p.inputFieldDimensions.d_x);
			ImGui::Text("Normalized: %s",     p.normalized ? "true" : "false");
			ImGui::Text("Circular: %s",       p.circular   ? "true" : "false");
			ImGui::Text("Couplings: %zu",     p.couplings.size());
			break;
		}
		case element::ElementLabel::NEURAL_FIELD_2D:
		{
			const auto nf = std::dynamic_pointer_cast<element::NeuralField2D>(element);
			const auto& p = nf->getParameters();
			ImGui::Text("Tau: %.2f", p.tau);
			ImGui::Text("Resting level: %.2f", p.startingRestingLevel);
			ImGui::Text("Activation fn: %s", p.activationFunction->toString().c_str());
			break;
		}
		case element::ElementLabel::GAUSS_STIMULUS_2D:
		{
			const auto gs = std::dynamic_pointer_cast<element::GaussStimulus2D>(element);
			const auto& p = gs->getParameters();
			ImGui::Text("Width: %.2f",       p.width);
			ImGui::Text("Amplitude: %.2f",   p.amplitude);
			ImGui::Text("Position x: %.2f",  p.position_x);
			ImGui::Text("Position y: %.2f",  p.position_y);
			ImGui::Text("Circular: %s",      p.circular   ? "true" : "false");
			ImGui::Text("Normalized: %s",    p.normalized ? "true" : "false");
			break;
		}
		case element::ElementLabel::GAUSS_KERNEL_2D:
		{
			const auto gk = std::dynamic_pointer_cast<element::GaussKernel2D>(element);
			const auto& p = gk->getParameters();
			ImGui::Text("Width: %.2f",        p.width);
			ImGui::Text("Amplitude: %.2f",    p.amplitude);
			ImGui::Text("Global amp: %.4f",   p.amplitudeGlobal);
			ImGui::Text("Circular: %s",       p.circular   ? "true" : "false");
			ImGui::Text("Normalized: %s",     p.normalized ? "true" : "false");
			break;
		}
		case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:
		{
			const auto mh = std::dynamic_pointer_cast<element::MexicanHatKernel2D>(element);
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
		case element::ElementLabel::NORMAL_NOISE_2D:
		{
			const auto nn = std::dynamic_pointer_cast<element::NormalNoise2D>(element);
			ImGui::Text("Amplitude: %.4f", nn->getParameters().amplitude);
			break;
		}
		case element::ElementLabel::OSCILLATORY_KERNEL_2D:
		{
			const auto ok = std::dynamic_pointer_cast<element::OscillatoryKernel2D>(element);
			const auto& p = ok->getParameters();
			ImGui::Text("Amplitude: %.2f",       p.amplitude);
			ImGui::Text("Decay: %.4f",           p.decay);
			ImGui::Text("Zero crossings: %.2f",  p.zeroCrossings);
			ImGui::Text("Global amp: %.4f",      p.amplitudeGlobal);
			ImGui::Text("Circular: %s",          p.circular   ? "true" : "false");
			ImGui::Text("Normalized: %s",        p.normalized ? "true" : "false");
			break;
		}
		case element::ElementLabel::TIMED_GAUSS_STIMULUS:
		{
			const auto tgs = std::dynamic_pointer_cast<element::TimedGaussStimulus>(element);
			const auto& p = tgs->getParameters();
			ImGui::Text("Width: %.2f",     p.width);
			ImGui::Text("Amplitude: %.2f", p.amplitude);
			ImGui::Text("Position: %.2f",  p.position);
			ImGui::Text("Intervals: %d",   static_cast<int>(p.onTimes.size()));
			ImGui::Text("Circular: %s",    p.circular    ? "true" : "false");
			ImGui::Text("Normalized: %s",  p.normalized  ? "true" : "false");
			break;
		}
		case element::ElementLabel::BOOST_STIMULUS_2D:
		{
			const auto bs = std::dynamic_pointer_cast<element::BoostStimulus2D>(element);
			const auto& p = bs->getParameters();
			ImGui::Text("Amplitude: %.2f", p.amplitude);
			ImGui::Text("Active: %s",      p.isActive ? "true" : "false");
			break;
		}
		case element::ElementLabel::TIMED_GAUSS_STIMULUS_2D:
		{
			const auto tgs = std::dynamic_pointer_cast<element::TimedGaussStimulus2D>(element);
			const auto& p = tgs->getParameters();
			ImGui::Text("Width: %.2f",      p.width);
			ImGui::Text("Amplitude: %.2f",  p.amplitude);
			ImGui::Text("Position x: %.2f", p.position_x);
			ImGui::Text("Position y: %.2f", p.position_y);
			ImGui::Text("Intervals: %d",    static_cast<int>(p.onTimes.size()));
			ImGui::Text("Circular: %s",     p.circular   ? "true" : "false");
			ImGui::Text("Normalized: %s",   p.normalized ? "true" : "false");
			break;
		}
		default:
			ImGui::TextDisabled("No parameters available.");
			break;
		}

		ImGui::EndTooltip();
	}

	bool NodeGraphWindow::isWeightMapElement(const element::ElementLabel label)
	{
		return label == element::ElementLabel::FIELD_COUPLING ||
		       label == element::ElementLabel::GAUSS_FIELD_COUPLING;
	}

	void NodeGraphWindow::drawInlineHeatmapAxes(ImDrawList* dl, const ImRect& hmRect,
		const int rows, const int cols, const double dMin, const double dMax, const int colormap)
	{
		constexpr float  fs      = 9.0f;
		constexpr ImU32  textCol = IM_COL32( 40,  40,  40, 230);
		constexpr ImU32  tickCol = IM_COL32( 80,  80,  80, 180);
		constexpr int    nTicks  = 4;
		ImFont* const    font    = ImGui::GetFont();

		// // X-axis ticks (column indices) below the heatmap
		// for (int i = 0; i <= nTicks; ++i)
		// {
		// 	const float t   = static_cast<float>(i) / nTicks;
		// 	const float x   = hmRect.Min.x + t * hmRect.GetWidth();
		// 	const int   idx = static_cast<int>(std::round(t * (cols - 1)));
		// 	char buf[8];
		// 	std::snprintf(buf, sizeof(buf), "%d", idx);
		// 	dl->AddLine(ImVec2(x, hmRect.Max.y), ImVec2(x, hmRect.Max.y + 2.0f), tickCol, 1.0f);
		// 	dl->AddText(font, fs, ImVec2(x - 5.0f, hmRect.Max.y + 2.0f), textCol, buf);
		// }
		//
		// // Y-axis ticks (row indices) left of the heatmap
		// for (int i = 0; i <= nTicks; ++i)
		// {
		// 	const float t   = static_cast<float>(i) / nTicks;
		// 	const float y   = hmRect.Min.y + t * hmRect.GetHeight();
		// 	const int   idx = static_cast<int>(std::round((1.0f - t) * (rows - 1)));
		// 	char buf[8];
		// 	std::snprintf(buf, sizeof(buf), "%d", idx);
		// 	dl->AddLine(ImVec2(hmRect.Min.x, y), ImVec2(hmRect.Min.x - 2.0f, y), tickCol, 1.0f);
		// 	dl->AddText(font, fs, ImVec2(hmRect.Min.x - 21.0f, y - fs * 0.5f), textCol, buf);
		// }

		// Amplitude colorbar: vertical strip to the right of the heatmap
		constexpr float barGap = 3.0f;
		constexpr float barW   = 7.0f;
		const float barX0 = hmRect.Max.x + barGap;
		const float barX1 = barX0 + barW;
		const int   steps = std::max(1, static_cast<int>(hmRect.GetHeight()));
		for (int s = 0; s < steps; ++s)
		{
			const float t  = static_cast<float>(s) / steps;
			const float y0 = hmRect.Max.y - (t + 1.0f / steps) * hmRect.GetHeight();
			const float y1 = hmRect.Max.y - t * hmRect.GetHeight();
			const ImVec4 c4  = ImPlot::SampleColormap(t, colormap);
			const ImU32  col = IM_COL32(static_cast<int>(c4.x * 255),
			                            static_cast<int>(c4.y * 255),
			                            static_cast<int>(c4.z * 255), 255);
			dl->AddRectFilled(ImVec2(barX0, y0), ImVec2(barX1, y1), col);
		}
		dl->AddRect(ImVec2(barX0, hmRect.Min.y), ImVec2(barX1, hmRect.Max.y), tickCol, 0.0f, 0, 0.5f);
		// Colorbar ticks and value labels
		char buf[16];
		for (int i = 0; i <= nTicks; ++i)
		{
			const float  t   = static_cast<float>(i) / nTicks;
			const float  y   = hmRect.Max.y - t * hmRect.GetHeight();
			const double val = dMin + t * (dMax - dMin);
			std::snprintf(buf, sizeof(buf), "%.1f", val);
			dl->AddLine(ImVec2(barX1, y), ImVec2(barX1 + 2.0f, y), tickCol, 1.0f);
			dl->AddText(font, fs, ImVec2(barX1 + 3.0f, y - fs * 0.5f), textCol, buf);
		}
	}

	void NodeGraphWindow::drawWeightHeatmap(ImDrawList* dl, const ImRect rect,
		const std::vector<double>& weights, const int rows, const int cols)
	{
		constexpr float pad = 3.0f;
		double wMin =  1e300, wMax = -1e300;
		for (const double v : weights) { wMin = std::min(wMin, v); wMax = std::max(wMax, v); }
		const double wRange = (wMax - wMin) < 1e-9 ? 1.0 : (wMax - wMin);
		const float cellW = (rect.GetWidth()  - 2*pad) / static_cast<float>(cols);
		const float cellH = (rect.GetHeight() - 2*pad) / static_cast<float>(rows);
		for (int r = 0; r < rows; ++r)
			for (int c = 0; c < cols; ++c)
			{
				const float  t   = static_cast<float>((weights[r*cols+c] - wMin) / wRange);
				const ImVec2 tl  = { rect.Min.x + pad + c*cellW, rect.Max.y - pad - (r+1)*cellH };
				const ImVec4 col = ImPlot::SampleColormap(t, ImPlotColormap_Deep);
				dl->AddRectFilled(tl, { tl.x+cellW, tl.y+cellH }, ImGui::ColorConvertFloat4ToU32(col));
			}
	}

	void NodeGraphWindow::draw2DFieldHeatmap(ImDrawList* dl, const ImRect rect,
		const std::vector<double>& data, const int rows, const int cols,
		const double wMin, const double wMax, const int colormap)
	{
		constexpr float pad = 3.0f;
		dl->AddRectFilled(rect.Min, rect.Max, IM_COL32(255, 255, 255, 40), 4.0f);
		dl->AddRect      (rect.Min, rect.Max, IM_COL32(0,   0,   0,   30), 4.0f);
		const double wRange = (wMax - wMin) < 1e-9 ? 1.0 : (wMax - wMin);
		const float cellW = (rect.GetWidth()  - 2*pad) / static_cast<float>(cols);
		const float cellH = (rect.GetHeight() - 2*pad) / static_cast<float>(rows);
		for (int r = 0; r < rows; ++r)
			for (int c = 0; c < cols; ++c)
			{
				const float  t   = static_cast<float>((data[r*cols+c] - wMin) / wRange);
				const ImVec2 tl  = { rect.Min.x + pad + c*cellW, rect.Max.y - pad - (r+1)*cellH };
				const ImVec4 col = ImPlot::SampleColormap(t, colormap);
				dl->AddRectFilled(tl, { tl.x+cellW, tl.y+cellH }, ImGui::ColorConvertFloat4ToU32(col));
			}
	}
}
