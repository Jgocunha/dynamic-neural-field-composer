#pragma once
#include "elements/element.h"

namespace dnf_composer::element
{
	struct ResizeParameters final : ElementSpecificParameters
	{
		int    outputSize = 100;
		double outputStep = 1.0;

		ResizeParameters() = default;
		explicit ResizeParameters(int outputSize, double outputStep = 1.0);
		bool operator==(const ResizeParameters&) const = default;
		std::string toString() const override;
	};

	class Resize final : public Element
	{
		ResizeParameters parameters;
	public:
		Resize(const ElementCommonParameters& common, ResizeParameters params);
		void   init() override;
		void   step(double t, double deltaT) override;
		std::shared_ptr<Element> clone() const override;
		std::string toString() const override;
		void setParameters(const ResizeParameters& params);
		ResizeParameters getParameters() const;
	};
}
