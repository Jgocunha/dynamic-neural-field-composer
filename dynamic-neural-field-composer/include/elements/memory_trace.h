#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>

#include "element.h"


namespace dnf_composer::element
{
	/// @brief Parameters governing MemoryTrace build-up and decay dynamics.
	/// @ingroup elements
	struct MemoryTraceParameters final : ElementSpecificParameters
	{
		double tauBuild;   ///< Time constant for trace accumulation (ms); smaller = faster build.
		double tauDecay;   ///< Time constant for trace decay (ms); larger = longer retention.
		double threshold;  ///< Minimum input value above which the trace accumulates.

		/// @brief Construct MemoryTrace parameters.
		/// @param tauBuild   Build time constant (default 100 ms).
		/// @param tauDecay   Decay time constant (default 1000 ms).
		/// @param threshold  Accumulation threshold (default 0.5).
		explicit MemoryTraceParameters(const double tauBuild = 100.0,
			const double tauDecay = 1000.0, const double threshold = 0.5)
			: tauBuild(tauBuild), tauDecay(tauDecay), threshold(threshold)
		{}

		bool operator==(const MemoryTraceParameters& other) const
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
				<< "TauBuild: " << tauBuild << ", "
				<< "TauDecay: " << tauDecay << ", "
				<< "Threshold: " << threshold
				<< "]";
			return result.str();
		}
	};

	/// @brief Persistent spatial memory that accumulates and decays field activity.
	///
	/// A MemoryTrace integrates above-threshold input from a NeuralField over time:
	/// while input exceeds @c threshold the trace grows with time constant @c tauBuild;
	/// below threshold it decays with time constant @c tauDecay. When fed back through
	/// a GaussKernel into the originating field, the trace sustains a localized peak
	/// even after the original stimulus is removed — implementing spatial working memory.
	///
	/// @ingroup elements
	class MemoryTrace final : public Element
	{
	private:
		MemoryTraceParameters parameters;
	public:
		/// @brief Construct a MemoryTrace element.
		/// @param elementCommonParameters  Name, label, and spatial dimensions.
		/// @param parameters               Trace dynamics parameters.
		MemoryTrace(const ElementCommonParameters& elementCommonParameters,
			const MemoryTraceParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const MemoryTraceParameters& parameters);
		MemoryTraceParameters getParameters() const;
	};
}
