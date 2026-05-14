// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/oscillatory_kernel_2d.h"


namespace dnf_composer::element
{
	OscillatoryKernel2D::OscillatoryKernel2D(const ElementCommonParameters& elementCommonParameters,
	                                         const OscillatoryKernel2DParameters& parameters)
		: Element(elementCommonParameters), parameters(parameters)
	{
		commonParameters.identifiers.label = ElementLabel::OSCILLATORY_KERNEL_2D;
	}

	void OscillatoryKernel2D::init()
	{
		const int size_x = commonParameters.dimensionParameters.size_x;
		const int size_y = commonParameters.dimensionParameters.size_y;

		// Effective range follows the 1D OscillatoryKernel convention.
		const double effectiveRange = std::max(1.0 / parameters.decay,
		                                       parameters.zeroCrossings * cutOffFactor);

		kernelRange_x = tools::math::computeKernelRange(effectiveRange, cutOffFactor, size_x, parameters.circular);
		kernelRange_y = tools::math::computeKernelRange(effectiveRange, cutOffFactor, size_y, parameters.circular);

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

		auto buildKernel1D = [&](const std::array<int, 2>& range) -> std::vector<double>
		{
			const int kSize = range[0] + range[1] + 1;
			std::vector<int> rangeVec(kSize);
			std::iota(rangeVec.begin(), rangeVec.end(), -static_cast<int>(range[0]));

			std::vector<double> k(kSize);
			for (int i = 0; i < kSize; ++i)
			{
				const double r = static_cast<double>(rangeVec[i]);
				const double decayFactor = std::exp(-parameters.decay * std::abs(r));
				const double oscillation = std::sin(parameters.decay * std::abs(parameters.zeroCrossings * r))
				                         + std::cos(parameters.zeroCrossings * r);
				k[i] = decayFactor * oscillation;
			}

			if (parameters.normalized)
			{
				const double normFactor = std::accumulate(k.begin(), k.end(), 0.0);
				if (std::abs(normFactor) > 1e-12)
					for (double& v : k) v /= normFactor;
			}

			for (double& v : k) v *= parameters.amplitude;
			return k;
		};

		kernel_1d_x = buildKernel1D(kernelRange_x);
		kernel_1d_y = buildKernel1D(kernelRange_y);

		fullSum = 0.0;
		std::ranges::fill(components["input"], 0.0);
		std::ranges::fill(components["output"], 0.0);
	}

	void OscillatoryKernel2D::step(double t, double deltaT)
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

	std::string OscillatoryKernel2D::toString() const
	{
		std::string result = "Oscillatory kernel 2D element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> OscillatoryKernel2D::clone() const
	{
		return std::make_shared<OscillatoryKernel2D>(*this);
	}

	void OscillatoryKernel2D::setParameters(const OscillatoryKernel2DParameters& p)
	{
		parameters = p;
		if (parameters.zeroCrossings < 0.0) parameters.zeroCrossings = 0.0;
		else if (parameters.zeroCrossings > 1.0) parameters.zeroCrossings = 1.0;
		if (parameters.decay <= 0.0) parameters.decay = 0.01;
		init();
	}

	OscillatoryKernel2DParameters OscillatoryKernel2D::getParameters() const
	{
		return parameters;
	}
}

