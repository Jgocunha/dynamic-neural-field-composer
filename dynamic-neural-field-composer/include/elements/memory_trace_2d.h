#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>

#include "element.h"

namespace dnf_composer::element
{
	/**
	 * @brief Parameters governing MemoryTrace2D build-up and decay dynamics.
	 *
	 * Identical to MemoryTraceParameters — the same three scalars govern
	 * the leaky-integrator for every cell in the 2D field.
	 * @ingroup elements
	 */
	struct MemoryTrace2DParameters final : ElementSpecificParameters
	{
		double tauBuild;   ///< Time constant for trace accumulation (ms).
		double tauDecay;   ///< Time constant for trace decay (ms).
		double threshold;  ///< Minimum input value above which the trace accumulates.

		explicit MemoryTrace2DParameters(double tauBuild = 100.0,
		                                 double tauDecay = 1000.0,
		                                 double threshold = 0.5)
			: tauBuild(tauBuild), tauDecay(tauDecay), threshold(threshold)
		{}

		bool operator==(const MemoryTrace2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(tauBuild  - other.tauBuild)  < epsilon &&
			       std::abs(tauDecay  - other.tauDecay)  < epsilon &&
			       std::abs(threshold - other.threshold) < epsilon;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "TauBuild: "  << tauBuild  << ", "
				<< "TauDecay: "  << tauDecay  << ", "
				<< "Threshold: " << threshold << "]";
			return result.str();
		}
	};

	/**
	 * @brief Persistent 2D spatial memory that accumulates and decays field activity.
	 *
	 * Applies the same per-cell leaky-integrator logic as MemoryTrace to every
	 * position of a 2D neural field: while input[i] exceeds @c threshold the trace
	 * grows with time constant @c tauBuild; below threshold it decays with
	 * @c tauDecay.
	 *
	 * @param elementCommonParameters  Common parameters (id, 2D dimensions).
	 * @param parameters               Trace dynamics parameters.
	 * @ingroup elements
	 */
	class MemoryTrace2D final : public Element
	{
	private:
		MemoryTrace2DParameters parameters;
	public:
		MemoryTrace2D(const ElementCommonParameters& elementCommonParameters,
		              const MemoryTrace2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		[[nodiscard]] std::string toString() const override;
		[[nodiscard]] std::shared_ptr<Element> clone() const override;

		void setParameters(const MemoryTrace2DParameters& parameters);
		[[nodiscard]] MemoryTrace2DParameters getParameters() const;
	};
}
