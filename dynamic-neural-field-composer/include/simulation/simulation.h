#pragma once

#include <vector>
#include <memory>
#include <string>
#include <filesystem>
#include <chrono>

#include "elements/element.h"
#include "exceptions/exception.h"
#include "tools/utils.h"

/// @defgroup simulation Simulation
/// @brief Core simulation loop and element registry.

namespace dnf_composer
{
	class Simulation;

	/// @brief Factory helper — create a Simulation with the given parameters.
	/// @param identifier  Human-readable name for the simulation.
	/// @param deltaT      Integration step size.
	/// @param tZero       Start time.
	/// @param t           Initial current time.
	/// @return A shared_ptr to the new Simulation.
	/// @ingroup simulation
	std::shared_ptr<Simulation> createSimulation(const std::string& identifier = "", double deltaT = 1, double tZero = 0, double t = 0);

	/// @brief Manages the element registry and drives the simulation loop.
	///
	/// Simulation owns a collection of @c Element objects. Calling @c init() prepares
	/// all elements; each call to @c step() advances every element by @c deltaT.
	/// The simulation can be paused, resumed, saved to / loaded from JSON, and queried
	/// for per-step and total run timing.
	///
	/// @ingroup simulation
	class Simulation : public std::enable_shared_from_this<Simulation>
	{
	protected:
		bool initialized;                                        ///< True after @c init() has been called.
		bool paused;                                             ///< True while the simulation is paused.
		std::vector<std::shared_ptr<element::Element>> elements; ///< Ordered element registry.
		std::string uniqueIdentifier;                            ///< Human-readable simulation name.
	public:
		double deltaT;  ///< Integration step size.
		double tZero;   ///< Start time.
		double t;       ///< Current simulation time.
	public:

		/// @brief Construct a simulation.
		/// @param identifier  Human-readable name.
		/// @param deltaT      Step size (default 1).
		/// @param tZero       Start time (default 0).
		/// @param t           Initial current time (default 0).
		Simulation(const std::string& identifier = "", double deltaT = 1, double tZero = 0, double t = 0);
		Simulation(const Simulation& other);
		Simulation& operator=(const Simulation& other);
		Simulation(Simulation&& other) noexcept;
		Simulation& operator=(Simulation&&) noexcept;

		/// @brief Initialize all registered elements. Must be called before @c step().
		void init();

		/// @brief Advance all elements by one @c deltaT.
		void step();

		/// @brief Run the simulation for @p runTime milliseconds (blocking).
		/// @param runTime  Duration to simulate in ms.
		void run(double runTime);

		/// @brief Run the simulation in real-time for @p milliseconds wall-clock ms.
		/// @param milliseconds  Wall-clock duration to run for.
		void runForRealTime(double milliseconds);

		/// @brief Release resources of all elements and reset timing state.
		void close();

		/// @brief Pause the simulation (subsequent @c step() calls are no-ops).
		void pause();

		void resume();

		/// @brief Remove all elements and reset the simulation to its initial state.
		void clean();

		/// @brief Serialize the simulation and its elements to a JSON file.
		/// @param savePath  Destination file path; if empty, a default path is used.
		void save(const std::string& savePath = {});

		/// @brief Deserialize the simulation state from a JSON file.
		/// @param readPath  Source file path; if empty, a default path is used.
		void read(const std::string& readPath = {});

		void addElement(const std::shared_ptr<element::Element>& element);

		/// @brief Remove and destroy the element with the given unique name.
		/// @param elementId  Unique name of the element to remove.
		void removeElement(const std::string& elementId);

		/// @brief Replace an existing element with a new one, preserving connections.
		/// @param idOfElementToReset  Unique name of the element to replace.
		/// @param newElement          Replacement element.
		void resetElement(const std::string& idOfElementToReset, const std::shared_ptr<element::Element>& newElement);

		/// @brief Wire @p stimulusElementId's @p stimulusComponent as input to @p receivingElementId.
		/// @param stimulusElementId   Source element name.
		/// @param stimulusComponent   Component name on the source (e.g. "output").
		/// @param receivingElementId  Destination element name.
		void createInteraction(const std::string& stimulusElementId, const std::string& stimulusComponent,
			const std::string& receivingElementId) const;

		void setUniqueIdentifier(const std::string& id);
		void setDeltaT(double deltaT);
		std::vector<std::shared_ptr<element::Element>> getElements() const;
		std::string getUniqueIdentifier() const;

		/// @brief Retrieve an element by its unique name. Throws if not found.
		/// @param id  Unique name of the element.
		std::shared_ptr<element::Element> getElement(const std::string& id) const;

		/// @brief Retrieve an element by its registry index.
		/// @param index  Zero-based index into the element list.
		std::shared_ptr<element::Element> getElement(int index) const;

		std::vector<double> getComponent(const std::string& id, const std::string& componentName) const;
		std::vector<double>* getComponentPtr(const std::string& id, const std::string& componentName) const;
		int getNumberOfElements() const;

		/// @brief Return all elements that list @p specifiedElement as an input.
		/// @param specifiedElement  Name of the element to search for.
		/// @param inputComponent    Component name to match (default "output").
		std::vector<std::shared_ptr<element::Element>> getElementsThatHaveSpecifiedElementAsInput(const std::string& specifiedElement,
		                                                                                           const std::string& inputComponent = "output") const;

		int getHighestElementIndex() const;

		/// @brief Return the simulation's unique identifier (alias for @c getUniqueIdentifier()).
		std::string getIdentifier() const;

		double getDeltaT() const;
		double getTZero() const;
		double getT() const;

		/// @brief Return the wall-clock duration of the most recent @c step() call.
		std::chrono::nanoseconds getLastStepDuration() const;

		/// @brief Return total wall-clock time since the last @c init() call.
		std::chrono::nanoseconds getTotalRunDuration() const;

		bool componentExists(const std::string& id, const std::string& componentName) const;

		/// @brief Write the named component to a CSV file in the output directory.
		void exportComponentToFile(const std::string& id, const std::string& componentName) const;

		/// @brief Return true if @c init() has been called and the simulation is ready.
		bool isInitialized() const;

		~Simulation() = default;
		std::chrono::nanoseconds lastStepDuration{ 0 };
		std::chrono::nanoseconds accumulatedRunDuration{ 0 };
		std::chrono::steady_clock::time_point runSegmentStart{};
	private:
		void generateUniqueIdentifier();
	};
}
