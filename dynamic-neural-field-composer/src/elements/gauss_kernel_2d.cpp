// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/gauss_kernel_2d.h"

namespace dnf_composer
{
	namespace element
	{
		GaussKernel2D::GaussKernel2D(const ElementCommonParameters& elementCommonParameters,
			const GaussKernel2DParameters& parameters)
			: Kernel(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::GAUSS_KERNEL_2D;
		}

		void GaussKernel2D::init()
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

			const int kRangeX = kernelRange_x[0] + kernelRange_x[1] + 1;
			std::vector<int> rangeX(kRangeX);
			std::iota(rangeX.begin(), rangeX.end(), -static_cast<int>(kernelRange_x[0]));

			const int kRangeY = kernelRange_y[0] + kernelRange_y[1] + 1;
			std::vector<int> rangeY(kRangeY);
			std::iota(rangeY.begin(), rangeY.end(), -static_cast<int>(kernelRange_y[0]));

			if (parameters.normalized)
			{
				kernel_1d_x = tools::math::gaussNorm(rangeX, 0.0, parameters.width);
				kernel_1d_y = tools::math::gaussNorm(rangeY, 0.0, parameters.width);
			}
			else
			{
				kernel_1d_x = tools::math::gauss(rangeX, 0.0, parameters.width);
				kernel_1d_y = tools::math::gauss(rangeY, 0.0, parameters.width);
			}

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

		void GaussKernel2D::step(double t, double deltaT)
		{
			updateInput();

			fullSum = std::accumulate(components["input"].begin(), components["input"].end(), 0.0);

			const int size_x = commonParameters.dimensionParameters.size_x;
			const int size_y = commonParameters.dimensionParameters.size_y;

			tools::math::conv2d_separable_into(
				scratchConvolution_, scratchTmp_,
				components["input"], kernel_1d_y, kernel_1d_x,
				size_y, size_x, extIndex_y, extIndex_x);

			const double globalOffset = parameters.amplitudeGlobal * fullSum;
			for (int i = 0; i < static_cast<int>(components["output"].size()); ++i)
				components["output"][i] = scratchConvolution_[i] + globalOffset;
		}

		std::string GaussKernel2D::toString() const
		{
			std::string result = "Gauss kernel 2D element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> GaussKernel2D::clone() const
		{
			return std::make_shared<GaussKernel2D>(*this);
		}

		void GaussKernel2D::setParameters(const GaussKernel2DParameters& p)
		{
			parameters = p;
			init();
		}

		GaussKernel2DParameters GaussKernel2D::getParameters() const
		{
			return parameters;
		}
	}
}
