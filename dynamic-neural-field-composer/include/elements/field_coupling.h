#pragma once

#include <set>
#include <utility>

#include "tools/math.h"
#include "element.h"
#include "neural_field.h"
#include "tools/utils.h"


namespace dnf_composer
{
	/// @brief Selects the synaptic weight update rule used by FieldCoupling.
	/// @ingroup elements
	enum class LearningRule : int
	{
		HEBB,  ///< Classic Hebbian: Δw ∝ pre × post.
		OJA,   ///< Oja's rule: Hebbian with weight-decay for stability.
		DELTA  ///< Supervised Widrow-Hoff/delta rule. Requires a target field connected
		       ///< to the coupling's Target pin; learning is disabled without one.
	};

	/// @brief Maps LearningRule values to human-readable strings.
	inline const std::map<LearningRule, std::string> LearningRuleToString = {
		{LearningRule::HEBB, "Hebb"},
		{LearningRule::OJA, "Oja"},
		{LearningRule::DELTA, "Delta"}
	};

	namespace element
	{
		/// @brief Parameters for a learned full-matrix field coupling.
		/// @ingroup elements
		struct FieldCouplingParameters final : ElementSpecificParameters
		{
			ElementDimensions inputFieldDimensions; ///< Spatial dimensions of the source (input) field.
			LearningRule learningRule;              ///< Which weight update rule to use.
			double scalar;                          ///< Scaling factor applied to the coupling output.
			double learningRate;                    ///< Learning rate η (step size for weight updates).
			double decayRate{0.0};                  ///< Weight decay coefficient, DELTA rule only. 0.0 disables decay.
			bool isLearningActive{false};                  ///< If true, weights are updated each step.

			/// @brief Construct FieldCoupling parameters.
			/// @param inputFieldDimensions  Dimensions of the source field.
			/// @param learningRule          Weight update rule (default HEBB).
			/// @param scalar                Output scaling factor (default 1.0).
			/// @param learningRate          Learning rate η (default 0.01).
			/// @param decayRate             Weight decay coefficient, DELTA rule only (default 0.0, disabled).
			explicit FieldCouplingParameters(const ElementDimensions& inputFieldDimensions = ElementDimensions{},
				const LearningRule learningRule = LearningRule::HEBB,
				const double scalar = 1.0, const double learningRate = 0.01,
				const double decayRate = 0.0)
					: inputFieldDimensions(inputFieldDimensions),
				learningRule(learningRule), scalar(scalar),
				learningRate(learningRate), decayRate(decayRate)
			{}

			bool operator==(const FieldCouplingParameters& other) const
			{
				constexpr double epsilon = 1e-6;

				return std::abs(inputFieldDimensions.x_max - other.inputFieldDimensions.x_max) < epsilon &&
					std::abs(inputFieldDimensions.d_x - other.inputFieldDimensions.d_x) < epsilon &&
					learningRule == other.learningRule &&
					std::abs(scalar - other.scalar) < epsilon &&
					std::abs(learningRate - other.learningRate) < epsilon &&
					std::abs(decayRate - other.decayRate) < epsilon;
			}

			[[nodiscard]] std::string toString() const override
			{
				std::ostringstream result;
				result << std::fixed << std::setprecision(2);
				result << "Parameters: ["
					<< "Input field dimensions: " << inputFieldDimensions.toString() << ", "
					<< "Learning rule: " << LearningRuleToString.at(learningRule) << ", "
					<< "Learning rate: " << learningRate << ", "
					<< "Decay rate: " << decayRate << ", "
					<< "Scalar: " << scalar
					<< "]";
				return result.str();
			}
		};

		/// @brief Full-matrix learned coupling between two neural fields.
		///
		/// FieldCoupling maintains an (output_size × input_size) weight matrix W.
		/// On each @c step() it computes `output = W * f(input)` (matrix-vector product
		/// of the weight matrix with the input field's "output" component) -- this is
		/// also the coupling's own forward estimate U(x_out,t) used as the DELTA rule's
		/// "actual" signal, see below.
		///
		/// When learning is active (@c setLearning(true)), weights are updated according
		/// to the selected @c LearningRule:
		///  - HEBB / OJA are unsupervised: Δw is driven by the (normalized) "activation"
		///    of the connected input and output fields alone.
		///  - DELTA is supervised (Widrow-Hoff): it additionally requires a third field
		///    connected to the Target pin (@c addInput(target, "target")), whose "output"
		///    (g(u)) is compared against this coupling's own forward pass. Without a
		///    target connected, DELTA learning is disabled (see checkValidConnections()).
		///    Both the input and target slots read whichever component they were wired
		///    from: connecting a field's Activation pin (component "activation") makes
		///    the forward pass, and for DELTA the "pre"/error term, use raw activation
		///    instead of g(u). HEBB/OJA are unaffected -- they always read "activation".
		///
		/// Weights can be persisted to and loaded from disk via @c writeWeights() / @c readWeights().
		///
		/// @ingroup elements
		class FieldCoupling final : public Element
		{
		protected:
			FieldCouplingParameters parameters;
			std::shared_ptr<Element> input;

