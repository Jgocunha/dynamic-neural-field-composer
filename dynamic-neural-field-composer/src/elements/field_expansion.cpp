// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/field_expansion.h"

namespace dnf_composer::element
{
	FieldExpansion::FieldExpansion(const ElementCommonParameters& elementCommonParameters,
	                               const FieldExpansionParameters& parameters)
		: Element(elementCommonParameters), parameters(parameters)
	{
		commonParameters.identifiers.label = ElementLabel::FIELD_EXPANSION;
	}

	void FieldExpansion::init()
	{
		const int size_x = commonParameters.dimensionParameters.size_x;
		const int size_y = commonParameters.dimensionParameters.size_y;

		if (parameters.expansionAxis == 0)
			components["input"].assign(size_x, 0.0);
		else
			components["input"].assign(size_y, 0.0);

		components["output"].assign(size_x * size_y, 0.0);
	}

	void FieldExpansion::step(double t, double deltaT)
	{
		if (!inputs.empty())
			updateInput();

		const int size_x = commonParameters.dimensionParameters.size_x;
		const int size_y = commonParameters.dimensionParameters.size_y;

		if (parameters.expansionAxis == 0)
		{
			for (int xi = 0; xi < size_x; ++xi)
				for (int yi = 0; yi < size_y; ++yi)
					components["output"][xi * size_y + yi] = components["input"][xi];
		}
		else
		{
			for (int xi = 0; xi < size_x; ++xi)
				for (int yi = 0; yi < size_y; ++yi)
					components["output"][xi * size_y + yi] = components["input"][yi];
		}
	}

	std::string FieldExpansion::toString() const
	{
		std::string result = "Field expansion element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> FieldExpansion::clone() const
	{
		return std::make_shared<FieldExpansion>(*this);
	}

	void FieldExpansion::setParameters(const FieldExpansionParameters& p)
	{
		parameters = p;
		init();
	}

	FieldExpansionParameters FieldExpansion::getParameters() const
	{
		return parameters;
	}
}
