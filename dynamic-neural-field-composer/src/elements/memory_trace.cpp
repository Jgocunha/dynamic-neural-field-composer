// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/memory_trace.h"

namespace dnf_composer
{
	namespace element
	{
		MemoryTrace::MemoryTrace(const ElementCommonParameters& elementCommonParameters,
			const MemoryTraceParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			this->commonParameters.identifiers.label = ElementLabel::MEMORY_TRACE;
		}

		void MemoryTrace::init()
		{
			std::fill(components["input"].begin(),  components["input"].end(),  0.0);
			std::fill(components["output"].begin(), components["output"].end(), 0.0);
		}

		void MemoryTrace::step(double t, double deltaT)
		{
			updateInput();

			const int size = commonParameters.dimensionParameters.size;
			for (int i = 0; i < size; ++i)
			{
				const double in = components["input"][i];
				if (in > parameters.threshold)
					components["output"][i] += deltaT * (1.0 / parameters.tauBuild) * (-components["output"][i] + in);
				else
					components["output"][i] += deltaT * (1.0 / parameters.tauDecay) * (-components["output"][i]);
			}
		}

		std::string MemoryTrace::toString() const
		{
			std::string result = "Memory trace element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> MemoryTrace::clone() const
		{
			auto cloned = std::make_shared<MemoryTrace>(*this);
			return cloned;
		}

		void MemoryTrace::setParameters(const MemoryTraceParameters& memoryTraceParameters)
		{
			parameters = memoryTraceParameters;
		}

		MemoryTraceParameters MemoryTrace::getParameters() const
		{
			return parameters;
		}
	}
}
