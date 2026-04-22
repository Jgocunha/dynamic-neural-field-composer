#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>

#include "element.h"
#include "activation_function.h"

namespace dnf_composer::element
{
	struct NeuralField2DParameters final : ElementSpecificParameters
	{
		double tau;
		double startingRestingLevel;
		std::unique_ptr<ActivationFunction> activationFunction;

		NeuralField2DParameters()
			: tau(25.0), startingRestingLevel(-5.0), activationFunction(nullptr)
		{}

		NeuralField2DParameters(double tau, double restingLevel, const ActivationFunction& af)
			: tau(tau), startingRestingLevel(restingLevel), activationFunction(af.clone())
		{}

		NeuralField2DParameters(const NeuralField2DParameters& other)
			: tau(other.tau), startingRestingLevel(other.startingRestingLevel),
			  activationFunction(other.activationFunction
				  ? other.activationFunction->clone()
				  : std::make_unique<SigmoidFunction>(0.0, 10.0))
		{}

		NeuralField2DParameters& operator=(const NeuralField2DParameters& other)
		{
			if (this != &other)
			{
				tau = other.tau;
				startingRestingLevel = other.startingRestingLevel;
				activationFunction = other.activationFunction
					? other.activationFunction->clone()
					: std::make_unique<SigmoidFunction>(0.0, 10.0);
			}
			return *this;
		}

		bool operator==(const NeuralField2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(tau - other.tau) < epsilon &&
			       std::abs(startingRestingLevel - other.startingRestingLevel) < epsilon;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << "Parameters: ["
				<< "Tau: " << std::fixed << std::setprecision(2) << tau << ", "
				<< "Resting level: " << startingRestingLevel << "]";
			return result.str();
		}
	};

	class NeuralField2D final : public Element
	{
	private:
		NeuralField2DParameters parameters;
	public:
		NeuralField2D(const ElementCommonParameters& elementCommonParameters,
		              const NeuralField2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const NeuralField2DParameters& parameters);
		NeuralField2DParameters getParameters() const;

	private:
		void calculateActivation(double deltaT);
		void calculateOutput();
	};
}
