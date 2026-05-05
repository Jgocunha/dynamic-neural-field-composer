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
		const CorrelatedNormalNoiseParameters& parameters)
		: Element(elementCommonParameters), parameters(parameters)
	{
		commonParameters.identifiers.label = ElementLabel::CORRELATED_NORMAL_NOISE;
	}

	void CorrelatedNormalNoise::init()
	{
		std::ranges::fill(components["output"], 0.0);

		const int fieldSize = commonParameters.dimensionParameters.size;

		// Build a normalised Gaussian kernel for spatial correlation.
		// Kernel half-width: 5 * sigma (matches GaussKernel cutoff convention).
		const int halfWidth = std::max(1, static_cast<int>(5.0 * parameters.width));
		const int kernelSize = 2 * halfWidth + 1;

		std::vector<int> rangeX(kernelSize);
		std::iota(rangeX.begin(), rangeX.end(), -halfWidth);
		correlationKernel = tools::math::gaussNorm(rangeX, 0.0, parameters.width);

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
		const std::vector<double> whiteNoise = tools::math::generateNormalVector(fieldSize);

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
		for (int i = 0; i < fieldSize; i++)
			components["output"][i] = scale * smoothed[i];
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
