#pragma once

#include "element.h"
#include "activation_function.h"
#include "simulation/simulation.h"
#include "elements/kernel.h"

namespace dnf_composer
{
	namespace element
	{
		struct NeuralFieldParameters : ElementSpecificParameters
		{
			double tau;
			double startingRestingLevel;
			std::unique_ptr<ActivationFunction> activationFunction;

			NeuralFieldParameters& operator=(const NeuralFieldParameters& other)
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

			bool operator==(const NeuralFieldParameters& other) const
			{
				constexpr double epsilon = 1e-6;
				return std::abs(tau - other.tau) < epsilon &&
					std::abs(startingRestingLevel - other.startingRestingLevel) < epsilon &&
					activationFunction == other.activationFunction;
			}

			NeuralFieldParameters()
				:tau(25.0), startingRestingLevel(-5.0), activationFunction(nullptr)
			{}

			NeuralFieldParameters(double tau, double restingLevel,
				const ActivationFunction& activationFunction)
				: tau(tau), startingRestingLevel(restingLevel),
				activationFunction(activationFunction.clone())
			{ }

			NeuralFieldParameters(const NeuralFieldParameters& other)
			{
				tau = other.tau;
				startingRestingLevel = other.startingRestingLevel;
				if (other.activationFunction == nullptr)
					activationFunction = std::make_unique<SigmoidFunction>(0.0, 10.0);
				else
					activationFunction = other.activationFunction->clone();
			}

			std::string toString() const override
			{
				std::ostringstream result;
				result << "Parameters: ["
					<< "Tau: " << std::fixed << std::setprecision(2) << tau << ", "
					<< "Resting level: " << std::fixed << std::setprecision(2) << startingRestingLevel << ", "
					<< "Activation Function: " << (activationFunction ? activationFunction->toString() : "None")
					<< "]";
				return result.str();
			}

		};

		struct NeuralFieldBump
		{
			double centroid;
			double startPosition;
			double endPosition;
			double amplitude;
			double width;
			double previousCentroid = 0.0;
			double velocity;
			double acceleration;

			NeuralFieldBump(double centroid = 0.0,
				double startPosition = 0.0,
				double endPosition = 0.0,
				double amplitude = 0.0,
				double width = 0.0,
				double previousCentroid = 0.0,
				double velocity = 0.0,
				double acceleration = 0.0)
				: centroid(centroid),
				startPosition(startPosition),
				endPosition(endPosition),
				amplitude(amplitude),
				width(width),
				previousCentroid(previousCentroid),
				velocity(velocity),
				acceleration(acceleration)
			{}

			std::string toString() const
			{
				std::string str = "Bump: [";
				str += "Centroid: " + std::format("{:.2f}", centroid) + ", ";
				str += "Amplitude: " + std::format("{:.2f}", amplitude) + ", ";
				str += "Width: " + std::format("{:.2f}", width) + ", ";
				str += "Start pos.: " + std::format("{:.2f}", startPosition) + ", ";
				str += "End pos.: " + std::format("{:.2f}", endPosition) + ", ";
				str += "Velocity: " + std::format("{:.2f}", velocity) + ", ";
				str += "Acceleration: " + std::format("{:.2f}", acceleration) + "]";
				return str;
			}

			void print() const;
			
		};

		struct NeuralFieldState
		{
			std::vector<NeuralFieldBump> bumps;
			bool stable;
			double lowestActivation;
			double highestActivation;
			double thresholdForStability;
			double previousActivationSum = 0.0;
			double previousActivationAvg = 0.0;
			double previousActivationNorm = 0.0;

			NeuralFieldState()
				:bumps({}), stable(false), lowestActivation(0.0),
				highestActivation(0.0), thresholdForStability(0.895)
			{}

			std::string toString() const
			{
				std::string str = "Neural field state [";
				str += "Stable: " + std::string(stable ? "true" : "false") + ", ";
				str += "Lowest act.: " + std::format("{:.2f}", lowestActivation) + ", ";
				str += "Highest act.: " + std::format("{:.2f}", highestActivation) + ", ";
				str += "Threshold: " + std::format("{:.2f}", thresholdForStability) + "]\n";
				str += "Bumps: {";
				for (const auto& bump : bumps)
					str += bump.toString();
				str += "}";

				return str;
			}

			void print() const;

		};

		class NeuralField : public Element
		{
		protected:
			NeuralFieldParameters parameters;
			NeuralFieldState state;
		public:
			NeuralField(const ElementCommonParameters& elementCommonParameters,
				const NeuralFieldParameters& parameters);

			void init() override;
			void step(double t, double deltaT) override;
			std::string toString() const override;
			std::shared_ptr<Element> clone() const override;

			void setThresholdForStability(double threshold) { state.thresholdForStability = threshold; }
			void setParameters(const NeuralFieldParameters& parameters);
			NeuralFieldParameters getParameters() const;
			//double getCentroid() const;
			bool isStable() const;
			double getLowestActivation() const { return state.lowestActivation; }
			double getHighestActivation() const { return state.highestActivation; }
			std::vector<NeuralFieldBump> getBumps() const { return state.bumps; }
			std::shared_ptr<Kernel> getSelfExcitationKernel() const;
			double getStabilityThreshold() const { return state.thresholdForStability; }
		protected:
			void calculateActivation(double t, double deltaT);
			void calculateOutput();
			//void calculateCentroid();
			void updateState(double deltaT);
			void checkStability();
			void updateMinMaxActivation();
			void updateBumps(double deltaT);
		};
	}
}