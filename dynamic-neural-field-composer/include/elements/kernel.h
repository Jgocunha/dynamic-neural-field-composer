#pragma once

#include "element.h"
#include <array>

namespace dnf_composer::element
{
	/// @brief Abstract base class for all convolution-based interaction kernels.
	///
	/// Manages the pre-computed kernel range, extended index (for circular
	/// convolution), and the full spatial sum used for global inhibition terms.
	/// Concrete subclasses (GaussKernel, MexicanHatKernel, …) implement @c init()
	/// and @c step() to build and apply the specific kernel shape.
	///
	/// @ingroup elements
	class Kernel : public Element
	{
	protected:
		std::array<int, 2> kernelRange; ///< [min, max] index range of the non-negligible kernel support.
		std::vector<int> extIndex;      ///< Extended index array for circular (toroidal) convolution.
		double fullSum;                 ///< Spatial integral of the kernel (used for global inhibition baseline).
		int cutOfFactor;                ///< Controls how far from the centre the kernel is truncated.
	public:
		/// @brief Construct a kernel element with the given common parameters.
		Kernel(const ElementCommonParameters& elementCommonParameters);
		~Kernel() override = default;

		/// @brief Return the non-negligible index range [min, max] of the kernel.
		std::array<int, 2> getKernelRange() const;

		/// @brief Return the extended index used for circular convolution.
		std::vector<int> getExtIndex() const;
	};
}
