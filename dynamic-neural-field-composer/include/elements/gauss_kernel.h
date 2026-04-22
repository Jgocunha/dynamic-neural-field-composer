#pragma once

#include <array>
#include <optional>

#include "tools/math.h"
#include "kernel.h"


namespace dnf_composer::element
{
	struct GaussKernelParameters final : ElementSpecificParameters
	{
		double width;
		double amplitude;
		double amplitudeGlobal;
		bool circular;
		bool normalized;
		std::optional<ElementDimensions> outputFieldDimensions;

		explicit GaussKernelParameters(const double width = 3.0, const double amp = 3.0, const double ampGlobal = -0.01,
		                      const bool circular = true, const bool normalized = true,
		                      const std::optional<ElementDimensions>& outputDims = std::nullopt)
			: width(width), amplitude(amp), amplitudeGlobal(ampGlobal),
			  circular(circular), normalized(normalized),
			  outputFieldDimensions(outputDims)
		{}

		bool operator==(const GaussKernelParameters& other) const {
			constexpr double epsilon = 1e-6;

			return std::abs(width - other.width) < epsilon &&
				std::abs(amplitude - other.amplitude) < epsilon &&
				std::abs(amplitudeGlobal - other.amplitudeGlobal) < epsilon &&
				circular == other.circular &&
				normalized == other.normalized &&
				outputFieldDimensions == other.outputFieldDimensions;
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
				<< "Normalized: " << (normalized ? "true" : "false");
			if (outputFieldDimensions.has_value())
				result << ", Output size: " << outputFieldDimensions->size;
			result << "]";
			return result.str();
		}
	};

	class GaussKernel final : public Kernel
	{
	private:
		GaussKernelParameters parameters;
		std::vector<double> scratchConvolution;
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
