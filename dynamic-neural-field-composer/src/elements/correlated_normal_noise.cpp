#include <utility>

#include "elements/correlated_normal_noise.h"

//https://github.com/stevenlovegrove/Pangolin/issues/352
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace dnf_composer::element
{
	CorrelatedNormalNoise::CorrelatedNormalNoise(
		const ElementCommonParameters& elementCommonParameters,
		CorrelatedNormalNoiseParameters  parameters)
		: Element(elementCommonParameters), parameters(std::move(parameters))
	{
		commonParameters.identifiers.label = ElementLabel::CORRELATED_NORMAL_NOISE;
	}

	void CorrelatedNormalNoise::init()
	{
		std::ranges::fill(components["output"], 0.0);

		const int fieldSize = commonParameters.dimensionParameters.size;

		// Build a normalized Gaussian kernel for spatial correlation.
		// Kernel half-width: 5 * sigma (matches GaussKernel cutoff convention).
		const double effectiveWidth = std::max(parameters.width, 1e-3);
		const int halfWidth = std::max(1, static_cast<int>(5.0 * effectiveWidth));
		const int kernelSize = (2 * halfWidth) + 1;

		std::vector<int> rangeX(kernelSize);
		std::iota(rangeX.begin(), rangeX.end(), -halfWidth);
		correlationKernel = tools::math::gaussNorm(rangeX, 0.0, effectiveWidth);

		// Precompute circular extension index for wrap-around convolution.
		if (parameters.circular)
		{
			const std::array<int, 2> kernelRange = { halfWidth, halfWidth };
			extIndex = tools::math::createExtendedIndex(fieldSize, kernelRange);
		}
		else
		{
			extIndex.clear();
		}
	}

	void CorrelatedNormalNoise::step(double t, double deltaT)
	{
		const int fieldSize = commonParameters.dimensionParameters.size;

		// Zero amplitude => output is identically zero; skip RNG + convolution.
		if (parameters.amplitude == 0.0)
		{
			std::ranges::fill(components["output"], 0.0);
			return;
		}

		// Reuse the member buffer (no per-step allocation) for the white noise.
		if (static_cast<int>(whiteNoise_.size()) != fieldSize)
			whiteNoise_.assign(fieldSize, 0.0);
		tools::math::fillNormal(whiteNoise_.data(), static_cast<std::size_t>(fieldSize));
		const std::vector<double>& whiteNoise = whiteNoise_;

		std::vector<double> smoothed;
		if (parameters.circular && !extIndex.empty())
		{
			const auto extended = tools::math::obtainCircularVector(extIndex, whiteNoise);
			smoothed = tools::math::conv_valid(extended, correlationKernel);
		}
		else
		{
			smoothed = tools::math::conv_same(whiteNoise, correlationKernel);
		}

		const double scale = parameters.amplitude / std::sqrt(deltaT);
		for (int i = 0; i < fieldSize; i++) {
			components["output"][i] = scale * smoothed[i];
		}
	}

	std::shared_ptr<Element> CorrelatedNormalNoise::clone() const
	{
		return std::make_shared<CorrelatedNormalNoise>(*this);
	}

	std::string CorrelatedNormalNoise::toString() const
	{
		std::string result;
		result += "Correlated normal noise element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	void CorrelatedNormalNoise::setParameters(const CorrelatedNormalNoiseParameters& p)
	{
		parameters = p;
		init();
	}

	CorrelatedNormalNoiseParameters CorrelatedNormalNoise::getParameters() const
	{
		return parameters;
	}
}
