#include "elements/element.h"


namespace dnf_composer::element
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

	void Element::changeDimensions(const ElementDimensions& newDimensions)
	{
		commonParameters.dimensionParameters = newDimensions;
		for (auto &vec: components | std::views::values)
			vec.assign(newDimensions.size, 0.0);
		inputPtr = nullptr; // components ["input"] was reallocated; rebuild cache on next updateInput()
		init();
	}

	void Element::close()
	{
		for (auto &val: components | std::views::values)
		{
			auto& component = val;
			std::ranges::fill(component, 0);
		}
	}

	void Element::print() const
	{
		log(tools::logger::LogLevel::INFO, toString());
	}

	void Element::addInput(const std::shared_ptr<Element>& inputElement, const std::string& inputComponent)
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
		inputPtr = nullptr;

		const std::string logMessage = "Input '" + inputElement->getUniqueName() +"' added successfully to '" +  this->getUniqueName() + ".";
		log(tools::logger::LogLevel::INFO, logMessage);
	}

	void Element::removeInput(const std::string& inputElementId)
	{
		for (auto& key : inputs | std::views::keys)
		{
			if (key->commonParameters.identifiers.uniqueName == inputElementId) {
				inputs.erase(key);
				inputPtr = nullptr;
				log(tools::logger::LogLevel::INFO, "Input '" + inputElementId + "' removed successfully from '"
				                                   + this->getUniqueName() + ". ");
				return;
			}
		}
	}

	void Element::removeInput(const int uniqueId)
	{
		for (auto& key : inputs | std::views::keys)
		{
			if (key->commonParameters.identifiers.uniqueIdentifier == uniqueId) {
				inputs.erase(key);
				inputPtr = nullptr;
				log(tools::logger::LogLevel::INFO, "Input '" + std::to_string(uniqueId) + "' removed successfully from '"
				                                   + this->getUniqueName() + ".");
				return;
			}
		}
	}

	bool Element::hasInput(const std::string& inputElementName, const std::string& inputComponent)
	{
		const bool found = std::ranges::any_of(inputs, [&](const auto& pair) {
			const auto& [key, value] = pair;
			return key->commonParameters.identifiers.uniqueName == inputElementName && value == inputComponent;
		});
		if (found)
			return true;
		return false;
	}

	bool Element::hasInput(int inputElementId, const std::string& inputComponent)
	{
		const bool found = std::ranges::any_of(inputs, [&](const auto& pair) {
			const auto& [key, value] = pair;
			return key->commonParameters.identifiers.uniqueIdentifier == inputElementId && value == inputComponent;
		});
		if (found)
			return true;
		return false;
	}

	void Element::removeInputs()
	{
		// views::keys can be used
		for (const auto& input_pair : inputs)
		{
			const auto inputElement = input_pair.first;
			inputElement->outputs.erase(this->shared_from_this());
		}
		inputs.clear();
		inputPtr = nullptr; // cachedInputs_ is now stale; rebuild on next updateInput()
	}

	void Element::buildInputCache()
	{
		auto& inputVec = components["input"];
		inputPtr  = inputVec.data();
		inputSize = inputVec.size();

		cachedInputs.clear();
		cachedInputs.reserve(inputs.size());
		for (const auto& [elem, compName] : inputs)
		{
			const auto& compVec = elem->components.at(compName);
			cachedInputs.push_back({compVec.data(), compVec.size()});
		}
	}

	void Element::updateInput()
	{
		if (!inputPtr)
			buildInputCache();
		std::fill_n(inputPtr, inputSize, 0.0);
		for (const auto&[src, size] : cachedInputs)
			for (std::size_t i = 0; i < size; ++i)
				inputPtr[i] += src[i];
	}

	int Element::getMaxSpatialDimension() const
	{
		return commonParameters.dimensionParameters.x_max;
	}

	double Element::getStepSize() const
	{
		return commonParameters.dimensionParameters.d_x;
	}

	bool Element::hasOutput(const std::string& outputElementName, const std::string& outputComponent)
	{
		const bool found = std::ranges::any_of(outputs, [&](const auto& pair) {
			const auto& [key, value] = pair;
			return key->commonParameters.identifiers.uniqueName == outputElementName && value == outputComponent;
		});
		if (found)
			return true;
		return false;
	}

	ElementCommonParameters Element::getElementCommonParameters() const
	{
		return commonParameters;
	}

	bool Element::hasOutput(int outputElementId, const std::string& outputComponent)
	{
		const bool found = std::ranges::any_of(outputs, [&](const auto& pair) {
			const auto& [key, value] = pair;
			return key->commonParameters.identifiers.uniqueIdentifier == outputElementId && value == outputComponent;
		});
		if (found)
			return true;
		return false;
	}

	void Element::removeOutputs()
	{
		// views::keys can be used
		for (const auto &key: outputs | std::views::keys)
		{
			const auto outputElement = key;
			outputElement->inputs.erase(this->shared_from_this());
		}
		outputs.clear();
	}

	int Element::getSize() const
	{
		return commonParameters.dimensionParameters.size;
	}

	void Element::removeOutput(const int uniqueId)
	{
		for (auto& key : outputs | std::views::keys)
		{
			if (key->commonParameters.identifiers.uniqueIdentifier == uniqueId) {
				outputs.erase(key);
				log(tools::logger::LogLevel::INFO, "Output '" + std::to_string(uniqueId) + "' removed successfully from '"
				                                   + this->getUniqueName() + ".");
				return;
			}
		}
	}

	void Element::removeOutput(const std::string& outputElementId)
	{
		for (auto& key : outputs | std::views::keys)
		{
			if (key->commonParameters.identifiers.uniqueName == outputElementId) {
				outputs.erase(key);
				log(tools::logger::LogLevel::INFO, "Output '" + outputElementId + "' removed successfully from '"
				                                   + this->getUniqueName() + ".");
				return;
			}
		}
	}

	std::string Element::getUniqueName() const
	{
		return commonParameters.identifiers.uniqueName;
	}

	void Element::setUniqueName(const std::string& name)
	{
		commonParameters.identifiers.uniqueName = name;
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

		for (const auto& key : components | std::views::keys)
		{
			const std::string& componentName = key;
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
		std::vector<std::shared_ptr<Element>> inputVec;
		inputVec.reserve(inputs.size());

		for (const auto& key : inputs | std::views::keys)
			inputVec.push_back(key);

		return inputVec;
	}

	std::unordered_map<std::shared_ptr<Element>, std::string> Element::getInputsAndComponents()
	{
		return inputs;
	}

	bool Element::hasOutput() const
	{
		return !outputs.empty();
	}

	bool Element::hasInput() const
	{
		return !inputs.empty();
	}

	std::vector<std::shared_ptr<Element>> Element::getOutputs()
	{
		std::vector<std::shared_ptr<Element>> outputVec;
		outputVec.reserve(outputs.size());

		for (const auto& key : outputs | std::views::keys)
			outputVec.push_back(key);

		return outputVec;
	}

}
