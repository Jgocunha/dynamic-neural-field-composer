#include "elements/field_coupling.h"
#include "tools/utils.h"
#include <filesystem>
#include <format>


	namespace dnf_composer::element
	{

		FieldCoupling::FieldCoupling(const ElementCommonParameters& elementCommonParameters,
			const FieldCouplingParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::FIELD_COUPLING;
			components["input"] = std::vector<double>(parameters.inputFieldDimensions.size);
			components["output"] = std::vector<double>(commonParameters.dimensionParameters.size);
			components["target"] = std::vector<double>(commonParameters.dimensionParameters.size);
			components["weights"] = std::vector<double>(components.at("input").size()
				* components.at("output").size());
			std::ranges::fill(components["weights"], 0);
			weightsDirectory = tools::utils::getResourceRoot() + "/data";
		}

		void FieldCoupling::init()
		{
			parameters.isLearningActive = false;
			std::ranges::fill(components["input"], 0);
			std::ranges::fill(components["output"], 0);
			std::ranges::fill(components["target"], 0);

			updateInputField();
			updateOutputField();
			updateTargetField();
			if(!checkValidConnections()) {
				return;
}
		}

		void FieldCoupling::step(double t, double deltaT)
		{
			updateInput();      // fills "input" AND "target"
			updateOutput();     // "output" = scalar * W * input == U(x_out,t)
			if (parameters.isLearningActive && checkValidConnections()) {
				updateWeights();
}
		}

		std::string FieldCoupling::toString() const
		{
			std::string result = "Field coupling element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> FieldCoupling::clone() const
		{
			auto cloned = std::make_shared<FieldCoupling>(*this);
			return cloned;
		}

		void FieldCoupling::changeDimensions(const ElementDimensions& newDimensions)
		{
			commonParameters.dimensionParameters = newDimensions;
			const int inputSize = static_cast<int>(components["input"].size());
			components["output"].assign(newDimensions.size, 0.0);
			components["target"].assign(newDimensions.size, 0.0);
			components["weights"].assign(static_cast<std::size_t>(inputSize) * newDimensions.size, 0.0);
			// A previously connected target field is now size-mismatched: sever it from
			// the input graph too, not just the targetField pointer, or updateInput()
			// keeps summing its stale entry into components["target"].
			if (targetField) {
				Element::removeInput(targetField->getUniqueName());
}
			clearTarget();
			init();
		}

		void FieldCoupling::changeInputDimensions(const ElementDimensions& newInputDimensions)
		{
			parameters.inputFieldDimensions = newInputDimensions;
			const int outputSize = static_cast<int>(components["output"].size());
			components["input"].assign(newInputDimensions.size, 0.0);
			components["weights"].assign(static_cast<std::size_t>(newInputDimensions.size) * outputSize, 0.0);
			invalidateInputCache(); // "input" was just reallocated
			init();
		}

		void FieldCoupling::setParameters(const FieldCouplingParameters& fcp)
		{
			parameters = fcp;
		}

		void FieldCoupling::setWeightsDirectory(const std::string& dir)
		{
			weightsDirectory = dir;
		}

		void FieldCoupling::setLearningRate(double learningRate)
		{
			parameters.learningRate = learningRate;
		}

		void FieldCoupling::setDecayRate(double decayRate)
		{
			parameters.decayRate = decayRate;
		}

		void FieldCoupling::setLearning(bool learning)
		{
			parameters.isLearningActive = learning;
		}

		FieldCouplingParameters FieldCoupling::getParameters() const
		{
			return parameters;
		}

		std::string FieldCoupling::getWeightsDirectory() const
		{
			return weightsDirectory;
		}

		std::shared_ptr<Element> FieldCoupling::getTargetField() const
		{
			return targetField;
		}

		void FieldCoupling::updateOutput()
		{
			std::ranges::fill(components["output"], 0.0);

			for (size_t i = 0; i < components["output"].size(); i++)
			{
				for (size_t j = 0; j < components["input"].size(); j++)
				{
					const size_t index = j * components["output"].size() + i;
					components["output"][i] += parameters.scalar * components["weights"][index] * components["input"][j];
				}
			}
		}

		std::pair<bool, std::string> FieldCoupling::parseSlot(const std::string& declaredComponent)
		{
			if (declaredComponent == "target") {
				return { true, "output" };
			}
			if (declaredComponent == "target:activation") {
				return { true, "activation" };
			}
			return { false, declaredComponent };
		}

		void FieldCoupling::addInput(const std::shared_ptr<Element>& inputElement,
			const std::string& inputComponent)
		{
			const auto [isTarget, sourceComponent] = parseSlot(inputComponent);
			if (isTarget)
			{
				if (inputElement == input)
				{
					log(tools::logger::LogLevel::ERROR, std::format(
						"Field coupling '{}' cannot use the same field as both input and target.",
						commonParameters.identifiers.uniqueName));
					return;
				}
				if (targetField)
				{
					log(tools::logger::LogLevel::ERROR, std::format(
						"Field coupling '{}' already has a target field ('{}') connected. "
						"Remove it before connecting a new one.",
						commonParameters.identifiers.uniqueName, targetField->getUniqueName()));
					return;
				}
				// The literal "target"/"target:activation" is kept as the stored component
				// name (rather than resolved here) so it round-trips through JSON
				// persistence and the GUI can route the link to the Target pin by reading
				// it straight back off `inputs`. Resolution to inputElement's "output" or
				// "activation" happens only in updateInput().
				Element::addInput(inputElement, inputComponent);
				targetField = inputElement;
				targetSourceComponent = sourceComponent;
				updateTargetField();
			}
			else
			{
				Element::addInput(inputElement, inputComponent);
			}
			updateInputField();
			updateOutputField();
		}

		void FieldCoupling::removeInput(const std::string& inputElementId)
		{
			if (targetField && targetField->getUniqueName() == inputElementId) {
				clearTarget();
}
			Element::removeInput(inputElementId);
			updateInputField();
		}

		void FieldCoupling::removeInput(int uniqueId)
		{
			if (targetField && targetField->getUniqueIdentifier() == uniqueId) {
				clearTarget();
}
			Element::removeInput(uniqueId);
			updateInputField();
		}

		void FieldCoupling::removeInputs()
		{
			clearTarget();
			Element::removeInputs();
			updateInputField();
		}

		void FieldCoupling::updateInput()
		{
			std::ranges::fill(components["input"], 0.0);
			std::ranges::fill(components["target"], 0.0);

			for (const auto& [src, declaredComponent] : inputs)
			{
				// "target"/"target:activation" name THIS element's destination slot; the
				// teaching signal itself is read from the source field's own component,
				// as declared (defaults to "output" for plain "target").
				const auto [isTarget, sourceComponent] = parseSlot(declaredComponent);
				const auto& srcVec = src->getComponents()->at(sourceComponent);
				auto& dst = components[isTarget ? "target" : "input"];
				const std::size_t n = std::min(srcVec.size(), dst.size());
				for (std::size_t i = 0; i < n; ++i) {
					dst[i] += srcVec[i];
}
			}
		}

		void FieldCoupling::updateInputField()
		{
			// A source declared on a target slot ("target"/"target:activation") is a
			// teaching signal, not the coupling's input field -- exclude it from the
			// count/selection below.
			int inputCount = 0;
			std::shared_ptr<Element> candidate;
			std::string candidateComponent;
			for (const auto& [src, declaredComponent] : inputs)
			{
				if (parseSlot(declaredComponent).first) {
					continue;
}
				++inputCount;
				candidate = src;
				candidateComponent = declaredComponent;
			}

			if (inputCount != 1)
			{
				const std::string logMessage = std::format(
					"Incorrect number of inputs for field coupling '{}'. Should be 1, is {}.",
					commonParameters.identifiers.uniqueName, inputCount);
				log(tools::logger::LogLevel::WARNING, logMessage);
				// A removed/invalid input graph must not leave a stale `input` pointer
				// behind -- checkValidConnections() and DELTA would otherwise keep
				// reading a field that is no longer (or not yet) actually connected.
				input = nullptr;
				inputSourceComponent = "output";
				return;
			}

			if (candidate->getLabel() != ElementLabel::NEURAL_FIELD)
			{
				const std::string logMessage = std::format(
					"Incorrect input type for field coupling '{}'. Should be a neural field, is {}.",
					commonParameters.identifiers.uniqueName, ElementLabelToString.at(candidate->getLabel()));
				log(tools::logger::LogLevel::WARNING, logMessage);
				input = nullptr;
				inputSourceComponent = "output";
				return;
			}

			input = candidate;
			inputSourceComponent = candidateComponent;
		}

		void FieldCoupling::updateTargetField()
		{
			if (!targetField) {
				return;
}

			if (targetField->getLabel() != ElementLabel::NEURAL_FIELD)
			{
				log(tools::logger::LogLevel::WARNING, std::format(
					"Incorrect target type for field coupling '{}'. Should be a neural field, is {}.",
					commonParameters.identifiers.uniqueName, ElementLabelToString.at(targetField->getLabel())));
				// Sever the graph entry too, not just the pointer -- otherwise
				// updateInput() keeps summing this invalid source into components["target"].
				Element::removeInput(targetField->getUniqueName());
				clearTarget();
				return;
			}

			if (targetField->getComponents()->at(targetSourceComponent).size() != components.at("target").size())
			{
				log(tools::logger::LogLevel::WARNING, std::format(
					"Target field size does not match the output size of field coupling '{}'. Target ignored.",
					commonParameters.identifiers.uniqueName));
				Element::removeInput(targetField->getUniqueName());
				clearTarget();
			}
		}

		void FieldCoupling::clearTarget()
		{
			targetField = nullptr;
			targetSourceComponent = "output";
			std::ranges::fill(components["target"], 0.0);
		}

		void FieldCoupling::updateOutputField()
		{
			if (outputs.size() != 1)
			{
				const std::string logMessage = std::format(
					"Incorrect number of outputs for field coupling '{}'. Should be 1, is {}.",
					commonParameters.identifiers.uniqueName, outputs.size());
				log(tools::logger::LogLevel::WARNING, logMessage);
				return;
			}

			if (outputs.begin()->first->getLabel() != ElementLabel::NEURAL_FIELD)
			{
				const std::string logMessage = std::format(
					"Incorrect output type for field coupling '{}'. Should be a neural field, is {}.",
					commonParameters.identifiers.uniqueName, ElementLabelToString.at(outputs.begin()->first->getLabel()));
				log(tools::logger::LogLevel::WARNING, logMessage);
				return;
			}

			output = outputs.begin()->first;
		}

		void FieldCoupling::updateWeights()
		{
			switch (parameters.learningRule)
			{
			case LearningRule::DELTA:
			{
				// Eq. 5/6/7: pre = g(u_in) (or raw activation, if input was wired from
				// the Activation pin -- see inputSourceComponent), err = target -
				// U(x_out,t), where U is this coupling's own forward pass, already
				// computed by updateOutput() (called immediately before this, in
				// step()) into components["output"]. Deliberately NOT gated by the
				// output field's own post-synaptic activity -- see
				// deltaLearningRuleWidrowHoff()'s doc comment for why (it deadlocks a
				// zero-initialized coupling that is the output field's only drive).
				// Also does NOT normalize() its operands -- unlike HEBB/OJA below,
				// pre/target here are already in their wired range, and normalize()
				// would map a uniform (e.g. constant) signal to all-zeros, silently
				// discarding it.
				tools::math::deltaLearningRuleWidrowHoff(components.at("weights"),
					input->getComponents()->at(inputSourceComponent), // pre  = g(u_in) or activation
					components.at("target"),                          // target = g(u_out^tar)
					components.at("output"),                          // actual = U(x_out,t)
					parameters.learningRate, parameters.decayRate);
				break;
			}
			case LearningRule::HEBB:
			case LearningRule::OJA:
			{
				std::vector<double> inputActivation = tools::math::normalize(input->getComponents()->at("activation"));
				std::vector<double> outputActivation = tools::math::normalize(output->getComponents()->at("activation"));
				if (parameters.learningRule == LearningRule::HEBB) {
					tools::math::hebbLearningRule(components["weights"], inputActivation, outputActivation, parameters.learningRate);
				} else {
					tools::math::ojaLearningRule(components["weights"], inputActivation, outputActivation, parameters.learningRate);
}
				break;
			}
			}
		}

		void FieldCoupling::readWeights()
		{
			const std::string filename = weightsDirectory + "/" + commonParameters.identifiers.uniqueName + "_weights.txt";
			std::ifstream file(filename);

			const size_t inputSize = components.at("input").size();
			const size_t outputSize = components.at("output").size();
			const size_t expectedSize = inputSize * outputSize;

			if (file.is_open()) 
			{
				std::vector<double> weights;
				weights.reserve(expectedSize);
				double element;

				while (file >> element) 
				{
					weights.emplace_back(element);
				}
				file.close();

				// Check if the total number of weights matches the expected size
				if (weights.size() != expectedSize)
				{
					log(tools::logger::LogLevel::ERROR, std::format(
						"Weight matrix read from file has a different size than expected! Expected: {}, Got: {}",
						expectedSize, weights.size()));
					return;
				}

				components["weights"] = weights;

				const std::string message = std::format("Weights '{}' read successfully from: {}.", this->getUniqueName(), filename);
				log(tools::logger::LogLevel::INFO, message);
			}
			else {
				const std::string message = std::format("Failed to read weights '{}' from: {}.", this->getUniqueName(), filename);
				log(tools::logger::LogLevel::ERROR, message);
			}
		}

		void FieldCoupling::tryReadWeights()
		{
			const std::string filename = weightsDirectory + "/" + commonParameters.identifiers.uniqueName + "_weights.txt";
			if (std::filesystem::exists(filename))
			{
				readWeights();
			}
			else
			{
				log(tools::logger::LogLevel::INFO, std::format(
					"No weights file found for '{}' at: {}. Starting with zero weights.",
					commonParameters.identifiers.uniqueName, filename));
			}
		}

		void FieldCoupling::writeWeights() const
		{
			const std::string filename = weightsDirectory + "/" + commonParameters.identifiers.uniqueName + "_weights.txt";
			std::ofstream file(filename);

			if (file.is_open()) 
			{
				const size_t inputSize = components.at("input").size();
				const size_t outputSize = components.at("output").size();

				for (size_t i = 0; i < inputSize; i++) 
				{
					for (size_t j = 0; j < outputSize; j++) 
					{
						const size_t index = i * outputSize + j;
						file << components.at("weights")[index] << " ";
					}
					file << '\n';
				}

				file.close();

				const std::string message = std::format("Saved weights '{}' to: {}.", this->getUniqueName(), filename);
				log(tools::logger::LogLevel::INFO, message);
			}
			else {
				const std::string message = std::format("Failed to save weights '{}' to: {}.", this->getUniqueName(), filename);
				log(tools::logger::LogLevel::ERROR, message);
			}
		}

		void FieldCoupling::clearWeights()
		{
			components["weights"] = std::vector<double>(components["weights"].size(), 0);
		}

		bool FieldCoupling::checkValidConnections()
		{
			if (!input)
			{
				const std::string logMessage = std::format(
					"Field coupling '{}' has no input field. Learning is disabled.", commonParameters.identifiers.uniqueName);
				log(tools::logger::LogLevel::WARNING, logMessage);
				parameters.isLearningActive = false;
				return false;
			}

			if (!output)
			{
				const std::string logMessage = std::format(
					"Field coupling '{}' has no output field. Learning is disabled.", commonParameters.identifiers.uniqueName);
				log(tools::logger::LogLevel::WARNING, logMessage);
				parameters.isLearningActive = false;
				return false;
			}

			if (parameters.learningRule == LearningRule::DELTA && !targetField)
			{
				const std::string logMessage = std::format(
					"Field coupling '{}' uses the DELTA learning rule but has no target field connected. "
					"Connect a neural field to the Target pin. Learning is disabled.",
					commonParameters.identifiers.uniqueName);
				log(tools::logger::LogLevel::WARNING, logMessage);
				parameters.isLearningActive = false;
				return false;
			}

			return true;
		}

	}
