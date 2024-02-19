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
#include "elements/lateral_interactions.h"

namespace dnf_composer
{
    namespace element
    {
	    class ElementFactory
	    {
	    private:
	        std::unordered_map<ElementLabel, std::function<std::shared_ptr<Element>(const ElementCommonParameters&, const ElementSpecificParameters&)>> elementCreators;
	    public:
			ElementFactory();
			std::shared_ptr<Element> createElement(ElementLabel type, const ElementCommonParameters& elementCommonParameters, const ElementSpecificParameters& elementSpecificParameters);
	    private:
			void setupElementCreators();
	    };
    }
    
}
