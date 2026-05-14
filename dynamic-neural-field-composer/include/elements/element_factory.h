#pragma once

#include "elements/element.h"
#include "elements/gauss_kernel.h"
#include "elements/gauss_stimulus.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/neural_field.h" 
#include "elements/normal_noise.h"
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

namespace dnf_composer::element
{
	class ElementFactory
	{
	private:
		std::unordered_map<ElementLabel, std::function<std::shared_ptr<Element>(
			                   const ElementCommonParameters&,
			                   const ElementSpecificParameters&)>> elementCreators;
	public:
		ElementFactory();
		std::shared_ptr<Element> createElement(ElementLabel type,
		                                       const ElementCommonParameters& elementCommonParameters,
		                                       const ElementSpecificParameters& elementSpecificParameters);
		std::shared_ptr<Element> createElement(ElementLabel type);
	private:
		void setupElementCreators();
	};
}