			/// Cached downstream element that reads this coupling's output, refreshed
			/// from the (non-owning, #168) base `Element::outputs` in updateOutputField().
			/// Deliberately a weak_ptr, not a shared_ptr: that downstream element already
			/// owns this coupling via its own `inputs`, so a strong reference back would
			/// form the same two-way ownership cycle #168 fixed at the `Element::outputs`
			/// level, just re-created one class down. Lock before use and treat an
			/// expired lock the same as "no output field connected".
			std::weak_ptr<Element> output;

			std::shared_ptr<Element> targetField; ///< DELTA rule's teaching signal; nullptr if unconnected.
			std::string inputSourceComponent{ "output" }; ///< Component read from `input` ("output" or "activation").
			std::string targetSourceComponent{ "output" }; ///< Component read from `targetField` ("output" or "activation").
			std::string weightsDirectory; ///< Directory used for weight serialization.
		public:
			/// @brief Construct a FieldCoupling.
			/// @param elementCommonParameters  Name, label, and dimensions of the output field.
			/// @param fc_parameters            Coupling parameters (input dimensions, learning rule).
			FieldCoupling(const ElementCommonParameters& elementCommonParameters,
				const FieldCouplingParameters& fc_parameters);

			void init() override;
			void step(double t, double deltaT) override;

			/// @brief Register an input, output, or (for DELTA) target connection.
			/// @param inputElement    The upstream element.
			/// @param inputComponent  "output" or "activation" registers @p inputElement as
			///                        the coupling's input field, read from that component.
			///                        The sentinel values "target" and "target:activation"
			///                        instead register it as the DELTA rule's teaching
			///                        signal: this coupling reads @p inputElement's own
			///                        "output" (or "activation", for the ":activation" form)
			///                        component into components["target"], never
			///                        accumulating it into components["input"].
			void addInput(const std::shared_ptr<Element>& inputElement,
				const std::string& inputComponent = "output") override;
			void removeInput(const std::string& inputElementId) override;
			void removeInput(int uniqueId) override;
			void removeInputs() override;
			std::string toString() const override;
			std::shared_ptr<Element> clone() const override;

			/// @brief Resize the output field dimensions and rebuild the weight matrix.
			/// Preserves input field dimensions and clears weights. Connections are not
			/// removed — call removeInputs()/removeOutputs() first if needed. Any
			/// previously connected target field is now size-mismatched and is cleared.
			void changeDimensions(const ElementDimensions& newDimensions) override;

			/// @brief Resize the input field dimensions and rebuild the weight matrix.
			/// Preserves output field dimensions and clears weights. Connections are not
			/// removed — call removeInputs()/removeOutputs() first if needed.
			void changeInputDimensions(const ElementDimensions& newInputDimensions);

			void setLearningRate(double learningRate);

			/// @brief Set the DELTA rule's weight decay coefficient (Eq. 5's eta). No effect
			/// on HEBB/OJA.
			void setDecayRate(double decayRate);

			/// @brief Enable or disable online weight updates.
			/// @param learning  True to activate learning.
			void setLearning(bool learning);

			void setParameters(const FieldCouplingParameters& fcp);

			/// @brief Set the directory used for @c readWeights() / @c writeWeights().
			void setWeightsDirectory(const std::string& dir);

			FieldCouplingParameters getParameters() const;
			std::string getWeightsDirectory() const;

			/// @brief The DELTA rule's connected teaching-signal field, or nullptr if none.
			std::shared_ptr<Element> getTargetField() const;

			/// @brief Load the weight matrix from a binary file in @c weightsDirectory.
			void readWeights();

			/// @brief Load weights if the file exists; log INFO in either case.
			/// Unlike @c readWeights(), this never logs an error — use it when weights
			/// may legitimately be absent (e.g. first run of a programmatic simulation).
			void tryReadWeights();

			/// @brief Save the current weight matrix to a binary file in @c weightsDirectory.
			void writeWeights() const;

			/// @brief Reset the weight matrix to all zeros.
			void clearWeights();
		private:
			/// @brief Pull data from all registered sources. Overrides Element::updateInput()
			/// to route a source declared with component "target" into components["target"]
			/// instead of summing it into components["input"] alongside the real input field.
			void updateInput() override;
			/// @brief No-op: updateInput() above reads `inputs` directly on every call
			/// instead of a prebuilt cache, so there is nothing to build. Overriding
			/// this as a no-op also avoids the base implementation's
			/// `elem->components.at(compName)` lookup, which would throw for a source
			/// declared on the "target" slot (no NeuralField has a "target" component --
			/// see addInput()).
			void buildInputCache() override {}
			void updateOutput();
			void updateInputField();
			void updateOutputField();
			/// @brief Validate the connected target field's label and size; on failure
			/// log a WARNING and reset targetField to nullptr.
			void updateTargetField();
			/// @brief Reset targetField to nullptr and zero components["target"].
			void clearTarget();
			void updateWeights();
			bool checkValidConnections();

			/// @brief Split a stored input-slot string into (is-target-slot, source component).
			/// "target" -> {true, "output"}; "target:activation" -> {true, "activation"};
			/// anything else (e.g. "output", "activation") -> {false, the string itself}.
			static std::pair<bool, std::string> parseSlot(const std::string& declaredComponent);
		};
	}
}
