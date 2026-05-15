# Elements

Elements are the building blocks of a DNF simulation. Every entity in the system — neural fields, kernels, stimuli, couplings, noise sources — is an `Element`.

```cpp
#include "elements/element.h"
#include "elements/element_factory.h"
```

---

## Element base class

All elements inherit from `dnf_composer::element::Element`. The interface that every element must implement:

```cpp
void init();                              // initialize buffers and state
void step(double t, double deltaT);       // advance one time step
std::shared_ptr<Element> clone() const;   // deep copy
std::string toString() const;             // human-readable description

// Resize the element to new spatial dimensions.
// All component buffers are reassigned and init() is called automatically.
// Note: this does NOT remove connections — call removeInputs()/removeOutputs()
// first if connected elements have a different size (Simulation::changeDimensions
// handles this automatically).
void changeDimensions(const ElementDimensions& newDimensions);
```

---

## Common parameters

Every element is identified and dimensioned by `ElementCommonParameters`, which bundles two structs:

### ElementIdentifiers

```cpp
struct ElementIdentifiers {
    int uniqueIdentifier;    // auto-assigned integer ID
    std::string uniqueName;  // human-readable name (your label + auto ID suffix)
    ElementLabel label;      // enum: NEURAL_FIELD, GAUSS_KERNEL, ...
};
```

You typically construct it with just a name string:

```cpp
ElementIdentifiers id{ "my field" };
```

### ElementDimensions

```cpp
struct ElementDimensions {
    int x_max;    // number of discrete positions (field size)
    double d_x;   // spatial step size
    int size;     // x_max / d_x  (computed)
};
```

```cpp
ElementDimensions dims{ 100, 1.0 };  // 100 positions, step 1.0
```

### Putting them together

```cpp
ElementCommonParameters params{
    ElementIdentifiers{ "my field" },
    ElementDimensions{ 100, 1.0 }
};
```

Shorthand constructors are also available:

```cpp
ElementCommonParameters{ "my field" }         // default dims (100, 1.0)
ElementCommonParameters{ "my field", 200 }    // x_max = 200, d_x = 1.0
ElementCommonParameters{ "my field", dims }   // explicit dims
```

---

## Element labels

The `ElementLabel` enum identifies element types and is used by `ElementFactory`:

| Label | String | Type |
|---|---|---|
| `NEURAL_FIELD` | `"neural field"` | `NeuralField` |
| `GAUSS_STIMULUS` | `"gauss stimulus"` | `GaussStimulus` |
| `GAUSS_KERNEL` | `"gauss kernel"` | `GaussKernel` |
| `MEXICAN_HAT_KERNEL` | `"mexican hat kernel"` | `MexicanHatKernel` |
| `OSCILLATORY_KERNEL` | `"oscillatory kernel"` | `OscillatoryKernel` |
| `ASYMMETRIC_GAUSS_KERNEL` | `"asymmetric gauss kernel"` | `AsymmetricGaussKernel` |
| `NORMAL_NOISE` | `"normal noise"` | `NormalNoise` |
| `FIELD_COUPLING` | `"field coupling"` | `FieldCoupling` |
| `GAUSS_FIELD_COUPLING` | `"gauss field coupling"` | `GaussFieldCoupling` |
| `BOOST_STIMULUS` | `"boost stimulus"` | `BoostStimulus` |
| `MEMORY_TRACE` | `"memory trace"` | `MemoryTrace` |
| `RESIZE` | `"resize"` | `Resize` |

---

## ElementFactory

The recommended way to create elements. Takes a label, common parameters, and type-specific parameters:

```cpp
ElementFactory factory;

auto field = factory.createElement(
    element::NEURAL_FIELD,
    ElementCommonParameters{ ElementIdentifiers{"name"}, dims },
    NeuralFieldParameters{ tau, restingLevel, SigmoidFunction(0.0, 10.0) }
);
```

A default-parameter overload is also available:

```cpp
auto field = factory.createElement(element::NEURAL_FIELD);
```

---

## Components

Each element exposes its internal state as named `std::vector<double>` buffers called **components**. Components are the bridge between elements and the visualization system.

```cpp
// Read a component (returns a copy)
std::vector<double> act = element->getComponent("activation");

// Get a live pointer (no copy — use carefully)
std::vector<double>* ptr = element->getComponentPtr("activation");

// List all available component names
std::vector<std::string> names = element->getComponentList();
```

Common component names:

| Component | Elements that have it |
|---|---|
| `"activation"` | `NeuralField` |
| `"output"` | All elements |
| `"input"` | `NeuralField`, kernels |
| `"weights"` | `FieldCoupling`, `GaussFieldCoupling` |

---

## Inputs and outputs

Elements are connected in a directed graph. Each connection specifies a **source element** and the **component** of that source to read from.

```cpp
// Connect elementB's "output" into elementA
elementA->addInput(elementB);

// Connect a specific component
elementA->addInput(elementB, "activation");

// Remove connections
elementA->removeInput("elementB name");
elementA->removeInputs();   // remove all

// Query connections
bool connected = elementA->hasInput("elementB name", "output");
auto inputs    = elementA->getInputs();          // vector of shared_ptr
auto inputMap  = elementA->getInputsAndComponents();  // map: element -> component name
auto outputs   = elementA->getOutputs();
```

Each `step()` call accumulates all registered input components into the element's `"input"` buffer before computing its own dynamics.

---

## Querying element properties

```cpp
int id             = element->getUniqueIdentifier();
std::string name   = element->getUniqueName();
ElementLabel label = element->getLabel();
int size           = element->getSize();       // number of positions
double stepSize    = element->getStepSize();   // d_x
ElementCommonParameters cp = element->getElementCommonParameters();
```