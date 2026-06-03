#pragma once

#include <map>
#include <sstream>

#include "tools/math.h"
#include "element.h"

//https://github.com/stevenlovegrove/Pangolin/issues/352
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace dnf_composer::element
{
	/// @brief Reduction applied when collapsing one axis of a 2D field to 1D.
	/// @ingroup elements
	enum class CompressionType : int
	{
		SUM,     ///< Sum along the collapsed axis.
		AVERAGE, ///< Mean along the collapsed axis.
		MAXIMUM, ///< Maximum along the collapsed axis.
		MINIMUM  ///< Minimum along the collapsed axis.
	};

	/// @brief Maps CompressionType values to human-readable strings.
	inline const std::map<CompressionType, std::string> CompressionTypeToString = {
		{CompressionType::SUM, "sum"},
		{CompressionType::AVERAGE, "average"},
		{CompressionType::MAXIMUM, "maximum"},
		{CompressionType::MINIMUM, "minimum"}
	};

	/// @brief Spatial axis a projection acts on.
	/// @ingroup elements
	enum class ProjectionAxis : int
	{
		X, ///< The x-axis.
		Y  ///< The y-axis.
	};

	/// @brief Maps ProjectionAxis values to human-readable strings.
	inline const std::map<ProjectionAxis, std::string> ProjectionAxisToString = {
		{ProjectionAxis::X, "x"},
		{ProjectionAxis::Y, "y"}
	};

	/// @brief Maps a CompressionType to the corresponding math reduction op.
	inline tools::math::ReduceOp toReduceOp(const CompressionType type)
	{
		switch (type)
		{
		case CompressionType::SUM:     return tools::math::ReduceOp::SUM;
		case CompressionType::AVERAGE: return tools::math::ReduceOp::AVERAGE;
		case CompressionType::MAXIMUM: return tools::math::ReduceOp::MAXIMUM;
		case CompressionType::MINIMUM: return tools::math::ReduceOp::MINIMUM;
		}
		return tools::math::ReduceOp::SUM;
	}

	/// @brief Parameters for a Collapse (2D -> 1D) element.
	/// @ingroup elements
	struct CollapseParameters final : ElementSpecificParameters
	{
		CompressionType compression;       ///< Reduction applied along the collapsed axis.
		ProjectionAxis keepAxis;           ///< Axis kept in the 1D output (the other is collapsed).
		ElementDimensions inputDimensions; ///< Spatial dimensions of the 2D source field.

		/// @brief Construct Collapse parameters.
		/// @param compression      Reduction along the collapsed axis (default SUM).
		/// @param keepAxis         Axis kept in the output (default X -> collapse over Y).
		/// @param inputDimensions  Dimensions of the 2D source field.
		explicit CollapseParameters(const CompressionType compression = CompressionType::SUM,
			const ProjectionAxis keepAxis = ProjectionAxis::X,
			const ElementDimensions& inputDimensions = ElementDimensions{ 100, 100, 1.0, 1.0 })
			: compression(compression), keepAxis(keepAxis), inputDimensions(inputDimensions)
		{}

		bool operator==(const CollapseParameters& other) const
		{
			return compression == other.compression &&
				keepAxis == other.keepAxis &&
				inputDimensions == other.inputDimensions;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << "Parameters: ["
				<< "Compression: " << CompressionTypeToString.at(compression) << ", "
				<< "Keep axis: " << ProjectionAxisToString.at(keepAxis) << ", "
				<< "Input field dimensions: " << inputDimensions.toString()
				<< "]";
			return result.str();
		}
	};

	/// @brief Collapses a 2D input field to a 1D output by reducing along one axis.
	///
	/// On each @c step(), Collapse reads its 2D input field's "output" component
	/// (a y-major flattened Nx x Ny matrix) and reduces it along the dropped axis
	/// using the configured @c CompressionType, producing a 1D output the size of the
	/// kept axis. Useful for driving a 1D field from the marginal of a 2D field.
	/// Because the input and output dimensionalities differ, Collapse overrides
	/// @c addInput() to size the "input" component to the source's size and accepts a
	/// single input.
	///
	/// @ingroup elements
	class Collapse final : public Element
	{
	private:
		CollapseParameters parameters;
		std::vector<double> scratch; ///< Reduction buffer (kept-axis sized); copied into "output".
	public:
		/// @brief Construct a Collapse element.
		/// @param elementCommonParameters  Name, label, and 1D output dimensions.
		/// @param parameters               Collapse-specific parameters.
		Collapse(const ElementCommonParameters& elementCommonParameters,
			const CollapseParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		void addInput(const std::shared_ptr<Element>& inputElement,
			const std::string& inputComponent = "output") override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		/// @brief Resize the 2D input field dimensions and rebuild the input buffer.
		void changeInputDimensions(const ElementDimensions& newInputDimensions);

		void setParameters(const CollapseParameters& parameters);
		[[nodiscard]] CollapseParameters getParameters() const;
	};
}
