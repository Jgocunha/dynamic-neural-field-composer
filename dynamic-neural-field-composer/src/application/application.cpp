// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "application/application.h"


namespace dnf_composer
{
	Application::Application(const std::shared_ptr<Simulation>& simulation, const std::shared_ptr<Visualization>& visualization)
		: simulation(simulation ? simulation : std::make_shared<Simulation>("default", 1.0, 0.0, 0.0)),
		visualization(visualization ? visualization : std::make_shared<Visualization>(this->simulation)),
		guiActive(true)
	{
		if (this->visualization->getSimulation() != this->simulation)
			throw Exception(ErrorCode::APP_VIS_SIM_MISMATCH);
		setGUIParameters();
	}

	void Application::init() const
	{
		simulation->init();
		gui->initialize();
		enableKeyboardShortcuts();
		appendFonts();
		log(tools::logger::LogLevel::INFO, "Application initialized successfully.");
	}

	void Application::step() const
	{
		simulation->step();
		if (guiActive)
		{
			gui->render();
		}
	}

	void Application::close() const
	{
		simulation->close();
		if (guiActive)
			gui->shutdown();
		log(tools::logger::LogLevel::INFO, "Application closed successfully.");
	}

	void Application::toggleGUI()
	{
		guiActive = !guiActive;
		log(tools::logger::LogLevel::INFO,std::string("GUI is ") + (guiActive ? "enabled." : "disabled."));
	}

	bool Application::hasGUIBeenClosed() const
	{
		if (guiActive)
			return gui->isShutdownRequested();
		return false;
	}

	bool Application::isGUIActive() const
	{
		return guiActive;
	}

