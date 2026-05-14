// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/field_projection.h"

namespace dnf_composer::element
{
	FieldProjection::FieldProjection(const ElementCommonParameters& elementCommonParameters,
	                                 const FieldProjectionParameters& parameters)
		: Element(elementCommonParameters), parameters(parameters)
	{
		commonParameters.identifiers.label = ElementLabel::FIELD_PROJECTION;

		const int size_x = commonParameters.dimensionParameters.size_x;
		const int size_y = commonParameters.dimensionParameters.size_y;
		if (parameters.projectionAxis == 0)
			components["output"].assign(size_x, 0.0);
		else
			components["output"].assign(size_y, 0.0);
	}

	void FieldProjection::init()
	{
		const int size_x = commonParameters.dimensionParameters.size_x;
		const int size_y = commonParameters.dimensionParameters.size_y;

		components["input"].assign(size_x * size_y, 0.0);

		if (parameters.projectionAxis == 0)
			components["output"].assign(size_x, 0.0);
		else
			components["output"].assign(size_y, 0.0);
	}

	void FieldProjection::step(double t, double deltaT)
	{
		if (!inputs.empty())
			updateInput();

		const int size_x = commonParameters.dimensionParameters.size_x;
		const int size_y = commonParameters.dimensionParameters.size_y;

		if (parameters.projectionAxis == 0)
		{
			for (int xi = 0; xi < size_x; ++xi)
			{
				double sum = 0.0;
				for (int yi = 0; yi < size_y; ++yi)
					sum += components["input"][xi * size_y + yi];
				components["output"][xi] = sum;
			}
		}
		else
		{
			for (int yi = 0; yi < size_y; ++yi)
			{
				double sum = 0.0;
				for (int xi = 0; xi < size_x; ++xi)
					sum += components["input"][xi * size_y + yi];
				components["output"][yi] = sum;
			}
		}
	}

	std::string FieldProjection::toString() const
	{
		std::string result = "Field projection element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> FieldProjection::clone() const
	{
		return std::make_shared<FieldProjection>(*this);
	}

	void FieldProjection::setParameters(const FieldProjectionParameters& p)
	{
		parameters = p;
		init();
	}

	FieldProjectionParameters FieldProjection::getParameters() const
	{
		return parameters;
	}
}
