#pragma once

#include <sstream>

#include "tools/math.h"
#include "element.h"
#include "collapse.h" // for ProjectionAxis and ProjectionAxisToString

namespace dnf_composer::element
{
	/// @brief Parameters for an Expand (1D -> 2D) element.
	/// @ingroup elements
	struct ExpandParameters final : ElementSpecificParameters
	{
		ProjectionAxis broadcastProfileAxis; ///< Axis the 1D profile maps to (it is repeated along the other).
		ElementDimensions inputDimensions;   ///< Spatial dimensions of the 1D source field.

		/// @brief Construct Expand parameters.
		/// @param broadcastProfileAxis  Axis the 1D profile lies along (default X -> repeated over Y).
		/// @param inputDimensions       Dimensions of the 1D source field.
		explicit ExpandParameters(const ProjectionAxis broadcastProfileAxis = ProjectionAxis::X,
			const ElementDimensions& inputDimensions = ElementDimensions{ 100, 1.0 })
			: broadcastProfileAxis(broadcastProfileAxis), inputDimensions(inputDimensions)
		{}

		bool operator==(const ExpandParameters& other) const
		{
			return broadcastProfileAxis == other.broadcastProfileAxis &&
				inputDimensions == other.inputDimensions;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << "Parameters: ["
				<< "Profile axis: " << ProjectionAxisToString.at(broadcastProfileAxis) << ", "
				<< "Input field dimensions: " << inputDimensions.toString()
				<< "]";
			return result.str();
		}
	};

	/// @brief Expands a 1D input field to a 2D output by broadcasting along one axis.
	///
	/// On each @c step(), Expand reads its 1D input field's "output" component (size N)
	/// and writes a y-major 2D output (Nx x Ny) in which the 1D profile lies along the
	/// configured axis and is repeated along the other (a "ridge"). The profile-axis
	/// size must equal N. Useful for driving a 2D field from a 1D feature profile.
	/// Because the input and output dimensionalities differ, Expand overrides
	/// @c addInput() to size the "input" component to the source's size and accepts a
	/// single input.
	///
	/// @ingroup elements
	class Expand final : public Element
	{
	private:
		ExpandParameters parameters;
	public:
		/// @brief Construct an Expand element.
		/// @param elementCommonParameters  Name, label, and 2D output dimensions.
		/// @param parameters               Expand-specific parameters.
		Expand(const ElementCommonParameters& elementCommonParameters,
			const ExpandParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		void addInput(const std::shared_ptr<Element>& inputElement,
			const std::string& inputComponent = "output") override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		/// @brief Resize the 1D input field dimensions and rebuild the input buffer.
		void changeInputDimensions(const ElementDimensions& newInputDimensions);

		void setParameters(const ExpandParameters& parameters);
		[[nodiscard]] ExpandParameters getParameters() const;
	};
}
