#pragma once
#include "elements/element.h"

namespace dnf_composer::element
{
	/// @brief Interpolation method used when resampling the input signal.
	/// @ingroup elements
	enum class ResizeInterpolation
	{
		NEAREST, ///< Nearest-neighbour — picks the closest input sample.
		LINEAR,  ///< Linear interpolation between the two bounding input samples (default).
		CUBIC    ///< Catmull-Rom cubic spline — smoother than linear, useful when up-sampling.
	};

	/// @brief Parameters for a Resize element.
	/// @ingroup elements
	struct ResizeParameters final : ElementSpecificParameters
	{
		int                outputSize    = 100;                        ///< Number of discrete positions in the output vector.
		double             outputStep    = 1.0;                        ///< Spatial step size associated with the output field.
		ResizeInterpolation interpolation = ResizeInterpolation::LINEAR; ///< Resampling algorithm.

		ResizeParameters() = default;

		/// @brief Construct a ResizeParameters set.
		/// @param outputSize     Desired number of output positions.
		/// @param outputStep     Spatial step size of the output field (default 1.0).
		/// @param interpolation  Resampling algorithm (default LINEAR).
		explicit ResizeParameters(int outputSize, double outputStep = 1.0,
			ResizeInterpolation interpolation = ResizeInterpolation::LINEAR);

		bool operator==(const ResizeParameters&) const = default;
		std::string toString() const override;
	};

	/// @brief Resamples a 1D signal from one spatial resolution to another.
	///
	/// `Resize` acts as a resolution bridge between two DNF elements that operate
	/// at different spatial sizes. The element's declared size (stored in
	/// `commonParameters.dimensionParameters`) is the **input** size, so the
	/// framework's `addInput()` size guard accepts upstream elements of that size.
	/// The output vector is independently sized to `ResizeParameters::outputSize`
	/// and is populated each `step()` call using the chosen interpolation method.
	///
	/// Three interpolation methods are supported:
	/// - **Nearest** — zero-order hold; fastest, introduces staircase artefacts.
	/// - **Linear** — first-order interpolation; smooth and suitable for most cases.
	/// - **Cubic** — Catmull-Rom spline; best quality, especially when up-sampling.
	///
	/// Typical use:
	/// @code
	/// // Bridge a field of 200 positions down to 100 positions.
	/// auto resize = std::make_shared<Resize>(
	///     ElementCommonParameters{ "rs", ElementDimensions{ 200, 1.0 } },
	///     ResizeParameters{ 100, 1.0, ResizeInterpolation::LINEAR });
	///
	/// resize->addInput(field200, "output");
	/// field100->addInput(resize, "output");
	/// @endcode
	///
	/// @ingroup elements
	class Resize final : public Element
	{
		ResizeParameters parameters;
	public:
		/// @brief Construct a Resize element.
		/// @param common   Name, label, and **input** spatial dimensions.
		/// @param params   Output size, step, and interpolation method.
		Resize(const ElementCommonParameters& common, ResizeParameters params);

		/// @brief Resize the output buffer and zero both input and output components.
		void   init() override;

		/// @brief Accumulate inputs then resample into the output buffer.
		void   step(double t, double deltaT) override;

		std::shared_ptr<Element> clone() const override;
		std::string toString() const override;

		/// @brief Update output size, step, and interpolation; calls init() internally.
		void setParameters(const ResizeParameters& params);

		/// @brief Return a copy of the current parameters.
		ResizeParameters getParameters() const;
	};
}
