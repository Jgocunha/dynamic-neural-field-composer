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

		virtual std::shared_ptr<Element> clone() const = 0;

		virtual ~Element() = default;

		virtual std::string toString() const = 0;

		void close();
		void print() const;

		/// @brief Register @p inputElement as an upstream source for this element.
		/// @param inputElement    The element whose output will be read.
		/// @param inputComponent  Which component of @p inputElement to read (default: "output").
		virtual void addInput(const std::shared_ptr<Element>& inputElement,
		                      const std::string& inputComponent = "output");

		void removeInput(const std::string& inputElementId);
		void removeInput(int uniqueId);
		void removeInputs();
		bool hasInput(const std::string& inputElementName, const std::string& inputComponent);
		bool hasInput(int inputElementId, const std::string& inputComponent);

		/// @brief Pull data from all registered input elements into this element's components.
		void updateInput();

		/// @brief Deregister this element as an input of @p outputElementId.
		void removeOutput(const std::string& outputElementId);

		/// @brief Deregister this element as an input of the element with @p uniqueId.
		void removeOutput(int uniqueId);

		void removeOutputs();
		bool hasOutput(const std::string& outputElementName, const std::string& outputComponent);
		bool hasOutput(int outputElementId, const std::string& outputComponent);

		int getMaxSpatialDimension() const;

		/// @brief Return the number of spatial samples (size = x_max / d_x).
		int getSize() const;

		/// @brief Return the spatial resolution (d_x).
		double getStepSize() const;

		ElementCommonParameters getElementCommonParameters() const;
		int getUniqueIdentifier() const;
		std::string getUniqueName() const;
		ElementLabel getLabel() const;
		bool hasOutput() const;
		bool hasInput() const;

		/// @brief Return a copy of the named component vector.
		/// @param componentName  E.g. "activation", "output", "input".
		std::vector<double> getComponent(const std::string& componentName);

		std::vector<double>* getComponentPtr(const std::string& componentName);
		std::vector<std::string> getComponentList() const;

		/// @brief Return a read-only pointer to the full components map.
		const std::unordered_map<std::string, std::vector<double>>* getComponents() const;

		std::vector<std::shared_ptr<Element>> getInputs();

		/// @brief Return all inputs mapped to the component name they expose.
		std::unordered_map<std::shared_ptr<Element>, std::string> getInputsAndComponents();

		std::vector<std::shared_ptr<Element>> getOutputs();
	};
}
