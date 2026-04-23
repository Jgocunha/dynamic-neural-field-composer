#pragma once

#include <set>

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
		DELTA  ///< Delta rule: error-driven weight updates.
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
			bool isLearningActive;                  ///< If true, weights are updated each step.

			/// @brief Construct FieldCoupling parameters.
			/// @param inputFieldDimensions  Dimensions of the source field.
			/// @param learningRule          Weight update rule (default HEBB).
			/// @param scalar                Output scaling factor (default 1.0).
			/// @param learningRate          Learning rate η (default 0.01).
			explicit FieldCouplingParameters(const ElementDimensions& inputFieldDimensions = ElementDimensions{},
				const LearningRule learningRule = LearningRule::HEBB,
				const double scalar = 1.0, const double learningRate = 0.01)
					: inputFieldDimensions(inputFieldDimensions),
				learningRule(learningRule), scalar(scalar),
				learningRate(learningRate), isLearningActive(false)
			{}

			bool operator==(const FieldCouplingParameters& other) const
			{
				constexpr double epsilon = 1e-6;

				return std::abs(inputFieldDimensions.x_max - other.inputFieldDimensions.x_max) < epsilon &&
					std::abs(inputFieldDimensions.d_x - other.inputFieldDimensions.d_x) < epsilon &&
					learningRule == other.learningRule &&
					std::abs(scalar - other.scalar) < epsilon &&
					std::abs(learningRate - other.learningRate) < epsilon;
			}

			[[nodiscard]] std::string toString() const override
			{
				std::ostringstream result;
				result << std::fixed << std::setprecision(2);
				result << "Parameters: ["
					<< "Input field dimensions: " << inputFieldDimensions.toString() << ", "
					<< "Learning rule: " << LearningRuleToString.at(learningRule) << ", "
					<< "Learning rate: " << learningRate << ", "
					<< "Scalar: " << scalar
					<< "]";
				return result.str();
			}
		};

		/// @brief Full-matrix learned coupling between two neural fields.
		///
		/// FieldCoupling maintains an (output_size × input_size) weight matrix W.
		/// On each @c step() it computes `output = W * f(input)` (matrix-vector product
		/// of the weight matrix with the input field's "output" component).
		///
		/// When learning is active (@c setLearning(true)), weights are updated according
		/// to the selected @c LearningRule (HEBB, OJA, or DELTA). Weights can be
		/// persisted to and loaded from disk via @c writeWeights() / @c readWeights().
		///
		/// @ingroup elements
		class FieldCoupling final : public Element
		{
		protected:
			FieldCouplingParameters parameters;
			std::shared_ptr<Element> input;
			std::shared_ptr<Element> output;
			std::string weightsDirectory; ///< Directory used for weight serialization.
		public:
			/// @brief Construct a FieldCoupling.
			/// @param elementCommonParameters  Name, label, and dimensions of the output field.
			/// @param fc_parameters            Coupling parameters (input dimensions, learning rule).
			FieldCoupling(const ElementCommonParameters& elementCommonParameters,
				const FieldCouplingParameters& fc_parameters);

			void init() override;
			void step(double t, double deltaT) override;
			std::string toString() const override;
			std::shared_ptr<Element> clone() const override;

			void setLearningRate(double learningRate);

			/// @brief Enable or disable online weight updates.
			/// @param learning  True to activate learning.
			void setLearning(bool learning);

			void setParameters(const FieldCouplingParameters& fcp);

			/// @brief Set the directory used for @c readWeights() / @c writeWeights().
			void setWeightsDirectory(const std::string& dir);

			FieldCouplingParameters getParameters() const;
			std::string getWeightsDirectory() const;

			/// @brief Load the weight matrix from a binary file in @c weightsDirectory.
			void readWeights();

			/// @brief Save the current weight matrix to a binary file in @c weightsDirectory.
			void writeWeights() const;

			/// @brief Reset the weight matrix to all zeros.
			void clearWeights();
		private:
			void updateOutput();
			void updateInputField();
			void updateOutputField();
			void updateWeights();
			bool checkValidConnections();
		};
	}
}
