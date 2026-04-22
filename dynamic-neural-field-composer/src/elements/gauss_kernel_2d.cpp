// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/gauss_kernel_2d.h"

namespace dnf_composer
{
	namespace element
	{
		GaussKernel2D::GaussKernel2D(const ElementCommonParameters& elementCommonParameters,
			const GaussKernel2DParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::GAUSS_KERNEL_2D;
		}

		void GaussKernel2D::init()
		{
			const int size_x = commonParameters.dimensionParameters.size_x;
			const int size_y = commonParameters.dimensionParameters.size_y;

			kernelRange_x = tools::math::computeKernelRange(parameters.width, cutOffFactor, size_x, parameters.circular);
			kernelRange_y = tools::math::computeKernelRange(parameters.width, cutOffFactor, size_y, parameters.circular);

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
			for (auto& v : kernel_1d_y) v *= parameters.amplitude;

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

			const auto convolution = tools::math::conv2d_separable(
				components["input"], kernel_1d_x, kernel_1d_y,
				size_x, size_y, extIndex_x, extIndex_y);

			for (int i = 0; i < static_cast<int>(components["output"].size()); ++i)
				components["output"][i] = convolution[i] + parameters.amplitudeGlobal * fullSum;
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
