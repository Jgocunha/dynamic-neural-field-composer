#pragma once

#include <sstream>

#include "element.h"


namespace dnf_composer::element
{
	/// @brief Parameters for StimulusSum. Empty: the element has no tunable
	/// dynamics beyond summing its connected inputs.
	/// @ingroup elements
	struct StimulusSumParameters final : ElementSpecificParameters
	{
		StimulusSumParameters() = default;

		bool operator==(const StimulusSumParameters& other) const
		{
			return true;
		}

		[[nodiscard]] std::string toString() const override
		{
			return "Parameters: [none]";
		}
	};

	/// @brief Passive aggregator that outputs the element-wise sum of an arbitrary
	/// number of same-size inputs.
	///
	/// StimulusSum performs no transform, applies no kernel, and has no internal
	/// dynamics: it exists so several stimuli feeding a field (e.g. multiple
	/// GaussStimulus elements) can be inspected as one combined signal, without
	/// conflating that signal with the field's own recurrent/kernel feedback and
	/// without plotting every stimulus separately.
	///
	/// @ingroup elements
	class StimulusSum final : public Element
	{
	private:
		StimulusSumParameters parameters;
	public:
		/// @brief Construct a StimulusSum element.
		/// @param elementCommonParameters  Name, label, and spatial dimensions.
		/// @param parameters               StimulusSum parameters (currently empty).
		StimulusSum(const ElementCommonParameters& elementCommonParameters,
			StimulusSumParameters parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const StimulusSumParameters& parameters);
		StimulusSumParameters getParameters() const;
	};
}
