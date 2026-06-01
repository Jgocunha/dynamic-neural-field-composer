#include "user_interface/help_window.h"

#include "application/application.h"

namespace dnf_composer::user_interface
{
	static constexpr int kPageCount = 4;

	static constexpr const char* kPageTitles[kPageCount] = {
		"About",
		"How to use",
		"Quick tips",
		"Resources",
	};

	void HelpWindow::draw()
	{
		if (isWindowActive && !ImGui::IsPopupOpen("Help##help_popup")) {
			ImGui::OpenPopup("Help##help_popup");
		}

		const float scale = ImGui::GetIO().FontGlobalScale;
		const ImGuiViewport* vp = ImGui::GetMainViewport();

		ImGui::SetNextWindowSize(ImVec2(640.0F * scale, 500.0F * scale), ImGuiCond_Always);
		ImGui::SetNextWindowPos(
			ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.5f,
			       vp->WorkPos.y + vp->WorkSize.y * 0.5f),
			ImGuiCond_Always,
			ImVec2(0.5f, 0.5f));

		constexpr ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoResize        |
			ImGuiWindowFlags_NoCollapse      |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoScrollbar     |
			ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoTitleBar;

		if (ImGui::BeginPopupModal("Help##help_popup", nullptr, flags))
		{
			// Custom title row matching other panel headers
			const float startY = ImGui::GetCursorPosY();
			const float yOff   = (g_MediumLargeFont->LegacySize - g_MediumIconsFont->LegacySize) * 0.5F;
			ImGui::SetCursorPosY(startY + yOff);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_CIRCLE_QUESTION);
			ImGui::PopFont();
			ImGui::PopStyleColor();
			ImGui::SameLine(0, 8.0F);
			ImGui::SetCursorPosY(startY);
			ImGui::PushFont(g_MediumLargeFont);
			ImGui::TextUnformatted("Help");
			ImGui::PopFont();

