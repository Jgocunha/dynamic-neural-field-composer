// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/correlated_normal_noise_2d.h"

//https://github.com/stevenlovegrove/Pangolin/issues/352
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace dnf_composer::element
{
	CorrelatedNormalNoise2D::CorrelatedNormalNoise2D(
		const ElementCommonParameters& elementCommonParameters,
		const CorrelatedNormalNoise2DParameters& parameters)
		: Element(elementCommonParameters), parameters(parameters)
	{
		commonParameters.identifiers.label = ElementLabel::CORRELATED_NORMAL_NOISE_2D;
	}

	void CorrelatedNormalNoise2D::init()
	{
		std::ranges::fill(components["output"], 0.0);

		const int size_x = commonParameters.dimensionParameters.size_x;
		const int size_y = commonParameters.dimensionParameters.size_y;

		const double effectiveWidth = std::max(parameters.width, 1e-3);
		const int halfWidth = std::max(1, static_cast<int>(5.0 * effectiveWidth));
		const int kernelSize = 2 * halfWidth + 1;

		std::vector<int> rangeVec(kernelSize);
		std::iota(rangeVec.begin(), rangeVec.end(), -halfWidth);
		correlationKernel_x = tools::math::gaussNorm(rangeVec, 0.0, effectiveWidth);
		correlationKernel_y = correlationKernel_x;

		if (parameters.circular)
		{
			const std::array<int, 2> kernelRange = { halfWidth, halfWidth };
			extIndex_x = tools::math::createExtendedIndex(size_x, kernelRange);
			extIndex_y = tools::math::createExtendedIndex(size_y, kernelRange);
		}
		else
		{
			extIndex_x.clear();
			extIndex_y.clear();
		}

		const int totalSize = size_x * size_y;
		scratchTmp_.assign(totalSize, 0.0);
		scratchConv_.assign(totalSize, 0.0);
		scratch2d_.ensure(size_x, size_y, extIndex_x.size(), extIndex_y.size());
	}

	void CorrelatedNormalNoise2D::step(double t, double deltaT)
	{
		const int totalSize = commonParameters.dimensionParameters.size;
		const int size_x    = commonParameters.dimensionParameters.size_x;
		const int size_y    = commonParameters.dimensionParameters.size_y;

		// Zero amplitude => output is identically zero; skip RNG + convolution.
		if (parameters.amplitude == 0.0)
		{
			std::ranges::fill(components["output"], 0.0);
			return;
		}

		// Reuse the member buffer (no per-step allocation) for the white noise.
		if (static_cast<int>(whiteNoise_.size()) != totalSize)
			whiteNoise_.assign(totalSize, 0.0);
		tools::math::fillNormal(whiteNoise_.data(), static_cast<std::size_t>(totalSize));
		const std::vector<double>& whiteNoise = whiteNoise_;

		tools::math::conv2d_separable_into(
			scratchConv_, scratchTmp_, scratch2d_,
			whiteNoise, correlationKernel_x, correlationKernel_y,
			size_x, size_y, extIndex_x, extIndex_y);

		const double scale = parameters.amplitude / std::sqrt(deltaT);
		for (int i = 0; i < totalSize; ++i)
			components["output"][i] = scale * scratchConv_[i];
	}

	std::string CorrelatedNormalNoise2D::toString() const
	{
		std::string result = "Correlated normal noise 2D element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> CorrelatedNormalNoise2D::clone() const
	{
		return std::make_shared<CorrelatedNormalNoise2D>(*this);
	}

	void CorrelatedNormalNoise2D::setParameters(const CorrelatedNormalNoise2DParameters& p)
	{
		parameters = p;
		init();
	}

	CorrelatedNormalNoise2DParameters CorrelatedNormalNoise2D::getParameters() const
	{
		return parameters;
	}
}