	void Application::setGUIParameters()
	{
		using namespace imgui_kit;
		const WindowParameters winParams{ "Dynamic Neural Field Composer" };
		const FontParameters fontParams({
		 											{std::string(PROJECT_DIR) + "/resources/fonts/Cera Pro Light.ttf", 18},
		 											{std::string(PROJECT_DIR) + "/resources/fonts/Cera Pro Medium.ttf", 18},
		 											{std::string(PROJECT_DIR) + "/resources/fonts/Cera Pro Bold.ttf", 22},
		 											{std::string(PROJECT_DIR) + "/resources/fonts/Cera Pro Black.ttf", 24},
													{std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Regular.ttf", 18},
		});
		const StyleParameters styleParams{Theme::Light, imgui_kit::colours::White};
#ifdef _WIN32
		const IconParameters iconParams{ std::string(PROJECT_DIR) + "/resources/icons/icon.ico" };
#else
		const IconParameters iconParams{ std::string(PROJECT_DIR) + "/resources/icons/icon.png" };
#endif
		const BackgroundImageParameters bgParams{ std::string(PROJECT_DIR) + "/resources/images/background.png", ImageFitType::ZOOM_TO_FIT };
		const UserInterfaceParameters guiParameters{ winParams, fontParams, styleParams, iconParams, bgParams };

		gui = std::make_shared<UserInterface>(guiParameters);
		imgui_kit::setGlobalWindowFlags(ImGuiWindowFlags_NoCollapse);
		log(tools::logger::LogLevel::INFO, "GUI parameters set successfully.");
	}

	void Application::enableKeyboardShortcuts()
	{
		auto io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	}

	void Application::appendFonts()
	{
		auto io = ImGui::GetIO();

		ImFontConfig cfg{};
		cfg.OversampleH = 2;          // 2 is often crisper than 3 at small sizes
		cfg.OversampleV = 2;
		cfg.PixelSnapH  = true;       // snap glyphs to the pixel boundary for crispness
		cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_LightHinting   // gentle hinting (good for UI)
							 | ImGuiFreeTypeBuilderFlags_Bitmap;        // keep bitmap rendering (fast)
		// If you want *maximum* crispness at small sizes (old-school look), try:
		 cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;

		if (io.Fonts->Fonts.size() < g_FontCount)
		{
			tools::logger::log(tools::logger::FATAL, "Not enough fonts in the font stack. Please add more fonts to the font stack.");
			throw Exception(ErrorCode::APP_INIT);
		}

		// Add fonts
		g_LightFont = io.Fonts->Fonts[0];
		g_MediumFont = io.Fonts->Fonts[1];
		g_BoldFont = io.Fonts->Fonts[2];
		g_BlackFont = io.Fonts->Fonts[3];
		g_MonoFont = io.Fonts->Fonts[4];

		// Set default font
		io.FontDefault = g_MediumFont;

		// Set icons font
		static constexpr ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true; // Merge icon font to the previous font if you want to have both icons and text
		io.Fonts->AddFontFromMemoryCompressedTTF(FA_compressed_data,
			FA_compressed_size, 16.0f, &icons_config, icons_ranges);
		g_IconsFont = io.Fonts->Fonts[io.Fonts->Fonts.Size - 1]; // Get the last font added to the font stack

		//If you want change between icons size you will need to create a new font
		//io.Fonts->AddFontFromMemoryCompressedTTF(FA_compressed_data, FA_compressed_size, 12.0f, &icons_config, icons_ranges);
		//io.Fonts->AddFontFromMemoryCompressedTTF(FA_compressed_data, FA_compressed_size, 20.0f, &icons_config, icons_ranges);

		io.Fonts->Build();

		ImGuiStyle& style = ImGui::GetStyle();

	    // ----- Metrics & layout: tuned to your UI scale & rounded cards -----
	    style.Alpha                 = 1.0f;
	    style.DisabledAlpha         = 0.6f;

	    style.WindowPadding         = ImVec2(14, 10);
	    style.WindowRounding        = 8.0f;
	    style.WindowBorderSize      = 1.0f;
	    style.WindowMinSize         = ImVec2(32, 32);
	    style.WindowTitleAlign      = ImVec2(0.0f, 0.5f); // left-aligned titles
	    style.WindowMenuButtonPosition = ImGuiDir_None;

	    style.ChildRounding         = 8.0f;
	    style.ChildBorderSize       = 1.0f;

	    style.PopupRounding         = 8.0f;
	    style.PopupBorderSize       = 1.0f;

	    style.FramePadding          = ImVec2(10, 6);
	    style.FrameRounding         = 6.0f;
	    style.FrameBorderSize       = 1.0f;

	    style.ItemSpacing           = ImVec2(10, 8);
	    style.ItemInnerSpacing      = ImVec2(6, 6);
	    style.CellPadding           = ImVec2(6, 4);

	    style.IndentSpacing         = 16.0f;
	    style.ColumnsMinSpacing     = 8.0f;

	    style.ScrollbarSize         = 14.0f;
	    style.ScrollbarRounding     = 8.0f;

	    style.GrabMinSize           = 18.0f;
	    style.GrabRounding          = 6.0f;

	    style.TabRounding           = 6.0f;
	    style.TabBorderSize         = 1.0f;

	    style.ColorButtonPosition   = ImGuiDir_Right;
	    style.ButtonTextAlign       = ImVec2(0.5f, 0.5f);
	    style.SelectableTextAlign   = ImVec2(0.0f, 0.5f);

	    // ----- Palette -----
	    // Core brand / accent (from sidebar highlight)
	    constexpr auto ACCENT         = ImVec4(64/255.f, 163/255.f, 130/255.f, 1.0f);   // #40A382  :contentReference[oaicite:2]{index=2}
	    constexpr auto ACCENT_HOVER   = ImVec4(64/255.f, 163/255.f, 130/255.f, 0.85f);
	    constexpr auto ACCENT_ACTIVE  = ImVec4(64/255.f, 163/255.f, 130/255.f, 1.0f);

	    // Light chrome (from zones drawing)
		constexpr auto PANEL_LIGHT    = ImVec4(245/255.f, 247/255.f, 250/255.f, 1.0f);  // sidebar bg  :contentReference[oaicite:3]{index=3}
	    constexpr auto BORDER_LIGHT   = ImVec4(225/255.f, 229/255.f, 235/255.f, 1.0f);  // subtle border :contentReference[oaicite:4]{index=4}
	    constexpr auto CARD_BG        = ImVec4(1.00f, 1.00f, 1.00f, 0.96f);             // matches white cards
	    constexpr auto WINDOW_BG      = ImVec4(0.95f, 0.97f, 0.98f, 0.90f);             // soft wash over bg image
	    constexpr auto TEXT           = imgui_kit::colours::Gray;              // dark, crisp
	    constexpr auto TEXT_MUTED     = ImVec4(0.58f, 0.60f, 0.64f, 1.0f);              // for secondary labels
	    constexpr auto TEXT_INVERTED  = ImVec4(1.00f, 1.00f, 1.00f, 1.0f);

	    // Subtle states
	    constexpr auto HOVER          = ImVec4(0.05f, 0.05f, 0.05f, 0.04f);
	    constexpr auto ACTIVE         = ImVec4(0.05f, 0.05f, 0.05f, 0.08f);
	    constexpr auto SELECT_BG      = ImVec4(64/255.f, 163/255.f, 130/255.f, 0.15f);  // text selected bg

	    // ----- Colors -----
	    auto& c = style.Colors;
	    c[ImGuiCol_Text]                 = TEXT;
	    c[ImGuiCol_TextDisabled]         = TEXT_MUTED;

	    // Windows / areas
	    c[ImGuiCol_WindowBg]             = WINDOW_BG;
	    c[ImGuiCol_ChildBg]              = ImVec4(0,0,0,0);         // children are drawn as part of your custom zones
	    c[ImGuiCol_PopupBg]              = ImVec4(1,1,1,0.98f);

	    // Borders
	    c[ImGuiCol_Border]               = BORDER_LIGHT;
	    c[ImGuiCol_BorderShadow]         = ImVec4(0,0,0,0);

	    // Frames (inputs, combos, etc.)
	    c[ImGuiCol_FrameBg]              = CARD_BG;
	    c[ImGuiCol_FrameBgHovered]       = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.20f);
	    c[ImGuiCol_FrameBgActive]        = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.30f);

