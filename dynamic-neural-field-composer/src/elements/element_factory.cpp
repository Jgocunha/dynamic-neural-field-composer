#include "elements/element_factory.h"

#include "exceptions/exception.h"

	namespace dnf_composer::element
	{
		namespace
		{
			/// @brief Human-readable name for an ElementLabel.
			///
			/// Falls back to the numeric value for labels outside the known enumeration
			/// (ElementLabel is a plain enum, so a caller can construct an out-of-range
			/// value via static_cast).
			std::string labelName(const ElementLabel label)
			{
				const auto it = ElementLabelToString.find(label);
				return it != ElementLabelToString.end()
					? it->second
					: "unknown (" + std::to_string(static_cast<int>(label)) + ")";
			}

			/// @brief Downcasts elementSpecificParameters to the concrete type a creator expects.
			///
			/// Every registered creator lambda receives its parameters through the common
			/// ElementSpecificParameters base reference and must downcast to the concrete
			/// type its element constructor needs. If the caller passed a mismatched
			/// ElementSpecificParameters subtype, dynamic_cast returns nullptr; dereferencing
			/// that pointer would be undefined behavior (#113), so this throws a descriptive
			/// Exception instead.
			///
			/// @tparam ParamsT              Concrete ElementSpecificParameters subtype expected.
			/// @param elementSpecificParameters  The parameters object supplied by the caller.
			/// @param label                      The element type being constructed (for the message).
			/// @param expectedTypeName           Name of ParamsT, for the error message.
			/// @throws Exception if elementSpecificParameters is not actually a ParamsT.
			template <typename ParamsT>
			const ParamsT& requireParams(const ElementSpecificParameters& elementSpecificParameters,
				const ElementLabel label, const char* expectedTypeName)
			{
				const auto* const params = dynamic_cast<const ParamsT*>(&elementSpecificParameters);
				if (!params)
				{
					throw Exception("ElementFactory: cannot create a '" + labelName(label) +
						"' element - expected parameters of type '" + expectedTypeName +
						"', but received an incompatible ElementSpecificParameters subtype.");
				}
				return *params;
			}
		}

		ElementFactory::ElementFactory()
		{
			setupElementCreators();
		}

		void ElementFactory::setupElementCreators()
		{
			// Register element creators for each element type. Each lambda downcasts
			// elementSpecificParameters via requireParams<>(), which throws a descriptive
			// Exception instead of dereferencing a failed dynamic_cast (#113).
			elementCreators[ElementLabel::NEURAL_FIELD] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<NeuralFieldParameters>(elementSpecificParameters, ElementLabel::NEURAL_FIELD, "NeuralFieldParameters");
					return std::make_shared<NeuralField>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::GAUSS_STIMULUS] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<GaussStimulusParameters>(elementSpecificParameters, ElementLabel::GAUSS_STIMULUS, "GaussStimulusParameters");
					return std::make_shared<GaussStimulus>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::GAUSS_KERNEL] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<GaussKernelParameters>(elementSpecificParameters, ElementLabel::GAUSS_KERNEL, "GaussKernelParameters");
					return std::make_shared<GaussKernel>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::MEXICAN_HAT_KERNEL] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<MexicanHatKernelParameters>(elementSpecificParameters, ElementLabel::MEXICAN_HAT_KERNEL, "MexicanHatKernelParameters");
					return std::make_shared<MexicanHatKernel>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::NORMAL_NOISE] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<NormalNoiseParameters>(elementSpecificParameters, ElementLabel::NORMAL_NOISE, "NormalNoiseParameters");
					return std::make_shared<NormalNoise>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::CORRELATED_NORMAL_NOISE] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<CorrelatedNormalNoiseParameters>(elementSpecificParameters, ElementLabel::CORRELATED_NORMAL_NOISE, "CorrelatedNormalNoiseParameters");
					return std::make_shared<CorrelatedNormalNoise>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::GAUSS_FIELD_COUPLING] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<GaussFieldCouplingParameters>(elementSpecificParameters, ElementLabel::GAUSS_FIELD_COUPLING, "GaussFieldCouplingParameters");
					return std::make_shared<GaussFieldCoupling>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::FIELD_COUPLING] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<FieldCouplingParameters>(elementSpecificParameters, ElementLabel::FIELD_COUPLING, "FieldCouplingParameters");
					return std::make_shared<FieldCoupling>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::OSCILLATORY_KERNEL] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<OscillatoryKernelParameters>(elementSpecificParameters, ElementLabel::OSCILLATORY_KERNEL, "OscillatoryKernelParameters");
					return std::make_shared<OscillatoryKernel>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::ASYMMETRIC_GAUSS_KERNEL] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<AsymmetricGaussKernelParameters>(elementSpecificParameters, ElementLabel::ASYMMETRIC_GAUSS_KERNEL, "AsymmetricGaussKernelParameters");
					return std::make_shared<AsymmetricGaussKernel>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::BOOST_STIMULUS] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<BoostStimulusParameters>(elementSpecificParameters, ElementLabel::BOOST_STIMULUS, "BoostStimulusParameters");
					return std::make_shared<BoostStimulus>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::MEMORY_TRACE] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<MemoryTraceParameters>(elementSpecificParameters, ElementLabel::MEMORY_TRACE, "MemoryTraceParameters");
					return std::make_shared<MemoryTrace>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::STIMULUS_SUM] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<StimulusSumParameters>(elementSpecificParameters, ElementLabel::STIMULUS_SUM, "StimulusSumParameters");
					return std::make_shared<StimulusSum>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::NEURAL_FIELD_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<NeuralField2DParameters>(elementSpecificParameters, ElementLabel::NEURAL_FIELD_2D, "NeuralField2DParameters");
					return std::make_shared<NeuralField2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::GAUSS_STIMULUS_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<GaussStimulus2DParameters>(elementSpecificParameters, ElementLabel::GAUSS_STIMULUS_2D, "GaussStimulus2DParameters");
					return std::make_shared<GaussStimulus2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::GAUSS_KERNEL_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<GaussKernel2DParameters>(elementSpecificParameters, ElementLabel::GAUSS_KERNEL_2D, "GaussKernel2DParameters");
					return std::make_shared<GaussKernel2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::MEXICAN_HAT_KERNEL_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<MexicanHatKernel2DParameters>(elementSpecificParameters, ElementLabel::MEXICAN_HAT_KERNEL_2D, "MexicanHatKernel2DParameters");
					return std::make_shared<MexicanHatKernel2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::NORMAL_NOISE_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<NormalNoise2DParameters>(elementSpecificParameters, ElementLabel::NORMAL_NOISE_2D, "NormalNoise2DParameters");
					return std::make_shared<NormalNoise2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::OSCILLATORY_KERNEL_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<OscillatoryKernel2DParameters>(elementSpecificParameters, ElementLabel::OSCILLATORY_KERNEL_2D, "OscillatoryKernel2DParameters");
					return std::make_shared<OscillatoryKernel2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::TIMED_GAUSS_STIMULUS] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<TimedGaussStimulusParameters>(elementSpecificParameters, ElementLabel::TIMED_GAUSS_STIMULUS, "TimedGaussStimulusParameters");
					return std::make_shared<TimedGaussStimulus>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::TIMED_GAUSS_STIMULUS_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<TimedGaussStimulus2DParameters>(elementSpecificParameters, ElementLabel::TIMED_GAUSS_STIMULUS_2D, "TimedGaussStimulus2DParameters");
					return std::make_shared<TimedGaussStimulus2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::BOOST_STIMULUS_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<BoostStimulus2DParameters>(elementSpecificParameters, ElementLabel::BOOST_STIMULUS_2D, "BoostStimulus2DParameters");
					return std::make_shared<BoostStimulus2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::CORRELATED_NORMAL_NOISE_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<CorrelatedNormalNoise2DParameters>(elementSpecificParameters, ElementLabel::CORRELATED_NORMAL_NOISE_2D, "CorrelatedNormalNoise2DParameters");
					return std::make_shared<CorrelatedNormalNoise2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<AsymmetricGaussKernel2DParameters>(elementSpecificParameters, ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D, "AsymmetricGaussKernel2DParameters");
					return std::make_shared<AsymmetricGaussKernel2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::MEMORY_TRACE_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<MemoryTrace2DParameters>(elementSpecificParameters, ElementLabel::MEMORY_TRACE_2D, "MemoryTrace2DParameters");
					return std::make_shared<MemoryTrace2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::RESIZE] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<ResizeParameters>(elementSpecificParameters, ElementLabel::RESIZE, "ResizeParameters");
					return std::make_shared<Resize>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::RESIZE_2D] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<Resize2DParameters>(elementSpecificParameters, ElementLabel::RESIZE_2D, "Resize2DParameters");
					return std::make_shared<Resize2D>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::COLLAPSE] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<CollapseParameters>(elementSpecificParameters, ElementLabel::COLLAPSE, "CollapseParameters");
					return std::make_shared<Collapse>(elementCommonParameters, params);
				};

			elementCreators[ElementLabel::EXPAND] = [](const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
				{
					const auto& params = requireParams<ExpandParameters>(elementSpecificParameters, ElementLabel::EXPAND, "ExpandParameters");
					return std::make_shared<Expand>(elementCommonParameters, params);
				};

			}

		std::shared_ptr<Element> ElementFactory::createElement(ElementLabel type, const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters)
		{
			const auto creator = elementCreators.find(type);

			if (creator != elementCreators.end())
			{
				return creator->second(elementCommonParameters, elementSpecificParameters);
			}

			throw Exception("ElementFactory: no element creator registered for ElementLabel '" + labelName(type) + "'.");
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
					case ElementLabel::STIMULUS_SUM:
						return creator->second(ElementCommonParameters(type), StimulusSumParameters());
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
					case ElementLabel::UNINITIALIZED:
						// Unreachable: UNINITIALIZED has no registered creator, so the
						// enclosing `creator != elementCreators.end()` check above already
						// excludes it; kept only so the switch stays exhaustive over
						// ElementLabel. Falls through to the throw below regardless.
						break;
				}
			}
			throw Exception("ElementFactory: no element creator registered for ElementLabel '" + labelName(type) + "'.");
		}
	}
