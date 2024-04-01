#pragma once

#include "element.h"
#include <array>

namespace dnf_composer
{
	namespace element
	{
		class Kernel : public Element
		{
		protected:
			bool circular; // default is true
			bool normalized;  // default is true
			std::array<int, 2> kernelRange;
			std::vector<int> extIndex;
			double fullSum;
			int cutOfFactor;
		public:
			Kernel(const ElementCommonParameters& elementCommonParameters);
			Kernel(const ElementCommonParameters& elementCommonParameters, bool circular, bool normalized);
			~Kernel() override = default;

			std::array<int, 2> getKernelRange() const;
			std::vector<int> getExtIndex() const;
			bool getCircular() const;
			bool getNormalized() const;
		};
	}

}

