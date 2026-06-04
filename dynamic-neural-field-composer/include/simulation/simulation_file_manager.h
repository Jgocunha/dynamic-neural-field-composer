#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <cmath>

#include "tools/utils.h"
#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/normal_noise.h"
#include "elements/correlated_normal_noise.h"
#include "elements/field_coupling.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_field_coupling.h"
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
	/// **Default output layout** when @p filePath is empty:
	/// ```
	/// data/<identifier>/
	///   <identifier>.dnf            ← element graph
	///   <coupling_name>_weights.txt ← one file per FieldCoupling element
	/// ```
	///
	/// When loading, FieldCoupling weight files are resolved relative to the
	/// directory that contains the `.dnf` file (i.e. `parent_path(filePath)`).
	///
	/// The optional path supplied to this manager has different behavior depending
	/// on the operation:
	/// - if non-empty, it is treated as the `.dnf` file path to save to or load from;
	/// - if empty, saving uses the default output location (`data/`) and a
	///   generated `<identifier>/<identifier>.dnf` file path.
	///
	/// @ingroup simulation_io
	class SimulationFileManager
	{
	private:
		std::shared_ptr<Simulation> simulation; ///< The simulation to serialize/deserialize.
		std::string filePath;                   ///< Optional .dnf file path override; if empty, save uses the default output location and generated file name.
	public:
		/// @brief Construct a SimulationFileManager.
		/// @param simulation  The simulation instance to read into or write from.
		/// @param filePath    Optional `.dnf` file path. When non-empty, it is the file
		///                    used for both save and load operations. When empty,
		///                    saveElementsToJson() writes to the default output
		///                    location using a generated `<identifier>.dnf` file
		///                    name, while loadElementsFromJson() expects a concrete
		///                    `.dnf` file path to be available at the resolved path.
		SimulationFileManager(const std::shared_ptr<Simulation>& simulation, const std::string& filePath = {});

		/// @brief Serialize all elements and their connections to a JSON file.
		/// When @p filePath is empty the output directory is
		/// `data/<identifier>/` and @c FieldCoupling weight matrices are written
		/// into the same directory before the JSON is saved.
		void saveElementsToJson() const;

		/// @brief Deserialize elements and connections from a JSON file into the simulation.
		/// After loading, @c FieldCoupling elements have their weight directory set to
		/// the parent directory of the JSON file and their weights are re-read from there.
		void loadElementsFromJson() const;

	private:
		static json elementToJson(const std::shared_ptr<element::Element>& element);
		void jsonToElements(const json& jsonElements) const;
	};
}
