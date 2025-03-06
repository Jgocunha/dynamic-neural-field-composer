// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/neural_field.h"


namespace dnf_composer
{
	namespace element
	{
		void NeuralFieldBump::print() const
		{
			const std::string str = toString();
			dnf_composer::tools::logger::log(dnf_composer::tools::logger::LogLevel::INFO, str);
		}

		void NeuralFieldState::print() const
		{
			const std::string str = toString();
			dnf_composer::tools::logger::log(dnf_composer::tools::logger::LogLevel::INFO, str);
		}
		

		NeuralField::NeuralField(const ElementCommonParameters& elementCommonParameters, 
			const NeuralFieldParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::NEURAL_FIELD;
			components["activation"] = std::vector<double>(commonParameters.dimensionParameters.size);
			components["resting level"] = std::vector<double>(commonParameters.dimensionParameters.size);
		}

		void NeuralField::init()
		{
			std::ranges::fill(components["activation"], parameters.startingRestingLevel);
			std::ranges::fill(components["input"], 0.0);
			std::ranges::fill(components["output"], 0.0);
			std::ranges::fill(components["resting level"], parameters.startingRestingLevel);
			calculateOutput();
		}

		void NeuralField::step(double t, double deltaT)
		{
			updateInput();
			calculateActivation(t, deltaT);
			calculateOutput();
			updateState(deltaT);
		}

		void NeuralField::setParameters(const NeuralFieldParameters& neuralFieldParameters)
		{
			parameters = neuralFieldParameters;
			init();
		}

		NeuralFieldParameters NeuralField::getParameters() const
		{
			return parameters;
		}

		/*double NeuralField::getCentroid() const
		{
			return state.centroid;
		}*/

		bool NeuralField::isStable() const
		{
			if (state.stable)
				return true;
			return false;
		}

		/*std::shared_ptr<Kernel> NeuralField::getSelfExcitationKernel() const
		{
			for (const auto& input : inputs)
			{
				if (input.first->getLabel() == ElementLabel::GAUSS_KERNEL ||
					input.first->getLabel() == ElementLabel::MEXICAN_HAT_KERNEL)
				{
					auto kernel = std::dynamic_pointer_cast<Kernel>(input.first);
					for (const auto& kernelInput : kernel->getInputs())
					{
						if (kernelInput->getUniqueName() == commonParameters.identifiers.uniqueName)
							return kernel;
					}
				}
			}
			return nullptr;
		}*/

		std::string NeuralField::toString() const
		{
			std::string result = "Neural field element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString() + '\n';
			result += state.toString();
			return result;
		}

