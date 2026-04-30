#include "elements/kernel.h"

namespace dnf_composer
{
	namespace element
	{
		Kernel::Kernel(const ElementCommonParameters& elementCommonParameters)
			: Element(elementCommonParameters)

		{
			kernelRange = { 0, 0 };
			extIndex = {};
			fullSum = 0.0;
			cutOfFactor = 5;
			components["kernel"] = std::vector<double>(commonParameters.dimensionParameters.size);
		}

		std::array<int, 2> Kernel::getKernelRange() const
		{
			return kernelRange;
		}

		std::vector<int> Kernel::getExtIndex() const
		{
			return extIndex;
		}
	}
}