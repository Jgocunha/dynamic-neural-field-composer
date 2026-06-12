#pragma once

#include "elements/field_coupling.h"

namespace dnf_composer
{
	namespace element
	{
		/// @brief Parameters for a supervised field coupling (Delta rule).
		/// @ingroup elements
		struct SupervisedFieldCouplingParameters final : FieldCouplingParameters
		{
			explicit SupervisedFieldCouplingParameters(
				const ElementDimensions& inputFieldDimensions = ElementDimensions{},
				const double scalar = 1.0, const double learningRate = 0.01)
				: FieldCouplingParameters(inputFieldDimensions, LearningRule::DELTA, scalar, learningRate)
			{}

			[[nodiscard]] std::string toString() const override
			{
				std::ostringstream result;
				result << std::fixed << std::setprecision(2);
				result << "Parameters: ["
					<< "Input field dimensions: " << inputFieldDimensions.toString() << ", "
					<< "Learning rule: Delta, "
					<< "Learning rate: " << learningRate << ", "
					<< "Scalar: " << scalar
					<< "]";
				return result.str();
			}
		};

		/// @brief Full-matrix learned coupling using the supervised Delta (Widrow-Hoff) rule.
		///
		/// Computes `output = scalar * W * input` and learns `W` from an *error* signal, so it
		/// needs a teacher. Requires three connections:
		///  - a normal input (via addInput(el, "activation") or addInput(el))
		///  - a normal output field (connected as a downstream element)
		///  - a reference source (via addInput(el, "reference"))
		///
		/// The reference source's "output" component is copied into components["reference"]
		/// each step. The Delta rule then computes the error `e_j = reference_j - output_j` and
		/// updates `Δw_ij = η · e_j · in_i`, where the pre-synaptic `in` is the coupling's wired
		/// `components["input"]` and the post-synaptic `out` is its own `components["output"]`
		/// (both normalized).
		///
		/// Why a reference? The Delta rule is error-correcting — with no target there is no
		/// error and nothing to learn. This is the defining trait of supervised learning, and
		/// the difference from the unsupervised Hebb/Oja rules (see UnsupervisedFieldCoupling).
		///
		/// @ingroup elements
		class SupervisedFieldCoupling final : public FieldCoupling
		{
		protected:
			std::shared_ptr<Element> referenceSource; ///< Source element providing the target signal.
		public:
			SupervisedFieldCoupling(const ElementCommonParameters& elementCommonParameters,
				const SupervisedFieldCouplingParameters& fc_parameters);

			void init() override;
			void step(double t, double deltaT) override;
			void addInput(const std::shared_ptr<Element>& inputElement,
				const std::string& inputComponent = "output") override;
			[[nodiscard]] std::string toString() const override;
			[[nodiscard]] std::shared_ptr<Element> clone() const override;

			void setParameters(const SupervisedFieldCouplingParameters& fcp);
			[[nodiscard]] SupervisedFieldCouplingParameters getParameters() const;

			/// @brief Returns the element currently serving as the reference (target) source.
			[[nodiscard]] std::shared_ptr<Element> getReferenceSource() const;

		protected:
			void updateWeights() override;
			bool checkValidConnections() override;
		};
	}
}
