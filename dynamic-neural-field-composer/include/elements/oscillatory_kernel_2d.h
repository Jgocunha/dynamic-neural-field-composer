#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>
#include <array>

#include "tools/math.h"
#include "kernel.h"

//https://github.com/stevenlovegrove/Pangolin/issues/352
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace dnf_composer::element
{
	/**
	 * @brief Parameters for OscillatoryKernel2D.
	 *
	 * Mirrors OscillatoryKernelParameters but without outputFieldDimensions,
	 * since this kernel always operates on a 2D field.
	 * zeroCrossings are clamped to [0, 1]; decay must be positive.
	 */
	struct OscillatoryKernel2DParameters final : ElementSpecificParameters
	{
		double amplitude;
		double decay;
		double zeroCrossings;
		double amplitudeGlobal;
		bool circular;
		bool normalized;

		explicit OscillatoryKernel2DParameters(double amplitude = 1.0, double decay = 0.08,
		                                       double zeroCrossings = 0.3,
		                                       double amplitudeGlobal = -0.01,
		                                       bool circular = true, bool normalized = true)
			: amplitude(amplitude), decay(decay), zeroCrossings(zeroCrossings),
			  amplitudeGlobal(amplitudeGlobal), circular(circular), normalized(normalized)
		{
			if (this->zeroCrossings < 0.0) this->zeroCrossings = 0.0;
			else if (this->zeroCrossings > 1.0) this->zeroCrossings = 1.0;
			if (this->decay <= 0.0) this->decay = 0.01;
		}

		bool operator==(const OscillatoryKernel2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(amplitude - other.amplitude) < epsilon &&
			       std::abs(decay - other.decay) < epsilon &&
			       std::abs(zeroCrossings - other.zeroCrossings) < epsilon &&
			       std::abs(amplitudeGlobal - other.amplitudeGlobal) < epsilon &&
			       circular == other.circular &&
			       normalized == other.normalized;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "Amplitude: " << amplitude << ", "
				<< "Decay: " << decay << ", "
				<< "Zero crossings: " << zeroCrossings << ", "
				<< "Amplitude global: " << amplitudeGlobal << ", "
				<< "Circular: " << (circular ? "true" : "false") << ", "
				<< "Normalized: " << (normalized ? "true" : "false") << "]";
			return result.str();
		}
	};

	/**
	 * @brief 2D oscillatory lateral-interaction kernel using separable convolution.
	 *
	 * Applies the same 1D oscillatory kernel independently along x and y via
	 * separable 2D convolution, producing an isotropic oscillatory interaction
	 * pattern on 2D neural fields. The effective 2D kernel is the outer product
	 * of the two 1D kernels. Suitable for architectures that require rhythmic
	 * lateral interactions in a 2D spatial domain.
	 *
	 * @param elementCommonParameters  Common parameters (id, 2D dimensions).
	 * @param parameters               Oscillatory kernel parameters.
	 */
	class OscillatoryKernel2D final : public Kernel
	{
	private:
		OscillatoryKernel2DParameters parameters;
		std::array<int, 2> kernelRange_x{};
		std::array<int, 2> kernelRange_y{};
		std::vector<int> extIndex_x;
		std::vector<int> extIndex_y;
		std::vector<double> kernel_1d_x;
		std::vector<double> kernel_1d_y;
		std::vector<double> scratchTmp_;
		std::vector<double> scratchConvolution_;
	public:
		OscillatoryKernel2D(const ElementCommonParameters& elementCommonParameters,
		                    const OscillatoryKernel2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		[[nodiscard]] std::string toString() const override;
		[[nodiscard]] std::shared_ptr<Element> clone() const override;

		void setParameters(const OscillatoryKernel2DParameters& parameters);
		[[nodiscard]] OscillatoryKernel2DParameters getParameters() const;
	};
}
