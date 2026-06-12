#pragma once

#include "elements/field_coupling.h"

namespace dnf_composer
{
	namespace element
	{
		/// @brief Parameters for an unsupervised field coupling (Hebb or Oja rule).
		/// @ingroup elements
		struct UnsupervisedFieldCouplingParameters final : FieldCouplingParameters
		{
			explicit UnsupervisedFieldCouplingParameters(
				const ElementDimensions& inputFieldDimensions = ElementDimensions{},
				const LearningRule learningRule = LearningRule::HEBB,
				const double scalar = 1.0, const double learningRate = 0.01)
				: FieldCouplingParameters(inputFieldDimensions, learningRule, scalar, learningRate)
			{}

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

		/// @brief Full-matrix learned coupling using unsupervised rules (Hebb or Oja).
		///
		/// Computes `output = scalar * W * input` and learns `W` from the *correlation* of
		/// pre- and post-synaptic activity, with no teacher signal:
		///   - Hebb: `Δw_ij = η · in_i · out_j`
		///   - Oja:  Hebb plus a `− out_j² · w_ij` decay term for stability.
		/// The pre-synaptic signal is the coupling's wired `components["input"]` (e.g. the
		/// source field's "activation", chosen by the user when connecting) and the
		/// post-synaptic signal is the coupling's own `components["output"]`; both are
		/// normalized before the rule is applied.
		///
		/// Extends FieldCoupling without adding new components. Restricts the allowed
		/// learning rules to HEBB and OJA — the DELTA rule requires a supervised reference
		/// signal and is only available in SupervisedFieldCoupling.
		///
		/// @ingroup elements
		class UnsupervisedFieldCoupling final : public FieldCoupling
		{
		public:
			UnsupervisedFieldCoupling(const ElementCommonParameters& elementCommonParameters,
				const UnsupervisedFieldCouplingParameters& fc_parameters);

			void init() override;
			void step(double t, double deltaT) override;
			void addInput(const std::shared_ptr<Element>& inputElement,
				const std::string& inputComponent = "output") override;
			[[nodiscard]] std::string toString() const override;
			[[nodiscard]] std::shared_ptr<Element> clone() const override;

			void setParameters(const UnsupervisedFieldCouplingParameters& fcp);
			[[nodiscard]] UnsupervisedFieldCouplingParameters getParameters() const;
		};
	}
}
