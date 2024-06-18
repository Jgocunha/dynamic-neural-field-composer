#include "simulation/simulation.h"

namespace dnf_composer
{
	Simulation::Simulation()
		: parameters(), state()
	{
	}
	Simulation::Simulation(SimulationParameters parameters)
		: parameters(std::move(parameters)), state()
	{
	}
	Simulation::Simulation(std::string id, double dt, double t0, double t)
		: parameters(std::move(id), dt, t0, t), state()
	{
	}

	void Simulation::initialize()
	{
		parameters.t = parameters.t0;
		//for (auto& element : elements)
			//element.initialize();
		state.initialized = true;
		log(tools::logger::LogLevel::INFO, "Simulation initialized.");
	}

	void Simulation::step()
	{
		if (!state.initialized)
			log(tools::logger::LogLevel::ERROR, "Cannot step simulation."
									   " Simulation not initialized.");
		if (isPaused())
			return;
		parameters.t += parameters.dt;
		//for (auto& element : elements)
			//element.step(t, dt);

	}

	void Simulation::terminate()
	{
		//for (auto& element : elements)
			//element.terminate();
		state.initialized = false;
		state.paused = false;
		log(tools::logger::LogLevel::INFO, "Simulation terminated.");
	}

	void Simulation::runFor(double runTime)
	{
		if (runTime <= 0)
			log(tools::logger::LogLevel::ERROR, "Cannot run simulation."
				" Invalid run time. Run time must be positive.");

		if (!isInitialized())
			initialize();

		while(parameters.t < parameters.t0 + runTime)
			step();

		terminate();
	}

	void Simulation::pause()
	{
		state.paused = true;
		log(tools::logger::LogLevel::INFO, "Simulation paused.");
	}

	void Simulation::resume()
	{
		state.paused = false;
		log(tools::logger::LogLevel::INFO, "Simulation resumed.");
	}

	void Simulation::reset()
	{
		terminate();
		initialize();
		log(tools::logger::LogLevel::INFO, "Simulation reset.");
	}


}