#include "element_parameters/neural_field_parameters.h"

namespace dnf_composer
{
	namespace element
	{
		NeuralFieldParameters::NeuralFieldParameters()
			:tau(0.0), startingRestingLevel(0.0), activationFunction(nullptr)
		{}

		NeuralFieldParameters::NeuralFieldParameters(double tau, double restingLevel,
						const ActivationFunction& activationFunction)
			: tau(tau), startingRestingLevel(restingLevel), activationFunction(activationFunction.clone())
		{}

		NeuralFieldParameters::NeuralFieldParameters(const NeuralFieldParameters& other)
		{
			tau = other.tau;
			startingRestingLevel = other.startingRestingLevel;
			if (other.activationFunction == nullptr)
				activationFunction = std::make_unique<HeavisideFunction>(0);
			else
				activationFunction = other.activationFunction->clone();
		}

		NeuralFieldParameters& NeuralFieldParameters::operator=(const NeuralFieldParameters& other)
		{
			if (this != &other)
			{
				tau = other.tau;
				startingRestingLevel = other.startingRestingLevel;
				if (other.activationFunction)
					activationFunction = other.activationFunction->clone();
				else
					activationFunction.reset();
			}
			return *this;
		}

		bool NeuralFieldParameters::operator==(const NeuralFieldParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(tau - other.tau) < epsilon &&
				std::abs(startingRestingLevel - other.startingRestingLevel) < epsilon &&
				activationFunction == other.activationFunction;
		}

		std::string NeuralFieldParameters::toString() const
		{
			std::string result = "Neural field parameters\n";
			result += "Tau: " + std::to_string(tau) + "\n";
			result += "Resting level: " + std::to_string(startingRestingLevel) + "\n";
			result += "Activation function: " + activationFunction->toString() + "\n";
			return result;
		}

		NeuralFieldBump::NeuralFieldBump(double centroid, double startPosition, double endPosition,
			double amplitude, double width)
			: centroid(centroid), startPosition(startPosition), endPosition(endPosition),
				amplitude(amplitude), width(width)
		{}

		bool NeuralFieldBump::operator==(const NeuralFieldBump& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(centroid - other.centroid) < epsilon &&
				std::abs(startPosition - other.startPosition) < epsilon &&
				std::abs(endPosition - other.endPosition) < epsilon &&
				std::abs(amplitude - other.amplitude) < epsilon &&
				std::abs(width - other.width) < epsilon;
		}	

		std::string NeuralFieldBump::toString() const
		{
			std::string str = "Bump\n";
			str += "Centroid: " + std::to_string(centroid) + "\n";
			str += "Amplitude: " + std::to_string(amplitude) + "\n";
			str += "Start position: " + std::to_string(startPosition) + "\n";
			str += "End position: " + std::to_string(endPosition) + "\n";
			str += "Width: " + std::to_string(width) + "\n";
			return str;
		}

		void NeuralFieldBump::print() const
		{
			const std::string str = toString();
			tools::logger::log(tools::logger::LogLevel::INFO, str);
		}


		NeuralFieldState::NeuralFieldState()
			: bumps({}), stable(false), lowestActivation(0.0), highestActivation(0.0),
				thresholdForStability(0.035)
		{}

		std::string NeuralFieldState::toString() const
		{
			std::string str = "Neural field state\n";
			str += "Stable: " + std::to_string(stable) + "\n";
			str += "Lowest activation: " + std::to_string(lowestActivation) + "\n";
			str += "Highest activation: " + std::to_string(highestActivation) + "\n";
			str += "Threshold for stability: " + std::to_string(thresholdForStability) + "\n";
			str += "Bumps:\n";
			for (const auto& bump : bumps)
				str += bump.toString();
			return str;
		}

		void NeuralFieldState::print() const
		{
			const std::string str = toString();
			tools::logger::log(tools::logger::LogLevel::INFO, str);
		}

	}
}