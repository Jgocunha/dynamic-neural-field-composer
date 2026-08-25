#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
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

		/// Upstream elements and the component they expose. Owning: an element
		/// depends on its inputs staying alive for as long as it does, so it holds
		/// a strong shared_ptr to each of them.
		std::unordered_map<std::shared_ptr<Element>, std::string> inputs;

		/// Downstream elements that read this element's output, and the component
		/// name they were registered under. Non-owning by design (#168): the
		/// downstream element already keeps *this* alive via its own `inputs`
		/// entry, so a strong reference here would form a two-way ownership cycle
		/// and neither element would ever be freed. A weak_ptr key, compared by
		/// std::owner_less (weak_ptr has no default hash/equality), lets this map
		/// observe a connection without owning it. A locked entry may be null if
		/// the downstream element has since been destroyed through any path other
		/// than removeInput()/removeInputs() -- callers must treat that as "not
		/// connected", never dereference it.
		std::map<std::weak_ptr<Element>, std::string, std::owner_less<std::weak_ptr<Element>>> outputs;
	private:
		// Caches a pointer to each connected input's *vector object* (not a raw
		// data() snapshot). A std::vector stored as an unordered_map value keeps its
		// address stable across resizes (only its internal buffer reallocates), so
		// re-reading ->size()/data() from it on every updateInput() call can never
		// dangle -- it always reflects the source's current dimensions, even if the
		// source was resized via changeDimensions() after this cache was built.
		std::vector<const std::vector<double>*> cachedInputs;
		double*     inputPtr  = nullptr;
		std::size_t inputSize = 0;

		/// @brief Remove any input whose source component no longer fits within
		///        this element's "input" buffer (e.g. the source was resized larger
		///        via changeDimensions() after the cache was built, so accumulating
		///        it in full would write out-of-bounds). Logs a warning per severed
		///        connection and forces a cache rebuild on the next updateInput().
		///        Assumes a single "input" sink -- a subclass that overrides
		///        updateInput() to route sources into additional buffers (e.g.
		///        FieldCoupling's "target") must not rely on this to guard them.
		void severIncompatibleInputs();

		/// @brief Erase any `outputs` entry whose downstream element has already
		///        been destroyed. Called opportunistically by the methods that
		///        walk `outputs`, so the map doesn't accumulate expired weak_ptr
		///        bookkeeping entries for elements destroyed without going through
		///        removeInput()/removeInputs().
		void pruneExpiredOutputs();
	protected:
		/// @brief Force the next updateInput() to rebuild its source cache from
		/// scratch. Call after directly reallocating a component that the cache
		/// may hold a stale reference into (e.g. a subclass's changeDimensions()
		/// override that resizes "input" or another routed-into buffer outside
		/// of the base class's own changeDimensions()).
		void invalidateInputCache();
	public:
		/// @brief Construct an element with the given common parameters.
		/// @param parameters  Name, label, and spatial dimensions.
		/// @throws Exception(ErrorCode::ELEM_INVALID_SIZE) if parameters.dimensionParameters.size
		///         is not positive (#118). In normal use ElementDimensions's own constructors
		///         already reject a non-positive size before it ever reaches here; this guards
		///         the invariant should dimensionParameters ever be mutated afterward (its
		///         fields are public) into an invalid state.
		explicit Element(const ElementCommonParameters& parameters);

		/// @brief Copy commonParameters/components/inputs/outputs; deliberately
		/// does NOT copy the input-cache (inputPtr/cachedInputs) -- see the .cpp
		/// for why a raw-pointer-value copy of inputPtr would alias the wrong
		/// object. Every clone() override that copy-constructs (`make_shared<T>(*this)`)
		/// relies on this to produce a correctly-behaving copy.
		Element(const Element& other);
		Element& operator=(const Element& other);

		/// @brief Initialize the element (called once before the simulation loop).
		virtual void init() = 0;

		/// @brief Resize all components to @p newDimensions and re-initialize.
		/// @note Does not remove connections — call Simulation::changeDimensions to
		///       disconnect neighbours before resizing.
		/// @param newDimensions  New spatial discretization.
		virtual void changeDimensions(const ElementDimensions& newDimensions);

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
		/// Wires both directions of the bookkeeping: this element strongly owns
		/// @p inputElement via `inputs`, and @p inputElement records this element
		/// as an observer via its non-owning `outputs` (#168).
		/// @param inputElement    The element whose output will be read.
		/// @param inputComponent  Which component of @p inputElement to read (default: "output").
		virtual void addInput(const std::shared_ptr<Element>& inputElement,
		                      const std::string& inputComponent = "output");

		/// @brief Whether this element manages its own input-shape validation.
		///
		/// Elements that intentionally bridge dimensionality or size (e.g. Collapse
		/// going 2D -> 1D, or Resize resampling to a different count) override
		/// @c addInput() to validate and resize their own "input" component before
		/// delegating to Element::addInput(). For those elements, addInput()'s direct
		/// dimensionality/shape comparison against the source is both redundant and
		/// unsafe to infer from buffer length alone, so it is skipped whenever this
		/// returns @c true. Plain elements keep the default (@c false) and get the
		/// full shape check.
		/// @return @c true if this element validates/resizes its own "input" component
		///         and should be exempt from Element::addInput()'s shape check.
		[[nodiscard]] virtual bool bridgesDimensions() const { return false; }

		/// @brief Deregister the input element named @p inputElementId, and erase
		///        the matching `outputs` entry on that element so the two sides of
		///        the connection stay symmetric (#168).
		/// Virtual so a subclass that routes some inputs into an additional
		/// buffer (e.g. FieldCoupling's "target") can clear that buffer too.
		virtual void removeInput(const std::string& inputElementId);

		/// @brief Deregister the input element with @p uniqueId, and erase the
		///        matching `outputs` entry on that element (#168).
		virtual void removeInput(int uniqueId);
		virtual void removeInputs();
		bool hasInput(const std::string& inputElementName, const std::string& inputComponent);
		bool hasInput(int inputElementId, const std::string& inputComponent);

		/// @brief Pull data from all registered input elements into this element's components.
		/// Virtual so a subclass can route some inputs into additional buffers
		/// instead of summing everything into "input" (e.g. FieldCoupling routes
		/// a source declared with component "target" into components["target"]).
		virtual void updateInput();

		/// @brief Cache raw pointers to input component data. Call after all element init()s complete.
		/// Virtual so a subclass whose updateInput() override doesn't use this cache
		/// (e.g. FieldCoupling, which reads `inputs` directly every call) can make it
		/// a no-op -- the base implementation assumes every entry in `inputs` names a
		/// component that exists on the source, which "target" deliberately does not.
		virtual void buildInputCache();

		/// @brief Deregister this element as an input of @p outputElementId.
		void removeOutput(const std::string& outputElementId);

		/// @brief Deregister this element as an input of the element with @p uniqueId.
		void removeOutput(int uniqueId);

		void removeOutputs();

		/// @brief True if a live (not-yet-destroyed) downstream element named
		///        @p outputElementName exposes @p outputComponent (#168).
		bool hasOutput(const std::string& outputElementName, const std::string& outputComponent);
		bool hasOutput(int outputElementId, const std::string& outputComponent);

		int getMaxSpatialDimension() const;

		/// @brief Return the number of spatial samples (size = round(x_max / d_x)).
		int getSize() const;

		/// @brief Return the spatial resolution (d_x).
		double getStepSize() const;

		ElementCommonParameters getElementCommonParameters() const;
		int getUniqueIdentifier() const;
		std::string getUniqueName() const;
		void setUniqueName(const std::string& name);
		ElementLabel getLabel() const;

		/// @brief True if at least one registered `outputs` entry still points to
		///        a live element (#168).
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

		/// @brief Return the currently live downstream elements. Each `outputs`
		///        entry is a weak_ptr; an entry whose element has been destroyed
		///        (without going through removeInput()/removeInputs()) is skipped
		///        rather than surfaced as a null pointer (#168).
		std::vector<std::shared_ptr<Element>> getOutputs();
	};
}
