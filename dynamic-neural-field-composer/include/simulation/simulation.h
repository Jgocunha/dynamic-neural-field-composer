#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "tools/logger.h"

namespace dnf_composer
{
	struct SimulationParameters
	{
		std::string id;
		double dt;
		double t0;
		double t;
		SimulationParameters(std::string id = "default", double dt = 1.0, double t0 = 0.0, double t = 0.0)
			: id(std::move(id)), dt(dt), t0(t0), t(t)
		{
			if (dt <= 0 || t0 > t)
				throw std::invalid_argument("Invalid simulation parameters dt, t0, or t. "
								"dt must be positive and t0 must be less than t.");
		}
	};

	struct SimulationState
	{
		bool initialized;
		bool paused;
		SimulationState() : initialized(false), paused(false) {}
	};

	class Simulation
	{
	private:
		SimulationParameters parameters;
		SimulationState state;
		//std::vector<element::Element> elements;
	public:
		Simulation();
		Simulation(SimulationParameters parameters);
		Simulation(std::string id, double dt, double t0, double t);

		void initialize();
		void step();
		void terminate();
		void runFor(double runTime);
		void pause();
		void resume();
		void reset();

		void addElement(const element::Element& element);
		void removeElement(const std::string& id);
		

		//bool save(const std::string& path);
		//bool read(const std::string& path);
		//std::string toString() const;
		//void print() const;
		//bool operator==(const Simulation& other) const;
		bool isInitialized() const { return state.initialized; }
		bool isPaused() const { return state.paused; }
	};
}