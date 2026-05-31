#pragma once

#include <string>
#include <vector>
#include <fstream>

/// @defgroup simulation_recorder Simulation Recorder
/// @brief Time-series recording and snapshot export of element component data.
/// @ingroup simulation

namespace dnf_composer
{
	class Simulation;

	/// @brief Unit for the recording sample interval.
	/// @ingroup simulation_recorder
	enum class RecordingIntervalUnit { Ticks, Milliseconds };

	/// @brief Manages ongoing time-series recordings and snapshot exports of element
	/// component data for a simulation.
	///
	/// Recordings write rows to a CSV file at `data/<simName>/recordings/` each time
	/// the configured sample interval elapses. Snapshots write a single row to
	/// `data/<simName>/exports/`.
	///
	/// CSV format — for 1D elements: header `ticks,ms,0,1,...,N-1`; data rows `<ticks>,<ms>,<v0>,...`.
	/// For 2D elements a `# size_x=W,size_y=H` comment line precedes the header so readers
	/// can reshape the flat row-major columns back into a 2D grid.
	///
	/// @ingroup simulation_recorder
	class SimulationRecorder
	{
	public:
		/// @brief Start a new continuous recording for the given element/component pair.
		/// Creates `data/<simName>/recordings/<elementId>_<componentName>_<timestamp>.csv`
		/// and writes the header row. No-op if an identical recording is already active.
		/// @param simName         Simulation unique identifier (used as the sub-folder name).
		/// @param elementId       Unique name of the element to record.
		/// @param componentName   Name of the component vector (e.g. "activation").
		/// @param sampleInterval  How often to sample (in the chosen unit).
		/// @param unit            Whether @p sampleInterval is in ticks or milliseconds.
		void startRecording(const std::string& simName,
		                    const std::string& elementId,
		                    const std::string& componentName,
		                    int sampleInterval,
		                    RecordingIntervalUnit unit);

		/// @brief Stop the recording for a specific element/component pair.
		/// The CSV file is closed and the session is removed. No-op if not recording.
		/// @param elementId      Unique name of the element.
		/// @param componentName  Name of the component.
		void stopRecording(const std::string& elementId, const std::string& componentName);

		/// @brief Stop all active recordings and close all open files.
		void stopAll();

		/// @brief Called each simulation step to append rows to active recordings.
		/// Ticks are derived from `sim.t`, `sim.tZero`, and `sim.deltaT`.
		/// @param sim  The simulation whose components are being recorded.
		void update(const Simulation& sim);

		/// @brief Write a single-row snapshot CSV for the given element/component.
		/// Output path: `data/<simName>/exports/<elementId>_<componentName>_<timestamp>.csv`
		/// @param simName        Simulation unique identifier.
		/// @param elementId      Unique name of the element.
		/// @param componentName  Name of the component.
		/// @param sim            The simulation (used to read the component and current time).
		void takeSnapshot(const std::string& simName,
		                  const std::string& elementId,
		                  const std::string& componentName,
		                  const Simulation& sim);

		/// @brief Return true if a recording is currently active for the given pair.
		bool isRecording(const std::string& elementId, const std::string& componentName) const;

		/// @brief Return true if at least one recording session is active.
		bool hasActiveRecordings() const;

	private:
		struct Session
		{
			std::string elementId;
			std::string componentName;
			int sampleInterval;
			RecordingIntervalUnit unit;
			std::ofstream file;
			double nextSampleAt; ///< Next tick (or ms) at which to append a row.
		};

		std::vector<Session> sessions;

		/// @brief Write the CSV header. For 2D elements (sizeY > 1) a metadata comment
		/// line `# size_x=W,size_y=H` is emitted first so readers can reconstruct the grid.
		static void writeHeader(std::ofstream& file, size_t componentSize,
		                        int sizeX = 1, int sizeY = 1);
		static void writeRow(std::ofstream& file, int ticks, double ms,
		                     const std::vector<double>& component);
	};
}
