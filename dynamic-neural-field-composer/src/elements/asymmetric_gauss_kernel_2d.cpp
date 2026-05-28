// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/asymmetric_gauss_kernel_2d.h"

namespace dnf_composer::element
{
	AsymmetricGaussKernel2D::AsymmetricGaussKernel2D(
		const ElementCommonParameters& elementCommonParameters,
		const AsymmetricGaussKernel2DParameters& parameters)
		: Kernel(elementCommonParameters), parameters(parameters)
	{
		commonParameters.identifiers.label = ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D;
	}

	void AsymmetricGaussKernel2D::init()
	{
		const int size_x = commonParameters.dimensionParameters.size_x;
		const int size_y = commonParameters.dimensionParameters.size_y;

		kernelRange_x = tools::math::computeKernelRange(parameters.width, cutOfFactor, size_x, parameters.circular);
		kernelRange_y = tools::math::computeKernelRange(parameters.width, cutOfFactor, size_y, parameters.circular);

		if (parameters.circular)
		{
			extIndex_x = tools::math::createExtendedIndex(size_x, kernelRange_x);
			extIndex_y = tools::math::createExtendedIndex(size_y, kernelRange_y);
		}
		else
		{
			extIndex_x.clear();
			extIndex_y.clear();
		}

		// Build x-axis kernel: gauss + timeShift_x * gaussDerivative (no amplitude yet)
		{
			const int kSize = kernelRange_x[0] + kernelRange_x[1] + 1;
			std::vector<int> rangeVec(kSize);
			std::iota(rangeVec.begin(), rangeVec.end(), -static_cast<int>(kernelRange_x[0]));

			std::vector<double> g, gd;
			if (parameters.normalized)
			{
				g  = tools::math::gaussNorm(rangeVec, 0.0, parameters.width);
				gd = tools::math::gaussDerivativeNorm(rangeVec, 0.0, parameters.width, 1.0);
			}
			else
			{
				g  = tools::math::gauss(rangeVec, 0.0, parameters.width);
				gd = tools::math::gaussDerivative(rangeVec, 0.0, parameters.width, 1.0);
			}

			kernel_1d_x.resize(kSize);
			for (int i = 0; i < kSize; ++i)
				kernel_1d_x[i] = g[i] + parameters.timeShift_x * gd[i];
		}

		// Build y-axis kernel: gauss + timeShift_y * gaussDerivative (no amplitude)
		{
			const int kSize = kernelRange_y[0] + kernelRange_y[1] + 1;
			std::vector<int> rangeVec(kSize);
			std::iota(rangeVec.begin(), rangeVec.end(), -static_cast<int>(kernelRange_y[0]));

			std::vector<double> g, gd;
			if (parameters.normalized)
			{
				g  = tools::math::gaussNorm(rangeVec, 0.0, parameters.width);
				gd = tools::math::gaussDerivativeNorm(rangeVec, 0.0, parameters.width, 1.0);
			}
			else
			{
				g  = tools::math::gauss(rangeVec, 0.0, parameters.width);
				gd = tools::math::gaussDerivative(rangeVec, 0.0, parameters.width, 1.0);
			}

			kernel_1d_y.resize(kSize);
			for (int i = 0; i < kSize; ++i)
				kernel_1d_y[i] = g[i] + parameters.timeShift_y * gd[i];
		}

		// Apply amplitude to x only to avoid amplitude² scaling in separable convolution
		for (auto& v : kernel_1d_x) v *= parameters.amplitude;

		// Populate components["kernel"] with the outer product (row-major)
		const int kx = static_cast<int>(kernel_1d_x.size());
		const int ky = static_cast<int>(kernel_1d_y.size());
		components["kernel"].resize(kx * ky);
		for (int i = 0; i < kx; ++i)
			for (int j = 0; j < ky; ++j)
				components["kernel"][j * kx + i] = kernel_1d_x[i] * kernel_1d_y[j];

		const int totalSize = size_x * size_y;
		scratchTmp_.assign(totalSize, 0.0);
		scratchConvolution_.assign(totalSize, 0.0);

		fullSum = 0.0;
		std::ranges::fill(components["input"], 0.0);
		std::ranges::fill(components["output"], 0.0);
	}

	void AsymmetricGaussKernel2D::step(double t, double deltaT)
	{
		updateInput();

		fullSum = std::accumulate(components["input"].begin(), components["input"].end(), 0.0);

		const int size_x = commonParameters.dimensionParameters.size_x;
		const int size_y = commonParameters.dimensionParameters.size_y;

		tools::math::conv2d_separable_into(
			scratchConvolution_, scratchTmp_,
			components["input"], kernel_1d_x, kernel_1d_y,
			size_x, size_y, extIndex_x, extIndex_y);

		const double globalOffset = parameters.amplitudeGlobal * fullSum;
		for (int i = 0; i < static_cast<int>(components["output"].size()); ++i)
			components["output"][i] = scratchConvolution_[i] + globalOffset;
	}

	std::string AsymmetricGaussKernel2D::toString() const
	{
		std::string result = "Asymmetric gauss kernel 2D element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> AsymmetricGaussKernel2D::clone() const
	{
		return std::make_shared<AsymmetricGaussKernel2D>(*this);
	}

	void AsymmetricGaussKernel2D::setParameters(const AsymmetricGaussKernel2DParameters& p)
	{
		parameters = p;
		init();
	}

	AsymmetricGaussKernel2DParameters AsymmetricGaussKernel2D::getParameters() const
	{
		return parameters;
	}
}
