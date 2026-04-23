#pragma once

#include "element.h"
#include "activation_function.h"
#include "simulation/simulation.h"
#include "elements/kernel.h"

namespace dnf_composer::element
{
	/// @brief Parameters that govern a NeuralField's dynamics.
	/// @ingroup elements
	struct NeuralFieldParameters final : ElementSpecificParameters
	{
		double tau;                                        ///< Time constant of the field's relaxation dynamics.
		double startingRestingLevel;                       ///< Homogeneous resting level (h); sub-threshold when negative.
		std::unique_ptr<ActivationFunction> activationFunction; ///< Nonlinearity applied to activation to produce output.

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

		/// @brief Default constructor: tau=25, restingLevel=-5, sigmoid(0, 10).
		NeuralFieldParameters()
			:tau(25.0), startingRestingLevel(-5.0), activationFunction(nullptr)
		{}

		/// @brief Construct with explicit tau, resting level, and activation function.
		/// @param tau             Time constant in ms.
		/// @param restingLevel    Homogeneous resting level h.
		/// @param activationFunction  Pointwise nonlinearity.
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

		[[nodiscard]] std::string toString() const override
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

	/// @brief Describes a single activation bump (peak) in a neural field.
	/// @ingroup elements
	struct NeuralFieldBump
	{
		double centroid;          ///< Spatial position of the bump's centre of mass.
		double startPosition;     ///< Left edge of the above-threshold region.
		double endPosition;       ///< Right edge of the above-threshold region.
		double amplitude;         ///< Peak activation value.
		double width;             ///< Width of the above-threshold region.
		double previousCentroid = 0.0; ///< Centroid at the previous time step.
		double velocity;          ///< Rate of change of the centroid (positions/step).
		double acceleration;      ///< Rate of change of velocity (positions/step²).

		explicit NeuralFieldBump(const double centroid = 0.0,
		                const double startPosition = 0.0,
		                const double endPosition = 0.0,
		                const double amplitude = 0.0,
		                const double width = 0.0,
		                const double previousCentroid = 0.0,
		                const double velocity = 0.0,
		                const double acceleration = 0.0)
			: centroid(centroid),
			  startPosition(startPosition),
			  endPosition(endPosition),
			  amplitude(amplitude),
			  width(width),
			  previousCentroid(previousCentroid),
			  velocity(velocity),
			  acceleration(acceleration)
		{}

		[[nodiscard]] std::string toString() const
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

	/// @brief Snapshot of a neural field's observable state.
	/// @ingroup elements
	struct NeuralFieldState
	{
		std::vector<NeuralFieldBump> bumps; ///< Currently active above-threshold peaks.
		bool stable;                        ///< True when the activation change falls below the stability threshold.
		double lowestActivation;            ///< Minimum activation across the field.
		double highestActivation;           ///< Maximum activation across the field.
		double thresholdForStability;       ///< Convergence criterion (default 0.895).
		double previousActivationSum = 0.0;
		double previousActivationAvg = 0.0;
		double previousActivationNorm = 0.0;

		NeuralFieldState()
			:bumps({}), stable(false), lowestActivation(0.0),
			 highestActivation(0.0), thresholdForStability(0.895)
		{}

		[[nodiscard]] std::string toString() const
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

	/// @brief Continuous attractor neural field — the core DFT building block.
	///
	/// Implements a one-dimensional neural field with Amari-type dynamics:
	/// @code
	///   tau * du/dt = -u + h + s(x, t) + (w * f(u))(x)
	/// @endcode
	/// where @c u is activation, @c h the resting level, @c s the summed external
	/// input, and @c w the lateral interaction kernel convolved with the output
	/// @c f(u) (determined by the activation function).
	///
	/// @ingroup elements
	class NeuralField final : public Element
	{
	protected:
		NeuralFieldParameters parameters; ///< Dynamics parameters (tau, h, activation function).
		NeuralFieldState state;           ///< Runtime state (bumps, stability, min/max).
	public:
		/// @brief Construct a neural field.
		/// @param elementCommonParameters  Name, label, and spatial dimensions.
		/// @param parameters               Field dynamics parameters.
		NeuralField(const ElementCommonParameters& elementCommonParameters,
		            const NeuralFieldParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		/// @brief Set the stability convergence threshold.
		/// @param threshold  New threshold value (default 0.895).
		void setThresholdForStability(const double threshold) { state.thresholdForStability = threshold; }

		void setParameters(const NeuralFieldParameters& parameters);
		NeuralFieldParameters getParameters() const;
		bool isStable() const;

		double getLowestActivation() const { return state.lowestActivation; }
		double getHighestActivation() const { return state.highestActivation; }

		/// @brief Return all currently detected above-threshold bumps.
		std::vector<NeuralFieldBump> getBumps() const { return state.bumps; }

		/// @brief Return the registered self-excitation kernel, if any.
		std::shared_ptr<Kernel> getSelfExcitationKernel() const;

		double getStabilityThreshold() const { return state.thresholdForStability; }
	protected:
		void calculateActivation(double t, double deltaT);
		void calculateOutput();
		void updateState(double deltaT);
		void checkStability();
		void updateMinMaxActivation();
		void updateBumps(double deltaT);
	};
}
