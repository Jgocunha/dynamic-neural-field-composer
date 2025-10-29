#pragma once
#include "plot_parameters.h"


namespace dnf_composer
{
	class Plot
	{
	protected:
		static inline int uniqueIdentifierCounter = 0;
		int uniqueIdentifier;
		PlotCommonParameters commonParameters;
	public:
		virtual ~Plot() = default;
		Plot(PlotCommonParameters parameters = PlotCommonParameters());

		int getUniqueIdentifier() const;
		PlotType getType() const;

		PlotDimensions getDimensions() const;
		PlotAnnotations getAnnotations() const;

		void setDimensions(const PlotDimensions& dimensions);
		void setAnnotations(const PlotAnnotations& annotations);

		virtual std::string toString() const = 0;

		virtual void render(const std::vector<std::vector<double>*>& data, const std::vector< std::string>& legends) = 0;
	};
}
