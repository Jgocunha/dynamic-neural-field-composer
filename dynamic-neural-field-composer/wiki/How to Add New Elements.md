# How to Add a New Element

This guide covers every step required to add a new element to dnf-composer.
Complete the steps in order and keep the element's 1D/2D counterpart as a reference.

---

## Step 1 — Create the header

**File:** `include/elements/your_element.h`

Declare the parameter struct and the class. Every element follows this pattern exactly:

```cpp
#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>

#include "element.h"

namespace dnf_composer::element
{
    /**
     * @brief Parameters for YourElement.
     *
     * Describe what the parameters control and their units/ranges.
     */
    struct YourElementParameters final : ElementSpecificParameters
    {
        double param1;
        bool   flag1;

        explicit YourElementParameters(double param1 = 1.0, bool flag1 = true)
            : param1(param1), flag1(flag1)
        {}

        bool operator==(const YourElementParameters& other) const
        {
            constexpr double epsilon = 1e-6;
            return std::abs(param1 - other.param1) < epsilon
                && flag1 == other.flag1;
        }

        [[nodiscard]] std::string toString() const override
        {
            std::ostringstream result;
            result << std::fixed << std::setprecision(2);
            result << "Parameters: [Param1: " << param1
                   << ", Flag1: " << (flag1 ? "true" : "false") << "]";
            return result.str();
        }
    };

    /**
     * @brief One-line description of what this element does in DFT.
     *
     * Longer description: the DFT role, typical connections, and any
     * implementation notes (e.g., separable kernel decomposition).
     *
     * @param elementCommonParameters  Common parameters (id, dimensions).
     * @param parameters               Element-specific parameters.
     */
    class YourElement final : public Element
    {
    private:
        YourElementParameters parameters;
        // declare any private working arrays here
    public:
        YourElement(const ElementCommonParameters& elementCommonParameters,
                    const YourElementParameters& parameters);

        void init() override;
        void step(double t, double deltaT) override;
        [[nodiscard]] std::string toString() const override;
        [[nodiscard]] std::shared_ptr<Element> clone() const override;

        void setParameters(const YourElementParameters& parameters);
        [[nodiscard]] YourElementParameters getParameters() const;
    };
}
```

---

## Step 2 — Implement the source

**File:** `src/elements/your_element.cpp`

Required method contracts:

| Method | Contract |
|---|---|
| **constructor** | Call `Element(commonParams, specificParams)`. Assign `parameters`. |
| **`init()`** | Resize and zero-initialise every component vector. |
| **`step(t, deltaT)`** | Pull inputs with `getInput("output")`, compute, write to the `"output"` component. |
| **`toString()`** | Return a human-readable summary of the element and its current parameters. |
| **`clone()`** | `return std::make_shared<YourElement>(*this);` |
| **`setParameters()`** | Assign the new parameters, then call `init()` to re-initialise working buffers. |
| **`getParameters()`** | Return the current `parameters` copy. |

Typical `init()` body — resize every component used in `step()`:

```cpp
void YourElement::init()
{
    // "output" is the standard component name kernels and fields pull from.
    components["output"].resize(commonParameters.dimensionParameters.size, 0.0);
    // add more components if needed, e.g.:
    // components["kernel"].resize(...);
    Element::init();
}
```

---

## Step 3 — Register the label

**File:** `include/element_parameters/element_parameters.h`

**3a.** Add to the `ElementLabel` enum before the closing brace:

```cpp
enum ElementLabel : int
{
    // ... existing labels ...
    NORMAL_NOISE_2D,
    YOUR_ELEMENT_NAME,   // <-- add here
};
```

**3b.** Add to the `ElementLabelToString` map:

```cpp
inline const std::map<ElementLabel, std::string> ElementLabelToString = {
    // ... existing entries ...
    { YOUR_ELEMENT_NAME, "your element name" },
};
```

---

## Step 4 — Register in the factory

**File:** `include/elements/element_factory.h`

Add the include near the end of the existing list:

```cpp
#include "elements/your_element.h"
```

**File:** `src/elements/element_factory.cpp`

**4a.** In `ElementFactory::setupElementCreators()`, add:

```cpp
elementCreators[ElementLabel::YOUR_ELEMENT_NAME] =
    [](const ElementCommonParameters& cp, const ElementSpecificParameters& sp)
    {
        const auto params = dynamic_cast<const YourElementParameters*>(&sp);
        return std::make_shared<YourElement>(cp, *params);
    };
```

**4b.** In `ElementFactory::createElement(ElementLabel type)` switch, add before `case ElementLabel::UNINITIALIZED`:

```cpp
case ElementLabel::YOUR_ELEMENT_NAME:
    return creator->second(ElementCommonParameters(type), YourElementParameters());
```

---

## Step 5 — Add to ElementWindow (parameter editing UI)

**File:** `include/user_interface/element_window.h`

Add the include at the top:

```cpp
#include "elements/your_element.h"
```

Add a private static method declaration:

```cpp
static void modifyElementYourElement(const std::shared_ptr<element::Element>& element);
```

**File:** `src/user_interface/element_window.cpp`

