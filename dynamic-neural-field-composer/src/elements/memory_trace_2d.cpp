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
		for (int i = 0; i < size; ++i)
		{
			const double in = components["input"][i];
			if (in > parameters.threshold)
				components["output"][i] += deltaT * (1.0 / parameters.tauBuild) * (-components["output"][i] + in);
			else
				components["output"][i] += deltaT * (1.0 / parameters.tauDecay) * (-components["output"][i]);
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
