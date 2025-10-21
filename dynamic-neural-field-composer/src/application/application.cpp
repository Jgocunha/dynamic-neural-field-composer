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
		const StyleParameters styleParams{ Theme::GreenLeaf };
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
	}
}
