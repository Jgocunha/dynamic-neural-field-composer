// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/field_coupling.h"

namespace dnf_composer
{
	namespace element
	{

		FieldCoupling::FieldCoupling(const ElementCommonParameters& elementCommonParameters, 
			const FieldCouplingParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::FIELD_COUPLING;
			components["input"] = std::vector<double>(parameters.inputFieldDimensions.size);
			components["output"] = std::vector<double>(commonParameters.dimensionParameters.size);
			components["weights"] = std::vector<double>(components.at("input").size()
				* components.at("output").size());
			std::ranges::fill(components["weights"], 0);
			weightsDirectory = std::string(OUTPUT_DIRECTORY) + "/inter-field-synaptic-connections";
			readWeights();
		}

		void FieldCoupling::init()
		{
			parameters.isLearningActive = false;
			std::ranges::fill(components["input"], 0);
			std::ranges::fill(components["output"], 0);

			updateInputField();
			updateOutputField();
			if(!checkValidConnections())
				return;
		}

		void FieldCoupling::step(double t, double deltaT)
		{
			updateInput();
			updateOutput();
			if (parameters.isLearningActive)
				if(checkValidConnections())
					updateWeights();
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

		void FieldCoupling::updateOutput()
		{
			components["output"] = std::vector<double>(components["output"].size(), 0);

			for (size_t i = 0; i < components["output"].size(); i++)
			{
				for (size_t j = 0; j < components["input"].size(); j++)
				{
					const size_t index = j * components["output"].size() + i;
					components["output"][i] += parameters.scalar * components["weights"][index] * components["input"][j];
				}
			}
		}

		void FieldCoupling::updateInputField()
		{
			if (inputs.size() != 1)
			{
				const std::string logMessage = "Incorrect number of inputs for field coupling '"
					+ commonParameters.identifiers.uniqueName + "'. Should be 1, is " + std::to_string(inputs.size()) + ".";
				log(tools::logger::LogLevel::WARNING, logMessage);
				return;
			}

			if (inputs.begin()->first->getLabel() != ElementLabel::NEURAL_FIELD)
			{
				const std::string logMessage = "Incorrect input type for field coupling '"
					+ commonParameters.identifiers.uniqueName + "'. Should be a neural field, is " + ElementLabelToString.at(inputs.begin()->first->getLabel()) + ".";
				log(tools::logger::LogLevel::WARNING, logMessage);
				return;
			}	

			input = inputs.begin()->first;
		}

		void FieldCoupling::updateOutputField()
		{
			if (outputs.size() != 1)
			{
				const std::string logMessage = "Incorrect number of outputs for field coupling '"
					+ commonParameters.identifiers.uniqueName + "'. Should be 1, is " + std::to_string(outputs.size()) + ".";
				log(tools::logger::LogLevel::WARNING, logMessage);
				return;
			}

			if (outputs.begin()->first->getLabel() != ElementLabel::NEURAL_FIELD)
			{
				const std::string logMessage = "Incorrect output type for field coupling '"
					+ commonParameters.identifiers.uniqueName + "'. Should be a neural field, is " + ElementLabelToString.at(outputs.begin()->first->getLabel()) + ".";
				log(tools::logger::LogLevel::WARNING, logMessage);
				return;
			}

			output = outputs.begin()->first;
		}

		void FieldCoupling::updateWeights()
		{
			std::vector<double> inputActivation = input->getComponents()->at("activation");
			std::vector<double> outputActivation = output->getComponents()->at("activation");

			inputActivation = tools::math::normalize(inputActivation);
			outputActivation = tools::math::normalize(outputActivation);

			switch (parameters.learningRule)
			{
			case LearningRule::DELTA:
				log(tools::logger::LogLevel::ERROR, "Unsupervised delta learning rule is not implemented yet.");
				//tools::math::unsupervisedDeltaLearningRule(components["weights"], inputActivation, outputActivation, parameters.learningRate);
				break;
			case LearningRule::HEBB:
				tools::math::hebbLearningRule(components["weights"], inputActivation, outputActivation, parameters.learningRate);
				break;
			case LearningRule::OJA:
				tools::math::ojaLearningRule(components["weights"], inputActivation, outputActivation, parameters.learningRate);
				break;
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
					log(tools::logger::LogLevel::ERROR,
						"Weight matrix read from file has a different size than expected! "
						"Expected: " + std::to_string(expectedSize) +
						", Got: " + std::to_string(weights.size()));
					return;
				}

				components["weights"] = weights;

				const std::string message = "Weights '" + this->getUniqueName() + "' read successfully from: " +
					filename + ".";
				log(tools::logger::LogLevel::INFO, message);
			}
			else {
				const std::string message = "Failed to read weights '" + this->getUniqueName() + "' from: " +
					filename + ".";
				log(tools::logger::LogLevel::ERROR, message);
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

				const std::string message = "Saved weights '" + this->getUniqueName() + "' to: " + filename + ".";
				log(tools::logger::LogLevel::INFO, message);
			}
			else {
				const std::string message = "Failed to save weights '" + this->getUniqueName() + "' to: " + filename + ".";
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
				const std::string logMessage = "Field coupling '" + commonParameters.identifiers.uniqueName +
					"' has no input field. Learning is disabled.";
				log(tools::logger::LogLevel::WARNING, logMessage);
				parameters.isLearningActive = false;
				return false;
			}

			if (!output)
			{
				const std::string logMessage = "Field coupling '" + commonParameters.identifiers.uniqueName +
					"' has no output field. Learning is disabled.";
				log(tools::logger::LogLevel::WARNING, logMessage);
				parameters.isLearningActive = false;
				return false;
			}

			return true;
		}

	}
}
