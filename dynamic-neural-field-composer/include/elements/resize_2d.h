#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>

#include "tools/math.h"
#include "element.h"
#include "resize.h" // for InterpolationMethod and InterpolationMethodToString

namespace dnf_composer::element
{
	/// @brief Parameters for a 2D Resize element.
	/// @ingroup elements
	struct Resize2DParameters final : ElementSpecificParameters
	{
		InterpolationMethod method;        ///< Interpolation method used for resampling.
		ElementDimensions inputDimensions; ///< Spatial dimensions of the source (input) field.

		/// @brief Construct Resize2D parameters.
		/// @param method            Interpolation method (default LINEAR).
		/// @param inputDimensions   Dimensions of the source field (drives the input buffer size).
		explicit Resize2DParameters(const InterpolationMethod method = InterpolationMethod::LINEAR,
			const ElementDimensions& inputDimensions = ElementDimensions{ 100, 100, 1.0, 1.0 })
			: method(method), inputDimensions(inputDimensions)
		{}

		bool operator==(const Resize2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return method == other.method &&
				inputDimensions.size_x == other.inputDimensions.size_x &&
				inputDimensions.size_y == other.inputDimensions.size_y &&
				std::abs(inputDimensions.d_x - other.inputDimensions.d_x) < epsilon &&
				std::abs(inputDimensions.d_y - other.inputDimensions.d_y) < epsilon;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << "Parameters: ["
				<< "Interpolation method: " << InterpolationMethodToString.at(method) << ", "
				<< "Input field dimensions: " << inputDimensions.toString()
				<< "]";
			return result.str();
		}
	};

	/// @brief Resamples a 2D input field of size (Nx x Ny) to this element's output size (Mx x My).
	///
	/// On each @c step(), Resize2D reads its input field's "output" component
	/// (a y-major flattened Nx x Ny matrix) and resamples it to its own output size
	/// (Mx x My) using the configured @c InterpolationMethod. Resampling is performed
	/// separably: each row is resampled along x, then each column along y, reusing the
	/// 1D resampling helpers in tools::math. As with Resize, the input and output
	/// sizes differ, so @c addInput() is overridden to size the "input" component to the
	/// source's size.
	///
	/// @ingroup elements
	class Resize2D final : public Element
	{
	private:
		Resize2DParameters parameters;
		std::vector<double> scratch; ///< Intermediate buffer (Mx x Ny) for the separable pass.
	public:
		/// @brief Construct a Resize2D element.
		/// @param elementCommonParameters  Name, label, and output dimensions (Mx x My).
		/// @param parameters               Resize-specific parameters (method, input dimensions).
		Resize2D(const ElementCommonParameters& elementCommonParameters,
			const Resize2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		void addInput(const std::shared_ptr<Element>& inputElement,
			const std::string& inputComponent = "output") override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		/// @brief Resize the input field dimensions and rebuild the input buffer.
		/// Connections are not removed automatically — call removeInputs()/removeOutputs()
		/// first if needed (the UI does this before calling).
		void changeInputDimensions(const ElementDimensions& newInputDimensions);

		void setParameters(const Resize2DParameters& parameters);
		[[nodiscard]] Resize2DParameters getParameters() const;
	};
}
