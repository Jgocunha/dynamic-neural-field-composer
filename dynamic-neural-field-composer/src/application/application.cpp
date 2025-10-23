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
		loadImGuiIniFile();
		enableKeyboardShortcuts();
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
		// const FontParameters fontParams({ {std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Regular.ttf", 16},
		// 										{std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Thin.ttf", 16},
		// 										{std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Medium.ttf", 16},
		// 										{std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Bold.ttf", 18},
		// 										{std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Italic.ttf", 16},
		// 										{std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Light.ttf", 16},
		//	});
		 const FontParameters fontParams({
		 											{std::string(PROJECT_DIR) + "/resources/fonts/Cera Pro Light.ttf", 16},
		 											{std::string(PROJECT_DIR) + "/resources/fonts/Cera Pro Medium.ttf", 16},
		 											{std::string(PROJECT_DIR) + "/resources/fonts/Cera Pro Bold.ttf", 22},
		 											{std::string(PROJECT_DIR) + "/resources/fonts/Cera Pro Black.ttf", 24},
			});
		const StyleParameters styleParams{Theme::Light, imgui_kit::colours::White};
#ifdef _WIN32
		const IconParameters iconParams{ std::string(PROJECT_DIR) + "/resources/icons/icon.ico" };
#else
		const IconParameters iconParams{ std::string(PROJECT_DIR) + "/resources/icons/icon.png" };
#endif
		//const BackgroundImageParameters bgParams{ std::string(PROJECT_DIR) + "/resources/images/background.png", ImageFitType::ZOOM_TO_FIT };
		const UserInterfaceParameters guiParameters{ winParams, fontParams, styleParams, iconParams, /*bgParams*/ };

		gui = std::make_shared<UserInterface>(guiParameters);
		imgui_kit::setGlobalWindowFlags(ImGuiWindowFlags_NoCollapse);
		log(tools::logger::LogLevel::INFO, "GUI parameters set successfully.");
	}

	void Application::loadImGuiIniFile() const
	{
		auto io = ImGui::GetIO();
		std::string iniFilePath = std::string(PROJECT_DIR) + "/resources/layouts/" + simulation->getIdentifier() + "_layout.ini";
		if (!std::filesystem::exists(iniFilePath))
		{
			log(tools::logger::LogLevel::INFO, "Layout file with simulation name does not exist. Using default layout file.");
			iniFilePath = std::string(PROJECT_DIR) + "/resources/layouts/default_layout.ini";
		}
		io.IniFilename = iniFilePath.c_str();
		ImGui::LoadIniSettingsFromDisk(io.IniFilename);
	}

	void Application::enableKeyboardShortcuts()
	{
		auto io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		// g_pIconFont = io.Fonts->AddFontFromMemoryTTF(icon_font, sizeof(icon_font), 17,
		// 	NULL, io.Fonts->GetGlyphRangesCyrillic());

		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true; // Merge icon font to the previous font if you want to have both icons and text
		io.Fonts->AddFontFromMemoryCompressedTTF(FA_compressed_data,
			FA_compressed_size, 16.0f, &icons_config, icons_ranges);

		//If you want change between icons size you will need to create a new font
		//io.Fonts->AddFontFromMemoryCompressedTTF(FA_compressed_data, FA_compressed_size, 12.0f, &icons_config, icons_ranges);
		//io.Fonts->AddFontFromMemoryCompressedTTF(FA_compressed_data, FA_compressed_size, 20.0f, &icons_config, icons_ranges);

		//To use brands icons you need do the same steps but using the brands header
		//If a quotation mark is displayed instead of the icon, probably the Icon header and Font Awesome version are not the same

		io.Fonts->Build();
	}
}
