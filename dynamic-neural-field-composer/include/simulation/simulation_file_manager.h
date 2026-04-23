#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <cmath>

#include "./simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/normal_noise.h"
#include "elements/field_coupling.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_field_coupling.h"
#include "elements/oscillatory_kernel.h"
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/boost_stimulus.h"
#include "elements/memory_trace.h"

/// @defgroup simulation_io Simulation I/O
/// @brief JSON serialization and deserialization of simulation architectures.
/// @ingroup simulation

namespace dnf_composer
{
	using json = nlohmann::json;

	/// @brief Serializes and deserializes a Simulation to / from a JSON file.
	///
	/// SimulationFileManager reads and writes every element in the simulation
	/// (including parameters and inter-element connections) as a JSON document.
	/// This allows pre-designed or evolved architectures to be saved and replayed
	/// without re-implementing them in code.
	///
	/// @ingroup simulation_io
	class SimulationFileManager
	{
	private:
		std::shared_ptr<Simulation> simulation; ///< The simulation to serialize/deserialize.
		std::string filePath;                   ///< Absolute path to the JSON file.
	public:
		/// @brief Construct a SimulationFileManager.
		/// @param simulation  The simulation instance to read into or write from.
		/// @param filePath    Path to the JSON file (empty = default location).
		SimulationFileManager(const std::shared_ptr<Simulation>& simulation, const std::string& filePath = {});

		/// @brief Serialize all elements and their connections to the JSON file.
		void saveElementsToJson() const;

		/// @brief Deserialize elements and connections from the JSON file into the simulation.
		void loadElementsFromJson() const;

	private:
		static json elementToJson(const std::shared_ptr<element::Element>& element);
		void jsonToElements(const json& jsonElements) const;
	};
}
