#pragma once

#include <sstream>

#include "elements/element.h"

namespace dnf_composer::element
{
	/**
	 * @brief Marginalises a 2D field onto one spatial dimension by summing over the other.
	 *
	 * Projects a flat size_x*size_y input onto either the X-axis (sum over Y) or the
	 * Y-axis (sum over X), producing a 1D output vector.  Corresponds to
	 * SumDimension (cosivina) / Projection (cedar).
	 *
	 * @param elementCommonParameters  Common parameters (id, 2D dimensions).
	 * @param parameters               Element-specific parameters.
	 */
	struct FieldProjectionParameters final : ElementSpecificParameters
	{
		int projectionAxis; // 0 = sum over Y (output varies along X), 1 = sum over X (output varies along Y)

		explicit FieldProjectionParameters(int projectionAxis = 0)
			: projectionAxis(projectionAxis) {}

		bool operator==(const FieldProjectionParameters& o) const
		{
			return projectionAxis == o.projectionAxis;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << "Parameters: [Projection axis: " << projectionAxis << "]";
			return result.str();
		}
	};

	class FieldProjection final : public Element
	{
	private:
		FieldProjectionParameters parameters;
	public:
		FieldProjection(const ElementCommonParameters& elementCommonParameters,
		                const FieldProjectionParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		[[nodiscard]] std::string toString() const override;
		[[nodiscard]] std::shared_ptr<Element> clone() const override;

		void setParameters(const FieldProjectionParameters& parameters);
		[[nodiscard]] FieldProjectionParameters getParameters() const;
	};
}
