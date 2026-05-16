#pragma once

#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <utility>
#include <utility>

#include "elements/element.h"

namespace dnf_composer::element
{
	enum class FieldProjectionCompression { SUM, AVERAGE, MAXIMUM, MINIMUM };
	enum class FieldProjectionDirection  { COMPRESS, EXPAND };

	struct FieldProjectionParameters final : ElementSpecificParameters
	{
		int projectionAxis = 0;
		FieldProjectionCompression compressionType = FieldProjectionCompression::SUM;
		FieldProjectionDirection   direction        = FieldProjectionDirection::COMPRESS;

		FieldProjectionParameters() = default;
		explicit FieldProjectionParameters(int projectionAxis,
			FieldProjectionCompression compressionType = FieldProjectionCompression::SUM,
			FieldProjectionDirection   direction        = FieldProjectionDirection::COMPRESS);
		bool operator==(const FieldProjectionParameters&) const = default;
		std::string toString() const override;
	};

	class FieldProjection final : public Element
	{
		FieldProjectionParameters parameters;
	public:
		FieldProjection(const ElementCommonParameters& common, FieldProjectionParameters params);

		void addInput(const std::shared_ptr<Element>& inputElement,
		              const std::string& inputComponent = "output") override;
		void init() override;
		void step(double t, double deltaT) override;
	private:
		void resizeComponents();
	public:
		[[nodiscard]] std::shared_ptr<Element> clone() const override;
		[[nodiscard]] std::string toString() const override;

		void setParameters(const FieldProjectionParameters& params);
		[[nodiscard]] FieldProjectionParameters getParameters() const;
	};
}
