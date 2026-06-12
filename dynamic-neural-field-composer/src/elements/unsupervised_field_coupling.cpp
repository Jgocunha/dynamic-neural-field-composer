#include "elements/unsupervised_field_coupling.h"

namespace dnf_composer
{
	namespace element
	{
		UnsupervisedFieldCoupling::UnsupervisedFieldCoupling(
			const ElementCommonParameters& elementCommonParameters,
			const UnsupervisedFieldCouplingParameters& fc_parameters)
			: FieldCoupling(elementCommonParameters, fc_parameters)
		{
			commonParameters.identifiers.label = ElementLabel::UNSUPERVISED_FIELD_COUPLING;
		}

		void UnsupervisedFieldCoupling::init()
		{
			FieldCoupling::init();
		}

		void UnsupervisedFieldCoupling::step(double t, double deltaT)
		{
			FieldCoupling::step(t, deltaT);
		}

		void UnsupervisedFieldCoupling::addInput(const std::shared_ptr<Element>& inputElement,
			const std::string& inputComponent)
		{
			if (parameters.learningRule == LearningRule::DELTA)
			{
				log(tools::logger::LogLevel::ERROR,
					"UnsupervisedFieldCoupling '" + commonParameters.identifiers.uniqueName +
					"' does not support the DELTA learning rule. Use SupervisedFieldCoupling instead.");
				return;
			}
			FieldCoupling::addInput(inputElement, inputComponent);
		}

		std::string UnsupervisedFieldCoupling::toString() const
		{
			std::string result = "Unsupervised field coupling element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> UnsupervisedFieldCoupling::clone() const
		{
			return std::make_shared<UnsupervisedFieldCoupling>(*this);
		}

		void UnsupervisedFieldCoupling::setParameters(const UnsupervisedFieldCouplingParameters& fcp)
		{
			FieldCoupling::setParameters(fcp);
		}

		UnsupervisedFieldCouplingParameters UnsupervisedFieldCoupling::getParameters() const
		{
			UnsupervisedFieldCouplingParameters fcp{
				parameters.inputFieldDimensions,
				parameters.learningRule,
				parameters.scalar,
				parameters.learningRate
			};
			fcp.isLearningActive = parameters.isLearningActive;
			return fcp;
		}
	}
}
