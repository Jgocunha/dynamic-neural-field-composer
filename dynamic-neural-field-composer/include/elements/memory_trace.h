#pragma once

#include <sstream>
#include <iomanip>

#include "element.h"

namespace dnf_composer
{
	namespace element
	{
		struct MemoryTraceParameters : ElementSpecificParameters
		{
			double tauBuild;    // time constant for building up the trace
			double tauDecay;    // time constant for decaying the trace
			double threshold;   // input threshold above which trace builds up

			MemoryTraceParameters(double tauBuild = 100.0, double tauDecay = 1000.0, double threshold = 0.5)
				: tauBuild(tauBuild), tauDecay(tauDecay), threshold(threshold)
			{}

			bool operator==(const MemoryTraceParameters& other) const
			{
				constexpr double epsilon = 1e-6;
				return std::abs(tauBuild  - other.tauBuild)  < epsilon &&
					   std::abs(tauDecay  - other.tauDecay)  < epsilon &&
					   std::abs(threshold - other.threshold) < epsilon;
			}

			std::string toString() const override
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

		class MemoryTrace : public Element
		{
		private:
			MemoryTraceParameters parameters;
		public:
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
}
