#include "visualization/plot.h"

namespace dnf_composer
{

	Plot::Plot(PlotCommonParameters parameters)
		: uniqueIdentifier(uniqueIdentifierCounter.fetch_add(1, std::memory_order_relaxed)), commonParameters(std::move(parameters))
	{
	}

	int Plot::getUniqueIdentifier() const
	{
		return uniqueIdentifier;
	}

	PlotType Plot::getType() const
	{
		return commonParameters.type;
	}

	PlotDimensions Plot::getDimensions() const
	{
		return commonParameters.dimensions;
	}

	PlotAnnotations Plot::getAnnotations() const
	{
		return commonParameters.annotations;
	}

	void Plot::setDimensions(const PlotDimensions& dimensions)
	{
		commonParameters.dimensions = dimensions;
	}

	void Plot::setAnnotations(const PlotAnnotations& annotations)
	{
		commonParameters.annotations = annotations;
	}
}