	    // Title bars (kept subtle; you mainly use custom cards)
	    c[ImGuiCol_TitleBg]              = PANEL_LIGHT;
	    c[ImGuiCol_TitleBgActive]        = ImVec4(1,1,1,1);
	    c[ImGuiCol_TitleBgCollapsed]     = PANEL_LIGHT;

	    // Menubar & scrollbars
	    c[ImGuiCol_MenuBarBg]            = PANEL_LIGHT;
	    c[ImGuiCol_ScrollbarBg]          = PANEL_LIGHT;
	    c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.66f,0.69f,0.73f, 0.85f);
	    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.55f,0.59f,0.63f, 0.85f);
	    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.45f,0.49f,0.53f, 0.95f);

	    // Checks, sliders, buttons
	    c[ImGuiCol_CheckMark]            = ACCENT;
	    c[ImGuiCol_SliderGrab]           = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.70f);
	    c[ImGuiCol_SliderGrabActive]     = ACCENT_ACTIVE;

	    c[ImGuiCol_Button]               = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.10f);
	    c[ImGuiCol_ButtonHovered]        = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.18f);
	    c[ImGuiCol_ButtonActive]         = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.26f);

	    // Headers (tree nodes, selectable headers, tables)
	    c[ImGuiCol_Header]               = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.12f);
	    c[ImGuiCol_HeaderHovered]        = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.20f);
	    c[ImGuiCol_HeaderActive]         = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.28f);

	    // Separators / resize grip
	    c[ImGuiCol_Separator]            = ImVec4(0.80f,0.84f,0.88f, 1.0f);
	    c[ImGuiCol_SeparatorHovered]     = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.70f);
	    c[ImGuiCol_SeparatorActive]      = ACCENT_ACTIVE;

	    c[ImGuiCol_ResizeGrip]           = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.10f);
	    c[ImGuiCol_ResizeGripHovered]    = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.22f);
	    c[ImGuiCol_ResizeGripActive]     = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.34f);

	    // Tabs
	    c[ImGuiCol_Tab]                  = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.18f);
	    c[ImGuiCol_TabHovered]           = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.30f);
	    c[ImGuiCol_TabActive]            = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.36f);
	    c[ImGuiCol_TabUnfocused]         = ImVec4(0.90f,0.93f,0.96f, 1.0f);
	    c[ImGuiCol_TabUnfocusedActive]   = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.28f);

	    // Plots
	    c[ImGuiCol_PlotLines]            = ImVec4(0.18f,0.35f,0.55f, 1.0f);
	    c[ImGuiCol_PlotLinesHovered]     = ACCENT_ACTIVE;
	    c[ImGuiCol_PlotHistogram]        = ImVec4(0.22f,0.48f,0.74f, 1.0f);
	    c[ImGuiCol_PlotHistogramHovered] = ACCENT_ACTIVE;

	    // Tables
	    c[ImGuiCol_TableHeaderBg]        = ImVec4(0.94f,0.96f,0.98f, 1.0f);
	    c[ImGuiCol_TableBorderStrong]    = ImVec4(0.85f,0.88f,0.92f, 1.0f);
	    c[ImGuiCol_TableBorderLight]     = ImVec4(0.90f,0.93f,0.95f, 1.0f);
	    c[ImGuiCol_TableRowBg]           = ImVec4(0,0,0,0);
	    c[ImGuiCol_TableRowBgAlt]        = ImVec4(0,0,0,0.03f);

	    // Selection / nav
	    c[ImGuiCol_TextSelectedBg]       = SELECT_BG;
	    c[ImGuiCol_DragDropTarget]       = ImVec4(1.0f, 0.8f, 0.0f, 0.90f);
	    c[ImGuiCol_NavHighlight]         = ACCENT;
	    c[ImGuiCol_NavWindowingHighlight]= ImVec4(1,1,1,0.70f);
	    c[ImGuiCol_NavWindowingDimBg]    = ImVec4(0,0,0,0.20f);
	    c[ImGuiCol_ModalWindowDimBg]     = ImVec4(0,0,0,0.35f);

		// Text
		c[ImGuiCol_TextDisabled] = TEXT_MUTED;
		c[ImGuiCol_Text] = TEXT;
	}

}
