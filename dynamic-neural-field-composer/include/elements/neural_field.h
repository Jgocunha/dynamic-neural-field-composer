#pragma once

#include "element.h"
#include "activation_function.h"
#include "simulation/simulation.h"
#include "elements/kernel.h"

namespace dnf_composer::element
{
	/// @brief Parameters that govern a NeuralField's dynamics.
	///
	/// ### Copy / move semantics (rule of five)
	/// `activationFunction` is a `unique_ptr<ActivationFunction>`, so this type owns a
	/// polymorphic value and cannot rely on compiler-generated copy/move:
	///  - **Copy** (constructor and assignment) deep-clones the source's activation
	///    function via `ActivationFunction::clone()`. **Null policy:** if the source's
	///    `activationFunction` is null, the copy materializes a default
	///    `SigmoidFunction(0, 10)` rather than propagating null. This is required for
	///    correctness, not just consistency: `NeuralField::calculateOutput()`
	///    dereferences `parameters.activationFunction` unconditionally (no null check),
	///    and `ElementFactory::createElement(NEURAL_FIELD)` constructs a default,
	///    null-activation `NeuralFieldParameters` that reaches a `NeuralField` only
	///    through this copy path -- so "null copies to null" would leave a
	///    `NeuralField` with a dangling dereference on the very next `init()`/`step()`.
	///    Copy constructor and copy assignment intentionally agree on this policy
	///    (previously they did not: the constructor substituted a default sigmoid while
	///    assignment reset to null -- see issue #119).
	///  - **Move** (constructor and assignment) transfers ownership of the source's
	///    `activationFunction` pointer directly -- no clone, no allocation. The
	///    moved-from source is left with `activationFunction == nullptr`, the same
	///    valid "unconfigured" state as a default-constructed instance.
	/// @ingroup elements
	struct NeuralFieldParameters final : ElementSpecificParameters
	{
		double tau;                                             ///< Time constant of the field's relaxation dynamics.
		double startingRestingLevel;                            ///< Homogeneous resting level (h); sub-threshold when negative.
		std::unique_ptr<ActivationFunction> activationFunction; ///< Nonlinearity applied to activation to produce output.

		/// @brief Default constructor: tau=25, restingLevel=-5, sigmoid(0, 10).
		NeuralFieldParameters()
			: tau(25.0), startingRestingLevel(-5.0), activationFunction(nullptr)
		{}

		/// @brief Construct with explicit tau, resting level, and activation function.
		/// @param tau                 Time constant in ms.
		/// @param restingLevel        Homogeneous resting level h.
		/// @param activationFunction  Pointwise nonlinearity.
		NeuralFieldParameters(double tau, double restingLevel,
		                      const ActivationFunction& activationFunction)
			: tau(tau), startingRestingLevel(restingLevel),
			  activationFunction(activationFunction.clone())
		{ }

		/// @brief Copy constructor. Deep-clones @p other's activation function.
		/// See the class-level doc comment for the null-source policy (materializes
		/// a default `SigmoidFunction(0, 10)`, matching copy assignment).
		NeuralFieldParameters(const NeuralFieldParameters& other)
			: tau(other.tau), startingRestingLevel(other.startingRestingLevel),
			  activationFunction(other.activationFunction
				  ? other.activationFunction->clone()
				  : std::make_unique<SigmoidFunction>(0.0, 10.0))
		{}

		/// @brief Copy assignment. Deep-clones @p other's activation function.
		/// Same null-source policy as the copy constructor -- see the class-level
		/// doc comment.
		NeuralFieldParameters& operator=(const NeuralFieldParameters& other)
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

		/// @brief Move constructor. Transfers @p other's activation function
		/// (no clone). @p other is left with `activationFunction == nullptr`.
		NeuralFieldParameters(NeuralFieldParameters&& other) noexcept = default;

		/// @brief Move assignment. Transfers @p other's activation function
		/// (no clone). @p other is left with `activationFunction == nullptr`.
		NeuralFieldParameters& operator=(NeuralFieldParameters&& other) noexcept = default;

		~NeuralFieldParameters() override = default;

