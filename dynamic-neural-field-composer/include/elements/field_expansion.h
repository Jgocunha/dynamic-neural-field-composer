#pragma once

#include <sstream>

#include "elements/element.h"

namespace dnf_composer::element
{
	/**
	 * @brief Lifts a 1D field into a 2D field by repeating values along the added dimension.
	 *
	 * Expands a 1D input that varies along X into a 2D size_x*size_y output (each row
	 * is a copy of the 1D input), or expands a 1D input that varies along Y into a 2D
	 * output (each column is a copy).  Corresponds to ExpandDimension2D (cosivina) /
	 * Projection (cedar).
	 *
	 * @param elementCommonParameters  Common parameters (id, 2D output dimensions).
	 * @param parameters               Element-specific parameters.
	 */
	struct FieldExpansionParameters final : ElementSpecificParameters
	{
		int expansionAxis; // 0 = input varies along X (repeated along Y), 1 = input varies along Y (repeated along X)

		explicit FieldExpansionParameters(int expansionAxis = 0)
			: expansionAxis(expansionAxis) {}

		bool operator==(const FieldExpansionParameters& o) const
		{
			return expansionAxis == o.expansionAxis;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << "Parameters: [Expansion axis: " << expansionAxis << "]";
			return result.str();
		}
	};

	class FieldExpansion final : public Element
	{
	private:
		FieldExpansionParameters parameters;
	public:
		FieldExpansion(const ElementCommonParameters& elementCommonParameters,
		               const FieldExpansionParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		[[nodiscard]] std::string toString() const override;
		[[nodiscard]] std::shared_ptr<Element> clone() const override;

		void setParameters(const FieldExpansionParameters& parameters);
		[[nodiscard]] FieldExpansionParameters getParameters() const;
	};
}