**5a.** Panel height — add a case to the `PanelHeightFor` lambda (use `h(N)` where N is the number of parameter rows):

```cpp
case element::ElementLabel::YOUR_ELEMENT_NAME: return h(3);
```

**5b.** Dispatch — add a case to `switchElementToModify()`:

```cpp
case element::ElementLabel::YOUR_ELEMENT_NAME:
    modifyElementYourElement(element);
    break;
```

**5c.** Implement the modify method following the `modifyElementGaussKernel2D` pattern:

```cpp
void ElementWindow::modifyElementYourElement(const std::shared_ptr<element::Element>& element)
{
    const float ui = ImGui::GetIO().FontGlobalScale;
    const auto elem = std::dynamic_pointer_cast<element::YourElement>(element);
    element::YourElementParameters p = elem->getParameters();

    auto param1 = static_cast<float>(p.param1);
    bool flag1  = p.flag1;

    std::string label = "##" + element->getUniqueName() + "Param1";
    ImGui::SetNextItemWidth(150.0f * ui);
    ImGui::DragFloat(label.c_str(), &param1, 0.1f, 0.0f, 30.0f);
    ImGui::SameLine(); ImGui::Text("Param1");

    label = "##" + element->getUniqueName() + "Flag1";
    ImGui::Checkbox(label.c_str(), &flag1);
    ImGui::SameLine(); ImGui::Text("Flag1");

    static constexpr double epsilon = 1e-6;
    if (std::abs(param1 - static_cast<float>(p.param1)) > epsilon || flag1 != p.flag1)
    {
        p.param1 = param1;
        p.flag1  = flag1;
        elem->setParameters(p);
    }
}
```

**5d.** Color — add a case to `getColorForElementType()`:

```cpp
case element::ElementLabel::YOUR_ELEMENT_NAME:
    return ImVec4(0.5f, 0.6f, 0.7f, 1.0f);
```

**5e.** Display name — add a case to `getElementTypeDisplayName()`:

```cpp
case element::ElementLabel::YOUR_ELEMENT_NAME:
    return "Your Element Display Name";
```

---

## Step 6 — Add to SimulationWindow (element creation UI)

**File:** `src/user_interface/simulation_window.cpp`

Add a case in `renderAddElementCard()`. Follow the `GAUSS_KERNEL_2D` block as a template.
Use `static` local variables so the widget state persists between frames:

```cpp
case element::ElementLabel::YOUR_ELEMENT_NAME:
{
    static char   id[CHAR_SIZE] = "your element";
    static int    x_max = 100;
    static double d_x   = 1.0;
    static double param1 = 1.0;
    static bool   flag1  = true;

    ImGui::InputTextWithHint("ID", "enter text here", id, IM_ARRAYSIZE(id));
    ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
    ImGui::InputInt("Size",    &x_max,  0, 0);
    ImGui::InputDouble("Step", &d_x,    0.0, 0.0, "%.2f");
    ImGui::InputDouble("Param1", &param1, 0.0, 0.0, "%.2f");
    ImGui::Checkbox("Flag1", &flag1);
    ImGui::PopItemWidth();

    if (addRequested)
    {
        const element::YourElementParameters p(param1, flag1);
        const element::ElementCommonParameters common{ std::string(id),
            element::ElementDimensions{ x_max, d_x } };
        simulation->addElement(std::make_shared<element::YourElement>(common, p));
    }
    break;
}
```

For 2D elements use `ElementDimensions{ x_max, y_max, d_x, d_y }` and add matching `y_max`/`d_y` fields.

---

## Step 7 — Add to NodeGraphWindow

**File:** `src/user_interface/node_graph_window.cpp`

**7a.** Column assignment — add a case to `getColumnForElement()`.
The column determines the initial horizontal position in the node graph:

| Column | Element types |
|--------|--------------|
| 0 | Sources (stimuli, noise) |
| 1 | Kernels |
| 2 | Couplings |
| 3 | Fields |

```cpp
// Example: kernel goes in column 1
case element::ElementLabel::YOUR_ELEMENT_NAME:
    return 1;
```

**7b.** 2D plot size — if the element output is a 2D matrix, add its label to both `is2DField` checks in `renderElementNode()` and `renderElementNodeConnections()`:

```cpp
const bool is2DField = (label == element::ElementLabel::NEURAL_FIELD_2D ||
                        label == element::ElementLabel::GAUSS_STIMULUS_2D ||
                        label == element::ElementLabel::GAUSS_KERNEL_2D  ||
                        label == element::ElementLabel::MEXICAN_HAT_KERNEL_2D ||
                        label == element::ElementLabel::NORMAL_NOISE_2D  ||
                        label == element::ElementLabel::YOUR_ELEMENT_NAME);  // <-- add
```

**7c.** Inspector panel — add a case to `renderNodeInspectorContent()` to show the parameters when a node is double-clicked:

```cpp
case element::ElementLabel::YOUR_ELEMENT_NAME:
{
    const auto e = std::dynamic_pointer_cast<element::YourElement>(element);
    const auto& p = e->getParameters();
    ImGui::Text("Param1: %.2f", p.param1);
    ImGui::Text("Flag1: %s",    p.flag1 ? "true" : "false");
    break;
}
```

