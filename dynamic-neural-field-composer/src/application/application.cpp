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
	}

	void Application::step() const
	{
		simulation->step();
		if (guiActive)
			gui->render();
	}

	void Application::close() const
	{
		simulation->close();
		if (guiActive)
			gui->shutdown();
	}

	void Application::toggleGUI()
	{
		guiActive = !guiActive;
		//if (guiActive)
		//{
		//	setGUIParameters();
		//	gui->initialize();
		//}
		//else
			//gui->shutdown();
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
		const FontParameters fontParams{ std::string(PROJECT_DIR) + "/resources/fonts/Lexend-Light.ttf", 15 };
		const StyleParameters styleParams{ ImVec4(0.3f, 0.3f, 0.3f, 0.6f), colours::White };
		const IconParameters iconParams{ std::string(PROJECT_DIR) + "/resources/icons/icon.ico" };
		const BackgroundImageParameters bgParams{ std::string(PROJECT_DIR) + "/resources/images/background.png", ImageFitType::ZOOM_TO_FIT };
		const UserInterfaceParameters guiParameters{ winParams, fontParams, styleParams, iconParams, bgParams };
		gui = std::make_shared<imgui_kit::UserInterface>(guiParameters);
	}
}
