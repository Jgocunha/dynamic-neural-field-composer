// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/element.h"


namespace dnf_composer
{
	namespace element
	{
		Element::Element(const ElementCommonParameters& parameters)
		{
			if(parameters.dimensionParameters.size <= 0)
			{
				const std::string logMessage = "Element '" + parameters.identifiers.uniqueName + 
					"' has an invalid size.";
				log(tools::logger::LogLevel::ERROR, logMessage);
				return;
			}
			commonParameters = parameters;
			components["output"] = std::vector<double>(commonParameters.dimensionParameters.size);
			components["input"] = std::vector<double>(commonParameters.dimensionParameters.size);
		}

		void Element::close()
		{
			// views::values can be used
			for (auto& pair : components)
			{
				auto& component = pair.second;
				std::ranges::fill(component, 0);
			}
		}

		void Element::print() const
		{
			log(tools::logger::LogLevel::INFO, toString());
		}

		/*void Element::addInput(const std::shared_ptr<Element>& inputElement, const std::string& inputComponent)
		{
			if (!inputElement)
			{
				const std::string logMessage = "Input is null.";
				log(tools::logger::LogLevel::ERROR, logMessage);
				return;
			}

			const auto existingInput = inputs.find(inputElement);
			if (existingInput != inputs.end())
			{
				const std::string logMessage = "Input '" + inputElement->getUniqueName() + "' already exists. ";
				log(tools::logger::LogLevel::ERROR, logMessage);
				return;
			}

			if (inputElement->getComponentPtr("output")->size() != this->getComponentPtr("input")->size())
			{
				if (inputElement->getComponentPtr("output")->size() != this->getSize())
				{
					const std::string logMessage = "Input '" + inputElement->getUniqueName() + "' has a different size than '"
						+ this->getUniqueName() + "'.";
					log(tools::logger::LogLevel::ERROR, logMessage);
					return;
				}
			}

			inputs[inputElement] = inputComponent;
			inputElement->outputs[this->shared_from_this()] = inputComponent;

			const std::string logMessage = "Input '" + inputElement->getUniqueName() +"' added successfully to '" +  this->getUniqueName() + ".";
			log(tools::logger::LogLevel::INFO, logMessage);
		}*/

		void Element::addInput(const std::shared_ptr<Element>& inputElement, const std::string& inputComponent)
		{
			if (!inputElement)
			{
				const std::string logMessage = "Input is null.";
				log(tools::logger::LogLevel::ERROR, logMessage);
				return;
			}

			// views::keys can be used
			for (const auto& [key, value] : inputs)
			{
				if (auto lockedKey = key.lock())	
				{
					if (lockedKey == inputElement)
					{
						const std::string logMessage = "Input '" + inputElement->getUniqueName() + "' already exists.";
						log(tools::logger::LogLevel::ERROR, logMessage);
						return;
					}
				}
			}

			if (inputElement->getComponentPtr("output")->size() != this->getComponentPtr("input")->size())
			{
				if (inputElement->getComponentPtr("output")->size() != this->getSize())
				{
					const std::string logMessage = "Input '" + inputElement->getUniqueName() + "' has a different size than '" + this->getUniqueName() + "'.";
					log(tools::logger::LogLevel::ERROR, logMessage);
					return;
				}
			}

			inputs[inputElement] = inputComponent;
			inputElement->outputs[this->shared_from_this()] = inputComponent;

			const std::string logMessage = "Input '" + inputElement->getUniqueName() + "' added successfully to '" + this->getUniqueName() + "'.";
			log(tools::logger::LogLevel::INFO, logMessage);
		}

		//void Element::removeInput(const std::string& inputElementId)
		//{
		//	for (auto& key : inputs | std::views::keys)
		//	{
		//		if (key->commonParameters.identifiers.uniqueName == inputElementId) {
		//			inputs.erase(key);
		//			log(tools::logger::LogLevel::INFO, "Input '" + inputElementId + "' removed successfully from '" 
		//				+ this->getUniqueName() + ". ");
		//			return;
		//		}
		//	}
		//}

		void Element::removeInput(const std::string& inputElementId)
		{
			for (auto it = inputs.begin(); it != inputs.end(); )
			{
				auto key = it->first.lock();
				if (key && key->commonParameters.identifiers.uniqueName == inputElementId)
				{
					it = inputs.erase(it);
					log(tools::logger::LogLevel::INFO, "Input '" + inputElementId + "' removed successfully from '"
						+ this->getUniqueName() + "'. ");
					return;
				}
				else
				{
					++it;
				}
			}
		}

		void Element::removeInput(int uniqueId)
		{
			/*for (auto& key : inputs | std::views::keys)
			{
				if (key->commonParameters.identifiers.uniqueIdentifier == uniqueId) {
					inputs.erase(key);
					log(tools::logger::LogLevel::INFO, "Input '" + std::to_string(uniqueId) + "' removed successfully from '" 
						+ this->getUniqueName() + ".");
					return;
				}
			}*/

			for (auto it = inputs.begin(); it != inputs.end(); )
			{
				auto key = it->first.lock();
				if (key && key->commonParameters.identifiers.uniqueIdentifier == uniqueId)
				{
					it = inputs.erase(it);
					log(tools::logger::LogLevel::INFO, "Input '" + std::to_string(uniqueId) + "' removed successfully from '"
						+ this->getUniqueName() + "'. ");
					return;
				}
				else
				{
					++it;
				}
			}
		}