			// Close button floated to the right
			const float closeW = ImGui::CalcTextSize(ICON_FA_XMARK).x + (ImGui::GetStyle().FramePadding.x * 2.0F);
			ImGui::SameLine(ImGui::GetContentRegionMax().x - closeW);
			ImGui::SetCursorPosY(startY + yOff);
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0F, 0.0F, 0.0F, 0.0F));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			if (ImGui::Button(ICON_FA_XMARK))
			{
				isWindowActive = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor(3);
			ImGui::PopFont();

			ImGui::Separator();
			renderPageNav();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0F, 0.0F, 0.0F, 0.0F));
			if (ImGui::BeginChild("##help_content", ImVec2(0.0F, 0.0F), 0, ImGuiWindowFlags_None))
			{
				switch (activePage)
				{
					case 0: renderPageAbout(); break;
					case 1: renderPageHowToUse();   break;
					case 2: renderPageQuickTips();  break;
					case 3: renderPageResources();  break;
					default: break;
				}
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();

			ImGui::EndPopup();
		}
		else
		{
			isWindowActive = false;
		}
	}

	void HelpWindow::renderPageNav()
	{
		const ImVec4 accent  = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
		const ImVec4 normal  = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		const ImVec4 textSel(1.0F, 1.0F, 1.0F, 1.0F);
		const ImVec4 textNorm = ImGui::GetStyleColorVec4(ImGuiCol_Text);

		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float totalW  = ImGui::GetContentRegionAvail().x;
		const float btnW    = (totalW - spacing * (kPageCount - 1)) / kPageCount;

		ImGui::PushFont(g_BoldMediumFont);
		for (int i = 0; i < kPageCount; ++i)
		{
			if (i > 0) {
				ImGui::SameLine(0, spacing);
			}

			const bool selected = (activePage == i);
			ImGui::PushStyleColor(ImGuiCol_Button,        selected ? accent  : normal);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selected ? accent  : ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,  accent);
			ImGui::PushStyleColor(ImGuiCol_Text,          selected ? textSel : textNorm);

			ImGui::PushID(i);
			if (ImGui::Button(kPageTitles[i], ImVec2(btnW, 0.0F)))
			{
				activePage = i;
			}
			ImGui::PopID();

			ImGui::PopStyleColor(4);
		}
		ImGui::PopFont();
	}

	void HelpWindow::renderPageAbout()
	{
		ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);

		ImGui::PushFont(g_BoldLargeFont);
		ImGui::TextUnformatted("dynamic-neural-field-composer");
		ImGui::PopFont();
		ImGui::Spacing();
		ImGui::TextWrapped(
			"A C++ library and interactive application for building and simulating Dynamic Neural "
			"Field (DNF) architectures. Design field architectures in code or through the visual "
			"node-graph editor, run them in real time, and inspect activation dynamics live — "
			"without restarting the simulation.");
		ImGui::Spacing();
		ImGui::TextWrapped(
			"Dynamic Neural Fields model how neuron populations represent and transform "
			"information over continuous feature dimensions. Their dynamics emerge from a resting "
			"level, lateral interaction kernels, and external inputs — producing working memory, "
			"winner-take-all selection, and sequence generation as emergent behaviours.");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::PushFont(g_BoldLargeFont);
		ImGui::TextUnformatted("What dynamic-neural-field-composer provides");
		ImGui::PopFont();
		ImGui::Spacing();

		struct Row { const char* label; const char* desc; };
		static constexpr Row kRows[] = {
			{ "Simulation engine",      "Time-stepped loop that manages and steps all elements." },
			{ "Element library",        "Neural fields, lateral interaction kernels, stimuli, field couplings, noise sources." },
			{ "Learnable couplings",    "Weight-matrix couplings with Hebbian, Oja, and Delta learning rules." },
			{ "Real-time visualization","Line plots and heatmaps rendered via ImPlot." },
			{ "Interactive GUI",        "Node-graph editor, element inspector, field metrics panel, plot controls." },
			{ "Serialization",          "Save and load complete simulation configurations as JSON." },
		};

		for (const auto& [label, desc] : kRows)
		{
			ImGui::PushFont(g_BoldMediumFont);
			ImGui::BulletText("%s", label);
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::TextWrapped("— %s", desc);
		}

		ImGui::PopTextWrapPos();
	}

	void HelpWindow::renderPageHowToUse()
	{
		ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);

		ImGui::PushFont(g_BoldLargeFont);
		ImGui::TextUnformatted("Control Bar  (top strip)");
		ImGui::PopFont();
		ImGui::Spacing();
		ImGui::TextWrapped(
			"Play — initialise and start the simulation loop.  "
			"Pause — suspend execution while preserving the current state.  "
			"Resume — continue a paused simulation from where it left off.  "
			"Step — advance the simulation by a single tick.  "
			"Stop — end the current simulation.  "
			"The time display shows the current simulation time (ms); the delta-T field sets the time step.");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::PushFont(g_BoldLargeFont);
		ImGui::TextUnformatted("Simulation Control  (left sidebar)");
		ImGui::PopFont();
		ImGui::Spacing();

		struct TabRow { const char* icon; const char* name; const char* desc; };
		static constexpr TabRow kTabs[] = {
			{ ICON_FA_PLUS,       "Add elements",    "Instantiate neural fields, kernels, stimuli, noise sources, or couplings." },
			{ ICON_FA_TRASH,      "Remove elements", "Delete an existing element and all of its connections." },
			{ ICON_FA_LINK,       "Set interactions","Connect and disconnect elements." },
			{ ICON_FA_FILE_LINES, "Log parameters",  "Print the current parameter values of any element to the console." },
			{ ICON_FA_DOWNLOAD,   "Export data",     "Record element components continuously to CSV, or take a one-time snapshot. Files are saved to data/<sim_name>/." },
			{ ICON_FA_HEART_PULSE,"Monitoring",      "Open the field-metrics panel to track peak detection and activity statistics." },
		};

		const float scale = ImGui::GetIO().FontGlobalScale;
		if (ImGui::BeginTable("##sim_ctrl_table", 2, ImGuiTableFlags_None))
		{
			ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, 160.0F * scale);
			ImGui::TableSetupColumn("##desc",  ImGuiTableColumnFlags_WidthStretch);

			for (const auto& [icon, name, desc] : kTabs)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::PushFont(g_MediumIconsFont);
				ImGui::TextUnformatted(icon);
				ImGui::PopFont();
				ImGui::SameLine(0, 6.0F);
				ImGui::PushFont(g_BoldMediumFont);
				ImGui::TextUnformatted(name);
				ImGui::PopFont();
				ImGui::TableSetColumnIndex(1);
				ImGui::TextWrapped("%s", desc);
			}
			ImGui::EndTable();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::PushFont(g_BoldLargeFont);
		ImGui::TextUnformatted("Element Control  (right panel)");
		ImGui::PopFont();
		ImGui::Spacing();
		ImGui::TextWrapped(
			"Select any element from the dropdown to inspect and modify its parameters in real "
			"time. Changes take effect immediately without restarting the simulation.");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::PushFont(g_BoldLargeFont);
		ImGui::TextUnformatted("Node Graph  (centre panel)");
		ImGui::PopFont();
		ImGui::Spacing();
		ImGui::TextWrapped(
			"Displays all elements as nodes and their interactions as directed edges. "
			"Scroll to zoom; middle-mouse drag to pan. "
			"Drag from an Output pin to an Input pin to connect two elements. "
			"Double-click a link to remove it. "
			"Double-click a node to open or close its plot card.");

		ImGui::PopTextWrapPos();
	}

	void HelpWindow::renderPageQuickTips()
	{
#ifdef __APPLE__
		constexpr const char* kCtrl = "Cmd";
#else
		constexpr const char* kCtrl = "Ctrl";
#endif

		ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);

		static constexpr const char* kTips[] = {
			"Toggle the console log with the terminal icon at the bottom of the sidebar to see real-time messages.",
			"Use the Element Control panel on the right to tweak parameters live — no restart needed.",
			"Open the Field Metrics window from the Monitoring tab to watch peak position and amplitude over time.",
			"Log Parameters (sidebar) prints element state to the console.",
			"Use the Export data tab (sidebar) to record element components continuously to CSV or take a one-time snapshot. Files are saved to data/<sim_name>/.",
		};

		ImGui::PushFont(g_BoldLargeFont);
		ImGui::TextUnformatted("Tips & Shortcuts");
		ImGui::PopFont();
		ImGui::Spacing();

		auto renderTip = [](const char* text) {
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_LIGHTBULB);
			ImGui::PopFont();
			ImGui::SameLine(0, 6.0F);
			ImGui::TextWrapped("%s", text);
			ImGui::Spacing();
		};

		char buf[256];
		snprintf(buf, sizeof(buf), "Adjust the UI scale (70-150%%) via View > UI Scale, or use %s + / %s - keyboard shortcuts.", kCtrl, kCtrl);
		renderTip(buf);
		snprintf(buf, sizeof(buf), "Save via File > Save (%s+S) or Save As (%s+Shift+S). Open a simulation via File > Open (%s+O).", kCtrl, kCtrl, kCtrl);
		renderTip(buf);

		for (const auto* tip : kTips)
			renderTip(tip);

		ImGui::PopTextWrapPos();
	}

	void HelpWindow::renderPageResources()
	{
		ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);

		ImGui::PushFont(g_BoldLargeFont);
		ImGui::TextUnformatted("External Resources");
		ImGui::PopFont();
		ImGui::Spacing();

		struct Resource { const char* icon; const char* label; const char* url; const char* note; };
		static constexpr Resource kResources[] = {
			{
				ICON_FA_CODE_BRANCH,
				"GitHub Repository",
				"https://github.com/Jgocunha/dynamic-neural-field-composer",
				"Source code, issues, and releases."
			},
			{
				ICON_FA_BOOK,
				"GitHub Wiki",
				"https://github.com/Jgocunha/dynamic-neural-field-composer/wiki",
				"Full documentation: getting started, architecture, element reference, examples."
			},
			{
				ICON_FA_FILE_CODE,
				"API Reference (Doxygen)",
				"dynamic-neural-field-composer/docs/html/index.html",
				"Generated HTML reference for all classes and functions (build locally)."
			},
		};

		for (const auto& [icon, label, url, note] : kResources)
		{
			ImGui::Spacing();
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			ImGui::TextUnformatted(icon);
			ImGui::PopStyleColor();
			ImGui::PopFont();
			ImGui::SameLine(0, 8.0F);
			ImGui::PushFont(g_BoldMediumFont);
			ImGui::TextUnformatted(label);
			ImGui::PopFont();
			ImGui::Spacing();
			ImGui::Indent(28.0F);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			if (ImGui::Selectable(url, false)) {
				ImGui::SetClipboardText(url);
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Click to copy");
			}
			ImGui::TextWrapped("%s", note);
			ImGui::Unindent(28.0F);

			ImGui::Spacing();
			ImGui::Separator();
		}

		ImGui::Spacing();
		ImGui::PushFont(g_BoldLargeFont);
		ImGui::TextUnformatted("Reporting Issues");
		ImGui::PopFont();
		ImGui::Spacing();
		static constexpr const char* kIssueUrl = "https://github.com/Jgocunha/dynamic-neural-field-composer/issues";
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
		if (ImGui::Selectable(kIssueUrl, false)) {
			ImGui::SetClipboardText(kIssueUrl);
		}
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Click to copy");
		}
		ImGui::Spacing();
		ImGui::TextWrapped("Found a bug or have a feature request? Open an issue on GitHub.");


		ImGui::PopTextWrapPos();
	}
}