---

## Step 8 — Update CMakeLists.txt

**File:** `dynamic-neural-field-composer/CMakeLists.txt`

Add the new source file to the library target's source list (find the block with the other element `.cpp` files):

```cmake
src/elements/your_element.cpp
```

---

## Step 9 — Update the public header

**File:** `include/dynamic-neural-field-composer.h`

Add the include near the other element headers:

```cpp
#include "elements/your_element.h"
```

---

## Step 10 — Write tests

**File:** `tests/elements/test_your_element.cpp`

Register in `tests/CMakeLists.txt` by adding the file to the test executable sources.

Every element test suite must cover:

| Test group | What to verify |
|---|---|
| Construction | Valid parameters do not throw; label matches |
| Initialisation | Component sizes equal the declared dimensions after `init()` |
| Step output | Numerical correctness: after one step with a known input, output values match a hand-computed reference |
| Parameter update | `setParameters()` followed by `step()` reflects the new values |
| Edge cases | Zero amplitude → all-zero output; circular vs. non-circular boundary behaviour |
| Clone | Cloned element produces identical output after the same step sequence |

Minimal structure:

```cpp
#include <gtest/gtest.h>
#include <memory>
#include "elements/your_element.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int x = 20)
{
    return ElementCommonParameters{ name, ElementDimensions(x, 1.0) };
}

TEST(YourElementConstruction, ValidDoesNotThrow)
{
    EXPECT_NO_THROW(YourElement(makeCP("ye"), YourElementParameters{}));
}

TEST(YourElementConstruction, LabelIsCorrect)
{
    YourElement ye(makeCP("ye"), YourElementParameters{});
    EXPECT_EQ(ye.getLabel(), ElementLabel::YOUR_ELEMENT_NAME);
}

TEST(YourElementStep, OutputSizeMatchesDimension)
{
    auto ye = std::make_shared<YourElement>(makeCP("ye", 20), YourElementParameters{});
    ye->init();
    ye->step(0.0, 1.0);
    EXPECT_EQ(static_cast<int>(ye->getComponent("output").size()), 20);
}

TEST(YourElementClone, CloneHasSameParameters)
{
    YourElement ye(makeCP("ye"), YourElementParameters{1.5, false});
    ye->init();
    const auto cloned = std::dynamic_pointer_cast<YourElement>(ye.clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->getParameters(), ye.getParameters());
}
```

---

## Step 11 — Write an example executable

**File:** `examples/ex_your_element.cpp`

Register in `examples/CMakeLists.txt`.


```cpp
#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"

int main()
{
    try
    {
        using namespace dnf_composer;

        const auto simulation    = std::make_shared<Simulation>("ex your element", 10.0, 0.0, 0.0);
        const auto visualization = std::make_shared<Visualization>(simulation);
        const Application app{ simulation, visualization };

        app.addWindow<user_interface::MainMenuBar>();
        app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

        // --- build the architecture ---
        const element::ElementCommonParameters cp{ "your element",
            element::ElementDimensions(100, 1.0) };
        const auto ye = std::make_shared<element::YourElement>(cp, element::YourElementParameters{});

        simulation->addElement(ye);
        visualization->plot({ {ye->getUniqueName(), "output"} });

        app.init();
        while (!app.hasGUIBeenClosed())
            app.step();
        app.close();
    }
    catch (const dnf_composer::Exception& ex)
    {
        log(dnf_composer::tools::logger::LogLevel::FATAL,
            "Exception: " + std::string(ex.what()),
            dnf_composer::tools::logger::LogOutputMode::CONSOLE);
        return static_cast<int>(ex.getErrorCode());
    }
    catch (const std::exception& ex)
    {
        log(dnf_composer::tools::logger::LogLevel::FATAL,
            "Exception caught: " + std::string(ex.what()),
            dnf_composer::tools::logger::LogOutputMode::CONSOLE);
        return 1;
    }
}
```

---

## Step 12 — Update README.md

Add the element to the elements table in the project README with:
- Name
- Dimensionality (1D / 2D / 1D+2D)
- One-line description

---

## Quick checklist

Copy this checklist into the PR description for each new element:

```
- [ ] include/elements/your_element.h
- [ ] src/elements/your_element.cpp
- [ ] Doxygen comments on the struct and class
- [ ] ElementLabel enum + ElementLabelToString entry
- [ ] ElementFactory: include, setupElementCreators, createElement switch
- [ ] ElementWindow: include, declare method, PanelHeightFor, switchElementToModify, modifyElement method, color, display name
- [ ] SimulationWindow: renderAddElementCard case
- [ ] NodeGraphWindow: getColumnForElement, is2DField (if 2D), renderNodeInspectorContent
- [ ] CMakeLists.txt: source file added
- [ ] include/dynamic-neural-field-composer.h: include added
- [ ] tests/elements/test_your_element.cpp + registered in tests/CMakeLists.txt
- [ ] examples/ex_your_element.cpp + registered in examples/CMakeLists.txt
- [ ] README.md elements table updated
```
