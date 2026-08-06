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

		/// @brief Validate every element up front, then construct and wire them.
		///
		/// Validation of the common fields runs over the whole array before anything is
		/// added, so a malformed entry anywhere aborts the load without leaving the
		/// simulation half-filled. Recoverable problems (a duplicate `uniqueName`, an
		/// unrecognised label, an interaction naming an element that was not loaded) are
		/// logged and skipped rather than failing the load, which is what lets a file
		/// written by a newer version still load here.
		///
		/// Every element-specific field read inside the per-label switch (e.g. `tau`,
		/// `amplitude`, `width`) is required and read with `json::at()`, which throws
		/// `json::out_of_range` if the key is missing -- the caller (buildElementsOrRollBack())
		/// catches that and reports the file as malformed. The only element-specific fields
		/// that are genuinely optional, with a documented fallback, are `activationFunction`
		/// (defaults to `SigmoidFunction(0.0, 10.0)`) and `input_x_max`/`input_d_x` on
		/// `field coupling`/`gauss field coupling` (default to `ElementDimensions{}`, i.e.
		/// x_max 100, d_x 1.0 -- see FieldCouplingParameters/GaussFieldCouplingParameters).
		///
		/// The four fields common to every element (`uniqueName`, `label`, `x_max`, `d_x`)
		/// and `inputs` are also read with `at()`, for the same reason. `uniqueName`/`label`/
		/// `x_max`/`d_x` are already guaranteed present by the up-front pre-check this
		/// function runs before either loop, so `at()` here is a local-safety guarantee
		/// rather than the primary defense; `inputs` is not covered by that pre-check, so
		/// its `at()` is the only thing rejecting a file that omits it.
		///
		/// @param jsonElements  The element array to build from.
		/// @return @c false if validation rejected the file, @c true otherwise.
		/// @throws nlohmann::json::out_of_range if an element object is missing a required,
		///         type-specific field for its label.
		[[nodiscard]] bool jsonToElements(const json& jsonElements) const;

		/// @brief Pull the element array out of a parsed `.dnf` root and apply its metadata.
		///
		/// Handles both accepted layouts: the legacy bare array of elements, and the
		/// current object carrying `identifier` / `deltaT` alongside an `elements` array.
		/// Metadata is applied to the simulation as a side effect; malformed metadata is
		/// logged and skipped rather than failing the load.
		///
		/// @note The metadata side effect is not undone by this function. A caller that
		///       goes on to fail the load is responsible for restoring the previous
		///       identifier and deltaT -- see loadElementsFromJson().
		///
		/// @param root          The parsed root of the `.dnf` document.
		/// @param elementsJson  Receives the element array on success.
		/// @return @c true if the root was a recognised layout, @c false otherwise.
		bool extractElementsAndMetadata(const json& root, json& elementsJson) const;

		/// @brief Build every element in @p elementsJson, or none of them.
		///
		/// Wraps jsonToElements() so that a throwing element constructor is reported as a
		/// malformed file instead of escaping the load. On failure, elements this call
		/// added are removed again; elements the simulation already held are left alone,
		/// since loading appends rather than replaces.
		///
		/// @param elementsJson  The element array to build from.
		/// @return @c true if every element was built, @c false if the load was aborted.
		[[nodiscard]] bool buildElementsOrRollBack(const json& elementsJson) const;
	};
}
