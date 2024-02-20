#pragma once

#include "exceptions/exception.h"
#include "simulation/simulation.h"
#include "simulation/visualization.h"
#include "user_interface/user_interface.h"
#include "user_interface/centroid_monitoring_window.h"
#include "user_interface/element_window.h"


namespace dnf_composer
{
	class Application
	{
	private:
		std::shared_ptr<Simulation> simulation;
		std::shared_ptr<user_interface::UserInterface> userInterface;
		bool activateUserInterface;
	public:
		Application(const std::shared_ptr<Simulation>& simulation, bool activateUserInterface = true);
		Application(const Application&) = delete;             
		Application& operator=(const Application&) = delete;  
		Application(Application&&) = delete;                  
		Application& operator=(Application&&) = delete;       

		void init() const;
		void step() const;
		void close() const;

		void activateUserInterfaceWindow(const std::shared_ptr<user_interface::UserInterfaceWindow>& window) const;
		void activateUserInterfaceWindow(user_interface::UserInterfaceWindowType winType, const user_interface::UserInterfaceWindowParameters& winParams = {}) const;
		void setActivateUserInterfaceAs(bool activateUI);

		bool getCloseUI() const;
		bool getActivateUserInterface() const;

		~Application() = default;
	};
}

