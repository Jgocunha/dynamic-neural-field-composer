#pragma once

#include "elements/element.h"
#include "elements/gauss_kernel.h"
#include "elements/gauss_stimulus.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/neural_field.h"
#include "elements/normal_noise.h"
#include "elements/correlated_normal_noise.h"
#include "elements/gauss_field_coupling.h"
#include "elements/field_coupling.h"
#include "simulation/simulation.h"
#include "elements/oscillatory_kernel.h"
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/boost_stimulus.h"
#include "elements/memory_trace.h"
#include "elements/neural_field_2d.h"
#include "elements/gauss_stimulus_2d.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/mexican_hat_kernel_2d.h"
#include "elements/normal_noise_2d.h"
#include "elements/oscillatory_kernel_2d.h"
#include "elements/timed_gauss_stimulus.h"
#include "elements/timed_gauss_stimulus_2d.h"
#include "elements/boost_stimulus_2d.h"
#include "elements/correlated_normal_noise_2d.h"
#include "elements/asymmetric_gauss_kernel_2d.h"
#include "elements/memory_trace_2d.h"
#include "elements/resize.h"
#include "elements/resize_2d.h"
#include "elements/collapse.h"
#include "elements/expand.h"

namespace dnf_composer::element
{
	/// @brief Factory that creates concrete Element objects from type labels and parameters.
	///
	/// ElementFactory maintains a dispatch table keyed on ElementLabel.
	/// Call @c createElement(label, commonParams, specificParams) to create any registered
	/// element type without depending on its concrete class.  A default-parameter overload
	/// is available for quick construction during prototyping.
	///
	/// @ingroup elements
	class ElementFactory
	{
	private:
		std::unordered_map<ElementLabel, std::function<std::shared_ptr<Element>(
			                   const ElementCommonParameters&,
			                   const ElementSpecificParameters&)>> elementCreators;
	public:
		ElementFactory();

		/// @brief Create and return an element of the given type.
		/// @param type                       Which element to create.
		/// @param elementCommonParameters    Name, label, and spatial dimensions.
		/// @param elementSpecificParameters  Type-specific parameter struct (cast internally).
		/// @return Shared pointer to the new element.
		std::shared_ptr<Element> createElement(ElementLabel type,
		                                       const ElementCommonParameters& elementCommonParameters,
		                                       const ElementSpecificParameters& elementSpecificParameters);

		/// @brief Create an element with default parameters.
		/// @param type  Which element to create.
		/// @return Shared pointer to the new element.
		std::shared_ptr<Element> createElement(ElementLabel type);
	private:
		void setupElementCreators();
	};
}
