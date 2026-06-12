#include "elements/supervised_field_coupling.h"

namespace dnf_composer
{
	namespace element
	{
		SupervisedFieldCoupling::SupervisedFieldCoupling(
			const ElementCommonParameters& elementCommonParameters,
			const SupervisedFieldCouplingParameters& fc_parameters)
			: FieldCoupling(elementCommonParameters, fc_parameters)
		{
			commonParameters.identifiers.label = ElementLabel::SUPERVISED_FIELD_COUPLING;
			// "reference" component: same size as output field, used as target signal
			components["reference"] = std::vector<double>(commonParameters.dimensionParameters.size, 0.0);
		}

		void SupervisedFieldCoupling::init()
		{
			std::ranges::fill(components["reference"], 0.0);
			FieldCoupling::init();
		}

		void SupervisedFieldCoupling::step(double t, double deltaT)
		{
			if (referenceSource)
				components["reference"] = referenceSource->getComponent("output");

			FieldCoupling::step(t, deltaT);
		}

		void SupervisedFieldCoupling::addInput(const std::shared_ptr<Element>& inputElement,
			const std::string& inputComponent)
		{
			if (inputComponent == "reference")
			{
				if (!inputElement)
				{
					log(tools::logger::LogLevel::ERROR,
						"SupervisedFieldCoupling '" + commonParameters.identifiers.uniqueName +
						"': reference source element is null.");
					return;
				}
				referenceSource = inputElement;
				log(tools::logger::LogLevel::INFO,
					"SupervisedFieldCoupling '" + commonParameters.identifiers.uniqueName +
					"': reference source set to '" + inputElement->getUniqueName() + "'.");
			}
			else
			{
				FieldCoupling::addInput(inputElement, inputComponent);
			}
		}

		std::string SupervisedFieldCoupling::toString() const
		{
			std::string result = "Supervised field coupling element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			if (referenceSource)
				result += "\nReference source: " + referenceSource->getUniqueName();
			return result;
		}

		std::shared_ptr<Element> SupervisedFieldCoupling::clone() const
		{
			return std::make_shared<SupervisedFieldCoupling>(*this);
		}

		void SupervisedFieldCoupling::setParameters(const SupervisedFieldCouplingParameters& fcp)
		{
			FieldCoupling::setParameters(fcp);
		}

		SupervisedFieldCouplingParameters SupervisedFieldCoupling::getParameters() const
		{
			SupervisedFieldCouplingParameters fcp{
				parameters.inputFieldDimensions,
				parameters.scalar,
				parameters.learningRate
			};
			fcp.isLearningActive = parameters.isLearningActive;
			return fcp;
		}

		std::shared_ptr<Element> SupervisedFieldCoupling::getReferenceSource() const
		{
			return referenceSource;
		}

		void SupervisedFieldCoupling::updateWeights()
		{
			const int inSz  = static_cast<int>(components["input"].size());
			const int outSz = static_cast<int>(components["output"].size());

			// reshape flat weights to 2D (required by math.h delta functions)
			std::vector<std::vector<double>> w2d(inSz, std::vector<double>(outSz));
			for (int i = 0; i < inSz; ++i)
				for (int j = 0; j < outSz; ++j)
					w2d[i][j] = components["weights"][static_cast<size_t>(i) * outSz + j];

			// Pre-synaptic = wired input, post-synaptic = own output, target = wired reference.
			const std::vector<double> inputAct  = tools::math::normalize(components.at("input"));
			const std::vector<double> outputAct = tools::math::normalize(components.at("output"));
			const std::vector<double> refAct    = tools::math::normalize(components.at("reference"));

			tools::math::deltaLearningRuleWidrowHoff(w2d, inputAct, outputAct, refAct,
				parameters.learningRate);

			// write back to flat storage
			for (int i = 0; i < inSz; ++i)
				for (int j = 0; j < outSz; ++j)
					components["weights"][static_cast<size_t>(i) * outSz + j] = w2d[i][j];
		}

		bool SupervisedFieldCoupling::checkValidConnections()
		{
			if (!FieldCoupling::checkValidConnections())
				return false;

			if (!referenceSource)
			{
				const std::string logMessage = "SupervisedFieldCoupling '" +
					commonParameters.identifiers.uniqueName +
					"' has no reference source. Learning is disabled.";
				log(tools::logger::LogLevel::WARNING, logMessage);
				parameters.isLearningActive = false;
				return false;
			}

			return true;
		}
	}
}
