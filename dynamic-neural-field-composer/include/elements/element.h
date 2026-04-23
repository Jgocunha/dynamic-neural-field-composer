#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <ranges>
#include <algorithm>
#include <numeric>

#include "exceptions/exception.h"
#include "tools/logger.h"
#include "element_parameters/element_parameters.h"

/// @defgroup elements Elements
/// @brief DFT element primitives: fields, kernels, stimuli, noise, and couplings.

namespace dnf_composer::element
{
	/// @brief Abstract base class for all simulation elements.
	///
	/// Every element owns a set of named data components (e.g. "activation", "output"),
	/// a list of input elements, and a list of output elements. Concrete subclasses
	/// implement the @c init / @c step lifecycle and exchange data via @c addInput().
	///
	/// @ingroup elements
	class Element : public std::enable_shared_from_this<Element>
	{
	protected:
		ElementCommonParameters commonParameters;                            ///< Name, label, and spatial dimensions.
		std::unordered_map<std::string, std::vector<double>> components;    ///< Named data arrays (e.g. "output").
		std::unordered_map<std::shared_ptr<Element>, std::string> inputs;   ///< Upstream elements and the component they expose.
		std::unordered_map<std::shared_ptr<Element>, std::string> outputs;  ///< Downstream elements that read this element's output.
	public:
		/// @brief Construct an element with the given common parameters.
		/// @param parameters  Name, label, and spatial dimensions.
		explicit Element(const ElementCommonParameters& parameters);

		/// @brief Initialize the element (called once before the simulation loop).
		virtual void init() = 0;

		/// @brief Advance the element by one time step.
		/// @param t       Current simulation time.
		/// @param deltaT  Integration step size.
		virtual void step(double t, double deltaT) = 0;

		/// @brief Return a deep copy of this element.
		virtual std::shared_ptr<Element> clone() const = 0;

		virtual ~Element() = default;

		/// @brief Return a human-readable description of the element and its parameters.
		virtual std::string toString() const = 0;

		/// @brief Release any resources held by the element.
		void close();

		/// @brief Print a description to the log.
		void print() const;

		/// @brief Register @p inputElement as an upstream source for this element.
		/// @param inputElement    The element whose output will be read.
		/// @param inputComponent  Which component of @p inputElement to read (default: "output").
		virtual void addInput(const std::shared_ptr<Element>& inputElement,
		                      const std::string& inputComponent = "output");

		/// @brief Remove the input element identified by @p inputElementId.
		void removeInput(const std::string& inputElementId);

		/// @brief Remove the input element with the given unique numeric ID.
		void removeInput(int uniqueId);

		/// @brief Remove all registered input elements.
		void removeInputs();

		/// @brief Return true if an input from @p inputElementName / @p inputComponent exists.
		bool hasInput(const std::string& inputElementName, const std::string& inputComponent);

		/// @brief Return true if an input from element @p inputElementId / @p inputComponent exists.
		bool hasInput(int inputElementId, const std::string& inputComponent);

		/// @brief Pull data from all registered input elements into this element's components.
		void updateInput();

		/// @brief Deregister this element as an input of @p outputElementId.
		void removeOutput(const std::string& outputElementId);

		/// @brief Deregister this element as an input of the element with @p uniqueId.
		void removeOutput(int uniqueId);

		/// @brief Remove all registered downstream outputs.
		void removeOutputs();

		/// @brief Return true if a downstream output to @p outputElementName / @p outputComponent exists.
		bool hasOutput(const std::string& outputElementName, const std::string& outputComponent);

		/// @brief Return true if a downstream output to element @p outputElementId / @p outputComponent exists.
		bool hasOutput(int outputElementId, const std::string& outputComponent);

		/// @brief Return the largest spatial dimension (x_max).
		int getMaxSpatialDimension() const;

		/// @brief Return the number of spatial samples (size = x_max / d_x).
		int getSize() const;

		/// @brief Return the spatial resolution (d_x).
		double getStepSize() const;

		/// @brief Return a copy of the element's common parameters.
		ElementCommonParameters getElementCommonParameters() const;

		/// @brief Return the element's numeric unique identifier.
		int getUniqueIdentifier() const;

		/// @brief Return the element's unique name string.
		std::string getUniqueName() const;

		/// @brief Return the element's type label.
		ElementLabel getLabel() const;

		/// @brief Return true if this element has at least one registered output.
		bool hasOutput() const;

		/// @brief Return true if this element has at least one registered input.
		bool hasInput() const;

		/// @brief Return a copy of the named component vector.
		/// @param componentName  E.g. "activation", "output", "input".
		std::vector<double> getComponent(const std::string& componentName);

		/// @brief Return a pointer to the named component vector.
		std::vector<double>* getComponentPtr(const std::string& componentName);

		/// @brief Return the names of all registered components.
		std::vector<std::string> getComponentList() const;

		/// @brief Return a read-only pointer to the full components map.
		const std::unordered_map<std::string, std::vector<double>>* getComponents() const;

		/// @brief Return all registered input elements.
		std::vector<std::shared_ptr<Element>> getInputs();

		/// @brief Return all inputs mapped to the component name they expose.
		std::unordered_map<std::shared_ptr<Element>, std::string> getInputsAndComponents();

		/// @brief Return all registered downstream output elements.
		std::vector<std::shared_ptr<Element>> getOutputs();
	};
}
