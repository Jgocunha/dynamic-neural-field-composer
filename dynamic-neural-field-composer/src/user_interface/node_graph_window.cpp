#include "user_interface/node_graph_window.h"

extern ImFont* g_BoldLargeFont;
extern ImFont* g_BlackLargeFont;
extern ImFont* g_BlackSmallFont;

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

		ImGui::PushFont(g_BlackLargeFont);
		const bool open = ImGui::Begin("Node Graph", nullptr, flags);
		ImGui::PopFont();

		if (open)
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

			// Record node graph window bounds for plot card clamping.
			const ImVec2 ngPos  = ImGui::GetWindowPos();
			const ImVec2 ngSize = ImGui::GetWindowSize();
			ngBoundsMin_ = ngPos;
			ngBoundsMax_ = ImVec2(ngPos.x + ngSize.x, ngPos.y + ngSize.y);
		}
		ImGui::End();

		// Plot cards are separate top-level windows â€” must be rendered outside Begin/End.
		renderNodePlotCards();
	}

	void NodeGraphWindow::renderGraph() const
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

		// Record node graph window bounds for plot card clamping.
		const ImVec2 ngPos  = ImGui::GetWindowPos();
		const ImVec2 ngSize = ImGui::GetWindowSize();
		ngBoundsMin_ = ngPos;
		ngBoundsMax_ = ImVec2(ngPos.x + ngSize.x, ngPos.y + ngSize.y);

		// Plot cards are separate top-level windows must be rendered outside Begin/End.
		renderNodePlotCards();
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
		// Apply initial positions queued in the previous frame.
		// SetNodePosition must be called before BeginNode to take effect this frame.
		for (const auto& [nodeId, pos] : pendingInitialPositions_)
			ImNodeEditor::SetNodePosition(nodeId, pos);
		pendingInitialPositions_.clear();

		for (const auto& element : simulation->getElements())
			renderElementNode(element);

		// After nodes have been submitted, we can read their actual canvas positions.
		// Any node still sitting at the origin (0,0) has no saved layout â€”
		// queue a grid position so it spreads out on the next frame.
		constexpr float colSpacing = 300.0f;
		constexpr float rowSpacing = 250.0f;
		constexpr float baseX     =  50.0f;
		constexpr float baseY     =  50.0f;
		constexpr int   maxRows   =   5;

		// Compute how many physical columns each type-group needs (ceil(count / maxRows)),
		// then assign cumulative x offsets so overflow columns never collide with the
		// next group's base column.
		//
		// columnCounts is derived from the *current* simulation's elements only, so it
		// resets naturally whenever a different simulation is loaded â€” no stale state
		// from a previously loaded (possibly larger) simulation bleeds through.
		const auto& elements = simulation->getElements();
		std::array<int, 4> groupCount    = {};
		std::array<int, 4> columnCounts  = {};   // already-positioned nodes per group
		for (const auto& el : elements)
		{
			const int g = getColumnForElement(el->getLabel());
			++groupCount[g];
			if (positionedNodeIds_.contains(getNodeId(el)))
				++columnCounts[g];
		}

		std::array<float, 4> groupBaseX = {};
		float curX = baseX;
		for (int g = 0; g < 4; ++g)
		{
			groupBaseX[g] = curX;
			const int cols = std::max(1, (groupCount[g] + maxRows - 1) / maxRows);
			curX += cols * colSpacing;
		}

		for (const auto& element : elements)
		{
			const size_t nodeId = getNodeId(element);
			if (!positionedNodeIds_.contains(nodeId))
			{
				const ImVec2 pos = ImNodeEditor::GetNodePosition(nodeId);
				if (std::abs(pos.x) < 1.0f && std::abs(pos.y) < 1.0f)
				{
					const int g      = getColumnForElement(element->getLabel());
					const int rowIdx = columnCounts[g]++;
					const int col    = rowIdx / maxRows;
					const int row    = rowIdx % maxRows;
					pendingInitialPositions_[nodeId] =
						ImVec2(groupBaseX[g] + col * colSpacing, baseY + row * rowSpacing);
				}
				positionedNodeIds_.insert(nodeId);
			}
		}

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

		// Uniform pastel: blend element color 35 % toward white
		const ImVec4 bodyVec4 = {
			headerVec4.x * 0.35f + 0.65f,
			headerVec4.y * 0.35f + 0.65f,
			headerVec4.z * 0.35f + 0.65f,
			1.0f
		};

		// No border, uniform color, tighter top padding, so the title sits higher
		ImNodeEditor::PushStyleVar(ImNodeEditor::StyleVar_NodeRounding,    10.0f);
		ImNodeEditor::PushStyleVar(ImNodeEditor::StyleVar_NodeBorderWidth, 0.0f);
		ImNodeEditor::PushStyleVar(ImNodeEditor::StyleVar_NodePadding,     ImVec4(8, 4, 8, 6));
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_NodeBg,     bodyVec4);
		ImNodeEditor::PushStyleColor(ImNodeEditor::StyleColor_NodeBorder, bodyVec4);

		util::BlueprintNodeBuilder builder;
		builder.Begin(nodeId);

		bool showTooltip = false;

		// ----- "HEADER" section used for the full node body ------------------
		// Using the same pastel color as NodeBg so the node is visually uniform.
		// Putting all content here forces Input/Output pins to appear below it.
		builder.Header(bodyVec4);
		{
			// Fixed node width â€” the name is clipped and scrolls on hover.
			static constexpr float minNodeSize = 250.0f;
			static constexpr float scrollSpeed  = 50.0f;  // px / sec
			static constexpr float scrollDelay  = 0.5f;   // sec pause before scrolling
			static constexpr float scrollPause  = 1.0f;   // sec pause after full scroll
			static std::unordered_map<size_t, double> s_hoverStart;

			ImGui::Dummy(ImVec2(minNodeSize, 0));

			ImGui::PushFont(g_BlackLargeFont);
			{
				const std::string& name  = element->getUniqueName();
				const float        lineH = ImGui::GetTextLineHeight();
				const float        textW = ImGui::CalcTextSize(name.c_str()).x;
				// Leave room for node padding on the right so the clip aligns with the body edge.
				constexpr float nodePad = 16.0f;
				const float     availW  = minNodeSize - nodePad;

				if (textW <= availW)
				{
					ImGui::TextUnformatted(name.c_str());
				}
				else
				{
					const ImVec2 origin = ImGui::GetCursorScreenPos();
					// Reserve the fixed slot â€” IsItemHovered() reads this rect.
					ImGui::Dummy(ImVec2(availW, lineH));

					const size_t id      = getNodeId(element);
					const double now     = ImGui::GetTime();
					const float  overflow = textW - availW;
					float        offsetX  = 0.0f;

					if (ImGui::IsItemHovered())
					{
						if (!s_hoverStart.contains(id))
							s_hoverStart[id] = now;

						const float elapsed = static_cast<float>(now - s_hoverStart.at(id));
						if (elapsed > scrollDelay)
						{
							const float scrollTime = elapsed - scrollDelay;
							const float cycleDur   = overflow / scrollSpeed + scrollPause;
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
			}
			ImGui::PopFont();

			// Info icon â€” hover shows parameters
			ImGui::Spacing();
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_CIRCLE_INFO);
			ImGui::PopFont();
			showTooltip = ImGui::IsItemHovered();
			ImGui::Spacing();

			// ---- Inline sparkline for all components ----
			{
				constexpr float plotW = minNodeSize;
				constexpr float pad   = 3.0f;
				const auto  label       = element->getLabel();
				const bool  isWeightMap = (label == element::ElementLabel::FIELD_COUPLING ||
				                           label == element::ElementLabel::GAUSS_FIELD_COUPLING);
				const float plotH = isWeightMap ? plotW : 60.0f;

				const ImVec2 origin = ImGui::GetCursorScreenPos();
				const ImRect rect(origin, ImVec2(origin.x + plotW, origin.y + plotH));

				ImDrawList* dl = ImGui::GetWindowDrawList();
				dl->AddRectFilled(rect.Min, rect.Max, IM_COL32(255, 255, 255, 40), 4.0f);
				dl->AddRect      (rect.Min, rect.Max, IM_COL32(0,   0,   0,   30), 4.0f);

				const auto* comps = element->getComponents();
				bool drewHeatmap = false;

				if (isWeightMap && comps && comps->count("weights"))
				{
					const auto& weights = comps->at("weights");
					const int cols = comps->count("output") ? static_cast<int>(comps->at("output").size()) : 0;
					const int rows = comps->count("input")  ? static_cast<int>(comps->at("input").size())  : 0;

					if (cols > 0 && rows > 0 && cols * rows == static_cast<int>(weights.size()))
					{
						double wMin =  1e300, wMax = -1e300;
						for (const double v : weights) { wMin = std::min(wMin, v); wMax = std::max(wMax, v); }
						const double wRange = (wMax - wMin) < 1e-9 ? 1.0 : (wMax - wMin);

						const float cellW = (rect.GetWidth()  - 2*pad) / float(cols);
						const float cellH = (rect.GetHeight() - 2*pad) / float(rows);

						for (int r = 0; r < rows; ++r)
						{
							for (int c = 0; c < cols; ++c)
							{
								const double v   = weights[r * cols + c];
								const float  t   = float((v - wMin) / wRange);
								const ImVec2 tl  = { rect.Min.x + pad + c * cellW, rect.Max.y - pad - (r + 1) * cellH };
								const ImVec2 br  = { tl.x + cellW, tl.y + cellH };
								const ImVec4 col = ImPlot::SampleColormap(t, ImPlotColormap_Deep);
								dl->AddRectFilled(tl, br, ImGui::ColorConvertFloat4ToU32(col));
							}
						}
						drewHeatmap = true;
					}
				}

				if (!drewHeatmap && !isWeightMap && comps)
				{
					double globalMin =  1e300, globalMax = -1e300;
					for (const auto& [name, data] : *comps)
						for (const double v : data) { globalMin = std::min(globalMin, v); globalMax = std::max(globalMax, v); }
					const double range = (globalMax - globalMin) < 1e-9 ? 1.0 : (globalMax - globalMin);

					int colorIdx = 0;
					for (const auto& [name, data] : *comps)
					{
						if (data.size() < 2) { ++colorIdx; continue; }
						const ImVec4 colF = ImPlot::GetColormapColor(colorIdx++, ImPlotColormap_Deep);
						const ImU32  col  = ImGui::ColorConvertFloat4ToU32(ImVec4(colF.x, colF.y, colF.z, 0.86f));
						const int   n   = static_cast<int>(data.size());

						auto toScreen = [&](int i) -> ImVec2 {
							const float x = rect.Min.x + pad + (rect.GetWidth()  - 2*pad) * (float(i) / float(n - 1));
							const float y = rect.Max.y - pad - (rect.GetHeight() - 2*pad) * float((data[i] - globalMin) / range);
							return { x, y };
						};

						for (int i = 0; i < n - 1; ++i)
							dl->AddLine(toScreen(i), toScreen(i + 1), col, 1.8f);
					}
				}

				// advance cursor past the plot area
				ImGui::Dummy(ImVec2(plotW, plotH));
			}
			ImGui::Spacing();

			// ---- Pin row: Input (flush left) | Output (flush right) ----
			constexpr ImVec4 pinColor = ImVec4(1.0f, 1.0f, 1.0f, 0.90f);
			constexpr ImVec2 iconSize = ImVec2(14, 14);

			// Source elements (no inputs by design)
			const auto lbl = element->getLabel();
			const bool hasInputPin =
				lbl != element::ElementLabel::GAUSS_STIMULUS &&
				lbl != element::ElementLabel::NORMAL_NOISE &&
				lbl != element::ElementLabel::BOOST_STIMULUS;

			if (ImGui::BeginTable("##pins", 2, ImGuiTableFlags_None, ImVec2(minNodeSize, 0.f)))
			{
				ImGui::TableSetupColumn("##in",  ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("##out", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableNextRow();

				// Left cell: Input pin (source elements have none)
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

		// Suspend/Resume outside the BeginHorizontal (builder.Header) context to
		// avoid corrupting the draw list clip-rect stack on long-name nodes.
		if (showTooltip)
		{
			ImNodeEditor::Suspend();
			renderElementTooltip(element);
			ImNodeEditor::Resume();
		}
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

		// Single click â€” select element in the control panel.
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			ElementWindow::setFocusedElement(element);

		// Double click â€” open plot card.
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			if (!plotCards_.contains(id))
			{
				PlotCardState state;
				const float midX = ngBoundsMin_.x + (ngBoundsMax_.x - ngBoundsMin_.x) * 0.5f;
				const float midY = ngBoundsMin_.y + (ngBoundsMax_.y - ngBoundsMin_.y) * 0.5f;
				state.initialPos = ImVec2(midX - state.size.x * 0.5f, midY - state.size.y * 0.5f);
				plotCards_[id] = state;
			}
		}
	}

	void NodeGraphWindow::renderNodePlotCards() const
	{
		for (auto it = plotCards_.begin(); it != plotCards_.end(); )
		{
			const size_t nodeId = it->first;
			PlotCardState& state = it->second;

			// Find the element for this node.
			std::shared_ptr<element::Element> element;
			for (const auto& el : simulation->getElements())
				if (getNodeId(el) == nodeId) { element = el; break; }
			if (!element) { it = plotCards_.erase(it); continue; }

			constexpr ImGuiWindowFlags cardFlags =
				ImGuiWindowFlags_NoCollapse      |
				ImGuiWindowFlags_NoDocking       |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_MenuBar;

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
			ImGui::PopStyleVar();  // only affects title bar, not content

			if (visible)
			{
				// Menu bar: Dimensions | Annotations | Line Thickness
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::BeginMenu("Dimensions"))
					{
						ImGui::DragFloat("X max",  &state.xMax,  0.1f, state.xMin, 1000.f,   "%.1f");
						ImGui::DragFloat("Y max",  &state.yMax,  0.1f, state.yMin, 1000.f,   "%.2f");
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
					if (ImGui::BeginMenu("Line Thickness"))
					{
						ImGui::SliderFloat("##lt", &state.lineThickness, 0.1f, 10.0f, "%.1f");
						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}

				const auto* comps = element->getComponents();
				const auto  lbl   = element->getLabel();
				const bool  isWM  = (lbl == element::ElementLabel::FIELD_COUPLING ||
				                     lbl == element::ElementLabel::GAUSS_FIELD_COUPLING);
				const float plotW = ImGui::GetContentRegionAvail().x;
				const float plotH = ImGui::GetContentRegionAvail().y;

				if (isWM && comps && comps->contains("weights"))
				{
					const auto& weights = comps->at("weights");
					const int rows = comps->contains("input")  ? static_cast<int>(comps->at("input").size())  : 0;
					const int cols = comps->contains("output") ? static_cast<int>(comps->at("output").size()) : 0;

					if (rows > 0 && cols > 0 && rows * cols == static_cast<int>(weights.size()))
					{
						constexpr float pad = 3.0f;
						const ImVec2 origin = ImGui::GetCursorScreenPos();
						ImGui::Dummy(ImVec2(plotW, plotH));
						const ImRect rect(origin, ImVec2(origin.x + plotW, origin.y + plotH));

						ImDrawList* dl = ImGui::GetWindowDrawList();
						double wMin =  1e300, wMax = -1e300;
						for (const double v : weights) { wMin = std::min(wMin, v); wMax = std::max(wMax, v); }
						const double wRange = (wMax - wMin) < 1e-9 ? 1.0 : (wMax - wMin);
						const float cellW = (rect.GetWidth()  - 2*pad) / static_cast<float>(cols);
						const float cellH = (rect.GetHeight() - 2*pad) / static_cast<float>(rows);

						for (int r = 0; r < rows; ++r)
							for (int c = 0; c < cols; ++c)
							{
								const float  t  = static_cast<float>((weights[r*cols+c] - wMin) / wRange);
								const ImVec2 tl = { rect.Min.x + pad + c*cellW, rect.Max.y - pad - (r+1)*cellH };
								const ImVec4 col = ImPlot::SampleColormap(t, ImPlotColormap_Deep);
								dl->AddRectFilled(tl, { tl.x+cellW, tl.y+cellH },
								                  ImGui::ColorConvertFloat4ToU32(col));
							}
					}
				}
				else if (!isWM && comps)
				{
					if (state.title[0] == '\0')
					{
						const std::string defaultTitle = element->getUniqueName() + " components";
						std::snprintf(state.title, sizeof(state.title), "%s", defaultTitle.c_str());
					}

					ImPlotFlags plotFlags = ImPlotFlags_Crosshairs;
					if (state.autoFit)
						plotFlags |= ImPlotFlags_Equal;

					const ImPlotAxisFlags axF = state.autoFit ? ImPlotAxisFlags_AutoFit : ImPlotAxisFlags_None;

					if (!state.autoFit)
					{
						ImPlot::SetNextAxesLimits(
							state.xMin, state.xMax,
							state.yMin, state.yMax,
							ImPlotCond_Always);
					}

					const std::string uniquePlotId = std::string(state.title) + "##node_" + element->getUniqueName();
					if (ImPlot::BeginPlot(uniquePlotId.c_str(), ImVec2(plotW, plotH), plotFlags))
					{
						ImPlot::SetupAxes(state.xLabel, state.yLabel, axF, axF);
						ImPlot::SetupLegend(ImPlotLocation_SouthWest, ImPlotLegendFlags_None);

						const ImPlotSpec lineSpec = { ImPlotProp_LineWeight, state.lineThickness };
						for (const auto& [name, data] : *comps)
						{
							if (data.size() < 2) continue;
							std::vector<float> xs(data.size()), ys(data.size());
							for (int i = 0; i < static_cast<int>(data.size()); ++i)
							{
								xs[i] = static_cast<float>(i + 1) * state.xStep;
								ys[i] = static_cast<float>(data[i]);
							}
							ImPlot::PlotLine(name.c_str(), xs.data(), ys.data(), static_cast<int>(xs.size()), lineSpec);
						}
						ImPlot::EndPlot();
					}
				}
			}

			if (!open) { it = plotCards_.erase(it); ImGui::End(); continue; }
			ImGui::End();
			++it;
		}
	}

	// -------------------------------------------------------------------------
	// Helpers
	// -------------------------------------------------------------------------

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
		case element::ElementLabel::BOOST_STIMULUS:
			return 0;
		case element::ElementLabel::GAUSS_KERNEL:
		case element::ElementLabel::MEXICAN_HAT_KERNEL:
		case element::ElementLabel::OSCILLATORY_KERNEL:
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:
		case element::ElementLabel::MEMORY_TRACE:
			return 1;
		case element::ElementLabel::FIELD_COUPLING:
		case element::ElementLabel::GAUSS_FIELD_COUPLING:
			return 2;
		case element::ElementLabel::NEURAL_FIELD:
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
		default:
			ImGui::TextDisabled("No parameters available.");
			break;
		}

		ImGui::EndTooltip();
	}
}