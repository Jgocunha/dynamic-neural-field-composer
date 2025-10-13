#pragma once

#include <array>

#include "tools/math.h"
#include "kernel.h"


namespace dnf_composer::element
{
	struct KernelCouplingParameters : ElementSpecificParameters
	{
		double width;
		double amplitude;
		double amplitudeGlobal;
		bool circular;
		bool normalized;

		// New parameters for different dimensionalities
		std::optional<int> inputSize = std::nullopt;  // If not set, uses commonParameters size
		std::optional<int> outputSize = std::nullopt; // If not set, uses commonParameters size


		explicit KernelCouplingParameters(double width = 3.0, double amp = 3.0, double ampGlobal = -0.01,
			bool circular = true, bool normalized = true, int inputSize = 0, int outputSize = 0)
			: width(width), amplitude(amp), amplitudeGlobal(ampGlobal),
				circular(circular), normalized(normalized),
				inputSize(inputSize), outputSize(outputSize)
		{}

		bool operator==(const KernelCouplingParameters& other) const {
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

	class KernelCoupling : public Kernel
	{
	private:
		KernelCouplingParameters parameters;
		int inputDimension, outputDimension;
	public:
		KernelCoupling(const ElementCommonParameters& elementCommonParameters,
			KernelCouplingParameters parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const KernelCouplingParameters& gk_parameters);
		KernelCouplingParameters getParameters() const;
	};
}
