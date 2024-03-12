// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/neural_field.h"

namespace dnf_composer
{
	namespace element
	{
		NeuralField::NeuralField(const ElementCommonParameters& elementCommonParameters, const NeuralFieldParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::NEURAL_FIELD;
			components["activation"] = std::vector<double>(commonParameters.dimensionParameters.size);
			components["resting level"] = std::vector<double>(commonParameters.dimensionParameters.size);
		}

		void NeuralField::init()
		{
			//std::ranges::fill(components["activation"], parameters.startingRestingLevel);
			//std::ranges::fill(components["input"], 0.0);
			//std::ranges::fill(components["resting level"], parameters.startingRestingLevel);
			calculateOutput();
		}

		void NeuralField::step(double t, double deltaT)
		{
			updateInput();
			calculateActivation(t, deltaT);
			calculateOutput();
			calculateCentroid();
			checkStability();
		}

		void NeuralField::close()
		{
		}

		void NeuralField::setParameters(const NeuralFieldParameters& neuralFieldParameters)
		{
			parameters = neuralFieldParameters;
			updateParameters();
		}

		NeuralFieldParameters NeuralField::getParameters() const
		{
			return parameters;
		}

		double NeuralField::getCentroid() const
		{
			return state.centroid;
		}

		bool NeuralField::isStable() const
		{
			if (state.stable)
				return true;
			return false;
		}

		void NeuralField::printParameters()
		{
			printCommonParameters();

			std::ostringstream logStream;

			logStream << "Logging specific element parameters" << std::endl;
			logStream << "StartingRestingLevel: " << parameters.startingRestingLevel << std::endl;
			logStream << "Tau: " << parameters.tau << std::endl;

			//logStream << "ActivationFunctionParameters: ";
			////logStream << "Type: " << ActivationFunctionTypeToString.at(parameters.activationFunctionParameters.type) << " | ";
			//logStream << "Steepness: " << parameters.activationFunction. << " | ";
			//logStream << "XShift: " << parameters.activationFunctionParameters.xShift << " | ";

			logStream << std::endl << "Current centroid value: " << getCentroid() << std::endl;

			log(tools::logger::LogLevel::INFO, logStream.str());
		}

		std::shared_ptr<Element> NeuralField::clone() const
		{
			auto cloned = std::make_shared<NeuralField>(*this);
			// If there are deep copy specifics that the copy constructor doesn't handle, do them here.
			return cloned;
		}

		void NeuralField::calculateActivation(double t, double deltaT)
		{
			for (int i = 0; i < commonParameters.dimensionParameters.size; i++)
			{
				components["activation"][i] = components["activation"][i] + deltaT / parameters.tau *
					(-components["activation"][i] + components["resting level"][i] + components["input"][i]);
			}
		}

		void NeuralField::calculateOutput()
		{
			components["output"] = parameters.activationFunction->operator()(components["activation"]);
		}

		void NeuralField::calculateCentroid()
		{
			const std::vector<double> f_output = tools::math::heaviside(components["activation"], 0.1);

			// if (*std::ranges::max_element(f_output) > 0)
			// {
			// 	const bool isAtLimits = (f_output[0] > 0) || (f_output[commonParameters.dimensionParameters.size - 1] > 0);

			// 	double sumActivation = 0.0;
			// 	double sumWeightedPositions = 0.0;

			// 	for (int i = 0; i < commonParameters.dimensionParameters.size; i++)
			// 	{
			// 		const double activation = f_output[i];

			// 		sumActivation += activation;

			// 		// Calculate the circular distance from the midpoint to the position
			// 		double distance = 0.0;
			// 		if (isAtLimits)
			// 			distance = fmod(static_cast<double>(i) - static_cast<double>(commonParameters.dimensionParameters.size) * 0.5 
			// 				+ static_cast<double>(commonParameters.dimensionParameters.size) * 10, static_cast<double>(commonParameters.dimensionParameters.size));
			// 		else
			// 			distance = fmod(static_cast<double>(i) - static_cast<double>(commonParameters.dimensionParameters.size) * 0.5, static_cast<double>(commonParameters.dimensionParameters.size));
			// 		sumWeightedPositions += distance * activation;
			// 	}

			// 	static constexpr double epsilon = 1e-6;
			// 	if (std::fabs(sumActivation) > epsilon)
			// 	{
			// 		// Shift the centroid back to the circular field
			// 		state.centroid = fmod(static_cast<double>(commonParameters.dimensionParameters.size) * 0.5 + sumWeightedPositions / sumActivation, static_cast<double>(commonParameters.dimensionParameters.size));
			// 		if (isAtLimits)
			// 			state.centroid = (state.centroid >= 0 ? state.centroid : state.centroid + static_cast<double>(commonParameters.dimensionParameters.size));
			// 	}
			// 	state.centroid = state.centroid * commonParameters.dimensionParameters.d_x + commonParameters.dimensionParameters.d_x;
			// }
			// else
			// {
			// 	state.centroid = -1.0;
			// 	return;
			// }
		}

		void NeuralField::checkStability()
		{
			static constexpr double threshold = 0.15; //ideally this threshold should be a calculation?
			const double currentActivationSum = tools::math::calculateVectorSum(components["activation"]);
			const double currentActivationAvg = tools::math::calculateVectorAvg(components["activation"]);
			const double currentActivationNorm = tools::math::calculateVectorNorm(components["activation"]);

			// this function is done like this, instead of comparing to a previously saved vector of activation,
			// because it is simply faster and takes up less memory.
			if (std::abs(currentActivationSum - state.prevActivationSum) < threshold)
			{
				if (std::abs(currentActivationAvg - state.prevActivationAvg) < threshold)
				{
					if(std::abs(currentActivationNorm - state.prevActivationNorm) < threshold)
					{
						state.prevActivationSum = currentActivationSum;
						state.prevActivationAvg = currentActivationAvg;
						state.prevActivationNorm = currentActivationNorm;
						state.stable = true;
						return;
					}
				}
			}

			state.prevActivationSum = currentActivationSum;
			state.prevActivationAvg = currentActivationAvg;
			state.prevActivationNorm = currentActivationNorm;
			state.stable = false;
		}

		void NeuralField::updateParameters()
		{
			//std::ranges::fill(components["resting level"], parameters.startingRestingLevel);
			calculateOutput();
		}
	}
}