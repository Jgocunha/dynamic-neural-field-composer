#include "elements/element.h"

#include <format>

namespace dnf_composer::element
{
	Element::Element(const ElementCommonParameters& parameters)
	{
		if(parameters.dimensionParameters.size <= 0)
		{
			const std::string logMessage = std::format("Element '{}' has an invalid size.", parameters.identifiers.uniqueName);
			log(tools::logger::LogLevel::ERROR, logMessage);
			return;
		}
		commonParameters = parameters;
		components["output"] = std::vector<double>(commonParameters.dimensionParameters.size);
		components["input"] = std::vector<double>(commonParameters.dimensionParameters.size);
	}

	Element::Element(const Element& other)
		: std::enable_shared_from_this<Element>(other),
		  commonParameters(other.commonParameters),
		  components(other.components),
		  inputs(other.inputs),
		  outputs(other.outputs)
	{
		// inputPtr/cachedInputs/inputSize are deliberately left at their default
		// member initializers (nullptr/empty/0), NOT copied from other. inputPtr
		// is a raw pointer into THIS object's own components["input"].data();
		// copying it by value (the implicit copy ctor's behaviour before this
		// was added) would leave a stepped-then-cloned element aliasing the
		// SOURCE's input buffer instead of its own freshly-copied one, so
		// updateInput() would silently read the wrong (and, worse, write into
		// the wrong) object's memory. Leaving it null forces updateInput() to
		// call buildInputCache() and re-derive it correctly on first use, the
		// same recovery path changeDimensions()/addInput()/removeInput() use.
	}

	Element& Element::operator=(const Element& other)
	{
		if (this != &other)
		{
			commonParameters = other.commonParameters;
			components = other.components;
			inputs = other.inputs;
			outputs = other.outputs;
			// See the copy constructor above for why these are reset, not copied.
			inputPtr = nullptr;
			inputSize = 0;
			cachedInputs.clear();
		}
		return *this;
	}

	void Element::changeDimensions(const ElementDimensions& newDimensions)
	{
		commonParameters.dimensionParameters = newDimensions;
		for (auto &vec: components | std::views::values) {
			vec.assign(newDimensions.size, 0.0);
}
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
			const std::string logMessage = std::format("Input '{}' already exists. ", inputElement->getUniqueName());
			log(tools::logger::LogLevel::ERROR, logMessage);
			return;
		}

		if (inputElement->getComponentPtr("output")->size() != this->getComponentPtr("input")->size())
		{
			if (inputElement->getComponentPtr("output")->size() != this->getSize())
			{
				const std::string logMessage = std::format("Input '{}' has a different size than '{}'.",
				                               inputElement->getUniqueName(), this->getUniqueName());
				log(tools::logger::LogLevel::ERROR, logMessage);
				return;
			}
		}

		inputs[inputElement] = inputComponent;
		inputElement->outputs[this->shared_from_this()] = inputComponent;
		inputPtr = nullptr;

