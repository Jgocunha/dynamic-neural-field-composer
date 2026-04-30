#include "elements/element_factory.h"

namespace dnf_composer
{
	namespace element
	{

		ElementFactory::ElementFactory()
		{
			setupElementCreators();
		}

		void ElementFactory::setupElementCreators()
		{
			// Register element creators for each element type
			elementCreators[ElementLabel::NEURAL_FIELD] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const NeuralFieldParameters*>(&elementSpecificParameters);
					return std::make_shared<NeuralField>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::GAUSS_STIMULUS] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const GaussStimulusParameters*>(&elementSpecificParameters);
					return std::make_shared<GaussStimulus>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::GAUSS_KERNEL] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const GaussKernelParameters*>(&elementSpecificParameters);
					return std::make_shared<GaussKernel>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::MEXICAN_HAT_KERNEL] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const MexicanHatKernelParameters*>(&elementSpecificParameters);
					return std::make_shared<MexicanHatKernel>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::NORMAL_NOISE] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const NormalNoiseParameters*>(&elementSpecificParameters);
					return std::make_shared<NormalNoise>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::GAUSS_FIELD_COUPLING] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const GaussFieldCouplingParameters*>(&elementSpecificParameters);
					return std::make_shared<GaussFieldCoupling>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::FIELD_COUPLING] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const FieldCouplingParameters*>(&elementSpecificParameters);
					return std::make_shared<FieldCoupling>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::OSCILLATORY_KERNEL] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const OscillatoryKernelParameters*>(&elementSpecificParameters);
					return std::make_shared<OscillatoryKernel>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::ASYMMETRIC_GAUSS_KERNEL] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const AsymmetricGaussKernelParameters*>(&elementSpecificParameters);
					return std::make_shared<AsymmetricGaussKernel>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::BOOST_STIMULUS] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const BoostStimulusParameters*>(&elementSpecificParameters);
					return std::make_shared<BoostStimulus>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::MEMORY_TRACE] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const MemoryTraceParameters*>(&elementSpecificParameters);
					return std::make_shared<MemoryTrace>(elementCommonParameters, *params);
				};
		}

		std::shared_ptr<Element> ElementFactory::createElement(ElementLabel type, const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
		{
			const auto creator = elementCreators.find(type);

			if (creator != elementCreators.end())
			{
				return creator->second(elementCommonParameters, elementSpecificParameters);
			}
			else
				return nullptr;
		}
		std::shared_ptr<Element> ElementFactory::createElement(ElementLabel type)
		{
			const auto creator = elementCreators.find(type);

			if (creator != elementCreators.end())
			{
				switch (type)
				{
					case ElementLabel::NEURAL_FIELD:
						return creator->second(ElementCommonParameters(type), NeuralFieldParameters());
					case ElementLabel::GAUSS_STIMULUS:
						return creator->second(ElementCommonParameters(type), GaussStimulusParameters());
					case ElementLabel::GAUSS_KERNEL:
						return creator->second(ElementCommonParameters(type), GaussKernelParameters());
					case ElementLabel::MEXICAN_HAT_KERNEL:
						return creator->second(ElementCommonParameters(type), MexicanHatKernelParameters());
					case ElementLabel::NORMAL_NOISE:
						return creator->second(ElementCommonParameters(type), NormalNoiseParameters());
					case ElementLabel::GAUSS_FIELD_COUPLING:
						return creator->second(ElementCommonParameters(type), GaussFieldCouplingParameters());
					case ElementLabel::FIELD_COUPLING:
						return creator->second(ElementCommonParameters(type), FieldCouplingParameters());
					case ElementLabel::OSCILLATORY_KERNEL:
						return creator->second(ElementCommonParameters(type), OscillatoryKernelParameters());
					case ElementLabel::ASYMMETRIC_GAUSS_KERNEL:
						return creator->second(ElementCommonParameters(type), AsymmetricGaussKernelParameters());
					case ElementLabel::BOOST_STIMULUS:
						return creator->second(ElementCommonParameters(type), BoostStimulusParameters());
					case ElementLabel::MEMORY_TRACE:
						return creator->second(ElementCommonParameters(type), MemoryTraceParameters());
					case ElementLabel::UNINITIALIZED:
						return nullptr;
				}
			}
			return nullptr;
		}
	}
}