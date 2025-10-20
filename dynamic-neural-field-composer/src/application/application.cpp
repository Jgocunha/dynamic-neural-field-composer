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
		const FontParameters fontParams({ {std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Regular.ttf", 16},
												{std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Thin.ttf", 16},
												{std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Medium.ttf", 16},
												{std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Bold.ttf", 18},
												{std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Italic.ttf", 16},
												{std::string(PROJECT_DIR) + "/resources/fonts/JetBrainsMono-Light.ttf", 16},
			});
		const StyleParameters styleParams{ Theme::Light };
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

	void Application::loadImGuiIniFile() const
	{
		// [Put this elsewhere] Load ImGui Ini File
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
		// [Put this elsewhere] Enable Keyboard Controls
		auto io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		//ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGuiStyle* style = &ImGui::GetStyle();

		ImGui::StyleColorsDark();

		style->Alpha = 1.f;
		style->WindowRounding = 5;
		style->FramePadding = ImVec2(4, 3);
		style->WindowPadding = ImVec2(0, 0);
		style->ItemInnerSpacing = ImVec2(4, 4);
		style->ItemSpacing = ImVec2(8, 0);
		style->FrameRounding = 12;
		style->ScrollbarSize = 2.f;
		style->ScrollbarRounding = 12.f;
		style->PopupRounding = 4.f;
		// style->Rounding = 11.f;

		ImVec4* colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_ChildBg] = ImColor(26, 30, 35, 0);
		colors[ImGuiCol_Border] = ImVec4(255, 255, 255, 0);
		colors[ImGuiCol_FrameBg] = ImColor(18, 19, 23, 255);
		colors[ImGuiCol_FrameBgActive] = ImColor(25, 25, 33, 255);
		colors[ImGuiCol_FrameBgHovered] = ImColor(25, 25, 33, 255);
		colors[ImGuiCol_Header] = ImColor(141, 142, 144, 255);
		colors[ImGuiCol_HeaderActive] = ImColor(141, 142, 144, 255);
		colors[ImGuiCol_HeaderHovered] = ImColor(141, 142, 144, 255);
		colors[ImGuiCol_PopupBg] = ImColor(141, 142, 144, 255);
		colors[ImGuiCol_Button] = ImColor(160, 30, 30, 255);
		colors[ImGuiCol_ButtonHovered] = ImColor(190, 45, 35, 255);
		colors[ImGuiCol_ButtonActive] = ImColor(220, 60, 40, 255);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(110 / 255.f, 122 / 255.f, 200 / 255.f, 1.f);
		colors[ImGuiCol_SliderGrab] = ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f);
		colors[ImGuiCol_CheckMark] = ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f);

		ImFontConfig font_config;
		font_config.OversampleH = 1; //or 2 is the same
		font_config.OversampleV = 1;
		font_config.PixelSnapH = 1;

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x0400, 0x044F, // Cyrillic
			0,
		};

		iconfont = io.Fonts->AddFontFromMemoryTTF((void*)icon, sizeof(icon), 30, &font_config, ranges);
	}
}