		bool Element::hasInput(const std::string& inputElementName, const std::string& inputComponent)
		{
			// commonParameters is not a member of 'std::weak_ptr'
			/*const bool found = std::ranges::any_of(inputs, [&](const auto& pair) {
				const auto& [key, value] = pair;
				return key->commonParameters.identifiers.uniqueName == inputElementName && value == inputComponent;
				});
			if (found)
				return true;
			return false;*/
			const bool found = std::ranges::any_of(inputs, [&](const auto& pair) {
				auto key = pair.first.lock();
				if (key)
					return key->commonParameters.identifiers.uniqueName == inputElementName && pair.second == inputComponent;
				return false;
				});
			if (found)
					return true;
				return false; 
		}

		bool Element::hasInput(int inputElementId, const std::string& inputComponent)
		{
			/*const bool found = std::ranges::any_of(inputs, [&](const auto& pair) {
				const auto& [key, value] = pair;
				return key->commonParameters.identifiers.uniqueIdentifier == inputElementId && value == inputComponent;
				});
			if (found)
				return true;
			return false;*/

			const bool found = std::ranges::any_of(inputs, [&](const auto& pair) {
				auto key = pair.first.lock();
				if (key)
					return key->commonParameters.identifiers.uniqueIdentifier == inputElementId && pair.second == inputComponent;
				return false;
				});
			if (found)
				return true;
			return false;
		}

		/*void Element::updateInput()
		{
			std::ranges::fill(components["input"], 0);

			for (const auto& input_pair : inputs) 
			{
				const auto inputElement = input_pair.first;
				auto inputElementComponent = input_pair.second;
				auto& inputElementComponents = inputElement->components;
				const auto& inputElementComponentValue = inputElementComponents.at(inputElementComponent);

				for (size_t i = 0; i < inputElementComponentValue.size(); i++)
					components["input"][i] += inputElementComponentValue[i];
			}
		}*/

		void Element::updateInput()
		{
			std::ranges::fill(components["input"], 0);

			for (auto it = inputs.begin(); it != inputs.end(); )
			{
				auto inputElement = it->first.lock();
				if (!inputElement)
				{
					it = inputs.erase(it);  // Remove expired weak_ptr from inputs
					continue;
				}

				const std::string& inputComponent = it->second;
				auto& inputElementComponents = inputElement->components;
				const auto& inputElementComponentValue = inputElementComponents.at(inputComponent);

				for (size_t i = 0; i < inputElementComponentValue.size(); i++)
					components["input"][i] += inputElementComponentValue[i];

				++it;
			}
		}

		int Element::getMaxSpatialDimension() const
		{
			return commonParameters.dimensionParameters.x_max;
		}

		double Element::getStepSize() const
		{
			return commonParameters.dimensionParameters.d_x;
		}

		ElementCommonParameters Element::getElementCommonParameters() const
		{
			return commonParameters;
		}

		int Element::getSize() const
		{
			return commonParameters.dimensionParameters.size;
		}

		std::string Element::getUniqueName() const
		{
			return commonParameters.identifiers.uniqueName;
		}

		int Element::getUniqueIdentifier() const
		{
			return commonParameters.identifiers.uniqueIdentifier;
		}

		ElementLabel Element::getLabel() const
		{
			return commonParameters.identifiers.label;
		}

		std::vector<double> Element::getComponent(const std::string& componentName)
		{
			if (components.contains(componentName))
				return components.at(componentName);
			throw Exception(ErrorCode::ELEM_COMP_NOT_FOUND, commonParameters.identifiers.uniqueName, componentName);
		}

		std::vector<double>* Element::getComponentPtr(const std::string& componentName)
		{
			if (components.contains(componentName))
				return &components.at(componentName);
			throw Exception(ErrorCode::ELEM_COMP_NOT_FOUND, commonParameters.identifiers.uniqueName, componentName);
		}

		std::vector<std::string> Element::getComponentList() const
		{

			std::vector<std::string> componentNames;
			componentNames.reserve(components.size());

			for (const auto& pair : components)
			{
				const std::string& componentName = pair.first;
				componentNames.push_back(componentName);
			}

			return componentNames;
		}

		const std::unordered_map<std::string, std::vector<double>>* Element::getComponents() const
		{
			return &components;
		}

		std::vector<std::shared_ptr<Element>> Element::getInputs()
		{
			/*std::vector<std::shared_ptr<Element>> inputVec;
			inputVec.reserve(inputs.size());

			for (const auto& key : inputs | std::views::keys)
				inputVec.push_back(key);

			return inputVec;*/

			std::vector<std::shared_ptr<Element>> inputVec;
			inputVec.reserve(inputs.size());

			for (const auto& pair : inputs)
			{
				auto key = pair.first.lock();
				if (key)
					inputVec.push_back(key);
			}

			return inputVec;
		}

		std::unordered_map<std::shared_ptr<Element>, std::string> Element::getInputsAndComponents()
		{
			//return inputs;
			std::unordered_map<std::shared_ptr<Element>, std::string> inputsAndComponents;

			for (const auto& pair : inputs)
			{
				auto key = pair.first.lock();
				if (key)
					inputsAndComponents[key] = pair.second;
			}

			return inputsAndComponents;
		}
	}
}
