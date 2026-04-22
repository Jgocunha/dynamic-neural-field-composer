#pragma once

#include "element.h"
#include <array>

namespace dnf_composer::element
{
	class Kernel : public Element
	{
	protected:
		std::array<int, 2> kernelRange;
		std::vector<int> extIndex;
		double fullSum;
		int cutOfFactor;
	public:
		Kernel(const ElementCommonParameters& elementCommonParameters);
		~Kernel() override = default;

		std::array<int, 2> getKernelRange() const;
		std::vector<int> getExtIndex() const;
	};
}