		/// @brief Value equality.
		///
		/// Compares @c tau and @c startingRestingLevel within an epsilon tolerance, and
		/// compares @c activationFunction *by value* (concrete type and parameters) --
		/// never by pointer identity. Two null activation functions compare equal; a
		/// null and a non-null one never do; two non-null functions of different
		/// concrete types never compare equal even when their numeric fields coincide
		/// (e.g. a `SigmoidFunction(0, 10)` and an `AbsSigmoidFunction(0, 10)`).
		bool operator==(const NeuralFieldParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			if (std::abs(tau - other.tau) >= epsilon ||
				std::abs(startingRestingLevel - other.startingRestingLevel) >= epsilon)
			{
				return false;
			}
			return activationFunctionsEqual(activationFunction, other.activationFunction);
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

	private:
		/// @brief Value-compare two possibly-null activation functions.
		///
		/// Both null compares equal; exactly one null never compares equal. When both
		/// are non-null, the concrete type is checked (via `dynamic_cast`, mirroring the
		/// dispatch pattern already used in `simulation_file_manager.cpp`) before
		/// delegating to that type's own value `operator==` -- this guards against two
		/// different activation-function types with numerically-coincidental fields
		/// wrongly comparing equal.
		[[nodiscard]] static bool activationFunctionsEqual(
			const std::unique_ptr<ActivationFunction>& lhs,
			const std::unique_ptr<ActivationFunction>& rhs)
		{
			if (!lhs || !rhs) {
				return !lhs && !rhs;
			}
			if (lhs->type != rhs->type) {
				return false;
			}
			switch (lhs->type)
			{
			case ActivationFunctionType::SIGMOID:
			{
				const auto* l = dynamic_cast<const SigmoidFunction*>(lhs.get());
				const auto* r = dynamic_cast<const SigmoidFunction*>(rhs.get());
				return l != nullptr && r != nullptr && (*l == *r);
			}
			case ActivationFunctionType::HEAVISIDE:
			{
				const auto* l = dynamic_cast<const HeavisideFunction*>(lhs.get());
				const auto* r = dynamic_cast<const HeavisideFunction*>(rhs.get());
				return l != nullptr && r != nullptr && (*l == *r);
			}
			case ActivationFunctionType::ABSSIGMOID:
			{
				const auto* l = dynamic_cast<const AbsSigmoidFunction*>(lhs.get());
				const auto* r = dynamic_cast<const AbsSigmoidFunction*>(rhs.get());
				return l != nullptr && r != nullptr && (*l == *r);
			}
			default:
				return false;
			}
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
		bool stable{false};                        ///< True when the activation change falls below the stability threshold.
		double lowestActivation{0.0};            ///< Minimum activation across the field.
		double highestActivation{0.0};           ///< Maximum activation across the field.
		double thresholdForStability{0.895};       ///< Convergence criterion (default 0.895).
		double previousActivationSum  = 0.0; ///< Activation sum at the previous step — used to detect convergence.
		double previousActivationAvg  = 0.0; ///< Activation average at the previous step — used to detect convergence.
		double previousActivationNorm = 0.0; ///< L2 norm of activation at the previous step — used to detect convergence.

		NeuralFieldState()
			:bumps({}) 
		{}

		[[nodiscard]] std::string toString() const
		{
			std::string str = "Neural field state [";
			str += "Stable: " + std::string(stable ? "true" : "false") + ", ";
			str += "Lowest act.: " + std::format("{:.2f}", lowestActivation) + ", ";
			str += "Highest act.: " + std::format("{:.2f}", highestActivation) + ", ";
			str += "Threshold: " + std::format("{:.2f}", thresholdForStability) + "]\n";
			str += "Bumps: {";
			for (const auto& bump : bumps) {
				str += bump.toString();
}
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
	private:
		// Cached raw pointers into component vectors — valid between init() calls, never resized during step().
		double* act_  = nullptr; ///< components["activation"].data()
		double* inp_  = nullptr; ///< components["input"].data()
		// The resting level is homogeneous by construction (init() fills
		// components["resting level"] uniformly with startingRestingLevel, and
		// nothing ever writes to it per-cell afterward), so the integration loop
		// reads it as a cached scalar instead of a third N-double array stream.
		// The component vector itself is kept (and still filled in init()) so
		// external readers/serializers see the expected per-cell representation.
		double restScalar_ = 0.0;

		bool computeStateMetrics_ = true; ///< When false, skip stability/bump/min-max updates.
		std::vector<NeuralFieldBump> prevBumps_; ///< Scratch buffer for updateBumps — avoids per-step allocation.
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

		/// @brief Enable or disable per-step state-metric computation (stability, bumps, min/max).
		/// The default (enabled) path uses a single fused O(N) pass and is fast enough for
		/// normal use. Disable only as an advanced micro-optimisation for headless batch runs
		/// where bump data and stability checks are never needed.
		/// Default: true (full state tracking enabled).
		void setComputeStateMetrics(bool enable) { computeStateMetrics_ = enable; }
		bool getComputeStateMetrics() const { return computeStateMetrics_; }
	protected:
		void calculateActivation(double t, double deltaT);
		void calculateOutput();
		void updateState(double deltaT);
		void updateBumps(double deltaT);
	};
}
