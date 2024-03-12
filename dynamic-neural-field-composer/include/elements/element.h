#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <ranges>
#include <algorithm>

#include "exceptions/exception.h"
#include "tools/logger.h"
#include "element_parameters.h"

namespace dnf_composer
{
	namespace element
	{
		class Element
		{
		protected:
			ElementCommonParameters commonParameters;
			std::unordered_map<std::string, std::vector<double>> components;
			std::unordered_map<std::shared_ptr<Element>, std::string> inputs;
		public:
			Element(const ElementCommonParameters& parameters);

			virtual void init() = 0;
			virtual void step(double t, double deltaT) = 0;
			virtual void close() = 0;
			virtual void printParameters() = 0;
			virtual std::shared_ptr<Element> clone() const = 0;
			virtual ~Element() = default;

			void addInput(const std::shared_ptr<Element>& inputElement, const std::string& inputComponent = "output");
			void removeInput(const std::string& inputElementId);
			void removeInput(int uniqueId);
			bool hasInput(const std::string& inputElementName, const std::string& inputComponent);
			bool hasInput(int inputElementId, const std::string& inputComponent);
			void updateInput();

			int getMaxSpatialDimension() const;
			int getSize() const;
			double getStepSize() const;
			ElementCommonParameters getElementCommonParameters() const;
			int getUniqueIdentifier() const;
			std::string getUniqueName() const;
			ElementLabel getLabel() const;

			std::vector<double> getComponent(const std::string& componentName);
			std::vector<double>* getComponentPtr(const std::string& componentName);
			std::vector<std::string> getComponentList() const;

			std::vector<std::shared_ptr<Element>> getInputs();
			std::unordered_map<std::shared_ptr<Element>, std::string> getInputsAndComponents();

		protected:
			void printCommonParameters() const;
		};
	}
}