		const std::string logMessage = std::format("Input '{}' added successfully to '{}.", inputElement->getUniqueName(), this->getUniqueName());
		log(tools::logger::LogLevel::INFO, logMessage);
	}

	void Element::removeInput(const std::string& inputElementId)
	{
		for (const auto& key : inputs | std::views::keys)
		{
			if (key->commonParameters.identifiers.uniqueName == inputElementId) {
				inputs.erase(key);
				inputPtr = nullptr;
				log(tools::logger::LogLevel::INFO, std::format("Input '{}' removed successfully from '{}. ",
				                                   inputElementId, this->getUniqueName()));
				return;
			}
		}
	}

	void Element::removeInput(const int uniqueId)
	{
		for (const auto& key : inputs | std::views::keys)
		{
			if (key->commonParameters.identifiers.uniqueIdentifier == uniqueId) {
				inputs.erase(key);
				inputPtr = nullptr;
				log(tools::logger::LogLevel::INFO, std::format("Input '{}' removed successfully from '{}.",
				                                   uniqueId, this->getUniqueName()));
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
		return found;
	}

	bool Element::hasInput(int inputElementId, const std::string& inputComponent)
	{
		const bool found = std::ranges::any_of(inputs, [&](const auto& pair) {
			const auto& [key, value] = pair;
			return key->commonParameters.identifiers.uniqueIdentifier == inputElementId && value == inputComponent;
		});
		return found;
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
			cachedInputs.push_back(&elem->components.at(compName));
		}
	}

	void Element::updateInput()
	{
		if (inputPtr == nullptr) {
			buildInputCache();
		}

		// Sum the input sources into the input buffer. Copy the first source
		// instead of zero-filling then adding it — this elides a full zero-fill
		// pass over the buffer every step (updateInput runs for every element).
		// Bit-identical to fill(0) + accumulate. Re-derive size/data from each
		// cached vector *object* on every call rather than trusting a size
		// snapshot taken when the cache was built: if the source was resized via
		// changeDimensions() since then, its buffer may have been reallocated,
		// but the vector object itself is stable, so this can never read a
		// dangling pointer -- only ever the source's current data. Some elements
		// (e.g. a circular GaussKernel) legitimately keep an "input" buffer
		// larger than the connected source's component -- that extra room is
		// padding, not a mismatch -- so only a source that grew *past* inputSize
		// is unsafe to accumulate in full; anything else fits.
		if (cachedInputs.empty())
		{
			std::fill_n(inputPtr, inputSize, 0.0);
			return;
		}

		bool incompatibleSourceFound = false;
		std::size_t firstIndex = 0;
		for (; firstIndex < cachedInputs.size(); ++firstIndex)
		{
			if (cachedInputs[firstIndex]->size() <= inputSize) {
				break;
}
			incompatibleSourceFound = true;
		}

		if (firstIndex == cachedInputs.size())
		{
			std::fill_n(inputPtr, inputSize, 0.0);
		}
		else
		{
			const std::vector<double>& first = *cachedInputs[firstIndex];
			const std::size_t n0 = first.size() < inputSize ? first.size() : inputSize;
			for (std::size_t i = 0; i < n0; ++i) {
				inputPtr[i] = first[i];
			}
			for (std::size_t i = n0; i < inputSize; ++i) {
				inputPtr[i] = 0.0;
			}

			for (std::size_t k = firstIndex + 1; k < cachedInputs.size(); ++k)
			{
				const std::vector<double>& srcVec = *cachedInputs[k];
				if (srcVec.size() > inputSize)
				{
					incompatibleSourceFound = true;
					continue;
				}
				for (std::size_t i = 0; i < srcVec.size(); ++i) {
					inputPtr[i] += srcVec[i];
				}
			}
		}

		if (incompatibleSourceFound) {
			severIncompatibleInputs();
}
	}

	void Element::severIncompatibleInputs()
	{
		std::vector<std::shared_ptr<Element>> toSever;
		for (const auto& [elem, compName] : inputs)
		{
			if (elem->components.at(compName).size() > inputSize) {
				toSever.push_back(elem);
}
		}

		for (const auto& elem : toSever)
		{
			const std::string logMessage = R"(Input ")" + elem->getUniqueName() +
				R"(" no longer matches the size of ")" + getUniqueName() +
				R"(" after being resized; severing the connection.)";
			log(tools::logger::LogLevel::WARNING, logMessage);
			elem->outputs.erase(this->shared_from_this());
			inputs.erase(elem);
		}

		inputPtr = nullptr; // inputs changed; rebuild cache on next updateInput()
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
		return found;
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
		return found;
	}

	void Element::removeOutputs()
	{
		// views::keys can be used
		for (const auto &key: outputs | std::views::keys)
		{
			const auto& outputElement = key;
			outputElement->inputs.erase(this->shared_from_this());
			outputElement->inputPtr = nullptr; // cachedInputs_ is now stale; rebuild on next updateInput()
		}
		outputs.clear();
	}

	int Element::getSize() const
	{
		return commonParameters.dimensionParameters.size;
	}

	void Element::removeOutput(const int uniqueId)
	{
		for (const auto& key : outputs | std::views::keys)
		{
			if (key->commonParameters.identifiers.uniqueIdentifier == uniqueId) {
				outputs.erase(key);
				log(tools::logger::LogLevel::INFO, std::format("Output '{}' removed successfully from '{}.",
				                                   uniqueId, this->getUniqueName()));
				return;
			}
		}
	}

	void Element::removeOutput(const std::string& outputElementId)
	{
		for (const auto& key : outputs | std::views::keys)
		{
			if (key->commonParameters.identifiers.uniqueName == outputElementId) {
				outputs.erase(key);
				log(tools::logger::LogLevel::INFO, std::format("Output '{}' removed successfully from '{}.",
				                                   outputElementId, this->getUniqueName()));
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
		if (components.contains(componentName)) {
			return components.at(componentName);
}
		throw Exception(ErrorCode::ELEM_COMP_NOT_FOUND, commonParameters.identifiers.uniqueName, componentName);
	}

	std::vector<double>* Element::getComponentPtr(const std::string& componentName)
	{
		if (components.contains(componentName)) {
			return &components.at(componentName);
}
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

		for (const auto& key : inputs | std::views::keys) {
			inputVec.push_back(key);
}

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

		for (const auto& key : outputs | std::views::keys) {
			outputVec.push_back(key);
}

		return outputVec;
	}

}
