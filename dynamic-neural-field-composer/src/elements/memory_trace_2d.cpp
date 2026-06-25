// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/memory_trace_2d.h"

namespace dnf_composer::element
{
	MemoryTrace2D::MemoryTrace2D(const ElementCommonParameters& elementCommonParameters,
	                             const MemoryTrace2DParameters& parameters)
		: Element(elementCommonParameters), parameters(parameters)
	{
		commonParameters.identifiers.label = ElementLabel::MEMORY_TRACE_2D;
	}

	void MemoryTrace2D::init()
	{
		std::ranges::fill(components["input"],  0.0);
		std::ranges::fill(components["output"], 0.0);
	}

	void MemoryTrace2D::step(double t, double deltaT)
	{
		updateInput();

		const int size = commonParameters.dimensionParameters.size;
		// Hoist the component buffers out of the per-cell loop: indexing
		// components["..."] inside the loop is an unordered_map<string> hash lookup
		// per cell (profiled as the dominant cost of this element).
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

	std::string MemoryTrace2D::toString() const
	{
		std::string result = "Memory trace 2D element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> MemoryTrace2D::clone() const
	{
		return std::make_shared<MemoryTrace2D>(*this);
	}

	void MemoryTrace2D::setParameters(const MemoryTrace2DParameters& p)
	{
		parameters = p;
		init();
	}

	MemoryTrace2DParameters MemoryTrace2D::getParameters() const
	{
		return parameters;
	}
}
