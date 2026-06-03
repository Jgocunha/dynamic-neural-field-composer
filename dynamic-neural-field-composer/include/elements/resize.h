#pragma once

#include <cmath>
#include <map>
#include <sstream>
#include <iomanip>

#include "tools/math.h"
#include "element.h"

namespace dnf_composer::element
{
	/// @brief Interpolation method used when resampling an input field to a new size.
	/// @ingroup elements
	enum class InterpolationMethod : int
	{
		LINEAR,  ///< Piecewise linear interpolation.
		NEAREST, ///< Nearest-neighbour sampling.
		CUBIC    ///< Catmull-Rom cubic spline interpolation.
	};

	/// @brief Maps InterpolationMethod values to human-readable strings.
	inline const std::map<InterpolationMethod, std::string> InterpolationMethodToString = {
		{InterpolationMethod::LINEAR, "linear"},
		{InterpolationMethod::NEAREST, "nearest"},
		{InterpolationMethod::CUBIC, "cubic"}
	};

	/// @brief Parameters for a 1D Resize element.
	/// @ingroup elements
	struct ResizeParameters final : ElementSpecificParameters
	{
		InterpolationMethod method;        ///< Interpolation method used for resampling.
		ElementDimensions inputDimensions; ///< Spatial dimensions of the source (input) field.

		/// @brief Construct Resize parameters.
		/// @param method            Interpolation method (default LINEAR).
		/// @param inputDimensions   Dimensions of the source field (drives the input buffer size).
		explicit ResizeParameters(const InterpolationMethod method = InterpolationMethod::LINEAR,
			const ElementDimensions& inputDimensions = ElementDimensions{})
			: method(method), inputDimensions(inputDimensions)
		{}

		bool operator==(const ResizeParameters& other) const
		{
			return method == other.method &&
				inputDimensions == other.inputDimensions;
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

	/// @brief Resamples a 1D input field of size N to this element's output size M.
	///
	/// On each @c step(), Resize reads its input field's "output" component (size N)
	/// and resamples it to its own output size M using the configured
	/// @c InterpolationMethod (linear, nearest, or cubic). The output size is the
	/// element's own declared dimension; the input size is taken from the connected
	/// source. Because the input and output sizes differ, Resize overrides
	/// @c addInput() to size the "input" component to the source's size.
	///
	/// @ingroup elements
	class Resize final : public Element
	{
	private:
		ResizeParameters parameters;
	public:
		/// @brief Construct a Resize element.
		/// @param elementCommonParameters  Name, label, and output dimensions (size M).
		/// @param parameters               Resize-specific parameters (method, input dimensions).
		Resize(const ElementCommonParameters& elementCommonParameters,
			const ResizeParameters& parameters);

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

		void setParameters(const ResizeParameters& parameters);
		[[nodiscard]] ResizeParameters getParameters() const;
	};
}
