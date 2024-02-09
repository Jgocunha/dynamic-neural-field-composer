#pragma once

#include <iostream>
#include <nlohmann/json.hpp>

#include "./simulation/simulation.h"

#include "./elements/neural_field.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/normal_noise.h"

namespace dnf_composer
{
	using json = nlohmann::json;

	class SimulationFileManager
	{
	private:
		std::shared_ptr<Simulation> simulation;
		std::string filePath;
	public:
		SimulationFileManager(const std::shared_ptr<Simulation>& simulation);

		void saveElementsToJson() const;

	private:
		static json elementToJson(const std::shared_ptr<element::Element>& element);

	};
}
