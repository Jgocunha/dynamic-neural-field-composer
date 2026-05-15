#pragma once

#include <cmath>
#include <queue>
#include <sstream>
#include <iomanip>
#include <format>

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
			: tau(25.0), startingRestingLevel(-5.0),
			  activationFunction(std::make_unique<SigmoidFunction>(0.0, 10.0))
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
				<< "Resting level: " << startingRestingLevel << ", "
				<< "Activation Function: " << (activationFunction ? activationFunction->toString() : "None")
				<< "]";
			return result.str();
		}
	};

	struct NeuralField2DBump
	{
		double centroid_x = 0.0;
		double centroid_y = 0.0;
		double amplitude  = 0.0;
		double area       = 0.0;
		double velocity_x = 0.0;
		double velocity_y = 0.0;

		[[nodiscard]] std::string toString() const
		{
			std::string str = "Bump 2D: [";
			str += "Centroid: (" + std::format("{:.2f}", centroid_x) + ", " + std::format("{:.2f}", centroid_y) + "), ";
			str += "Amplitude: " + std::format("{:.2f}", amplitude) + ", ";
			str += "Area: " + std::format("{:.2f}", area) + ", ";
			str += "Velocity: (" + std::format("{:.2f}", velocity_x) + ", " + std::format("{:.2f}", velocity_y) + ")]";
			return str;
		}

		void print() const
		{
			dnf_composer::tools::logger::log(dnf_composer::tools::logger::LogLevel::INFO, toString());
		}
	};

	struct NeuralField2DState
	{
		std::vector<NeuralField2DBump> bumps;
		bool   stable                 = false;
		double lowestActivation       = 0.0;
		double highestActivation      = 0.0;
		double thresholdForStability  = 0.895;
		double previousActivationSum  = 0.0;
		double previousActivationAvg  = 0.0;
		double previousActivationNorm = 0.0;

		[[nodiscard]] std::string toString() const
		{
			std::string str = "Neural field 2D state [";
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

		void print() const
		{
			dnf_composer::tools::logger::log(dnf_composer::tools::logger::LogLevel::INFO, toString());
		}
	};

	class NeuralField2D final : public Element
	{
	private:
		NeuralField2DParameters        parameters;
		NeuralField2DState             state;
		double* act_  = nullptr;
		double* inp_  = nullptr;
		double* rest_ = nullptr;
		bool    computeStateMetrics_ = true;
		std::vector<NeuralField2DBump> prevBumps_;
	public:
		NeuralField2D(const ElementCommonParameters& elementCommonParameters,
		              const NeuralField2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const NeuralField2DParameters& parameters);
		NeuralField2DParameters getParameters() const;

		void   setThresholdForStability(double threshold) { state.thresholdForStability = threshold; }
		double getStabilityThreshold()  const { return state.thresholdForStability; }
		bool   isStable()               const { return state.stable; }
		double getLowestActivation()    const { return state.lowestActivation; }
		double getHighestActivation()   const { return state.highestActivation; }
		std::vector<NeuralField2DBump> getBumps() const { return state.bumps; }
		void   setComputeStateMetrics(bool enable) { computeStateMetrics_ = enable; }
		bool   getComputeStateMetrics() const { return computeStateMetrics_; }

	private:
		void calculateActivation(double t, double deltaT);
		void calculateOutput();
		void updateState(double deltaT);
		void updateBumps(double deltaT);
	};
}
