#include "user_interface/plots_window.h"

namespace dnf_composer::user_interface
{
	PlotsWindow::PlotsWindow(const std::shared_ptr<Visualization>& visualization)
		:visualization(visualization), simulation(visualization->getSimulation())
	{}

	void PlotsWindow::render()
	{
		visualization->render();
	}
}
