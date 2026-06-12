#include "element_parameters/neural_field_parameters.h"
#include <format>

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
			return std::format(
        "Neural field parameters\n"
        "Tau: {:.2f}\n"
        "Resting level: {:.2f}\n"
        "Activation function: {}\n",
        tau, startingRestingLevel, activationFunction->toString());
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
			return std::format(
        "Bump\n"
        "Centroid: {:.2f}\n"
        "Amplitude: {:.2f}\n"
        "Start position: {:.2f}\n"
        "End position: {:.2f}\n"
        "Width: {:.2f}\n",
        centroid, amplitude, startPosition, endPosition, width);
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
			std::string str = std::format(
				"Neural field state\n"
				"Stable: {}\n"
				"Lowest activation: {:.2f}\n"
				"Highest activation: {:.2f}\n"
				"Threshold for stability: {:.2f}\n"
				"Bumps:\n",
				stable, lowestActivation, highestActivation, thresholdForStability);
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