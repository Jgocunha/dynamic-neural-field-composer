#pragma once

#include <sstream>
#include <iomanip>
#include <array>

#include "tools/math.h"
#include "kernel.h"

namespace dnf_composer::element
{
	/**
	 * @brief Parameters for AsymmetricGaussKernel2D.
	 *
	 * Mirrors AsymmetricGaussKernelParameters but uses independent time-shift
	 * values for the x and y axes, allowing directional drift to be specified
	 * per dimension on a 2D field.
	 */
	struct AsymmetricGaussKernel2DParameters final : ElementSpecificParameters
	{
		double width;           ///< Gaussian standard deviation σ (same for both axes).
		double amplitude;       ///< Peak amplitude.
		double amplitudeGlobal; ///< Spatially uniform offset added after convolution.
		double timeShift_x;     ///< Spatial shift along x (positive = rightward drift).
		double timeShift_y;     ///< Spatial shift along y (positive = downward drift).
		bool circular;          ///< Enable circular (toroidal) convolution.
		bool normalized;        ///< Normalise the Gaussian before applying amplitude.

		explicit AsymmetricGaussKernel2DParameters(double width = 3.0, double amplitude = 3.0,
		                                            double amplitudeGlobal = 0.0,
		                                            double timeShift_x = 0.0,
		                                            double timeShift_y = 0.0,
		                                            bool circular = true, bool normalized = true)
			: width(width), amplitude(amplitude), amplitudeGlobal(amplitudeGlobal),
			  timeShift_x(timeShift_x), timeShift_y(timeShift_y),
			  circular(circular), normalized(normalized)
		{}

		bool operator==(const AsymmetricGaussKernel2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(width          - other.width)          < epsilon &&
			       std::abs(amplitude      - other.amplitude)      < epsilon &&
			       std::abs(amplitudeGlobal - other.amplitudeGlobal) < epsilon &&
			       std::abs(timeShift_x    - other.timeShift_x)    < epsilon &&
			       std::abs(timeShift_y    - other.timeShift_y)    < epsilon &&
			       circular  == other.circular &&
			       normalized == other.normalized;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "Width: "           << width          << ", "
				<< "Amplitude: "       << amplitude      << ", "
				<< "Amplitude global: "<< amplitudeGlobal << ", "
				<< "Time shift x: "    << timeShift_x    << ", "
				<< "Time shift y: "    << timeShift_y    << ", "
				<< "Circular: "  << (circular   ? "true" : "false") << ", "
				<< "Normalized: "<< (normalized  ? "true" : "false") << "]";
			return result.str();
		}
	};

	/**
	 * @brief 2D asymmetric Gaussian kernel that induces directional peak drift.
	 *
	 * Combines each 1D Gaussian with its spatial derivative (scaled by the
	 * corresponding time-shift parameter) to break symmetry in both x and y,
	 * producing a kernel whose outer product biases activity peaks toward higher
	 * (positive shift) or lower (negative shift) positions per axis.
	 * Applied via separable 2D convolution using pre-allocated scratch buffers.
	 *
	 * @param elementCommonParameters  Common parameters (id, 2D dimensions).
	 * @param parameters               Asymmetric kernel parameters.
	 */
	class AsymmetricGaussKernel2D final : public Kernel
	{
	private:
		AsymmetricGaussKernel2DParameters parameters;
		std::array<int, 2> kernelRange_x{};
		std::array<int, 2> kernelRange_y{};
		std::vector<int>    extIndex_x;
		std::vector<int>    extIndex_y;
		std::vector<double> kernel_1d_x;
		std::vector<double> kernel_1d_y;
		std::vector<double> scratchTmp_;
		std::vector<double> scratchConvolution_;
	public:
		AsymmetricGaussKernel2D(const ElementCommonParameters& elementCommonParameters,
		                        const AsymmetricGaussKernel2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		[[nodiscard]] std::string toString() const override;
		[[nodiscard]] std::shared_ptr<Element> clone() const override;

		void setParameters(const AsymmetricGaussKernel2DParameters& parameters);
		[[nodiscard]] AsymmetricGaussKernel2DParameters getParameters() const;
	};
}
