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

			elementCreators[ElementLabel::CORRELATED_NORMAL_NOISE] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const CorrelatedNormalNoiseParameters*>(&elementSpecificParameters);
					return std::make_shared<CorrelatedNormalNoise>(elementCommonParameters, *params);
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

			elementCreators[ElementLabel::NEURAL_FIELD_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const NeuralField2DParameters*>(&elementSpecificParameters);
					return std::make_shared<NeuralField2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::GAUSS_STIMULUS_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const GaussStimulus2DParameters*>(&elementSpecificParameters);
					return std::make_shared<GaussStimulus2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::GAUSS_KERNEL_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const GaussKernel2DParameters*>(&elementSpecificParameters);
					return std::make_shared<GaussKernel2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::MEXICAN_HAT_KERNEL_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const MexicanHatKernel2DParameters*>(&elementSpecificParameters);
					return std::make_shared<MexicanHatKernel2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::NORMAL_NOISE_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const NormalNoise2DParameters*>(&elementSpecificParameters);
					return std::make_shared<NormalNoise2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::OSCILLATORY_KERNEL_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const OscillatoryKernel2DParameters*>(&elementSpecificParameters);
					return std::make_shared<OscillatoryKernel2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::TIMED_GAUSS_STIMULUS] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const TimedGaussStimulusParameters*>(&elementSpecificParameters);
					return std::make_shared<TimedGaussStimulus>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::TIMED_GAUSS_STIMULUS_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const TimedGaussStimulus2DParameters*>(&elementSpecificParameters);
					return std::make_shared<TimedGaussStimulus2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::BOOST_STIMULUS_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const BoostStimulus2DParameters*>(&elementSpecificParameters);
					return std::make_shared<BoostStimulus2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::CORRELATED_NORMAL_NOISE_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const CorrelatedNormalNoise2DParameters*>(&elementSpecificParameters);
					return std::make_shared<CorrelatedNormalNoise2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const AsymmetricGaussKernel2DParameters*>(&elementSpecificParameters);
					return std::make_shared<AsymmetricGaussKernel2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::MEMORY_TRACE_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const MemoryTrace2DParameters*>(&elementSpecificParameters);
					return std::make_shared<MemoryTrace2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::RESIZE] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const ResizeParameters*>(&elementSpecificParameters);
					return std::make_shared<Resize>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::RESIZE_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const Resize2DParameters*>(&elementSpecificParameters);
					return std::make_shared<Resize2D>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::COLLAPSE] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const CollapseParameters*>(&elementSpecificParameters);
					return std::make_shared<Collapse>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::EXPAND] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const ExpandParameters*>(&elementSpecificParameters);
					return std::make_shared<Expand>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::UNSUPERVISED_FIELD_COUPLING] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const UnsupervisedFieldCouplingParameters*>(&elementSpecificParameters);
					return std::make_shared<UnsupervisedFieldCoupling>(elementCommonParameters, *params);
				};

			elementCreators[ElementLabel::SUPERVISED_FIELD_COUPLING] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto params = dynamic_cast<const SupervisedFieldCouplingParameters*>(&elementSpecificParameters);
					return std::make_shared<SupervisedFieldCoupling>(elementCommonParameters, *params);
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
					case ElementLabel::CORRELATED_NORMAL_NOISE:
						return creator->second(ElementCommonParameters(type), CorrelatedNormalNoiseParameters());
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
					case ElementLabel::NEURAL_FIELD_2D:
						return creator->second(ElementCommonParameters(type), NeuralField2DParameters());
					case ElementLabel::GAUSS_STIMULUS_2D:
						return creator->second(ElementCommonParameters(type), GaussStimulus2DParameters());
					case ElementLabel::GAUSS_KERNEL_2D:
						return creator->second(ElementCommonParameters(type), GaussKernel2DParameters());
					case ElementLabel::MEXICAN_HAT_KERNEL_2D:
						return creator->second(ElementCommonParameters(type), MexicanHatKernel2DParameters());
					case ElementLabel::NORMAL_NOISE_2D:
						return creator->second(ElementCommonParameters(type), NormalNoise2DParameters());
					case ElementLabel::OSCILLATORY_KERNEL_2D:
						return creator->second(ElementCommonParameters(type), OscillatoryKernel2DParameters());
					case ElementLabel::TIMED_GAUSS_STIMULUS:
						return creator->second(ElementCommonParameters(type), TimedGaussStimulusParameters());
					case ElementLabel::TIMED_GAUSS_STIMULUS_2D:
						return creator->second(ElementCommonParameters(type), TimedGaussStimulus2DParameters());
					case ElementLabel::BOOST_STIMULUS_2D:
						return creator->second(ElementCommonParameters(type), BoostStimulus2DParameters());
					case ElementLabel::CORRELATED_NORMAL_NOISE_2D:
						return creator->second(ElementCommonParameters(type), CorrelatedNormalNoise2DParameters());
					case ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D:
						return creator->second(ElementCommonParameters(type), AsymmetricGaussKernel2DParameters());
					case ElementLabel::MEMORY_TRACE_2D:
						return creator->second(ElementCommonParameters(type), MemoryTrace2DParameters());
					case ElementLabel::RESIZE:
						return creator->second(ElementCommonParameters(type), ResizeParameters());
					case ElementLabel::RESIZE_2D:
						return creator->second(ElementCommonParameters(type), Resize2DParameters());
					case ElementLabel::COLLAPSE:
						return creator->second(ElementCommonParameters(type), CollapseParameters());
					case ElementLabel::EXPAND:
						return creator->second(ElementCommonParameters(type), ExpandParameters());
					case ElementLabel::UNSUPERVISED_FIELD_COUPLING:
						return creator->second(ElementCommonParameters(type), UnsupervisedFieldCouplingParameters());
					case ElementLabel::SUPERVISED_FIELD_COUPLING:
						return creator->second(ElementCommonParameters(type), SupervisedFieldCouplingParameters());
					case ElementLabel::UNINITIALIZED:
						return nullptr;
				}
			}
			return nullptr;
		}
	}
}