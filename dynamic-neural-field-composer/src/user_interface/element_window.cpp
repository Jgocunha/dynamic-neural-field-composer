#include "user_interface/element_window.h"

#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <cctype>

#include "elements/neural_field.h"
#include "elements/field_coupling.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/normal_noise.h"
#include "elements/correlated_normal_noise.h"
#include "elements/gauss_field_coupling.h"
#include "elements/oscillatory_kernel.h"
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/boost_stimulus.h"
#include "elements/memory_trace.h"
#include "elements/neural_field_2d.h"
#include "elements/gauss_stimulus_2d.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/mexican_hat_kernel_2d.h"
#include "elements/normal_noise_2d.h"
#include "elements/oscillatory_kernel_2d.h"
#include "elements/timed_gauss_stimulus.h"
#include "elements/timed_gauss_stimulus_2d.h"
#include "elements/boost_stimulus_2d.h"
#include "elements/correlated_normal_noise_2d.h"
#include "elements/asymmetric_gauss_kernel_2d.h"
#include "elements/memory_trace_2d.h"
#include "user_interface/fonts/IconsFontAwesome6.h"

extern ImFont* g_MonoMediumFont;
extern ImFont* g_BlackSmallFont;

namespace dnf_composer::user_interface
{
	// ── param-table helpers ─────────────────────────────────────────────────
	namespace {
		static bool ewBeginTable(const char* id) {
			const float ui     = ImGui::GetIO().FontGlobalScale;
			const float tableW = ImGui::GetContentRegionAvail().x - 10.0f * ui;
			return ImGui::BeginTable(id, 2, ImGuiTableFlags_None, ImVec2(tableW, 0));
		}
		static void ewTableSetup() {
			ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthStretch, 1.0f);
			ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch, 1.0f);
		}
		static void ewEndTable() { ImGui::EndTable(); }
		static void ewRowDrag(const char* lbl, const char* wid, float* v,
		                      float spd, float mn, float mx, const char* fmt = "%.3f") {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(lbl);
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::DragFloat(wid, v, spd, mn, mx, fmt);
			ImGui::PopFont();
		}
		static void ewRowBool(const char* lbl, const char* wid, bool* v) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(lbl);
			ImGui::TableSetColumnIndex(1); ImGui::Checkbox(wid, v);
		}
		static void ewScrollingText(const std::string& name, float availW, int uid) {
			static constexpr float scrollSpeed = 50.0f;
			static constexpr float scrollDelay = 0.5f;
			static constexpr float scrollPause = 1.0f;
			static std::unordered_map<int, double> s_hoverStart;

			const float textW = ImGui::CalcTextSize(name.c_str()).x;
			const float lineH = ImGui::GetTextLineHeight();

			if (textW <= availW) {
				ImGui::TextUnformatted(name.c_str());
				return;
			}

			const ImVec2 origin  = ImGui::GetCursorScreenPos();
			ImGui::Dummy(ImVec2(availW, lineH));

			const double now      = ImGui::GetTime();
			const float  overflow = textW - availW;
			float        offsetX  = 0.0f;

			if (ImGui::IsItemHovered()) {
				if (!s_hoverStart.contains(uid))
					s_hoverStart[uid] = now;
				const auto elapsed = static_cast<float>(now - s_hoverStart.at(uid));
				if (elapsed > scrollDelay) {
					const float cycleDur = (overflow / scrollSpeed) + scrollPause;
					offsetX = -std::min(std::fmod(elapsed - scrollDelay, cycleDur) * scrollSpeed, overflow);
				}
			} else {
				s_hoverStart.erase(uid);
			}

			ImGui::PushClipRect(origin, ImVec2(origin.x + availW, origin.y + lineH), true);
			ImGui::GetWindowDrawList()->AddText(
				ImVec2(origin.x + offsetX, origin.y),
				ImGui::GetColorU32(ImGuiCol_Text),
				name.c_str());
			ImGui::PopClipRect();
		}
		static void ewSectionLabel(const char* lbl) {
			ImGui::Spacing();
			const float ui    = ImGui::GetIO().FontGlobalScale;
			const float lineW = ImGui::GetContentRegionAvail().x - 10.0f * ui;
			const ImVec2 pos  = ImGui::GetCursorScreenPos();
			const float textW = ImGui::CalcTextSize(lbl).x;
			const float lineY = pos.y + ImGui::GetTextLineHeight() * 0.5f;
			const float x0    = pos.x + textW + 6.0f * ui;
			const float x1    = pos.x + lineW;
			ImGui::TextUnformatted(lbl);
			if (x1 > x0)
				ImGui::GetWindowDrawList()->AddLine(
					ImVec2(x0, lineY), ImVec2(x1, lineY),
					ImGui::GetColorU32(ImGuiCol_Separator), 1.0f);
		}
	}

	std::shared_ptr<element::Element> ElementWindow::focusedElement = nullptr;

	// Set when a rename is committed; renderElementControlCard updates selectedName to match.
	static std::string s_pendingRenameOld;
	static std::string s_pendingRenameNew;

	void ElementWindow::setFocusedElement(const std::shared_ptr<element::Element>& element)
	{
		focusedElement = element;
	}

	ElementWindow::ElementWindow(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{}

	void ElementWindow::render()
	{
		const ImGuiViewport* vp = ImGui::GetMainViewport();
		const float panelY = vp->WorkPos.y + 52.0f;
		const float panelH = vp->WorkSize.y - 52.0f - 28.0f;
		ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.25f, panelY), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x * 0.22f, panelH), ImGuiCond_FirstUseEver);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyleColorVec4(ImGuiCol_TitleBg));
		const bool open = ImGui::Begin("Element Control##element_control", nullptr,
			imgui_kit::getGlobalWindowFlags() | ImGuiWindowFlags_NoTitleBar);
		ImGui::PopStyleColor();
		if (open)
		{
			const float startY = ImGui::GetCursorPosY();
			const float yOff = (g_BlackLargeFont->LegacySize - g_MediumIconsFont->LegacySize) * 0.5f;
			ImGui::SetCursorPosY(startY + yOff);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_SLIDERS);
			ImGui::PopFont();
			ImGui::PopStyleColor();
			ImGui::SameLine(0, 8.0F);
			ImGui::SetCursorPosY(startY);
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::TextUnformatted("Element Control");
			ImGui::PopFont();
			ImGui::Separator();
			ImGui::Spacing();
			renderElementControlCard();
		}
		ImGui::End();
	}

	void ElementWindow::renderElementControlCard()
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		// Validate focusedElement still in simulation
		if (focusedElement)
		{
			bool stillValid = false;
			for (const auto& el : simulation->getElements())
				if (el == focusedElement) { stillValid = true; break; }
			if (!stillValid) focusedElement = nullptr;
		}

		// Search bar
		static char searchBuf[128] = {};
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##ew_search", "Search elements...", searchBuf, sizeof(searchBuf));
		ImGui::Spacing();

		// Sync external focus (from node graph)
		static std::string selectedName;
		if (focusedElement)
		{
			selectedName = focusedElement->getUniqueName();
			focusedElement = nullptr;
		}
		if (!s_pendingRenameOld.empty())
		{
			if (selectedName == s_pendingRenameOld)
				selectedName = s_pendingRenameNew;
			s_pendingRenameOld.clear();
			s_pendingRenameNew.clear();
		}

		constexpr ImGuiWindowFlags childFlags = ImGuiWindowFlags_NoSavedSettings;
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0F, 0.0F, 0.0F, 0.0F));
		ImGui::BeginChild("##element_scroll", ImVec2(0, 0), false, childFlags);
		ImGui::PopStyleColor();

		// Build a lowercase search string
		std::string searchLower(searchBuf);
		std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(),
			[](unsigned char c) { return std::tolower(c); });

		// Short badge label per type
		auto shortTypeName = [](const element::ElementLabel lbl) -> const char* {
			switch (lbl) {
				case element::ElementLabel::GAUSS_STIMULUS:              return "Stimulus";
				case element::ElementLabel::NEURAL_FIELD:                return "Field";
				case element::ElementLabel::GAUSS_KERNEL:                return "Kernel";
				case element::ElementLabel::NORMAL_NOISE:                return "Noise";
				case element::ElementLabel::MEMORY_TRACE:                return "Memory";
				case element::ElementLabel::FIELD_COUPLING:              return "Coupling";
				case element::ElementLabel::GAUSS_FIELD_COUPLING:        return "Coupling";
				case element::ElementLabel::MEXICAN_HAT_KERNEL:          return "Kernel";
				case element::ElementLabel::OSCILLATORY_KERNEL:          return "Kernel";
				case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:     return "Kernel";
				case element::ElementLabel::BOOST_STIMULUS:              return "Stimulus";
				case element::ElementLabel::CORRELATED_NORMAL_NOISE:     return "Noise";
				case element::ElementLabel::NEURAL_FIELD_2D:             return "Field2D";
				case element::ElementLabel::GAUSS_STIMULUS_2D:           return "Stimulus2D";
				case element::ElementLabel::GAUSS_KERNEL_2D:             return "Kernel2D";
				case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:       return "Kernel2D";
				case element::ElementLabel::NORMAL_NOISE_2D:             return "Noise2D";
				case element::ElementLabel::OSCILLATORY_KERNEL_2D:       return "Kernel2D";
				case element::ElementLabel::TIMED_GAUSS_STIMULUS:        return "Stimulus";
				case element::ElementLabel::TIMED_GAUSS_STIMULUS_2D:     return "Stimulus2D";
				case element::ElementLabel::BOOST_STIMULUS_2D:           return "Stimulus2D";
				case element::ElementLabel::CORRELATED_NORMAL_NOISE_2D:  return "Noise2D";
				case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D:  return "Kernel2D";
				case element::ElementLabel::MEMORY_TRACE_2D:             return "Memory2D";
				default:                                                 return "Element";
			}
		};

		// Group elements by type
		std::map<element::ElementLabel, std::vector<std::shared_ptr<element::Element>>> byType;
		for (const auto& e : simulation->getElements())
			byType[e->getLabel()].push_back(e);

		if (byType.empty())
		{
			ImGui::TextDisabled("Add elements to the simulation.");
			ImGui::EndChild();
			return;
		}

		static constexpr ImVec4 kCardBg     = { 1.0f, 1.0f, 1.0f, 1.0f };
		static constexpr ImVec4 kCardBorder = { 0.82f, 0.85f, 0.89f, 1.0f };
		static constexpr float  kCardRound  = 8.0f;
		static constexpr float  kCardBordSz = 1.5f;
		static constexpr float  kStripW     = 4.0f;

		ImDrawList* dl   = ImGui::GetWindowDrawList();
		const float rowH = ImGui::GetFrameHeight();
		const float padX = 10.0f * ui;
		const float padY =  8.0f * ui;

		// renderRow: collapsed row or, if selected, a fully auto-sized bordered card
		auto renderRow = [&](const std::shared_ptr<element::Element>& e,
		                     const ImVec4& col, const char* /*badge*/)
		{
			const std::string& name   = e->getUniqueName();
			const bool         isSel  = (name == selectedName);
			const float        avail  = ImGui::GetContentRegionAvail().x;
			const ImU32        dotClr = ImGui::ColorConvertFloat4ToU32(col);

			if (!isSel)
			{
				// ── Collapsed row ─────────────────────────────────────────────
				const ImVec2 rowMin = ImGui::GetCursorScreenPos();
				ImGui::Selectable(("##ew_" + name).c_str(), false, 0, {avail, rowH});
				if (ImGui::IsItemClicked())
					selectedName = name;

				dl->AddCircleFilled({rowMin.x + 10.f, rowMin.y + rowH * 0.5f}, 5.f, dotClr);
				ImGui::SetCursorScreenPos({rowMin.x + 22.f,
					rowMin.y + (rowH - ImGui::GetTextLineHeight()) * 0.5f});
				ewScrollingText(name, avail - 22.f, e->getUniqueIdentifier());
			}
			else
			{
				// ── Expanded card — panel auto-sizes via draw-list channels ───
				const int    dimN    = e->getElementCommonParameters()
				                         .dimensionParameters.dimensionality;
				const ImVec2 panelTL  = ImGui::GetCursorScreenPos();
				const ImU32  fillClr  = ImGui::GetColorU32(kCardBg);
				const ImU32  borderClr = ImGui::GetColorU32(kCardBorder);

				// Channel 0 = background rect (written after content is measured)
				// Channel 1 = content
				dl->ChannelsSplit(2);
				dl->ChannelsSetCurrent(1);

				const float innerW = avail - padX * 2.0f;
				ImGui::SetCursorScreenPos({panelTL.x + padX, panelTL.y + padY});
				ImGui::BeginGroup();

				// Header row — dot + bold name, click to deselect
				{
					const ImVec2 hMin = ImGui::GetCursorScreenPos();
					ImGui::Selectable(("##ew_hdr_" + name).c_str(), false, 0, {innerW, rowH});
					if (ImGui::IsItemClicked())
						selectedName = "";

					dl->AddCircleFilled({hMin.x + 10.f, hMin.y + rowH * 0.5f}, 5.f, dotClr);
					ImGui::SetCursorScreenPos({hMin.x + 22.f,
						hMin.y + (rowH - ImGui::GetTextLineHeight()) * 0.5f});
					ImGui::PushFont(g_BlackMediumFont);
					const char* badge  = shortTypeName(e->getLabel());
					const float badgeW = ImGui::CalcTextSize(badge).x;
					ewScrollingText(name, innerW - 22.f - badgeW - 8.f, e->getUniqueIdentifier());
					ImGui::PopFont();
					dl->AddText(
						ImVec2(hMin.x + innerW - badgeW,
						       hMin.y + (rowH - ImGui::GetTextLineHeight()) * 0.5f),
						ImGui::GetColorU32(ImGuiCol_TextDisabled),
						badge);
				}

				renderIdentifiersSection(e);

				if (dimN == 1)
				{
					ewSectionLabel("Dimensions");
					renderDimensionControls(e);
				}
				else if (dimN == 2)
				{
					ewSectionLabel("Dimensions");
					renderDimensionControls2D(e);
				}
				switchElementToModify(e);

				ImGui::EndGroup();

				// Measure actual content bounds, then draw background behind it
				const ImVec2 contentMax = ImGui::GetItemRectMax();
				const ImVec2 panelBR    = {panelTL.x + avail, contentMax.y + padY};

				dl->ChannelsSetCurrent(0);
				dl->AddRectFilled(panelTL, panelBR, fillClr,   kCardRound);
				dl->AddRect      (panelTL, panelBR, borderClr, kCardRound, 0, kCardBordSz);
				dl->ChannelsMerge();

				ImGui::SetCursorScreenPos({panelTL.x, panelBR.y + 6.0f * ui});
			}
		};

		// Pin the selected element at the top
		if (!selectedName.empty())
		{
			for (const auto& e : simulation->getElements())
			{
				if (e->getUniqueName() == selectedName)
				{
					renderRow(e, getColorForElementType(e->getLabel()), shortTypeName(e->getLabel()));
					ImGui::Separator();
					ImGui::Spacing();
					break;
				}
			}
		}

		// Grouped list — a selected element is skipped (already pinned above)
		for (const auto& [label, elems] : byType)
		{
			std::vector<std::shared_ptr<element::Element>> filtered;
			for (const auto& e : elems)
			{
				if (e->getUniqueName() == selectedName) continue;
				if (searchLower.empty()) { filtered.push_back(e); continue; }
				std::string nameLow = e->getUniqueName();
				std::transform(nameLow.begin(), nameLow.end(), nameLow.begin(),
					[](unsigned char c) { return std::tolower(c); });
				std::string typeLow = shortTypeName(label);
				std::transform(typeLow.begin(), typeLow.end(), typeLow.begin(),
					[](unsigned char c) { return std::tolower(c); });
				if (nameLow.find(searchLower) != std::string::npos ||
					typeLow.find(searchLower) != std::string::npos)
					filtered.push_back(e);
			}
			if (filtered.empty()) continue;

			const std::string hdr = getElementTypeDisplayName(label) + "  \xc2\xb7  " + std::to_string(filtered.size());
			ImGui::PushFont(g_BlackSmallFont);
			ImGui::Text("%s", hdr.c_str());
			ImGui::PopFont();
			ImGui::Separator();

			const ImVec4 col   = getColorForElementType(label);
			const char*  badge = shortTypeName(label);

			for (const auto& e : filtered)
				renderRow(e, col, badge);

			ImGui::Spacing();
		}

		ImGui::EndChild();
	}

	void ElementWindow::renderModifyElementParameters()
	{
		// Group elements by type
		std::map<element::ElementLabel, std::vector<std::shared_ptr<element::Element>>> elementsByType;

		for (const auto& element : simulation->getElements())
		{
			elementsByType[element->getLabel()].push_back(element);
		}

		// Render each group
		for (const auto& [label, elements] : elementsByType)
		{
			ImVec4 color = getColorForElementType(label);
			ImGui::PushStyleColor(ImGuiCol_Header, color);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(color.x * 1.1f, color.y * 1.1f, color.z * 1.1f, color.w));

			std::string headerName = getElementTypeDisplayName(label) + " (" + std::to_string(elements.size()) + ")";

			if (ImGui::CollapsingHeader(headerName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (const auto& element : elements)
				{
					switchElementToModify(element);
					ImGui::Separator();
				}
			}
			ImGui::PopStyleColor(2);
		}

	}

	void ElementWindow::renderIdentifiersSection(const std::shared_ptr<element::Element>& element) const
	{
		ewSectionLabel("Identifiers");

		static std::unordered_map<int, std::string> stagedName;
		const int uid    = element->getUniqueIdentifier();
		auto& staged     = stagedName[uid];
		if (staged.empty())
			staged = element->getUniqueName();

		const std::string elemId = element->getUniqueName();
		ImGui::PushID(("##ids_" + elemId).c_str());

		if (ewBeginTable("##ids_tbl"))
		{
			ewTableSetup();

			// ID (read-only)
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("ID");
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
			char idBuf[32];
			std::snprintf(idBuf, sizeof(idBuf), "%d", uid);
			ImGui::BeginDisabled(true);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::InputText("##uid_val", idBuf, sizeof(idBuf));
			ImGui::PopFont();
			ImGui::EndDisabled();

			// Name (editable)
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Name");
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
			char nameBuf[256];
			std::strncpy(nameBuf, staged.c_str(), sizeof(nameBuf) - 1);
			nameBuf[sizeof(nameBuf) - 1] = '\0';
			ImGui::PushFont(g_MonoMediumFont);
			if (ImGui::InputText("##uname_val", nameBuf, sizeof(nameBuf)))
				staged = nameBuf;
			ImGui::PopFont();
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				const std::string newName(nameBuf);
				if (!newName.empty() && newName != element->getUniqueName())
				{
					s_pendingRenameOld = element->getUniqueName();
					s_pendingRenameNew = newName;
					simulation->renameElement(element->getUniqueName(), newName);
				}
				staged.clear();
			}

			ewEndTable();
		}

		ImGui::PopID();
	}

	void ElementWindow::renderDimensionControls2D(const std::shared_ptr<element::Element>& element) const
	{
		const std::string elemId = element->getUniqueName();

		struct Staged2D { float xMax = 0, yMax = 0, dx = 0, dy = 0; };
		static std::unordered_map<int, Staged2D> staged;
		const int id = element->getUniqueIdentifier();
		auto& s = staged[id];

		const auto& dim = element->getElementCommonParameters().dimensionParameters;
		if (s.xMax == 0.0f)
		{
			s.xMax = static_cast<float>(dim.x_max);
			s.yMax = static_cast<float>(dim.y_max);
			s.dx   = static_cast<float>(dim.d_x);
			s.dy   = static_cast<float>(dim.d_y);
		}

		ImGui::PushID(("##dim2d_" + elemId).c_str());

		if (ewBeginTable("##dim2d_tbl"))
		{
			ewTableSetup();

			auto commitIf = [&]() {
				if (ImGui::IsItemDeactivatedAfterEdit() && s.xMax > 0 && s.yMax > 0 && s.dx > 0 && s.dy > 0)
				{
					const element::ElementDimensions nd(
						static_cast<int>(s.xMax), static_cast<int>(s.yMax),
						static_cast<double>(s.dx), static_cast<double>(s.dy));
					simulation->changeDimensions(elemId, nd);
					const auto& d = element->getElementCommonParameters().dimensionParameters;
					s = { static_cast<float>(d.x_max), static_cast<float>(d.y_max),
					      static_cast<float>(d.d_x),   static_cast<float>(d.d_y) };
				}
			};

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("x size");
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushFont(g_MonoMediumFont); ImGui::DragFloat("##xmax", &s.xMax, 1.0f, 1.0f, 10000.0f, "%.1f"); ImGui::PopFont();
			commitIf();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("x step");
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushFont(g_MonoMediumFont); ImGui::DragFloat("##dx", &s.dx, 0.01f, 0.001f, 10.0f, "%.2f"); ImGui::PopFont();
			commitIf();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("y size");
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushFont(g_MonoMediumFont); ImGui::DragFloat("##ymax", &s.yMax, 1.0f, 1.0f, 10000.0f, "%.1f"); ImGui::PopFont();
			commitIf();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("y step");
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushFont(g_MonoMediumFont); ImGui::DragFloat("##dy", &s.dy, 0.01f, 0.001f, 10.0f, "%.2f"); ImGui::PopFont();
			commitIf();

			ewEndTable();
		}

		ImGui::PopID();
	}

	void ElementWindow::renderDimensionControls(const std::shared_ptr<element::Element>& element) const
	{
		const float ui     = ImGui::GetIO().FontGlobalScale;
		const float inputW = 150.0f * ui;
		const std::string elemId = element->getUniqueName();
		const element::ElementLabel label = element->getLabel();
		const bool isCoupling = label == element::ElementLabel::FIELD_COUPLING ||
		                        label == element::ElementLabel::GAUSS_FIELD_COUPLING;

		static std::unordered_map<int, std::pair<float, float>> staged;
		static std::unordered_map<int, std::pair<float, float>> stagedIn;
		const int id = element->getUniqueIdentifier();
		auto& [stagedXmax, stagedDx] = staged[id];

		const auto& dim = element->getElementCommonParameters().dimensionParameters;
		if (stagedXmax == 0.0f && stagedDx == 0.0f)
		{
			stagedXmax = static_cast<float>(dim.x_max);
			stagedDx   = static_cast<float>(dim.d_x);
		}

		ImGui::PushID(("##dim_" + elemId).c_str());

		if (ewBeginTable("##dim_tbl")) {
			ewTableSetup();

			// Size row
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(isCoupling ? "Out size" : "Size");
			ImGui::SameLine();
			widgets::renderHelpMarker(
				"Changing the field size will disconnect all existing connections\n"
				"to and from this element."
			);
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::DragFloat("##x_max", &stagedXmax, 1.0f, 1.0f, 10000.0f, "%.1f");
			ImGui::PopFont();
			if (ImGui::IsItemDeactivatedAfterEdit() && stagedXmax > 0.0f && stagedDx > 0.0f)
			{
				const element::ElementDimensions newDim(static_cast<int>(stagedXmax), static_cast<double>(stagedDx));
				simulation->changeDimensions(elemId, newDim);
				stagedXmax = static_cast<float>(newDim.x_max);
				stagedDx   = static_cast<float>(newDim.d_x);
			}

			// Step row
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(isCoupling ? "Out step" : "Step");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::DragFloat("##dx", &stagedDx, 0.01f, 0.001f, 10.0f, "%.2f");
			ImGui::PopFont();
			if (ImGui::IsItemDeactivatedAfterEdit() && stagedXmax > 0.0f && stagedDx > 0.0f)
			{
				const element::ElementDimensions newDim(static_cast<int>(stagedXmax), static_cast<double>(stagedDx));
				simulation->changeDimensions(elemId, newDim);
				stagedXmax = static_cast<float>(newDim.x_max);
				stagedDx   = static_cast<float>(newDim.d_x);
			}

			ewEndTable();
		}

		if (isCoupling)
		{
			auto& [stagedInXmax, stagedInDx] = stagedIn[id];

			element::ElementDimensions currentInputDim{};
			if (label == element::ElementLabel::FIELD_COUPLING)
			{
				const auto fc = std::dynamic_pointer_cast<element::FieldCoupling>(element);
				currentInputDim = fc->getParameters().inputFieldDimensions;
			}
			else
			{
				const auto gfc = std::dynamic_pointer_cast<element::GaussFieldCoupling>(element);
				currentInputDim = gfc->getInputFieldDimensions();
			}

			if (stagedInXmax == 0.0f && stagedInDx == 0.0f)
			{
				stagedInXmax = static_cast<float>(currentInputDim.x_max);
				stagedInDx   = static_cast<float>(currentInputDim.d_x);
			}

			auto applyInputDim = [&]() {
				if (stagedInXmax <= 0.0f || stagedInDx <= 0.0f) return;
				const element::ElementDimensions newInDim(static_cast<int>(stagedInXmax), static_cast<double>(stagedInDx));
				element->removeInputs();
				element->removeOutputs();
				if (label == element::ElementLabel::FIELD_COUPLING)
					std::dynamic_pointer_cast<element::FieldCoupling>(element)->changeInputDimensions(newInDim);
				else
					std::dynamic_pointer_cast<element::GaussFieldCoupling>(element)->changeInputDimensions(newInDim);
				stagedInXmax = static_cast<float>(newInDim.x_max);
				stagedInDx   = static_cast<float>(newInDim.d_x);
			};

			if (ewBeginTable("##in_dim_tbl")) {
				ewTableSetup();

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("In size");
				ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::PushFont(g_MonoMediumFont);
				ImGui::DragFloat("##in_x_max", &stagedInXmax, 1.0f, 1.0f, 10000.0f, "%.1f");
				ImGui::PopFont();
				if (ImGui::IsItemDeactivatedAfterEdit()) applyInputDim();

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("In step");
				ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::PushFont(g_MonoMediumFont);
				ImGui::DragFloat("##in_dx", &stagedInDx, 0.01f, 0.001f, 10.0f, "%.2f");
				ImGui::PopFont();
				if (ImGui::IsItemDeactivatedAfterEdit()) applyInputDim();

				ewEndTable();
			}
		}

		ImGui::PopID();
	}

	void ElementWindow::switchElementToModify(const std::shared_ptr<element::Element>& element)
	{
		const std::string elementId = element->getUniqueName();
		static bool missingElementAcknowledged = false;

		switch (const element::ElementLabel label = element->getLabel())
		{
		case element::ElementLabel::NEURAL_FIELD:
			modifyElementNeuralField(element);
			break;
		case element::ElementLabel::GAUSS_STIMULUS:
			modifyElementGaussStimulus(element);
			break;
		case element::ElementLabel::FIELD_COUPLING:
			modifyElementFieldCoupling(element);
			break;
		case element::ElementLabel::GAUSS_KERNEL:
			modifyElementGaussKernel(element);
			break;
		case element::ElementLabel::MEXICAN_HAT_KERNEL:
			modifyElementMexicanHatKernel(element);
			break;
		case element::ElementLabel::NORMAL_NOISE:
			modifyElementNormalNoise(element);
			break;
		case element::ElementLabel::CORRELATED_NORMAL_NOISE:
			modifyElementCorrelatedNormalNoise(element);
			break;
		case element::ElementLabel::GAUSS_FIELD_COUPLING:
			modifyElementGaussFieldCoupling(element);
			break;
		case element::ElementLabel::OSCILLATORY_KERNEL:
			modifyElementOscillatoryKernel(element);
			break;
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:
			modifyElementAsymmetricGaussKernel(element);
			break;
		case element::ElementLabel::BOOST_STIMULUS:
			modifyElementBoostStimulus(element);
			break;
		case element::ElementLabel::MEMORY_TRACE:
			modifyElementMemoryTrace(element);
			break;
		case element::ElementLabel::NEURAL_FIELD_2D:
			modifyElementNeuralField2D(element);
			break;
		case element::ElementLabel::GAUSS_STIMULUS_2D:
			modifyElementGaussStimulus2D(element);
			break;
		case element::ElementLabel::GAUSS_KERNEL_2D:
			modifyElementGaussKernel2D(element);
			break;
		case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:
			modifyElementMexicanHatKernel2D(element);
			break;
		case element::ElementLabel::NORMAL_NOISE_2D:
			modifyElementNormalNoise2D(element);
			break;
		case element::ElementLabel::OSCILLATORY_KERNEL_2D:
			modifyElementOscillatoryKernel2D(element);
			break;
		case element::ElementLabel::TIMED_GAUSS_STIMULUS:
			modifyElementTimedGaussStimulus(element);
			break;
		case element::ElementLabel::TIMED_GAUSS_STIMULUS_2D:
			modifyElementTimedGaussStimulus2D(element);
			break;
		case element::ElementLabel::BOOST_STIMULUS_2D:
			modifyElementBoostStimulus2D(element);
			break;
		case element::ElementLabel::CORRELATED_NORMAL_NOISE_2D:
			modifyElementCorrelatedNormalNoise2D(element);
			break;
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D:
			modifyElementAsymmetricGaussKernel2D(element);
			break;
		case element::ElementLabel::MEMORY_TRACE_2D:
			modifyElementMemoryTrace2D(element);
			break;
		case element::ElementLabel::UNINITIALIZED:
			break;
		default:
			if (!missingElementAcknowledged)
			{
				log(tools::logger::LogLevel::ERROR, "There is a missing element in the "
					"TreeNode in simulation window.");
				missingElementAcknowledged = true;
			}
			break;
		}
	}

	void ElementWindow::modifyElementNeuralField(const std::shared_ptr<element::Element>& element)
	{
		const auto neuralField = std::dynamic_pointer_cast<element::NeuralField>(element);
		element::NeuralFieldParameters nfp = neuralField->getParameters();
		const std::string uid = element->getUniqueName();

		auto restingLevel = static_cast<float>(nfp.startingRestingLevel);
		auto tau          = static_cast<float>(nfp.tau);

		ewSectionLabel("Dynamics");
		if (ewBeginTable(("##nf_dyn" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Resting level", ("##nf_rl"  + uid).c_str(), &restingLevel, 0.1f, -30.0f,   0.0f);
			ewRowDrag("Time scale",           ("##nf_tau"  + uid).c_str(), &tau,          0.5f,   1.0f, 300.0f);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(restingLevel - static_cast<float>(nfp.startingRestingLevel)) > epsilon ||
		    std::abs(tau          - static_cast<float>(nfp.tau))                  > epsilon)
		{
			nfp.startingRestingLevel = restingLevel;
			nfp.tau = tau;
			neuralField->setParameters(nfp);
		}

		bool updated = false;
		ewSectionLabel("Activation function");
		if (ewBeginTable(("##nf_act" + uid).c_str())) {
			ewTableSetup();
			static const char* actFnNames[] = { "Sigmoid", "Heaviside", "AbsSigmoid" };
			int actFnType = nfp.activationFunction
				? static_cast<int>(nfp.activationFunction->type) : element::SIGMOID;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Function");
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::Combo(("##nf_fn" + uid).c_str(), &actFnType, actFnNames, 3))
			{
				switch (actFnType) {
				case element::SIGMOID:    nfp.activationFunction = std::make_unique<element::SigmoidFunction>(0.0, 10.0); break;
				case element::HEAVISIDE:  nfp.activationFunction = std::make_unique<element::HeavisideFunction>(0.0); break;
				case element::ABSSIGMOID: nfp.activationFunction = std::make_unique<element::AbsSigmoidFunction>(0.0, 100.0); break;
				default: break;
				}
				updated = true;
			}
			if (actFnType == element::SIGMOID)
			{
				const auto* sig = dynamic_cast<const element::SigmoidFunction*>(nfp.activationFunction.get());
				const float storedXShift    = sig ? static_cast<float>(sig->getXShift())    : 0.0f;
				const float storedSteepness = sig ? static_cast<float>(sig->getSteepness()) : 10.0f;
				auto xShift    = storedXShift;
				auto steepness = storedSteepness;
				ewRowDrag("x shift",   ("##nf_xs" + uid).c_str(), &xShift,    0.1f,  -30.0f, 30.0f);
				ewRowDrag("Steepness", ("##nf_st" + uid).c_str(), &steepness, 0.5f,    0.1f, 500.0f);
				if (std::abs(xShift - storedXShift) > epsilon || std::abs(steepness - storedSteepness) > epsilon)
				{
					nfp.activationFunction = std::make_unique<element::SigmoidFunction>(
						static_cast<double>(xShift), static_cast<double>(steepness));
					updated = true;
				}
			}
			else if (actFnType == element::HEAVISIDE)
			{
				const auto* hv = dynamic_cast<const element::HeavisideFunction*>(nfp.activationFunction.get());
				const float storedXShift = hv ? static_cast<float>(hv->getXShift()) : 0.0f;
				auto xShift = storedXShift;
				ewRowDrag("x shift", ("##nf_hxs" + uid).c_str(), &xShift, 0.1f, -30.0f, 30.0f);
				if (std::abs(xShift - storedXShift) > epsilon)
				{
					nfp.activationFunction = std::make_unique<element::HeavisideFunction>(static_cast<double>(xShift));
					updated = true;
				}
			}
			else if (actFnType == element::ABSSIGMOID)
			{
				const auto* ab = dynamic_cast<const element::AbsSigmoidFunction*>(nfp.activationFunction.get());
				const float storedXShift = ab ? static_cast<float>(ab->getXShift()) : 0.0f;
				const float storedBeta   = ab ? static_cast<float>(ab->getBeta())   : 100.0f;
				auto xShift = storedXShift;
				auto beta   = storedBeta;
				ewRowDrag("x shift", ("##nf_axs" + uid).c_str(), &xShift, 0.1f,  -30.0f,  30.0f);
				ewRowDrag("Beta",    ("##nf_ab"  + uid).c_str(), &beta,   1.0f,    0.1f, 1000.0f);
				if (std::abs(xShift - storedXShift) > epsilon || std::abs(beta - storedBeta) > epsilon)
				{
					nfp.activationFunction = std::make_unique<element::AbsSigmoidFunction>(
						static_cast<double>(xShift), static_cast<double>(beta));
					updated = true;
				}
			}
			ewEndTable();
		}
		if (updated) neuralField->setParameters(nfp);
	}

	void ElementWindow::modifyElementGaussStimulus(const std::shared_ptr<element::Element>& element)
	{
		const auto stimulus = std::dynamic_pointer_cast<element::GaussStimulus>(element);
		element::GaussStimulusParameters gsp = stimulus->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude = static_cast<float>(gsp.amplitude);
		auto width     = static_cast<float>(gsp.width);
		auto position  = static_cast<float>(gsp.position);
		bool circular  = gsp.circular;
		bool normalized = gsp.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##gs_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude", ("##gs_amp" + uid).c_str(), &amplitude, 0.1f,  0.0f, 30.0f);
			ewRowDrag("Width",     ("##gs_w"   + uid).c_str(), &width,     0.01f, 0.1f, 30.0f);
			ewRowDrag("Position",  ("##gs_pos" + uid).c_str(), &position,  0.1f,  0.0f,
				static_cast<float>(stimulus->getElementCommonParameters().dimensionParameters.x_max));
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##gs_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",  ("##gs_c" + uid).c_str(), &circular);
			ewRowBool("Normalized",("##gs_n" + uid).c_str(), &normalized);
			ewEndTable();
		}

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(gsp.amplitude)) > epsilon ||
			std::abs(width     - static_cast<float>(gsp.width))     > epsilon ||
			std::abs(position  - static_cast<float>(gsp.position))  > epsilon ||
			circular != gsp.circular || normalized != gsp.normalized)
		{
			gsp.amplitude = amplitude; gsp.width = width; gsp.position = position;
			gsp.circular = circular;   gsp.normalized = normalized;
			stimulus->setParameters(gsp);
		}
	}

	void ElementWindow::modifyElementTimedGaussStimulus(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;
		const auto stimulus = std::dynamic_pointer_cast<element::TimedGaussStimulus>(element);
		element::TimedGaussStimulusParameters tgsp = stimulus->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude  = static_cast<float>(tgsp.amplitude);
		auto width      = static_cast<float>(tgsp.width);
		auto position   = static_cast<float>(tgsp.position);
		bool circular   = tgsp.circular;
		bool normalized = tgsp.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##tgs_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude", ("##tgs_amp" + uid).c_str(), &amplitude, 0.1f,  0.0f, 30.0f);
			ewRowDrag("Width",     ("##tgs_w"   + uid).c_str(), &width,     0.01f, 0.1f, 30.0f);
			ewRowDrag("Position",  ("##tgs_pos" + uid).c_str(), &position,  0.1f,  0.0f,
				static_cast<float>(stimulus->getElementCommonParameters().dimensionParameters.x_max));
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##tgs_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",   ("##tgs_c" + uid).c_str(), &circular);
			ewRowBool("Normalized", ("##tgs_n" + uid).c_str(), &normalized);
			ewEndTable();
		}

		ewSectionLabel("Timing");
		static std::unordered_map<int, std::pair<float, float>> tgsPending;
		const int tgsEid = element->getUniqueIdentifier();
		if (!tgsPending.count(tgsEid)) tgsPending[tgsEid] = { 0.0f, 1.0f };
		auto& [tgsNs, tgsNe] = tgsPending[tgsEid];

		int tgsDeleteIdx = -1;
		bool tgsAddClicked = false;
		{
			const float tgsBtnColW = g_MediumIconsFont->LegacySize + ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetStyle().CellPadding.x * 2.0f;
			const float tgsTableW  = ImGui::GetContentRegionAvail().x - 10.0f * ui;
			constexpr ImGuiTableFlags tgsTFlags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersInnerV;
			if (ImGui::BeginTable(("##tgs_tm" + uid).c_str(), 3, tgsTFlags, ImVec2(tgsTableW, 0)))
			{
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, tgsBtnColW);

				static constexpr float tgsEps = 1e-4f;
				for (int i = 0; i < static_cast<int>(tgsp.onTimes.size()); ++i)
				{
					const float storedStart = static_cast<float>(tgsp.onTimes[i].first);
					const float storedEnd   = static_cast<float>(tgsp.onTimes[i].second);
					auto start = storedStart, end = storedEnd;

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::SetNextItemWidth(-FLT_MIN);
					ImGui::PushFont(g_MonoMediumFont);
					ImGui::DragFloat(("##tgs_s" + uid + std::to_string(i)).c_str(), &start, 0.1f, 0.0f, 1e6f, "%.2f");
					ImGui::PopFont();
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(-FLT_MIN);
					ImGui::PushFont(g_MonoMediumFont);
					ImGui::DragFloat(("##tgs_e" + uid + std::to_string(i)).c_str(), &end, 0.1f, 0.0f, 1e6f, "%.2f");
					ImGui::PopFont();
					ImGui::TableSetColumnIndex(2);
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.25f, 0.25f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.06f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0, 0, 0, 0.12f));
					ImGui::PushFont(g_MediumIconsFont);
					if (ImGui::Button((ICON_FA_TRASH "##tgs_del" + uid + std::to_string(i)).c_str()))
						tgsDeleteIdx = i;
					ImGui::PopFont();
					ImGui::PopStyleColor(4);
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove interval");

					if ((std::abs(start - storedStart) > tgsEps || std::abs(end - storedEnd) > tgsEps) && start < end)
					{
						tgsp.onTimes[i] = { static_cast<double>(start), static_cast<double>(end) };
						stimulus->setParameters(tgsp);
					}
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::PushFont(g_MonoMediumFont);
				ImGui::DragFloat(("##tgs_ns" + uid).c_str(), &tgsNs, 0.1f, 0.0f, 1e6f, "%.2f");
				ImGui::PopFont();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::PushFont(g_MonoMediumFont);
				ImGui::DragFloat(("##tgs_ne" + uid).c_str(), &tgsNe, 0.1f, 0.0f, 1e6f, "%.2f");
				ImGui::PopFont();
				ImGui::TableSetColumnIndex(2);
				ImGui::PushFont(g_MediumIconsFont);
				ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.06f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0, 0, 0, 0.12f));
				tgsAddClicked = ImGui::Button((ICON_FA_CIRCLE_PLUS "##tgs_add" + uid).c_str());
				ImGui::PopFont();
				ImGui::PopStyleColor(3);
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Add interval");

				ImGui::EndTable();
			}
		}

		if (tgsDeleteIdx >= 0)
		{
			tgsp.onTimes.erase(tgsp.onTimes.begin() + tgsDeleteIdx);
			stimulus->setParameters(tgsp);
			return;
		}
		if (tgsAddClicked && tgsNs < tgsNe)
		{
			tgsp.onTimes.emplace_back(static_cast<double>(tgsNs), static_cast<double>(tgsNe));
			stimulus->setParameters(tgsp);
			tgsNs = 0.0f; tgsNe = 1.0f;
			return;
		}

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(tgsp.amplitude)) > epsilon ||
		    std::abs(width     - static_cast<float>(tgsp.width))     > epsilon ||
		    std::abs(position  - static_cast<float>(tgsp.position))  > epsilon ||
		    circular != tgsp.circular || normalized != tgsp.normalized)
		{
			tgsp.amplitude = amplitude; tgsp.width = width; tgsp.position = position;
			tgsp.circular  = circular;  tgsp.normalized = normalized;
			stimulus->setParameters(tgsp);
		}
	}

	void ElementWindow::modifyElementTimedGaussStimulus2D(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;
		const auto stimulus = std::dynamic_pointer_cast<element::TimedGaussStimulus2D>(element);
		element::TimedGaussStimulus2DParameters p = stimulus->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude  = static_cast<float>(p.amplitude);
		auto width      = static_cast<float>(p.width);
		auto posX       = static_cast<float>(p.position_x);
		auto posY       = static_cast<float>(p.position_y);
		bool circular   = p.circular;
		bool normalized = p.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##tgs2_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude",  ("##tgs2_amp" + uid).c_str(), &amplitude, 0.1f,  0.0f, 30.0f);
			ewRowDrag("Width",      ("##tgs2_w"   + uid).c_str(), &width,     0.01f, 0.1f, 30.0f);
			ewRowDrag("Position x", ("##tgs2_px"  + uid).c_str(), &posX,      0.5f,  0.0f,
				static_cast<float>(stimulus->getElementCommonParameters().dimensionParameters.x_max));
			ewRowDrag("Position y", ("##tgs2_py"  + uid).c_str(), &posY,      0.5f,  0.0f,
				static_cast<float>(stimulus->getElementCommonParameters().dimensionParameters.y_max));
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##tgs2_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",   ("##tgs2_c" + uid).c_str(), &circular);
			ewRowBool("Normalized", ("##tgs2_n" + uid).c_str(), &normalized);
			ewEndTable();
		}

		ewSectionLabel("Timing");
		static std::unordered_map<int, std::pair<float, float>> tgs2Pending;
		const int tgs2Eid = element->getUniqueIdentifier();
		if (!tgs2Pending.count(tgs2Eid)) tgs2Pending[tgs2Eid] = { 0.0f, 1.0f };
		auto& [tgs2Ns, tgs2Ne] = tgs2Pending[tgs2Eid];

		int tgs2DeleteIdx = -1;
		bool tgs2AddClicked = false;
		{
			const float tgs2BtnColW = g_MediumIconsFont->LegacySize + ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetStyle().CellPadding.x * 2.0f;
			const float tgs2TableW  = ImGui::GetContentRegionAvail().x - 10.0f * ui;
			constexpr ImGuiTableFlags tgs2TFlags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersInnerV;
			if (ImGui::BeginTable(("##tgs2_tm" + uid).c_str(), 3, tgs2TFlags, ImVec2(tgs2TableW, 0)))
			{
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, tgs2BtnColW);

				static constexpr float tgs2Eps = 1e-4f;
				for (int i = 0; i < static_cast<int>(p.onTimes.size()); ++i)
				{
					const float storedStart = static_cast<float>(p.onTimes[i].first);
					const float storedEnd   = static_cast<float>(p.onTimes[i].second);
					auto start = storedStart, end = storedEnd;

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::SetNextItemWidth(-FLT_MIN);
					ImGui::PushFont(g_MonoMediumFont);
					ImGui::DragFloat(("##tgs2_s" + uid + std::to_string(i)).c_str(), &start, 0.1f, 0.0f, 1e6f, "%.2f");
					ImGui::PopFont();
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(-FLT_MIN);
					ImGui::PushFont(g_MonoMediumFont);
					ImGui::DragFloat(("##tgs2_e" + uid + std::to_string(i)).c_str(), &end, 0.1f, 0.0f, 1e6f, "%.2f");
					ImGui::PopFont();
					ImGui::TableSetColumnIndex(2);
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.25f, 0.25f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.06f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0, 0, 0, 0.12f));
					ImGui::PushFont(g_MediumIconsFont);
					if (ImGui::Button((ICON_FA_TRASH "##tgs2_del" + uid + std::to_string(i)).c_str()))
						tgs2DeleteIdx = i;
					ImGui::PopFont();
					ImGui::PopStyleColor(4);
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove interval");

					if ((std::abs(start - storedStart) > tgs2Eps || std::abs(end - storedEnd) > tgs2Eps) && start < end)
					{
						p.onTimes[i] = { static_cast<double>(start), static_cast<double>(end) };
						stimulus->setParameters(p);
					}
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::PushFont(g_MonoMediumFont);
				ImGui::DragFloat(("##tgs2_ns" + uid).c_str(), &tgs2Ns, 0.1f, 0.0f, 1e6f, "%.2f");
				ImGui::PopFont();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::PushFont(g_MonoMediumFont);
				ImGui::DragFloat(("##tgs2_ne" + uid).c_str(), &tgs2Ne, 0.1f, 0.0f, 1e6f, "%.2f");
				ImGui::PopFont();
				ImGui::TableSetColumnIndex(2);
				ImGui::PushFont(g_MediumIconsFont);
				ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.06f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0, 0, 0, 0.12f));
				tgs2AddClicked = ImGui::Button((ICON_FA_CIRCLE_PLUS "##tgs2_add" + uid).c_str());
				ImGui::PopFont();
				ImGui::PopStyleColor(3);
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Add interval");

				ImGui::EndTable();
			}
		}

		if (tgs2DeleteIdx >= 0)
		{
			p.onTimes.erase(p.onTimes.begin() + tgs2DeleteIdx);
			stimulus->setParameters(p);
			return;
		}
		if (tgs2AddClicked && tgs2Ns < tgs2Ne)
		{
			p.onTimes.emplace_back(static_cast<double>(tgs2Ns), static_cast<double>(tgs2Ne));
			stimulus->setParameters(p);
			tgs2Ns = 0.0f; tgs2Ne = 1.0f;
			return;
		}

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(p.amplitude))   > epsilon ||
		    std::abs(width     - static_cast<float>(p.width))       > epsilon ||
		    std::abs(posX      - static_cast<float>(p.position_x))  > epsilon ||
		    std::abs(posY      - static_cast<float>(p.position_y))  > epsilon ||
		    circular != p.circular || normalized != p.normalized)
		{
			p.amplitude = amplitude; p.width = width;
			p.position_x = posX;    p.position_y = posY;
			p.circular = circular;  p.normalized = normalized;
			stimulus->setParameters(p);
		}
	}

	void ElementWindow::modifyElementFieldCoupling(const std::shared_ptr<element::Element>& element)
	{
		const auto fieldCoupling = std::dynamic_pointer_cast<element::FieldCoupling>(element);
		element::FieldCouplingParameters fcp = fieldCoupling->getParameters();
		const std::string uid = element->getUniqueName();

		auto scalar       = static_cast<float>(fcp.scalar);
		auto learningRate = static_cast<float>(fcp.learningRate);
		bool activateLearning = fcp.isLearningActive;

		ewSectionLabel("Parameters");
		if (ewBeginTable(("##fc_tbl" + uid).c_str())) {
			ewTableSetup();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Learning rule");
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::BeginCombo(("##fc_lr" + uid).c_str(), LearningRuleToString.at(fcp.learningRule).c_str()))
			{
				for (size_t i = 0; i < LearningRuleToString.size(); ++i)
				{
					const char* name = LearningRuleToString.at(static_cast<LearningRule>(i)).c_str();
					if (ImGui::Selectable(name, fcp.learningRule == static_cast<LearningRule>(i)))
					{
						fcp.learningRule = static_cast<LearningRule>(i);
						fieldCoupling->setParameters(fcp);
					}
				}
				ImGui::EndCombo();
			}

			ewRowDrag("Learning rate", ("##fc_lrate" + uid).c_str(), &learningRate, 0.01f, 0.0f, 10.0f);
			ewRowDrag("Scalar",        ("##fc_sc"    + uid).c_str(), &scalar,       0.1f, -20.0f, 20.0f);
			ewRowBool("Activate learning", ("##fc_al" + uid).c_str(), &activateLearning);

			ewEndTable();
		}

		static constexpr double epsilon = 1e-6;
		if (std::abs(scalar - static_cast<float>(fcp.scalar)) > epsilon)
			{ fcp.scalar = scalar; fieldCoupling->setParameters(fcp); }
		if (activateLearning != fcp.isLearningActive)
			{ fcp.isLearningActive = activateLearning; fieldCoupling->setParameters(fcp); }
		if (std::abs(learningRate - static_cast<float>(fcp.learningRate)) > epsilon)
			{ fcp.learningRate = learningRate; fieldCoupling->setParameters(fcp); }

		ImGui::PushID(uid.c_str());
		if (ImGui::Button("Load"))  fieldCoupling->readWeights();
		ImGui::SameLine();
		if (ImGui::Button("Save"))  fieldCoupling->writeWeights();
		ImGui::SameLine();
		if (ImGui::Button("Clear")) fieldCoupling->clearWeights();
		ImGui::PopID();
	}

	void ElementWindow::modifyElementGaussKernel(const std::shared_ptr<element::Element>& element)
	{
		const auto kernel = std::dynamic_pointer_cast<element::GaussKernel>(element);
		element::GaussKernelParameters gkp = kernel->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude       = static_cast<float>(gkp.amplitude);
		auto width           = static_cast<float>(gkp.width);
		auto amplitudeGlobal = static_cast<float>(gkp.amplitudeGlobal);
		bool circular        = gkp.circular;
		bool normalized      = gkp.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##gk_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude",   ("##gk_amp" + uid).c_str(), &amplitude,       0.1f, -50.0f, 50.0f);
			ewRowDrag("Width",       ("##gk_w"   + uid).c_str(), &width,           0.1f,   0.1f, 30.0f);
			ewRowDrag("Amp. global", ("##gk_ag"  + uid).c_str(), &amplitudeGlobal, 0.1f, -10.0f, 10.0f);
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##gk_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",    ("##gk_c" + uid).c_str(), &circular);
			ewRowBool("Normalized",  ("##gk_n" + uid).c_str(), &normalized);
			ewEndTable();
		}

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude       - static_cast<float>(gkp.amplitude))       > epsilon ||
			std::abs(width           - static_cast<float>(gkp.width))           > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(gkp.amplitudeGlobal)) > epsilon ||
			circular != gkp.circular || normalized != gkp.normalized)
		{
			gkp.amplitude = amplitude; gkp.width = width; gkp.amplitudeGlobal = amplitudeGlobal;
			gkp.circular  = circular;  gkp.normalized = normalized;
			kernel->setParameters(gkp);
		}
	}

	void ElementWindow::modifyElementMexicanHatKernel(const std::shared_ptr<element::Element>& element)
	{
		const auto kernel = std::dynamic_pointer_cast<element::MexicanHatKernel>(element);
		element::MexicanHatKernelParameters mhkp = kernel->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitudeExc    = static_cast<float>(mhkp.amplitudeExc);
		auto widthExc        = static_cast<float>(mhkp.widthExc);
		auto amplitudeInh    = static_cast<float>(mhkp.amplitudeInh);
		auto widthInh        = static_cast<float>(mhkp.widthInh);
		auto amplitudeGlobal = static_cast<float>(mhkp.amplitudeGlobal);
		bool circular        = mhkp.circular;
		bool normalized      = mhkp.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##mhk_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amp. exc.",   ("##mhk_ae" + uid).c_str(), &amplitudeExc,    0.1f,  -50.0f,  50.0f);
			ewRowDrag("Width exc.",  ("##mhk_we" + uid).c_str(), &widthExc,        0.1f,    0.1f,  30.0f);
			ewRowDrag("Amp. inh.",   ("##mhk_ai" + uid).c_str(), &amplitudeInh,    0.1f,    0.0f, 100.0f);
			ewRowDrag("Width inh.",  ("##mhk_wi" + uid).c_str(), &widthInh,        0.1f,    0.1f,  30.0f);
			ewRowDrag("Amp. global", ("##mhk_ag" + uid).c_str(), &amplitudeGlobal, 0.01f, -10.0f,   0.0f);
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##mhk_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",    ("##mhk_c" + uid).c_str(), &circular);
			ewRowBool("Normalized",  ("##mhk_n" + uid).c_str(), &normalized);
			ewEndTable();
		}

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitudeExc    - static_cast<float>(mhkp.amplitudeExc))    > epsilon ||
			std::abs(widthExc        - static_cast<float>(mhkp.widthExc))        > epsilon ||
			std::abs(amplitudeInh    - static_cast<float>(mhkp.amplitudeInh))    > epsilon ||
			std::abs(widthInh        - static_cast<float>(mhkp.widthInh))        > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(mhkp.amplitudeGlobal)) > epsilon ||
			circular != mhkp.circular || normalized != mhkp.normalized)
		{
			mhkp.amplitudeExc = amplitudeExc; mhkp.widthExc = widthExc;
			mhkp.amplitudeInh = amplitudeInh; mhkp.widthInh = widthInh;
			mhkp.amplitudeGlobal = amplitudeGlobal;
			mhkp.circular = circular; mhkp.normalized = normalized;
			kernel->setParameters(mhkp);
		}
	}

	void ElementWindow::modifyElementNormalNoise(const std::shared_ptr<element::Element>& element)
	{
		const auto normalNoise = std::dynamic_pointer_cast<element::NormalNoise>(element);
		element::NormalNoiseParameters nnp = normalNoise->getParameters();
		const std::string uid = element->getUniqueName();
		auto amplitude = static_cast<float>(nnp.amplitude);

		ewSectionLabel("Parameters");
		if (ewBeginTable(("##nn_tbl" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude", ("##nn_amp" + uid).c_str(), &amplitude, 0.01f, 0.0f, 5.0f);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(nnp.amplitude)) > epsilon)
			{ nnp.amplitude = amplitude; normalNoise->setParameters(nnp); }
	}

	void ElementWindow::modifyElementCorrelatedNormalNoise(const std::shared_ptr<element::Element>& element)
	{
		const auto cnn = std::dynamic_pointer_cast<element::CorrelatedNormalNoise>(element);
		element::CorrelatedNormalNoiseParameters p = cnn->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude = static_cast<float>(p.amplitude);
		auto width     = static_cast<float>(p.width);
		bool circular  = p.circular;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##cnn_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude", ("##cnn_amp" + uid).c_str(), &amplitude, 0.001f, 0.0f,  5.0f, "%.4f");
			ewRowDrag("Width",     ("##cnn_w"   + uid).c_str(), &width,     0.1f,   0.1f, 30.0f);
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##cnn_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",  ("##cnn_c" + uid).c_str(), &circular);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(p.amplitude)) > epsilon ||
		    std::abs(width     - static_cast<float>(p.width))     > epsilon ||
		    circular != p.circular)
			{ p.amplitude = amplitude; p.width = width; p.circular = circular; cnn->setParameters(p); }
	}

	void ElementWindow::modifyElementGaussFieldCoupling(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;
		const auto gfc = std::dynamic_pointer_cast<element::GaussFieldCoupling>(element);
		element::GaussFieldCouplingParameters gfcp = gfc->getParameters();
		const std::string uid = element->getUniqueName();
		const int  size       = gfc->getMaxSpatialDimension();
		const auto other_size = gfc->getInputFieldDimensions().x_max;

		bool normalized = gfcp.normalized;
		bool circular   = gfcp.circular;

		ewSectionLabel("Options");
		if (ewBeginTable(("##gfc_hdr" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",   ("##gfc_c" + uid).c_str(), &circular);
			ewRowBool("Normalized", ("##gfc_n" + uid).c_str(), &normalized);
			ewEndTable();
		}
		if (normalized != gfcp.normalized || circular != gfcp.circular)
		{
			gfcp.normalized = normalized; gfcp.circular = circular;
			gfc->setParameters(gfcp);
		}

		ewSectionLabel("Couplings");

		static std::unordered_map<int, std::tuple<float, float, float, float>> gfcPending;
		const int gfcEid = element->getUniqueIdentifier();
		if (!gfcPending.count(gfcEid)) gfcPending[gfcEid] = { 0.0f, 0.0f, 1.0f, 1.0f };
		auto& [gfcNxi, gfcNxj, gfcNamp, gfcNw] = gfcPending[gfcEid];

		int  gfcDeleteIdx  = -1;
		bool gfcAddClicked = false;

		const float gfcBtnColW = g_MediumIconsFont->LegacySize
			+ ImGui::GetStyle().FramePadding.x * 2.0f;
		const float gfcTableW = ImGui::GetContentRegionAvail().x - 10.0f * ui;
		constexpr ImGuiTableFlags gfcTFlags =
			ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersInnerV;

		if (ImGui::BeginTable(("##gfc_tbl" + uid).c_str(), 5, gfcTFlags | ImGuiTableFlags_BordersOuter, ImVec2(gfcTableW, 0)))
		{
			ImGui::TableSetupColumn("x in",   ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("x out",  ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("amp",    ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("width",  ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("",       ImGuiTableColumnFlags_WidthFixed, gfcBtnColW);
			ImGui::TableHeadersRow();

			static constexpr float gfcEps = 1e-4f;
			for (int i = 0; i < static_cast<int>(gfcp.couplings.size()); ++i)
			{
				auto& c = gfcp.couplings[i];
				const std::string si = std::to_string(i);
				auto xi  = static_cast<float>(c.x_i);
				auto xj  = static_cast<float>(c.x_j);
				auto amp = static_cast<float>(c.amplitude);
				auto w   = static_cast<float>(c.width);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0); ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::PushFont(g_MonoMediumFont);
				ImGui::DragFloat(("##gfc_xi"  + uid + si).c_str(), &xi,  0.05f, 0.0f, static_cast<float>(other_size), "%.1f");
				ImGui::PopFont();
				ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::PushFont(g_MonoMediumFont);
				ImGui::DragFloat(("##gfc_xj"  + uid + si).c_str(), &xj,  0.05f, 0.0f, static_cast<float>(size), "%.1f");
				ImGui::PopFont();
				ImGui::TableSetColumnIndex(2); ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::PushFont(g_MonoMediumFont);
				ImGui::DragFloat(("##gfc_amp" + uid + si).c_str(), &amp, 0.1f,  0.0f, 100.0f, "%.2f");
				ImGui::PopFont();
				ImGui::TableSetColumnIndex(3); ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::PushFont(g_MonoMediumFont);
				ImGui::DragFloat(("##gfc_w"   + uid + si).c_str(), &w,   0.1f,  1.0f,  30.0f, "%.2f");
				ImGui::PopFont();
				ImGui::TableSetColumnIndex(4);
				ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.85f, 0.25f, 0.25f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.06f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0, 0, 0, 0.12f));
				ImGui::PushFont(g_MediumIconsFont);
				if (ImGui::Button((ICON_FA_TRASH "##gfc_del" + uid + si).c_str()))
					gfcDeleteIdx = i;
				ImGui::PopFont();
				ImGui::PopStyleColor(4);
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove coupling");

				if (std::abs(xi  - static_cast<float>(c.x_i))        > gfcEps ||
				    std::abs(xj  - static_cast<float>(c.x_j))        > gfcEps ||
				    std::abs(amp - static_cast<float>(c.amplitude))   > gfcEps ||
				    std::abs(w   - static_cast<float>(c.width))       > gfcEps)
				{
					c.x_i = xi; c.x_j = xj; c.amplitude = amp; c.width = w;
					gfc->setParameters(gfcp);
				}
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::DragFloat(("##gfc_nxi"  + uid).c_str(), &gfcNxi,  0.05f, 0.0f, static_cast<float>(other_size), "%.1f");
			ImGui::PopFont();
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::DragFloat(("##gfc_nxj"  + uid).c_str(), &gfcNxj,  0.05f, 0.0f, static_cast<float>(size), "%.1f");
			ImGui::PopFont();
			ImGui::TableSetColumnIndex(2); ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::DragFloat(("##gfc_namp" + uid).c_str(), &gfcNamp, 0.1f,  0.0f, 100.0f, "%.2f");
			ImGui::PopFont();
			ImGui::TableSetColumnIndex(3); ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::DragFloat(("##gfc_nw"   + uid).c_str(), &gfcNw,   0.1f,  1.0f,  30.0f, "%.2f");
			ImGui::PopFont();
			ImGui::TableSetColumnIndex(4);
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.06f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0, 0, 0, 0.12f));
			gfcAddClicked = ImGui::Button((ICON_FA_CIRCLE_PLUS "##gfc_add" + uid).c_str());
			ImGui::PopFont();
			ImGui::PopStyleColor(3);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Add coupling");

			ImGui::EndTable();
		}

		if (gfcDeleteIdx >= 0)
		{
			gfcp.couplings.erase(gfcp.couplings.begin() + gfcDeleteIdx);
			gfc->setParameters(gfcp);
			gfc->init();
			return;
		}
		if (gfcAddClicked)
		{
			gfc->addCoupling({ static_cast<double>(gfcNxi), static_cast<double>(gfcNxj),
			                   static_cast<double>(gfcNamp), static_cast<double>(gfcNw) });
			gfc->init();
			gfcNxi = 0.0f; gfcNxj = 0.0f; gfcNamp = 1.0f; gfcNw = 1.0f;
			return;
		}
	}

	void ElementWindow::modifyElementOscillatoryKernel(const std::shared_ptr<element::Element>& element)
	{
		const auto kernel = std::dynamic_pointer_cast<element::OscillatoryKernel>(element);
		element::OscillatoryKernelParameters okp = kernel->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude       = static_cast<float>(okp.amplitude);
		auto decay           = static_cast<float>(okp.decay);
		auto zeroCrossings   = static_cast<float>(okp.zeroCrossings);
		auto amplitudeGlobal = static_cast<float>(okp.amplitudeGlobal);
		bool circular        = okp.circular;
		bool normalized      = okp.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##ok_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude",      ("##ok_amp" + uid).c_str(), &amplitude,       0.1f,   0.0f,  50.0f);
			ewRowDrag("Decay",          ("##ok_dec" + uid).c_str(), &decay,           0.005f, 0.001f, 10.0f);
			ewRowDrag("Zero crossings", ("##ok_zc"  + uid).c_str(), &zeroCrossings,   0.005f, 0.0f,   1.0f);
			ewRowDrag("Amp. global",    ("##ok_ag"  + uid).c_str(), &amplitudeGlobal, 0.01f, -10.0f,  0.0f);
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##ok_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",       ("##ok_c" + uid).c_str(), &circular);
			ewRowBool("Normalized",     ("##ok_n" + uid).c_str(), &normalized);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude       - static_cast<float>(okp.amplitude))       > epsilon ||
			std::abs(decay           - static_cast<float>(okp.decay))           > epsilon ||
			std::abs(zeroCrossings   - static_cast<float>(okp.zeroCrossings))   > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(okp.amplitudeGlobal)) > epsilon ||
			circular != okp.circular || normalized != okp.normalized)
		{
			okp.amplitude = amplitude; okp.decay = decay; okp.zeroCrossings = zeroCrossings;
			okp.amplitudeGlobal = amplitudeGlobal; okp.circular = circular; okp.normalized = normalized;
			kernel->setParameters(okp);
		}
	}

	void ElementWindow::modifyElementAsymmetricGaussKernel(const std::shared_ptr<element::Element>& element)
	{
		const auto kernel = std::dynamic_pointer_cast<element::AsymmetricGaussKernel>(element);
		element::AsymmetricGaussKernelParameters agkp = kernel->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude       = static_cast<float>(agkp.amplitude);
		auto width           = static_cast<float>(agkp.width);
		auto amplitudeGlobal = static_cast<float>(agkp.amplitudeGlobal);
		auto timeShift       = static_cast<float>(agkp.timeShift);
		bool circular        = agkp.circular;
		bool normalized      = agkp.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##agk_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude",   ("##agk_amp" + uid).c_str(), &amplitude,       0.05f,  -30.0f, 30.0f);
			ewRowDrag("Width",       ("##agk_w"   + uid).c_str(), &width,           0.005f,   0.1f, 30.0f);
			ewRowDrag("Amp. global", ("##agk_ag"  + uid).c_str(), &amplitudeGlobal, 0.005f, -10.0f,  0.0f);
			ewRowDrag("Time shift",  ("##agk_ts"  + uid).c_str(), &timeShift,       0.01f,  -10.0f, 10.0f);
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##agk_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",    ("##agk_c" + uid).c_str(), &circular);
			ewRowBool("Normalized",  ("##agk_n" + uid).c_str(), &normalized);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude       - static_cast<float>(agkp.amplitude))       > epsilon ||
			std::abs(width           - static_cast<float>(agkp.width))           > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(agkp.amplitudeGlobal)) > epsilon ||
			std::abs(timeShift       - static_cast<float>(agkp.timeShift))       > epsilon ||
			circular != agkp.circular || normalized != agkp.normalized)
		{
			agkp.amplitude = amplitude; agkp.width = width;
			agkp.amplitudeGlobal = amplitudeGlobal; agkp.timeShift = timeShift;
			agkp.circular = circular; agkp.normalized = normalized;
			kernel->setParameters(agkp);
		}
	}

	void ElementWindow::modifyElementBoostStimulus(const std::shared_ptr<element::Element>& element)
	{
		const auto boostStimulus = std::dynamic_pointer_cast<element::BoostStimulus>(element);
		element::BoostStimulusParameters bsp = boostStimulus->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude = static_cast<float>(bsp.amplitude);
		bool isActive  = bsp.isActive;

		ewSectionLabel("Parameters");
		if (ewBeginTable(("##bs_tbl" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude", ("##bs_amp" + uid).c_str(), &amplitude, 0.1f, -30.0f, 30.0f);
			ewRowBool("Active",    ("##bs_act" + uid).c_str(), &isActive);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(bsp.amplitude)) > epsilon || isActive != bsp.isActive)
			{ bsp.amplitude = amplitude; bsp.isActive = isActive; boostStimulus->setParameters(bsp); }
	}

	void ElementWindow::modifyElementBoostStimulus2D(const std::shared_ptr<element::Element>& element)
	{
		const auto stimulus = std::dynamic_pointer_cast<element::BoostStimulus2D>(element);
		element::BoostStimulus2DParameters p = stimulus->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude = static_cast<float>(p.amplitude);
		bool isActive  = p.isActive;

		ewSectionLabel("Parameters");
		if (ewBeginTable(("##bs2_tbl" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude", ("##bs2_amp" + uid).c_str(), &amplitude, 0.1f, -30.0f, 30.0f);
			ewRowBool("Active",    ("##bs2_act" + uid).c_str(), &isActive);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(p.amplitude)) > epsilon || isActive != p.isActive)
			{ p.amplitude = amplitude; p.isActive = isActive; stimulus->setParameters(p); }
	}

	void ElementWindow::modifyElementCorrelatedNormalNoise2D(const std::shared_ptr<element::Element>& element)
	{
		const auto cnn = std::dynamic_pointer_cast<element::CorrelatedNormalNoise2D>(element);
		element::CorrelatedNormalNoise2DParameters p = cnn->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude = static_cast<float>(p.amplitude);
		auto width     = static_cast<float>(p.width);
		bool circular  = p.circular;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##cnn2_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude", ("##cnn2_amp" + uid).c_str(), &amplitude, 0.001f, 0.0f,  5.0f, "%.4f");
			ewRowDrag("Width",     ("##cnn2_w"   + uid).c_str(), &width,     0.1f,   0.1f, 30.0f);
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##cnn2_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",  ("##cnn2_c" + uid).c_str(), &circular);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(p.amplitude)) > epsilon ||
		    std::abs(width     - static_cast<float>(p.width))     > epsilon || circular != p.circular)
			{ p.amplitude = amplitude; p.width = width; p.circular = circular; cnn->setParameters(p); }
	}

	void ElementWindow::modifyElementAsymmetricGaussKernel2D(const std::shared_ptr<element::Element>& element)
	{
		const auto kernel = std::dynamic_pointer_cast<element::AsymmetricGaussKernel2D>(element);
		element::AsymmetricGaussKernel2DParameters p = kernel->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude       = static_cast<float>(p.amplitude);
		auto width           = static_cast<float>(p.width);
		auto amplitudeGlobal = static_cast<float>(p.amplitudeGlobal);
		auto timeShift_x     = static_cast<float>(p.timeShift_x);
		auto timeShift_y     = static_cast<float>(p.timeShift_y);
		bool circular        = p.circular;
		bool normalized      = p.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##agk2_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude",    ("##agk2_amp" + uid).c_str(), &amplitude,       0.05f,  -30.0f, 30.0f);
			ewRowDrag("Width",        ("##agk2_w"   + uid).c_str(), &width,           0.005f,   0.1f, 30.0f);
			ewRowDrag("Amp. global",  ("##agk2_ag"  + uid).c_str(), &amplitudeGlobal, 0.005f, -10.0f,  0.0f);
			ewRowDrag("Time shift x", ("##agk2_tsx" + uid).c_str(), &timeShift_x,     0.01f,  -10.0f, 10.0f);
			ewRowDrag("Time shift y", ("##agk2_tsy" + uid).c_str(), &timeShift_y,     0.01f,  -10.0f, 10.0f);
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##agk2_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",     ("##agk2_c" + uid).c_str(), &circular);
			ewRowBool("Normalized",   ("##agk2_n" + uid).c_str(), &normalized);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude       - static_cast<float>(p.amplitude))       > epsilon ||
		    std::abs(width           - static_cast<float>(p.width))           > epsilon ||
		    std::abs(amplitudeGlobal - static_cast<float>(p.amplitudeGlobal)) > epsilon ||
		    std::abs(timeShift_x     - static_cast<float>(p.timeShift_x))     > epsilon ||
		    std::abs(timeShift_y     - static_cast<float>(p.timeShift_y))     > epsilon ||
		    circular != p.circular || normalized != p.normalized)
		{
			p.amplitude = amplitude; p.width = width; p.amplitudeGlobal = amplitudeGlobal;
			p.timeShift_x = timeShift_x; p.timeShift_y = timeShift_y;
			p.circular = circular; p.normalized = normalized;
			kernel->setParameters(p);
		}
	}

	void ElementWindow::modifyElementMemoryTrace2D(const std::shared_ptr<element::Element>& element)
	{
		const auto mt = std::dynamic_pointer_cast<element::MemoryTrace2D>(element);
		element::MemoryTrace2DParameters p = mt->getParameters();
		const std::string uid = element->getUniqueName();

		auto tauBuild  = static_cast<float>(p.tauBuild);
		auto tauDecay  = static_cast<float>(p.tauDecay);
		auto threshold = static_cast<float>(p.threshold);

		ewSectionLabel("Dynamics");
		if (ewBeginTable(("##mt2_tbl" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Time scale build",  ("##mt2_tb" + uid).c_str(), &tauBuild,  1.0f,  1.0f,  10000.0f);
			ewRowDrag("Time scale decay",  ("##mt2_td" + uid).c_str(), &tauDecay,  5.0f,  1.0f, 100000.0f);
			ewRowDrag("Threshold",  ("##mt2_th" + uid).c_str(), &threshold, 0.01f, -2.0f,     2.0f);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(tauBuild  - static_cast<float>(p.tauBuild))  > epsilon ||
		    std::abs(tauDecay  - static_cast<float>(p.tauDecay))  > epsilon ||
		    std::abs(threshold - static_cast<float>(p.threshold)) > epsilon)
			{ p.tauBuild = tauBuild; p.tauDecay = tauDecay; p.threshold = threshold; mt->setParameters(p); }
	}

	void ElementWindow::modifyElementMemoryTrace(const std::shared_ptr<element::Element>& element)
	{
		const auto memoryTrace = std::dynamic_pointer_cast<element::MemoryTrace>(element);
		element::MemoryTraceParameters mtp = memoryTrace->getParameters();
		const std::string uid = element->getUniqueName();

		auto tauBuild  = static_cast<float>(mtp.tauBuild);
		auto tauDecay  = static_cast<float>(mtp.tauDecay);
		auto threshold = static_cast<float>(mtp.threshold);

		ewSectionLabel("Dynamics");
		if (ewBeginTable(("##mt_tbl" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Time scale build",  ("##mt_tb" + uid).c_str(), &tauBuild,  1.0f,  1.0f,  10000.0f);
			ewRowDrag("Time scale decay",  ("##mt_td" + uid).c_str(), &tauDecay,  5.0f,  1.0f, 100000.0f);
			ewRowDrag("Threshold",  ("##mt_th" + uid).c_str(), &threshold, 0.01f, -2.0f,     2.0f);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(tauBuild  - static_cast<float>(mtp.tauBuild))  > epsilon ||
			std::abs(tauDecay  - static_cast<float>(mtp.tauDecay))  > epsilon ||
			std::abs(threshold - static_cast<float>(mtp.threshold)) > epsilon)
			{ mtp.tauBuild = tauBuild; mtp.tauDecay = tauDecay; mtp.threshold = threshold; memoryTrace->setParameters(mtp); }
	}

	void ElementWindow::modifyElementNeuralField2D(const std::shared_ptr<element::Element>& element)
	{
		const auto nf = std::dynamic_pointer_cast<element::NeuralField2D>(element);
		element::NeuralField2DParameters p = nf->getParameters();
		const std::string uid = element->getUniqueName();

		auto tau          = static_cast<float>(p.tau);
		auto restingLevel = static_cast<float>(p.startingRestingLevel);

		ewSectionLabel("Dynamics");
		if (ewBeginTable(("##nf2_tbl" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Time scale",           ("##nf2_tau" + uid).c_str(), &tau,          0.5f,   1.0f, 1000.0f);
			ewRowDrag("Resting level", ("##nf2_rl"  + uid).c_str(), &restingLevel, 0.1f, -30.0f,   30.0f);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(tau          - static_cast<float>(p.tau))                  > epsilon ||
		    std::abs(restingLevel - static_cast<float>(p.startingRestingLevel)) > epsilon)
		{
			p.tau = tau;
			p.startingRestingLevel = restingLevel;
			nf->setParameters(p);
		}

		bool updated = false;
		ewSectionLabel("Activation function");
		if (ewBeginTable(("##nf2_act" + uid).c_str())) {
			ewTableSetup();
			static const char* actFnNames[] = { "Sigmoid", "Heaviside", "AbsSigmoid" };
			int actFnType = p.activationFunction
				? static_cast<int>(p.activationFunction->type) : element::SIGMOID;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Function");
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::Combo(("##nf2_fn" + uid).c_str(), &actFnType, actFnNames, 3))
			{
				switch (actFnType) {
				case element::SIGMOID:    p.activationFunction = std::make_unique<element::SigmoidFunction>(0.0, 10.0); break;
				case element::HEAVISIDE:  p.activationFunction = std::make_unique<element::HeavisideFunction>(0.0); break;
				case element::ABSSIGMOID: p.activationFunction = std::make_unique<element::AbsSigmoidFunction>(0.0, 100.0); break;
				default: break;
				}
				updated = true;
			}
			if (actFnType == element::SIGMOID)
			{
				const auto* sig = dynamic_cast<const element::SigmoidFunction*>(p.activationFunction.get());
				const float storedXShift    = sig ? static_cast<float>(sig->getXShift())    : 0.0f;
				const float storedSteepness = sig ? static_cast<float>(sig->getSteepness()) : 10.0f;
				auto xShift    = storedXShift;
				auto steepness = storedSteepness;
				ewRowDrag("x shift",   ("##nf2_xs" + uid).c_str(), &xShift,    0.1f,  -30.0f, 30.0f);
				ewRowDrag("Steepness", ("##nf2_st" + uid).c_str(), &steepness, 0.5f,    0.1f, 500.0f);
				if (std::abs(xShift - storedXShift) > epsilon || std::abs(steepness - storedSteepness) > epsilon)
				{
					p.activationFunction = std::make_unique<element::SigmoidFunction>(
						static_cast<double>(xShift), static_cast<double>(steepness));
					updated = true;
				}
			}
			else if (actFnType == element::HEAVISIDE)
			{
				const auto* hv = dynamic_cast<const element::HeavisideFunction*>(p.activationFunction.get());
				const float storedXShift = hv ? static_cast<float>(hv->getXShift()) : 0.0f;
				auto xShift = storedXShift;
				ewRowDrag("x shift", ("##nf2_hxs" + uid).c_str(), &xShift, 0.1f, -30.0f, 30.0f);
				if (std::abs(xShift - storedXShift) > epsilon)
				{
					p.activationFunction = std::make_unique<element::HeavisideFunction>(static_cast<double>(xShift));
					updated = true;
				}
			}
			else if (actFnType == element::ABSSIGMOID)
			{
				const auto* ab = dynamic_cast<const element::AbsSigmoidFunction*>(p.activationFunction.get());
				const float storedXShift = ab ? static_cast<float>(ab->getXShift()) : 0.0f;
				const float storedBeta   = ab ? static_cast<float>(ab->getBeta())   : 100.0f;
				auto xShift = storedXShift;
				auto beta   = storedBeta;
				ewRowDrag("x shift", ("##nf2_axs" + uid).c_str(), &xShift, 0.1f,  -30.0f,  30.0f);
				ewRowDrag("Beta",    ("##nf2_ab"  + uid).c_str(), &beta,   1.0f,    0.1f, 1000.0f);
				if (std::abs(xShift - storedXShift) > epsilon || std::abs(beta - storedBeta) > epsilon)
				{
					p.activationFunction = std::make_unique<element::AbsSigmoidFunction>(
						static_cast<double>(xShift), static_cast<double>(beta));
					updated = true;
				}
			}
			ewEndTable();
		}
		if (updated) nf->setParameters(p);
	}

	void ElementWindow::modifyElementGaussStimulus2D(const std::shared_ptr<element::Element>& element)
	{
		const auto gs = std::dynamic_pointer_cast<element::GaussStimulus2D>(element);
		element::GaussStimulus2DParameters p = gs->getParameters();
		const std::string uid = element->getUniqueName();

		auto width      = static_cast<float>(p.width);
		auto amplitude  = static_cast<float>(p.amplitude);
		auto posX       = static_cast<float>(p.position_x);
		auto posY       = static_cast<float>(p.position_y);
		bool circular   = p.circular;
		bool normalized = p.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##gs2_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude",  ("##gs2_amp" + uid).c_str(), &amplitude, 0.5f,  -50.0f,   50.0f);
			ewRowDrag("Width",      ("##gs2_w"   + uid).c_str(), &width,     0.1f,    0.1f,  100.0f);
			ewRowDrag("Position x", ("##gs2_px"  + uid).c_str(), &posX,      0.5f,    0.0f, 1000.0f);
			ewRowDrag("Position y", ("##gs2_py"  + uid).c_str(), &posY,      0.5f,    0.0f, 1000.0f);
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##gs2_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",   ("##gs2_c" + uid).c_str(), &circular);
			ewRowBool("Normalized", ("##gs2_n" + uid).c_str(), &normalized);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(width - static_cast<float>(p.width)) > epsilon ||
			std::abs(amplitude - static_cast<float>(p.amplitude)) > epsilon ||
			std::abs(posX - static_cast<float>(p.position_x)) > epsilon ||
			std::abs(posY - static_cast<float>(p.position_y)) > epsilon ||
			circular != p.circular || normalized != p.normalized)
		{
			p.width = width; p.amplitude = amplitude;
			p.position_x = posX; p.position_y = posY;
			p.circular = circular; p.normalized = normalized;
			gs->setParameters(p);
		}
	}

	void ElementWindow::modifyElementGaussKernel2D(const std::shared_ptr<element::Element>& element)
	{
		const auto gk = std::dynamic_pointer_cast<element::GaussKernel2D>(element);
		element::GaussKernel2DParameters p = gk->getParameters();
		const std::string uid = element->getUniqueName();

		auto width           = static_cast<float>(p.width);
		auto amplitude       = static_cast<float>(p.amplitude);
		auto amplitudeGlobal = static_cast<float>(p.amplitudeGlobal);
		bool circular        = p.circular;
		bool normalized      = p.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##gk2_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude",        ("##gk2_amp" + uid).c_str(), &amplitude,       0.1f,  -50.0f, 50.0f);
			ewRowDrag("Width",            ("##gk2_w"   + uid).c_str(), &width,           0.1f,    0.1f, 100.0f);
			ewRowDrag("Amplitude global", ("##gk2_ag"  + uid).c_str(), &amplitudeGlobal, 0.001f,  -1.0f,  1.0f);
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##gk2_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",         ("##gk2_c" + uid).c_str(), &circular);
			ewRowBool("Normalized",       ("##gk2_n" + uid).c_str(), &normalized);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(width - static_cast<float>(p.width)) > epsilon ||
			std::abs(amplitude - static_cast<float>(p.amplitude)) > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(p.amplitudeGlobal)) > epsilon ||
			circular != p.circular || normalized != p.normalized)
		{
			p.width = width; p.amplitude = amplitude; p.amplitudeGlobal = amplitudeGlobal;
			p.circular = circular; p.normalized = normalized;
			gk->setParameters(p);
		}
	}

	void ElementWindow::modifyElementMexicanHatKernel2D(const std::shared_ptr<element::Element>& element)
	{
		const auto mhk = std::dynamic_pointer_cast<element::MexicanHatKernel2D>(element);
		element::MexicanHatKernel2DParameters p = mhk->getParameters();
		const std::string uid = element->getUniqueName();

		auto widthExc        = static_cast<float>(p.widthExc);
		auto amplitudeExc    = static_cast<float>(p.amplitudeExc);
		auto widthInh        = static_cast<float>(p.widthInh);
		auto amplitudeInh    = static_cast<float>(p.amplitudeInh);
		auto amplitudeGlobal = static_cast<float>(p.amplitudeGlobal);
		bool circular        = p.circular;
		bool normalized      = p.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##mhk2_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude exc",    ("##mhk2_ae" + uid).c_str(), &amplitudeExc,    0.1f,   -50.0f, 50.0f);
			ewRowDrag("Width exc",        ("##mhk2_we" + uid).c_str(), &widthExc,        0.1f,     0.1f, 100.0f);
			ewRowDrag("Amplitude inh",    ("##mhk2_ai" + uid).c_str(), &amplitudeInh,    0.1f,   -50.0f, 50.0f);
			ewRowDrag("Width inh",        ("##mhk2_wi" + uid).c_str(), &widthInh,        0.1f,     0.1f, 100.0f);
			ewRowDrag("Amplitude global", ("##mhk2_ag" + uid).c_str(), &amplitudeGlobal, 0.001f,  -1.0f,   1.0f);
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##mhk2_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",         ("##mhk2_c" + uid).c_str(), &circular);
			ewRowBool("Normalized",       ("##mhk2_n" + uid).c_str(), &normalized);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(widthExc        - static_cast<float>(p.widthExc))        > epsilon ||
			std::abs(amplitudeExc    - static_cast<float>(p.amplitudeExc))    > epsilon ||
			std::abs(widthInh        - static_cast<float>(p.widthInh))        > epsilon ||
			std::abs(amplitudeInh    - static_cast<float>(p.amplitudeInh))    > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(p.amplitudeGlobal)) > epsilon ||
			circular != p.circular || normalized != p.normalized)
		{
			p.widthExc = widthExc; p.amplitudeExc = amplitudeExc;
			p.widthInh = widthInh; p.amplitudeInh = amplitudeInh;
			p.amplitudeGlobal = amplitudeGlobal;
			p.circular = circular; p.normalized = normalized;
			mhk->setParameters(p);
		}
	}

	void ElementWindow::modifyElementNormalNoise2D(const std::shared_ptr<element::Element>& element)
	{
		const auto nn = std::dynamic_pointer_cast<element::NormalNoise2D>(element);
		element::NormalNoise2DParameters p = nn->getParameters();
		const std::string uid = element->getUniqueName();
		auto amplitude = static_cast<float>(p.amplitude);

		ewSectionLabel("Parameters");
		if (ewBeginTable(("##nn2_tbl" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude", ("##nn2_amp" + uid).c_str(), &amplitude, 0.001f, 0.0f, 10.0f);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(p.amplitude)) > epsilon)
			{ p.amplitude = amplitude; nn->setParameters(p); }
	}

	void ElementWindow::modifyElementOscillatoryKernel2D(const std::shared_ptr<element::Element>& element)
	{
		const auto kernel = std::dynamic_pointer_cast<element::OscillatoryKernel2D>(element);
		element::OscillatoryKernel2DParameters p = kernel->getParameters();
		const std::string uid = element->getUniqueName();

		auto amplitude       = static_cast<float>(p.amplitude);
		auto decay           = static_cast<float>(p.decay);
		auto zeroCrossings   = static_cast<float>(p.zeroCrossings);
		auto amplitudeGlobal = static_cast<float>(p.amplitudeGlobal);
		bool circular        = p.circular;
		bool normalized      = p.normalized;

		ewSectionLabel("Shape");
		if (ewBeginTable(("##ok2_shp" + uid).c_str())) {
			ewTableSetup();
			ewRowDrag("Amplitude",      ("##ok2_amp" + uid).c_str(), &amplitude,       0.1f,   0.0f,  50.0f);
			ewRowDrag("Decay",          ("##ok2_dec" + uid).c_str(), &decay,           0.005f, 0.001f, 10.0f);
			ewRowDrag("Zero crossings", ("##ok2_zc"  + uid).c_str(), &zeroCrossings,   0.005f, 0.0f,   1.0f);
			ewRowDrag("Amp. global",    ("##ok2_ag"  + uid).c_str(), &amplitudeGlobal, 0.01f, -10.0f,  0.0f);
			ewEndTable();
		}
		ewSectionLabel("Options");
		if (ewBeginTable(("##ok2_opt" + uid).c_str())) {
			ewTableSetup();
			ewRowBool("Circular",       ("##ok2_c" + uid).c_str(), &circular);
			ewRowBool("Normalized",     ("##ok2_n" + uid).c_str(), &normalized);
			ewEndTable();
		}
		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude       - static_cast<float>(p.amplitude))       > epsilon ||
		    std::abs(decay           - static_cast<float>(p.decay))           > epsilon ||
		    std::abs(zeroCrossings   - static_cast<float>(p.zeroCrossings))   > epsilon ||
		    std::abs(amplitudeGlobal - static_cast<float>(p.amplitudeGlobal)) > epsilon ||
		    circular != p.circular || normalized != p.normalized)
		{
			p.amplitude = amplitude; p.decay = decay; p.zeroCrossings = zeroCrossings;
			p.amplitudeGlobal = amplitudeGlobal; p.circular = circular; p.normalized = normalized;
			kernel->setParameters(p);
		}
	}

	ImVec4 ElementWindow::getColorForElementType(const element::ElementLabel label)
	{
		switch (label)
		{
		case element::ElementLabel::NEURAL_FIELD:
			return ImVec4(0.337f, 0.502f, 0.749f, 1.0f);  // Soft Blue
		case element::ElementLabel::NORMAL_NOISE:
			return ImVec4(0.875f, 0.580f, 0.329f, 1.0f);  // Warm Orange
		case element::ElementLabel::CORRELATED_NORMAL_NOISE:
			return ImVec4(0.820f, 0.490f, 0.200f, 1.0f);  // Deep Orange
		case element::ElementLabel::GAUSS_KERNEL:
			return ImVec4(0.749f, 0.247f, 0.247f, 1.0f);  // Muted Red
		case element::ElementLabel::GAUSS_STIMULUS:
			return ImVec4(0.498f, 0.749f, 0.498f, 1.0f);  // Sage Green
		case element::ElementLabel::MEXICAN_HAT_KERNEL:
			return ImVec4(0.604f, 0.475f, 0.749f, 1.0f);  // Lavender
		case element::ElementLabel::GAUSS_FIELD_COUPLING:
			return ImVec4(0.647f, 0.400f, 0.278f, 1.0f);  // Warm Brown
		case element::ElementLabel::FIELD_COUPLING:
			return ImVec4(0.831f, 0.753f, 0.475f, 1.0f);  // Cream Gold
		case element::ElementLabel::OSCILLATORY_KERNEL:
			return ImVec4(0.686f, 0.522f, 0.733f, 1.0f);  // Dusty Rose
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:
			return ImVec4(0.580f, 0.698f, 0.714f, 1.0f);  // Soft Teal
		case element::ElementLabel::BOOST_STIMULUS:
			return ImVec4(0.949f, 0.820f, 0.325f, 1.0f);  // Warm Yellow
		case element::ElementLabel::MEMORY_TRACE:
			return ImVec4(0.431f, 0.627f, 0.549f, 1.0f);  // Sage Green
		case element::ElementLabel::NEURAL_FIELD_2D:
			return ImVec4(0.293f, 0.437f, 0.651f, 1.0f);  // Deeper Soft Blue
		case element::ElementLabel::GAUSS_STIMULUS_2D:
			return ImVec4(0.433f, 0.651f, 0.433f, 1.0f);  // Deeper Sage Green
		case element::ElementLabel::GAUSS_KERNEL_2D:
			return ImVec4(0.651f, 0.216f, 0.216f, 1.0f);  // Deeper Muted Red
		case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:
			return ImVec4(0.525f, 0.412f, 0.651f, 1.0f);  // Deeper Lavender
		case element::ElementLabel::NORMAL_NOISE_2D:
			return ImVec4(0.761f, 0.506f, 0.286f, 1.0f);  // Deeper Warm Orange
		case element::ElementLabel::OSCILLATORY_KERNEL_2D:
			return ImVec4(0.596f, 0.455f, 0.639f, 1.0f);  // Deeper Dusty Rose
		case element::ElementLabel::TIMED_GAUSS_STIMULUS:
			return ImVec4(0.380f, 0.631f, 0.380f, 1.0f);  // Darker Sage Green
		case element::ElementLabel::TIMED_GAUSS_STIMULUS_2D:
			return ImVec4(0.314f, 0.522f, 0.314f, 1.0f);  // Deepest Sage Green
		case element::ElementLabel::BOOST_STIMULUS_2D:
			return ImVec4(0.825f, 0.714f, 0.283f, 1.0f);  // Deeper Warm Yellow
		case element::ElementLabel::CORRELATED_NORMAL_NOISE_2D:
			return ImVec4(0.714f, 0.427f, 0.173f, 1.0f);  // Deeper Deep Orange
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D:
			return ImVec4(0.506f, 0.608f, 0.622f, 1.0f);  // Deeper Soft Teal
		case element::ElementLabel::MEMORY_TRACE_2D:
			return ImVec4(0.376f, 0.545f, 0.478f, 1.0f);  // Deeper Sage Green
		default:
			return ImVec4(0.498f, 0.498f, 0.498f, 1.0f);  // Neutral Gray
		}
	}

	std::string ElementWindow::getElementTypeDisplayName(const element::ElementLabel label)
	{
		switch (label)
		{
		case element::ElementLabel::NEURAL_FIELD: return "Neural Fields";
		case element::ElementLabel::GAUSS_STIMULUS: return "Gaussian Stimuli";
		case element::ElementLabel::FIELD_COUPLING: return "Field Couplings";
		case element::ElementLabel::GAUSS_KERNEL: return "Gaussian Kernels";
		case element::ElementLabel::MEXICAN_HAT_KERNEL: return "Mexican Hat Kernels";
		case element::ElementLabel::NORMAL_NOISE: return "Normal Noise";
		case element::ElementLabel::CORRELATED_NORMAL_NOISE: return "Correlated Normal Noise";
		case element::ElementLabel::GAUSS_FIELD_COUPLING: return "Gaussian Field Couplings";
		case element::ElementLabel::OSCILLATORY_KERNEL: return "Oscillatory Kernels";
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL: return "Asymmetric Gaussian Kernels";
		case element::ElementLabel::BOOST_STIMULUS: return "Boost Stimuli";
		case element::ElementLabel::MEMORY_TRACE:  return "Memory Traces";
		case element::ElementLabel::NEURAL_FIELD_2D: return "Neural Fields 2D";
		case element::ElementLabel::GAUSS_STIMULUS_2D: return "Gauss Stimuli 2D";
		case element::ElementLabel::GAUSS_KERNEL_2D: return "Gauss Kernels 2D";
		case element::ElementLabel::MEXICAN_HAT_KERNEL_2D: return "Mexican Hat Kernels 2D";
		case element::ElementLabel::NORMAL_NOISE_2D: return "Normal Noise 2D";
		case element::ElementLabel::OSCILLATORY_KERNEL_2D: return "Oscillatory Kernels 2D";
		case element::ElementLabel::TIMED_GAUSS_STIMULUS:    return "Timed Gaussian Stimuli";
		case element::ElementLabel::TIMED_GAUSS_STIMULUS_2D: return "Timed Gaussian Stimuli 2D";
		case element::ElementLabel::BOOST_STIMULUS_2D:       return "Boost Stimuli 2D";
		case element::ElementLabel::CORRELATED_NORMAL_NOISE_2D: return "Correlated Normal Noise 2D";
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D: return "Asymmetric Gauss Kernels 2D";
		case element::ElementLabel::MEMORY_TRACE_2D:         return "Memory Traces 2D";
		default: return "Unknown Elements";
		}
	}

	PanelScope ElementWindow::beginElementPanel(const ImVec4& baseColor, const ImVec2& size)
	{
		// Draw background first, then place the cursor inside for content

		PanelScope p{};
		p.ui       = ImGui::GetIO().FontGlobalScale;
		p.rounding = 12.0f * p.ui;
		p.pad      = ImVec2(10.0f * p.ui, 8.0f * p.ui);

		// soft fill/border from base color
		p.fill   = ImGui::GetColorU32(ImVec4(baseColor.x, baseColor.y, baseColor.z, 0.18f));
		p.border = ImGui::GetColorU32(ImVec4(baseColor.x, baseColor.y, baseColor.z, 0.35f));

		// fixed rect from the current cursor (screen coords)
		const ImVec2 topLeft = ImGui::GetCursorScreenPos();
		const auto bottomRight = ImVec2(topLeft.x + size.x, topLeft.y + size.y);
		p.rect = ImRect(topLeft, bottomRight);

		// draw the panel *behind* content
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddRectFilled(p.rect.Min, p.rect.Max, p.fill, p.rounding);
		dl->AddRect      (p.rect.Min, p.rect.Max, p.border, p.rounding, 0, 1.5f * p.ui);

		// move the cursor inside, honoring padding, then start the content group
		const auto innerPos = ImVec2(p.rect.Min.x + p.pad.x, p.rect.Min.y + p.pad.y);
		ImGui::SetCursorScreenPos(innerPos);
		ImGui::BeginGroup();
		return p;
	}

	void ElementWindow::endElementPanel(const PanelScope& p)
	{
		ImGui::EndGroup();
		// advance cursor to just below the panel, keeping normal flow
		ImGui::SetCursorScreenPos(ImVec2(p.rect.Min.x, p.rect.Max.y + 6.0f * p.ui));
	}
}