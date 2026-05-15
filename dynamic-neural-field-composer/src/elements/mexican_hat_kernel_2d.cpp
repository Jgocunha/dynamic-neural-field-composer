// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/mexican_hat_kernel_2d.h"

namespace dnf_composer
{
	namespace element
	{
		MexicanHatKernel2D::MexicanHatKernel2D(const ElementCommonParameters& elementCommonParameters,
			const MexicanHatKernel2DParameters& parameters)
			: Kernel(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::MEXICAN_HAT_KERNEL_2D;
		}

		void MexicanHatKernel2D::init()
		{
			const int size_x = commonParameters.dimensionParameters.size_x;
			const int size_y = commonParameters.dimensionParameters.size_y;

			auto buildKernel = [&](double width, std::array<int,2>& rangeX, std::array<int,2>& rangeY,
			                       std::vector<int>& extX, std::vector<int>& extY,
			                       std::vector<double>& kx, std::vector<double>& ky)
			{
				rangeX = tools::math::computeKernelRange(width, cutOfFactor, size_x, parameters.circular);
				rangeY = tools::math::computeKernelRange(width, cutOfFactor, size_y, parameters.circular);
				if (parameters.circular)
				{
					extX = tools::math::createExtendedIndex(size_x, rangeX);
					extY = tools::math::createExtendedIndex(size_y, rangeY);
				}
				else { extX.clear(); extY.clear(); }

				const int kx_size = rangeX[0] + rangeX[1] + 1;
				std::vector<int> rxVec(kx_size);
				std::iota(rxVec.begin(), rxVec.end(), -static_cast<int>(rangeX[0]));

				const int ky_size = rangeY[0] + rangeY[1] + 1;
				std::vector<int> ryVec(ky_size);
				std::iota(ryVec.begin(), ryVec.end(), -static_cast<int>(rangeY[0]));

				if (parameters.normalized)
				{
					kx = tools::math::gaussNorm(rxVec, 0.0, width);
					ky = tools::math::gaussNorm(ryVec, 0.0, width);
				}
				else
				{
					kx = tools::math::gauss(rxVec, 0.0, width);
					ky = tools::math::gauss(ryVec, 0.0, width);
				}
			};

			buildKernel(parameters.widthExc,
				kernelRangeExc_x, kernelRangeExc_y,
				extIndexExc_x, extIndexExc_y,
				kernelExc_x, kernelExc_y);

			buildKernel(parameters.widthInh,
				kernelRangeInh_x, kernelRangeInh_y,
				extIndexInh_x, extIndexInh_y,
				kernelInh_x, kernelInh_y);

			for (auto& v : kernelExc_x) v *= parameters.amplitudeExc;
			for (auto& v : kernelInh_x) v *= parameters.amplitudeInh;

			// Populate components["kernel"] with the net outer product (exc - inh), row-major.
			// Use the larger of the two kernel ranges as the output size, centering the smaller kernel.
			const int kx = static_cast<int>(std::max(kernelExc_x.size(), kernelInh_x.size()));
			const int ky = static_cast<int>(std::max(kernelExc_y.size(), kernelInh_y.size()));
			components["kernel"].assign(kx * ky, 0.0);
			auto addProduct = [&](const std::vector<double>& kxVec, const std::vector<double>& kyVec, double sign)
			{
				const int offX = (kx - static_cast<int>(kxVec.size())) / 2;
				const int offY = (ky - static_cast<int>(kyVec.size())) / 2;
				for (int i = 0; i < static_cast<int>(kxVec.size()); ++i)
					for (int j = 0; j < static_cast<int>(kyVec.size()); ++j)
						components["kernel"][(i + offX) * ky + (j + offY)] += sign * kxVec[i] * kyVec[j];
			};
			addProduct(kernelExc_x, kernelExc_y, +1.0);
			addProduct(kernelInh_x, kernelInh_y, -1.0);

			const int totalSize = size_x * size_y;
			scratchTmp_.assign(totalSize, 0.0);
			scratchExcConv_.assign(totalSize, 0.0);
			scratchInhConv_.assign(totalSize, 0.0);

			fullSum = 0.0;
			std::ranges::fill(components["input"], 0.0);
			std::ranges::fill(components["output"], 0.0);
		}

		void MexicanHatKernel2D::step(double t, double deltaT)
		{
			updateInput();

			fullSum = std::accumulate(components["input"].begin(), components["input"].end(), 0.0);

			const int size_x = commonParameters.dimensionParameters.size_x;
			const int size_y = commonParameters.dimensionParameters.size_y;

			tools::math::conv2d_separable_into(
				scratchExcConv_, scratchTmp_,
				components["input"], kernelExc_x, kernelExc_y,
				size_x, size_y, extIndexExc_x, extIndexExc_y);

			tools::math::conv2d_separable_into(
				scratchInhConv_, scratchTmp_,
				components["input"], kernelInh_x, kernelInh_y,
				size_x, size_y, extIndexInh_x, extIndexInh_y);

			const double globalOffset = parameters.amplitudeGlobal * fullSum;
			for (int i = 0; i < static_cast<int>(components["output"].size()); ++i)
				components["output"][i] = scratchExcConv_[i] - scratchInhConv_[i] + globalOffset;
		}

		std::string MexicanHatKernel2D::toString() const
		{
			std::string result = "Mexican hat kernel 2D element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> MexicanHatKernel2D::clone() const
		{
			return std::make_shared<MexicanHatKernel2D>(*this);
		}

		void MexicanHatKernel2D::setParameters(const MexicanHatKernel2DParameters& p)
		{
			parameters = p;
			init();
		}

		MexicanHatKernel2DParameters MexicanHatKernel2D::getParameters() const
		{
			return parameters;
		}
	}
}
