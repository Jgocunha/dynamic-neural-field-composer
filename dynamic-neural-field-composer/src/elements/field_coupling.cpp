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
			if (parameters.inputFieldSize <= 0)
				throw Exception(ErrorCode::ELEM_SIZE_NOT_ALLOWED, commonParameters.identifiers.uniqueName);

			commonParameters.identifiers.label = ElementLabel::FIELD_COUPLING;

			components["input"] = std::vector<double>(parameters.inputFieldSize);
			tools::utils::resizeMatrix(weights, static_cast<int>(components["input"].size()),
				static_cast<int>(components["output"].size()));

			tools::utils::fillMatrixWithRandomValues(weights, -1, 1);

			weightsFilePath = std::string(OUTPUT_DIRECTORY) + "/inter-field-synaptic-connections/" + 
				commonParameters.identifiers.uniqueName + "_weights.txt";

			updateAllWeights = true;
			trained = false;

			// new add
			components["kernel"] = std::vector<double>(commonParameters.dimensionParameters.size * parameters.inputFieldSize);

		}

		void FieldCoupling::init()
		{
			std::ranges::fill(components["input"], 0);
			std::ranges::fill(components["output"], 0);

			if (readWeights())
				trained = true;
			else
				fillWeightsRandomly();

			// new add
			components["kernel"] = tools::math::flattenMatrix(weights);
		}

		void FieldCoupling::step(double t, double deltaT)
		{
			getInputFunction();
			computeOutput();
			scaleOutput();
		}

		std::string FieldCoupling::toString() const
		{
			std::string result = "Field coupling element\n";
			result += commonParameters.toString();
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> FieldCoupling::clone() const
		{
			auto cloned = std::make_shared<FieldCoupling>(*this);
			return cloned;
		}

		void FieldCoupling::getInputFunction()
		{
			updateInput();
			for (auto& value : components["input"])
				if (value < 0)
					value = 0;
		}

		void FieldCoupling::computeOutput()
		{
			components["output"] = std::vector<double>(components["output"].size(), 0);

			for (int i = 0; i < components["output"].size(); i++)
				for (int j = 0; j < components["input"].size(); j++)
					components["output"][i] += weights[j][i] * components["input"][j];

			for (auto& value : components["output"])
				if (value < 0)
					value = 0;
		}

		void FieldCoupling::scaleOutput()
		{
			for (auto& value : components["output"])
				value *= parameters.scalar;
		}

		void FieldCoupling::resetWeights()
		{
			tools::utils::fillMatrixWithRandomValues(weights, 0, 0);
		}

		void FieldCoupling::setUpdateAllWeights(bool updateAllWeights)
		{
			this->updateAllWeights = updateAllWeights;
		}

		void FieldCoupling::setParameters(const FieldCouplingParameters& fcp)
		{
			parameters = fcp;
		}

		void FieldCoupling::updateWeights(const std::vector<double>& input, const std::vector<double>& output)
		{
			switch (parameters.learningRule)
			{
			case LearningRule::HEBBIAN:
				weights = tools::math::hebbLearningRule(weights, input, output, parameters.learningRate);
				break;
			case LearningRule::DELTA_WIDROW_HOFF:
				weights = tools::math::deltaLearningRuleWidrowHoff(weights, input, output,
					parameters.learningRate);
				break;
			case LearningRule::DELTA_KROGH_HERTZ:
				weights = tools::math::deltaLearningRuleKroghHertz(weights, input, output, 
					parameters.learningRate);
			case LearningRule::OJA:
				weights = tools::math::ojaLearningRule(weights, input, output, parameters.learningRate);
				break;
			}
		}

		void FieldCoupling::setLearningRate(double learningRate)
		{
			parameters.learningRate = learningRate;
		}

		const std::vector<std::vector<double>>& FieldCoupling::getWeights() const
		{
			return weights;
		}

		FieldCouplingParameters FieldCoupling::getParameters() const
		{
			return parameters;
		}

		bool FieldCoupling::readWeights()
		{
			std::ifstream file(weightsFilePath);

			const std::tuple<int, int> initialWeightsSize = std::make_tuple( static_cast<int>(weights.size()), 
				static_cast<int>(weights[0].size()) );
			std::tuple<int, int> readWeightsSize = std::make_tuple(0, 0);

			if (file.is_open()) {
				tools::utils::resizeMatrix(weights, 0, 0);
				double element;
				std::vector<double> row;
				while (file >> element) 
				{  
					row.push_back(element);  
					if (row.size() == components["output"].size())
					{
						weights.push_back(row);  
						row.clear(); 
					}
				}
				file.close();
				const std::string message = "Weights '" + this->getUniqueName() + "' read successfully from: " +
					weightsFilePath + ".";
				log(tools::logger::LogLevel::INFO, message);

				readWeightsSize = std::make_tuple(static_cast<int>(weights.size()), 
					static_cast<int>(weights[0].size()));
				if(initialWeightsSize != readWeightsSize)
				{
					log(tools::logger::LogLevel::ERROR, "Weight matrix read from file has a different "
										 "dimensionality compared to the actual matrix size! ");
					return false;
				}
				return true;	
			}

			const std::string message = "Failed to read weights '" + this->getUniqueName() + "' from: " +
				weightsFilePath + ". ";
			log(tools::logger::LogLevel::ERROR, message);
			
			return false;
		}

		void FieldCoupling::writeWeights() const
		{
			std::ofstream file(weightsFilePath);

			if (file.is_open()) {
				for (const auto& row : weights) {
					for (const auto& element : row) {
						file << element << " ";  
					}
					file << '\n'; 
				}
				file.close();
				const std::string message = "Saved weights '" + this->getUniqueName() +"' to: " + weightsFilePath + ".";
				log(tools::logger::LogLevel::INFO, message);
			}
			else
			{
				const std::string message = "Failed to saved weights '" + this->getUniqueName() + "' to: " + 
					weightsFilePath + ". ";
				log(tools::logger::LogLevel::ERROR, message);
			}
		}

		void FieldCoupling::fillWeightsRandomly()
		{
			tools::utils::resizeMatrix(weights, static_cast<int>(components["input"].size()),
				static_cast<int>(components["output"].size()));
			tools::utils::fillMatrixWithRandomValues(weights, 0.0, 0.0);
			log(tools::logger::LogLevel::INFO, "Filling the weight matrix with random values.");
			trained = false;
			writeWeights();
		}

		void FieldCoupling::saveWeights() const
		{
			writeWeights();
		}

		void FieldCoupling::setWeightsFilePath(const std::string& filePath)
		{
			weightsFilePath = filePath + "/" + commonParameters.identifiers.uniqueName + "_weights.txt";
		}
	}
}
