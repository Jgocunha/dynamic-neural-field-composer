#pragma once

#include <vector>
#include <memory>
#include <string>
#include <filesystem>
#include <chrono>

#include "elements/element.h"
#include "exceptions/exception.h"
#include "tools/utils.h"

namespace dnf_composer
{
	class Simulation;
	std::shared_ptr<Simulation> createSimulation(const std::string& identifier = "", double deltaT = 1, double tZero = 0, double t = 0);

	class Simulation : public std::enable_shared_from_this<Simulation>
	{
	protected:
		bool initialized;
		bool paused;
		std::vector<std::shared_ptr<element::Element>> elements;
		std::string uniqueIdentifier;
	public:
		double deltaT;
		double tZero;
		double t;
	public:

		Simulation(const std::string& identifier = "", double deltaT = 1, double tZero = 0, double t = 0);
		Simulation(const Simulation& other);
		Simulation& operator=(const Simulation& other);
		Simulation(Simulation&& other) noexcept;
		Simulation& operator=(Simulation&&) noexcept;

		void init();
		void step();
		void run(double runTime);
		void runForRealTime(double milliseconds);
		void close();
		void pause();
		void resume();
		void clean();
		void save(const std::string& savePath = {});
		void read(const std::string& readPath = {});

		void addElement(const std::shared_ptr<element::Element>& element);
		void removeElement(const std::string& elementId);
		void resetElement(const std::string& idOfElementToReset, const std::shared_ptr<element::Element>& newElement);
		void createInteraction(const std::string& stimulusElementId, const std::string& stimulusComponent, 
			const std::string& receivingElementId) const;

		void setUniqueIdentifier(const std::string& id);
		void setDeltaT(double deltaT);

		std::vector<std::shared_ptr<element::Element>> getElements() const;
		std::string getUniqueIdentifier() const;
		std::shared_ptr<element::Element> getElement(const std::string& id) const;
		std::shared_ptr<element::Element> getElement(int index) const;
		std::vector<double> getComponent(const std::string& id, const std::string& componentName) const;
		std::vector<double>* getComponentPtr(const std::string& id, const std::string& componentName) const;
		int getNumberOfElements() const;
		std::vector < std::shared_ptr<element::Element>> getElementsThatHaveSpecifiedElementAsInput(const std::string& specifiedElement, 
		                                                                                            const std::string& inputComponent = "output") const;
		int getHighestElementIndex() const;
		std::string getIdentifier() const;
		double getDeltaT() const;
		double getTZero() const;
		double getT() const;
		std::chrono::nanoseconds getLastStepDuration() const;
		std::chrono::nanoseconds getTotalRunDuration() const;  // wall time since last init()

		bool componentExists(const std::string& id, const std::string& componentName) const;

		void exportComponentToFile(const std::string& id, const std::string& componentName) const;

		bool isInitialized() const;

		~Simulation() = default;
		std::chrono::nanoseconds lastStepDuration{ 0 };
		std::chrono::nanoseconds accumulatedRunDuration{ 0 };
		std::chrono::steady_clock::time_point runSegmentStart{};
	private:
		void generateUniqueIdentifier();
	};
}
