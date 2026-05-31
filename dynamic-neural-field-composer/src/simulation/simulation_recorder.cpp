#include "simulation/simulation_recorder.h"
#include "simulation/simulation.h"
#include "tools/utils.h"
#include "tools/logger.h"

#include <algorithm>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace dnf_composer
{
	namespace
	{
		std::string makeTimestamp()
		{
			const auto now  = std::chrono::system_clock::now();
			const auto t    = std::chrono::system_clock::to_time_t(now);
			const auto ms   = std::chrono::duration_cast<std::chrono::milliseconds>(
			                      now.time_since_epoch()) % 1000;
			std::ostringstream oss;
			oss << std::put_time(std::localtime(&t), "%Y-%m-%d_%H-%M-%S")
			    << "_" << std::setfill('0') << std::setw(3) << ms.count();
			return oss.str();
		}

		int deriveTicks(const Simulation& sim)
		{
			return static_cast<int>(std::round((sim.t - sim.tZero) / sim.deltaT));
		}
	}

	void SimulationRecorder::writeHeader(std::ofstream& file, const size_t componentSize,
	                                     const int sizeX, const int sizeY)
	{
		if (sizeY > 1)
			file << "# size_x=" << sizeX << ",size_y=" << sizeY << "\n";
		file << "ticks,ms";
		for (size_t i = 0; i < componentSize; ++i)
			file << "," << i;
		file << "\n";
	}

	void SimulationRecorder::writeRow(std::ofstream& file, const int ticks, const double ms,
	                                  const std::vector<double>& component)
	{
		file << ticks << "," << std::fixed << std::setprecision(6) << ms;
		for (const double v : component)
			file << "," << v;
		file << "\n";
	}

	void SimulationRecorder::startRecording(const std::string& simName,
	                                        const std::string& elementId,
	                                        const std::string& componentName,
	                                        const int sampleInterval,
	                                        const RecordingIntervalUnit unit)
	{
		if (isRecording(elementId, componentName))
		{
			tools::logger::log(tools::logger::LogLevel::WARNING,
				"Recording already active for '" + elementId + "' / '" + componentName + "'.");
			return;
		}

		const std::filesystem::path dir = std::filesystem::path(tools::utils::getResourceRoot())
			/ "data" / simName / "recordings";
		std::filesystem::create_directories(dir);

		const std::string filename = (dir / (elementId + "_" + componentName + "_" + makeTimestamp() + ".csv")).string();

		Session session;
		session.elementId       = elementId;
		session.componentName   = componentName;
		session.sampleInterval  = sampleInterval;
		session.unit            = unit;
		session.nextSampleAt    = 0.0;
		session.file.open(filename);

		if (!session.file.is_open())
		{
			tools::logger::log(tools::logger::LogLevel::ERROR,
				"Failed to open recording file: " + filename);
			return;
		}

		tools::logger::log(tools::logger::LogLevel::INFO,
			"Recording started: " + filename);

		sessions.push_back(std::move(session));
	}

	void SimulationRecorder::stopRecording(const std::string& elementId,
	                                       const std::string& componentName)
	{
		const auto it = std::find_if(sessions.begin(), sessions.end(),
			[&](const Session& s) {
				return s.elementId == elementId && s.componentName == componentName;
			});

		if (it == sessions.end())
			return;

		if (it->file.is_open())
			it->file.close();

		tools::logger::log(tools::logger::LogLevel::INFO,
			"Recording stopped for '" + elementId + "' / '" + componentName + "'.");

		sessions.erase(it);
	}

	void SimulationRecorder::stopAll()
	{
		for (auto& s : sessions)
			if (s.file.is_open())
				s.file.close();
		sessions.clear();
	}

	void SimulationRecorder::update(const Simulation& sim)
	{
		if (sessions.empty())
			return;

		const int    ticks = deriveTicks(sim);
		const double ms    = sim.t;

		std::vector<size_t> toStop;

		for (size_t i = 0; i < sessions.size(); ++i)
		{
			auto& s = sessions[i];

			const auto element = sim.getElement(s.elementId);
			if (!element)
			{
				tools::logger::log(tools::logger::LogLevel::WARNING,
					"Stopping recording for '" + s.elementId + "': element no longer exists.");
				toStop.push_back(i);
				continue;
			}

			const std::vector<double>* component = element->getComponentPtr(s.componentName);
			if (!component)
			{
				tools::logger::log(tools::logger::LogLevel::WARNING,
					"Stopping recording for '" + s.elementId + "' / '" + s.componentName + "': component unavailable.");
				toStop.push_back(i);
				continue;
			}

			const double current = (s.unit == RecordingIntervalUnit::Ticks)
				? static_cast<double>(ticks) : ms;

			if (current < s.nextSampleAt)
				continue;

			if (s.file.tellp() == 0)
			{
				const auto& dims = element->getElementCommonParameters().dimensionParameters;
				writeHeader(s.file, component->size(), dims.size_x, dims.size_y);
			}

			writeRow(s.file, ticks, ms, *component);
			s.file.flush();

			s.nextSampleAt = current + static_cast<double>(s.sampleInterval);
		}

		for (auto it = toStop.rbegin(); it != toStop.rend(); ++it)
		{
			if (sessions[*it].file.is_open())
				sessions[*it].file.close();
			sessions.erase(sessions.begin() + static_cast<std::ptrdiff_t>(*it));
		}
	}

	void SimulationRecorder::takeSnapshot(const std::string& simName,
	                                      const std::string& elementId,
	                                      const std::string& componentName,
	                                      const Simulation& sim)
	{
		const auto element = sim.getElement(elementId);
		if (!element)
		{
			tools::logger::log(tools::logger::LogLevel::ERROR,
				"Snapshot failed: element '" + elementId + "' not found.");
			return;
		}

		const std::vector<double>* component = element->getComponentPtr(componentName);
		if (!component)
		{
			tools::logger::log(tools::logger::LogLevel::ERROR,
				"Snapshot failed: component '" + componentName + "' not found on '" + elementId + "'.");
			return;
		}

		const std::filesystem::path dir = std::filesystem::path(tools::utils::getResourceRoot())
			/ "data" / simName / "exports";
		std::filesystem::create_directories(dir);

		const std::string filename = (dir / (elementId + "_" + componentName + "_" + makeTimestamp() + ".csv")).string();

		std::ofstream file(filename);
		if (!file.is_open())
		{
			tools::logger::log(tools::logger::LogLevel::ERROR,
				"Failed to open snapshot file: " + filename);
			return;
		}

		const int    ticks = deriveTicks(sim);
		const double ms    = sim.t;

		const auto& dims = element->getElementCommonParameters().dimensionParameters;
		writeHeader(file, component->size(), dims.size_x, dims.size_y);
		writeRow(file, ticks, ms, *component);

		tools::logger::log(tools::logger::LogLevel::INFO,
			"Snapshot exported to: " + filename);
	}

	bool SimulationRecorder::isRecording(const std::string& elementId,
	                                     const std::string& componentName) const
	{
		return std::any_of(sessions.begin(), sessions.end(),
			[&](const Session& s) {
				return s.elementId == elementId && s.componentName == componentName;
			});
	}

	bool SimulationRecorder::hasActiveRecordings() const
	{
		return !sessions.empty();
	}
}
