#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>
#include <optional>

#include "tools/math.h"
#include "kernel.h"

namespace dnf_composer::element
{
	struct GaussKernel2DParameters final : ElementSpecificParameters
	{
		double width;           // isotropic sigma (same for x and y)
		double amplitude;
		double amplitudeGlobal;
		bool circular;
		bool normalized;

		explicit GaussKernel2DParameters(double width = 3.0, double amplitude = 3.0,
		                                 double amplitudeGlobal = -0.01,
		                                 bool circular = true, bool normalized = true)
			: width(width), amplitude(amplitude), amplitudeGlobal(amplitudeGlobal),
			  circular(circular), normalized(normalized)
		{}

		bool operator==(const GaussKernel2DParameters& other) const
		{
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
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "Width: " << width << ", "
				<< "Amplitude: " << amplitude << ", "
				<< "Amplitude global: " << amplitudeGlobal << ", "
				<< "Circular: " << (circular ? "true" : "false") << ", "
				<< "Normalized: " << (normalized ? "true" : "false") << "]";
			return result.str();
		}
	};

	class GaussKernel2D final : public Kernel
	{
	private:
		GaussKernel2DParameters parameters;
		std::array<int, 2> kernelRange_x{};
		std::array<int, 2> kernelRange_y{};
		std::vector<int> extIndex_x;
		std::vector<int> extIndex_y;
		std::vector<double> kernel_1d_x;
		std::vector<double> kernel_1d_y;
		std::vector<double> scratchTmp_;
		std::vector<double> scratchConvolution_;
	public:
		GaussKernel2D(const ElementCommonParameters& elementCommonParameters,
		              const GaussKernel2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const GaussKernel2DParameters& parameters);
		GaussKernel2DParameters getParameters() const;
	};
}
