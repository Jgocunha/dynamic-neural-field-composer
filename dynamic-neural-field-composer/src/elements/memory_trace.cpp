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
			// Hoist component buffers out of the per-cell loop (see MemoryTrace2D):
			// avoids an unordered_map<string> hash lookup per cell.
			const double* __restrict in  = components["input"].data();
			double* __restrict       out = components["output"].data();
			const double invBuild = 1.0 / parameters.tauBuild;
			const double invDecay = 1.0 / parameters.tauDecay;
			for (int i = 0; i < size; ++i)
			{
				if (in[i] > parameters.threshold)
					out[i] += deltaT * invBuild * (-out[i] + in[i]);
				else
					out[i] += deltaT * invDecay * (-out[i]);
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