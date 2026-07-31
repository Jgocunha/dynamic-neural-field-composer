#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>
#include <numeric>

#include "tools/math.h"
#include "tools/fft_convolution.h"
#include "element.h"

namespace dnf_composer::element
{
	struct CorrelatedNormalNoise2DParameters final : ElementSpecificParameters
	{
		double amplitude;
		double width;
		bool circular;

		explicit CorrelatedNormalNoise2DParameters(double amplitude = 0.05,
		                                           double width = 1.0,
		                                           bool circular = true)
			: amplitude(amplitude), width(width), circular(circular)
		{}

		bool operator==(const CorrelatedNormalNoise2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(amplitude - other.amplitude) < epsilon &&
			       std::abs(width - other.width) < epsilon &&
			       circular == other.circular;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << "Parameters: ["
			       << "Amplitude: " << std::fixed << std::setprecision(4) << amplitude << ", "
			       << "Width: " << std::fixed << std::setprecision(2) << width << ", "
			       << "Circular: " << (circular ? "true" : "false") << "]";
			return result.str();
		}
	};

	class CorrelatedNormalNoise2D final : public Element
	{
	private:
		CorrelatedNormalNoise2DParameters parameters;
		// This element derives from Element, not Kernel, so it has no
		// cutOfFactor member; match Kernel::Kernel's default (kernel.cpp:14).
		static constexpr int kCutOfFactor = 5;
		std::array<int, 2> kernelRange_x{};
		std::array<int, 2> kernelRange_y{};
		std::vector<double> correlationKernel_x;
		std::vector<double> correlationKernel_y;
		std::vector<int>    extIndex_x;
		std::vector<int>    extIndex_y;
		std::vector<double> scratchTmp_;
		std::vector<double> scratchConv_;
		std::vector<double> whiteNoise_;       ///< Reusable white-noise buffer (avoids per-step alloc).
		tools::math::Conv2dScratch<double> scratch2d_;

		// Spectral (FFTW) path — used instead of the direct separable path above
		// when shouldUseSpectral2D (tools/fft_convolution.h) says the kernel is
		// wide enough and circular=true. Shared dispatch rule/member pair with
		// every other 2D convolution element (see MexicanHatKernel2D). The
		// convolution input (fresh white noise every step) is itself random, so
		// direct-vs-spectral comparisons must re-seed tools::math::seedNormal
		// before each run rather than compare live stochastic output.
		bool useFFT_ = false;
		tools::math::SpectralConvolver2D spectral_;
	public:
		CorrelatedNormalNoise2D(const ElementCommonParameters& elementCommonParameters,
		                        CorrelatedNormalNoise2DParameters  parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const CorrelatedNormalNoise2DParameters& parameters);
		CorrelatedNormalNoise2DParameters getParameters() const;
	};
}
