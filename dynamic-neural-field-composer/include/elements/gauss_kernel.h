#pragma once

#include <array>

#include "tools/math.h"
#include "kernel.h"


namespace dnf_composer
{
	namespace element
	{
		struct GaussKernelParameters : ElementSpecificParameters
		{
			double width;
			double amplitude;
			double amplitudeGlobal;
			bool circular;
			bool normalized;

			explicit GaussKernelParameters(double width = 3.0, double amp = 3.0, double ampGlobal = -0.01,
				bool circular = true, bool normalized = true)
				: width(width), amplitude(amp), amplitudeGlobal(ampGlobal),
					circular(circular), normalized(normalized)
			{}

			bool operator==(const GaussKernelParameters& other) const {
				constexpr double epsilon = 1e-6;

				return std::abs(width - other.width) < epsilon &&
					std::abs(amplitude - other.amplitude) < epsilon &&
					std::abs(amplitudeGlobal - other.amplitudeGlobal) < epsilon &&
					circular == other.circular &&
					normalized == other.normalized;
			}

			[[nodiscard]] std::string toString() const override
			{
				std::ostringstream result;
				result << std::fixed << std::setprecision(2); // Ensures numbers have 2 decimal places
				result << "Parameters: ["
					<< "Width: " << width << ", "
					<< "Amplitude: " << amplitude << ", "
					<< "Amplitude global: " << amplitudeGlobal << ", "
					<< "Circular: " << (circular ? "true" : "false") << ", "
					<< "Normalized: " << (normalized ? "true" : "false")
					<< "]";
				return result.str();
			}
		};

		class GaussKernel : public Kernel
		{
		private:
			GaussKernelParameters parameters;
		public:
			GaussKernel(const ElementCommonParameters& elementCommonParameters, GaussKernelParameters parameters);

			void init() override;
			void step(double t, double deltaT) override;
			std::string toString() const override;
			std::shared_ptr<Element> clone() const override;

			void setParameters(const GaussKernelParameters& gk_parameters);
			GaussKernelParameters getParameters() const;
		};
	}
}