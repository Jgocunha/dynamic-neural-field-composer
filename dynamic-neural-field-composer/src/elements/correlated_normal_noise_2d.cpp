// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <utility>

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
		CorrelatedNormalNoise2DParameters  parameters)
		: Element(elementCommonParameters), parameters(std::move(parameters))
	{
		commonParameters.identifiers.label = ElementLabel::CORRELATED_NORMAL_NOISE_2D;
	}

	void CorrelatedNormalNoise2D::init()
	{
		std::ranges::fill(components["output"], 0.0);

		const int size_x = commonParameters.dimensionParameters.size_x;
		const int size_y = commonParameters.dimensionParameters.size_y;

		// Clamp kernel support to the field size per axis via computeKernelRange
		// (matching every other 2D kernel element), rather than the previous
		// unclamped halfWidth = 5*effectiveWidth. Unclamped, a wide width on a
		// small field (e.g. width=3.0 on a 10x10 field -> halfWidth=15) made
		// createExtendedIndex return a negative starting index, which
		// conv2d_separable_into's circular x-pass then read as an out-of-bounds
		// offset before the row buffer -- a real, reachable heap OOB read (the
		// UI allows width up to 30), not merely a semantic wart.
		const double effectiveWidth = std::max(parameters.width, 1e-3);
		kernelRange_x = tools::math::computeKernelRange(effectiveWidth, kCutOfFactor, size_x, parameters.circular);
		kernelRange_y = tools::math::computeKernelRange(effectiveWidth, kCutOfFactor, size_y, parameters.circular);

		// Built per axis (not copied from x to y): computeKernelRange's clamp
		// can differ between axes -- both when size_x != size_y, and on an
		// EVEN field size where the circular clamp itself yields kR0 != kR1
		// (e.g. size=100 -> {49,50}) even when both fields are square.
		auto buildTaps = [&](const std::array<int, 2>& range)
		{
			std::vector<int> rangeVec(range[0] + range[1] + 1);
			std::iota(rangeVec.begin(), rangeVec.end(), -range[0]);
			return tools::math::gaussNorm(rangeVec, 0.0, effectiveWidth);
		};
		correlationKernel_x = buildTaps(kernelRange_x);
		correlationKernel_y = buildTaps(kernelRange_y);

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

		const int totalSize = size_x * size_y;
		scratchTmp_.assign(totalSize, 0.0);
		scratchConv_.assign(totalSize, 0.0);
		scratch2d_.ensure(size_x, size_y, extIndex_x.size(), extIndex_y.size());

		const int totalTaps = (kernelRange_x[0] + kernelRange_x[1] + 1)
		                    + (kernelRange_y[0] + kernelRange_y[1] + 1);
		useFFT_ = tools::math::shouldUseSpectral2D(parameters.circular, totalTaps, size_x, size_y);
		if (useFFT_)
		{
			spectral_.init(size_x, size_y);
			spectral_.setKernel(tools::math::buildWrappedSeparableKernel2D(size_x, size_y,
				{ tools::math::SeparableKernelTerm2D{ correlationKernel_x, kernelRange_x[0], correlationKernel_y, kernelRange_y[0], +1.0 } }));
		}
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

		if (useFFT_)
			spectral_.apply(whiteNoise.data(), scratchConv_.data());
		else
			tools::math::conv2d_separable_into(
				scratchConv_, scratchTmp_, scratch2d_,
				whiteNoise, correlationKernel_x, correlationKernel_y,
				size_x, size_y, extIndex_x, extIndex_y);

		const double scale = parameters.amplitude / std::sqrt(deltaT);
		// Hoist the output buffer out of the per-cell loop (unordered_map lookup).
		double* __restrict out = components["output"].data();
		for (int i = 0; i < totalSize; ++i)
			out[i] = scale * scratchConv_[i];
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
