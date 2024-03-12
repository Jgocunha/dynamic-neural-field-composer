// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/field_coupling.h"


namespace dnf_composer
{
	namespace element
	{

		FieldCoupling::FieldCoupling(const ElementCommonParameters& elementCommonParameters, const FieldCouplingParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			if (parameters.inputFieldSize <= 0)
				throw Exception(ErrorCode::ELEM_SIZE_NOT_ALLOWED, commonParameters.identifiers.uniqueName);

			commonParameters.identifiers.label = ElementLabel::FIELD_COUPLING;

			components["input"] = std::vector<double>(parameters.inputFieldSize);
			tools::utils::resizeMatrix(weights, static_cast<int>(components["input"].size()), static_cast<int>(components["output"].size()));

			// Initialize the weight matrix with random values
			tools::utils::fillMatrixWithRandomValues(weights, -1, 1);

			weightsFilePath = std::string(OUTPUT_DIRECTORY) + "/inter-field-synaptic-connections/" + commonParameters.identifiers.uniqueName + "_weights.txt";

			updateAllWeights = true;
			trained = false;
		}

		void FieldCoupling::init()
		{
			//std::ranges::fill(components["input"], 0);
			//std::ranges::fill(components["output"], 0);

			if (readWeights())
				trained = true;
			else
				fillWeightsRandomly();
		}

		void FieldCoupling::step(double t, double deltaT)
		{
			getInputFunction();
			computeOutput();
			scaleOutput();
		}

		void FieldCoupling::close()
		{
			//std::ranges::fill(components["input"], 0);
			//std::ranges::fill(components["output"], 0);
			resetWeights();
		}

		void FieldCoupling::printParameters()
		{
			printCommonParameters();

			std::ostringstream logStream;

			logStream << "Logging specific element parameters" << std::endl;
			logStream << "Input Field Size: " << parameters.inputFieldSize << std::endl;
			logStream << "Scalar: " << parameters.scalar << std::endl;
			logStream << "Learning Rate: " << parameters.learningRate << std::endl;
			logStream << "Learning Rule: ";
			switch (parameters.learningRule)
			{
			case LearningRule::HEBBIAN:
				logStream << "Hebbian learning rule" << std::endl;;
				break;
			case LearningRule::DELTA_WIDROW_HOFF:
				logStream << "Delta learning rule Widrow Hoff variation" << std::endl;;
				break;
			case LearningRule::DELTA_KROGH_HERTZ:
				logStream << "Delta learning rule Krogh and Hertz variation" << std::endl;
			case LearningRule::OJA:
				logStream << "Oja learning rule" << std::endl;
				break;
			}

			log(tools::logger::LogLevel::INFO, logStream.str());
		}

		std::shared_ptr<Element> FieldCoupling::clone() const
		{
			auto cloned = std::make_shared<FieldCoupling>(*this);
			// If there are deep copy specifics that the copy constructor doesn't handle, do them here.
			return cloned;
		}

		void FieldCoupling::getInputFunction()
		{
			// get input
			updateInput();
			// only the positive values of the input are considered
			for (auto& value : components["input"])
				if (value < 0)
					value = 0;
		}

		void FieldCoupling::computeOutput()
		{
			// multiply the input by the weights to get output
			for (int i = 0; i < components["output"].size(); i++)
				for (int j = 0; j < components["input"].size(); j++)
					components["output"][i] += weights[j][i] * components["input"][j];

			// only the positive values of the output are considered
			for (auto& value : components["output"])
				if (value < 0)
					value = 0;
		}

		void FieldCoupling::scaleOutput()
		{
			// Scale the output by parameter k
			for (auto& value : components["output"])
				value *= parameters.scalar;
		}

		void FieldCoupling::resetWeights()
		{
			// empty weight matrix
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
				weights = tools::math::deltaLearningRuleWidrowHoff(weights, input, output, parameters.learningRate);
				break;
			case LearningRule::DELTA_KROGH_HERTZ:
				weights = tools::math::deltaLearningRuleKroghHertz(weights, input, output, parameters.learningRate);
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

			const std::tuple<int, int> initialWeightsSize = std::make_tuple( static_cast<int>(weights.size()), static_cast<int>(weights[0].size()) );
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
				const std::string message = "Weights '" + this->getUniqueName() + "' read successfully from: " + weightsFilePath + ". \n";
				log(tools::logger::LogLevel::INFO, message);

				// Check if dimensions are correct.
				readWeightsSize = std::make_tuple(static_cast<int>(weights.size()), static_cast<int>(weights[0].size()));
				if(initialWeightsSize != readWeightsSize)
				{
					log(tools::logger::LogLevel::ERROR, "Weight matrix read from file has a different dimensionality compared to the actual matrix size! \n");
					return false;
				}
				return true;	
			}

			const std::string message = "Failed to read weights '" + this->getUniqueName() + "' from: " + weightsFilePath + ". \n";
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
				const std::string message = "Saved weights '" + this->getUniqueName() +"' to: " + weightsFilePath + ". \n";
				log(tools::logger::LogLevel::INFO, message);
			}
			else
			{
				const std::string message = "Failed to saved weights '" + this->getUniqueName() + "' to: " + weightsFilePath + ". \n";
				log(tools::logger::LogLevel::ERROR, message);
			}
		}

		void FieldCoupling::fillWeightsRandomly()
		{
			tools::utils::resizeMatrix(weights, static_cast<int>(components["input"].size()), static_cast<int>(components["output"].size()));
			tools::utils::fillMatrixWithRandomValues(weights, 0.0, 0.0);
			log(tools::logger::LogLevel::INFO, "Filling the weight matrix with random values. \n");
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