		std::shared_ptr<Element> NeuralField::clone() const
		{
			auto cloned = std::make_shared<NeuralField>(*this);
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

		//void NeuralField::calculateCentroid()
		//{
		//	const std::vector<double> f_output = tools::math::heaviside(components["activation"], 0.1);

		//	if (*std::ranges::max_element(f_output) > 0)
		//	{
		//		const bool isAtLimits = (f_output[0] > 0) || (f_output[commonParameters.dimensionParameters.size - 1] > 0);

		//		double sumActivation = 0.0;
		//		double sumWeightedPositions = 0.0;

		//		for (int i = 0; i < commonParameters.dimensionParameters.size; i++)
		//		{
		//			const double activation = f_output[i];

		//			sumActivation += activation;

		//			// Calculate the circular distance from the midpoint to the position
		//			double distance = 0.0;
		//			if (isAtLimits)
		//				distance = fmod(static_cast<double>(i) - static_cast<double>(commonParameters.dimensionParameters.size) * 0.5 
		//					+ static_cast<double>(commonParameters.dimensionParameters.size) * 10, static_cast<double>(commonParameters.dimensionParameters.size));
		//			else
		//				distance = fmod(static_cast<double>(i) - static_cast<double>(commonParameters.dimensionParameters.size) * 0.5, static_cast<double>(commonParameters.dimensionParameters.size));
		//			sumWeightedPositions += distance * activation;
		//		}

		//		static constexpr double epsilon = 1e-6;
		//		if (std::fabs(sumActivation) > epsilon)
		//		{
		//			// Shift the centroid back to the circular field
		//			state.centroid = fmod(static_cast<double>(commonParameters.dimensionParameters.size) * 0.5 + sumWeightedPositions / sumActivation, static_cast<double>(commonParameters.dimensionParameters.size));
		//			if (isAtLimits)
		//				state.centroid = (state.centroid >= 0 ? state.centroid : state.centroid + static_cast<double>(commonParameters.dimensionParameters.size));
		//		}
		//		state.centroid = state.centroid * commonParameters.dimensionParameters.d_x + commonParameters.dimensionParameters.d_x;
		//	}
		//	else
		//	{
		//		state.centroid = -1.0;
		//	}
		//}

		void NeuralField::updateState(double deltaT)
		{
			//calculateCentroid();
			updateMinMaxActivation();
			updateBumps(deltaT);
			checkStability();
		}

		void NeuralField::checkStability()
		{
			const double currentActivationSum = tools::math::calculateVectorSum(components["activation"]);
			const double currentActivationAvg = tools::math::calculateVectorAvg(components["activation"]);
			const double currentActivationNorm = tools::math::calculateVectorNorm(components["activation"]);

			// this function is done like this, instead of comparing to a previously saved vector of activation,
			// because it is simply faster and takes up less memory.
			if (std::abs(currentActivationSum - state.previousActivationSum) < state.thresholdForStability)
			{
				if (std::abs(currentActivationAvg - state.previousActivationAvg) < state.thresholdForStability)
				{
					if(std::abs(currentActivationNorm - state.previousActivationNorm) < state.thresholdForStability)
					{
						state.previousActivationSum = currentActivationSum;
						state.previousActivationAvg = currentActivationAvg;
						state.previousActivationNorm = currentActivationNorm;
						state.stable = true;
						return;
					}
				}
			}

			state.previousActivationSum = currentActivationSum;
			state.previousActivationAvg = currentActivationAvg;
			state.previousActivationNorm = currentActivationNorm;
			state.stable = false;

			// also valid and simpler approach
			/*static double previousHighestActivation = parameters.startingRestingLevel;
			if (std::fabs(state.highestActivation - previousHighestActivation) < state.thresholdForStability)
				state.stable = true;
			else
				state.stable = false;
			previousHighestActivation = state.highestActivation;*/
		}

		void NeuralField::updateMinMaxActivation()
		{
			if (components["activation"].empty())
				return;
			state.lowestActivation = *std::ranges::min_element(components["activation"]);
			state.highestActivation = *std::ranges::max_element(components["activation"]);
		}

		void NeuralField::updateBumps(double deltaT)
		{
			const auto oldBumps = state.bumps;
			state.bumps.clear();

			constexpr double activationThreshold = 0.00001; // Define a threshold for what counts as a 'bump'
			bool inBump = false;
			NeuralFieldBump currentBump(0, 0, 0, 0, 0);

			for (int i = 0; i < commonParameters.dimensionParameters.size; ++i)
			{
				double activation = components["activation"][i];
				if (activation > activationThreshold && !inBump)
				{
					// Start of a new bump
					inBump = true;

					currentBump = NeuralFieldBump(); // Reset the bump !

					currentBump.startPosition = (i + 1) * commonParameters.dimensionParameters.d_x;
					currentBump.amplitude = activation;
					currentBump.width = 1;
				}
				else if (activation > activationThreshold && inBump)
				{
					// Continue the current bump
					currentBump.amplitude = std::max(currentBump.amplitude, activation);
					currentBump.width++;
				}
				else if (inBump)
				{
					// End of current bump
					currentBump.width -= 1;
					currentBump.width *= commonParameters.dimensionParameters.d_x;
					currentBump.endPosition = i * commonParameters.dimensionParameters.d_x;
					currentBump.centroid = ((currentBump.startPosition + currentBump.endPosition) / 2);

					// in state.bumps find a bump with the same centroid
					static constexpr double epsilon = 2.0;//1e-6;
					const auto it = std::find_if(oldBumps.begin(), oldBumps.end(),
						[&currentBump](const NeuralFieldBump& bump) {
							return std::abs(bump.centroid - currentBump.centroid) < epsilon;
						});
					if (it != oldBumps.end()) 
					{
						// if the bump is found, update it
						currentBump.velocity = (currentBump.centroid - it->centroid) / deltaT;
						currentBump.acceleration = (currentBump.velocity - it->velocity) / deltaT;
					}
					else
					{
						// if the bump is not found, set the velocity and acceleration to zero
						currentBump.velocity = 0.0;
						currentBump.acceleration = 0.0;
					}

					state.bumps.push_back(currentBump);
					inBump = false;
				}
			}

			// If the last element ended in a bump
			if (inBump)
				state.bumps.push_back(currentBump);

			// Check if the first and last bumps are connected (wrap-around)
			if (!state.bumps.empty() && components["activation"].front() > 
				activationThreshold && components["activation"].back() > activationThreshold)
			{
				// Get the first and the last bump
				const auto& firstBump = state.bumps.front();
				const auto& lastBump = state.bumps.back();

				// Only merge if they are different bumps
				if (&firstBump != &lastBump)
				{
					// Create a new bump that combines the properties of the first and last bump
					NeuralFieldBump newBump;
					newBump.startPosition = lastBump.startPosition;
					newBump.endPosition = firstBump.endPosition;
					newBump.amplitude = std::max(firstBump.amplitude, lastBump.amplitude);
					newBump.width = commonParameters.dimensionParameters.x_max - 
						(newBump.startPosition - newBump.endPosition);
					newBump.centroid = fmod(
						((newBump.startPosition + newBump.endPosition + 
							commonParameters.dimensionParameters.x_max) / 2.0), 
							commonParameters.dimensionParameters.x_max);

					// in state.bumps find a bump with the same centroid
					static constexpr double epsilon = 2.0;// 1e-6;
					const auto it = std::find_if(oldBumps.begin(), oldBumps.end(),
						[&newBump](const NeuralFieldBump& bump) {
							return std::abs(bump.centroid - newBump.centroid) < epsilon;
						});
					if (it != oldBumps.end()) {
						// if the bump is found, update it
						newBump.velocity = (newBump.centroid - it->centroid) / deltaT;
						newBump.acceleration = (newBump.velocity - it->velocity) / deltaT;
					}
					else
					{
						// if the bump is not found, set the velocity and acceleration to zero
						newBump.velocity = 0.0;
						newBump.acceleration = 0.0;
					}

					// Remove the first and last bump
					state.bumps.pop_back(); // remove last
					state.bumps.erase(state.bumps.begin()); // remove first

					// Add the new merged bump
					state.bumps.push_back(newBump);
				}
			}
		}
	}
}
