// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/kernel_coupling.h"


namespace dnf_composer::element
{
	KernelCoupling::KernelCoupling(const ElementCommonParameters& elementCommonParameters,
		KernelCouplingParameters gk_parameters)
		: Kernel(elementCommonParameters), parameters(std::move(gk_parameters))
	{
		commonParameters.identifiers.label = ElementLabel::KERNEL_COUPLING;

		// Determine input and output dimensions
		inputDimension = parameters.inputSize.value_or(commonParameters.dimensionParameters.size);
		outputDimension = parameters.outputSize.value_or(commonParameters.dimensionParameters.size);

		components["kernel"] = std::vector<double>(outputDimension);
	}

	void KernelCoupling::init()
	{
		// Use inputDimension for kernel range computation
		kernelRange = tools::math::computeKernelRange(parameters.width, cutOfFactor, inputDimension, parameters.circular);

		if (parameters.circular)
		{
			extIndex = tools::math::createExtendedIndex(inputDimension, kernelRange);
			components["input"].resize(extIndex.size());
		}
		else
		{
			extIndex = {};
			components["input"].resize(inputDimension);
		}

		// Create kernel based on output dimension
		int rangeXsize = kernelRange[0] + kernelRange[1] + 1;
		std::vector<int> rangeX(rangeXsize);
		const int startingValue = static_cast<int>(kernelRange[0]);
		std::iota(rangeX.begin(), rangeX.end(), -startingValue);

		std::vector<double> gauss;
		if (parameters.normalized)
			gauss = tools::math::gaussNorm(rangeX, 0.0, parameters.width);
		else
			gauss = tools::math::gauss(rangeX, 0.0, parameters.width);

		components["kernel"].resize(rangeX.size());
		for (int i = 0; i < components["kernel"].size(); i++)
			components["kernel"][i] = parameters.amplitude * gauss[i];

		// Initialize output with correct dimension
		components["output"].resize(outputDimension);

		fullSum = 0.0;
		std::ranges::fill(components["input"], 0.0);
		std::ranges::fill(components["output"], 0.0);
	}

	void KernelCoupling::step(double t, double deltaT)
	{
		updateInput();

        fullSum = std::accumulate(components["input"].begin(), components["input"].end(), 0.0);

        std::vector<double> convolution;
        const std::vector<double> subDataInput = tools::math::obtainCircularVector(extIndex, components["input"]);

        if (parameters.circular)
            convolution = tools::math::conv_valid(subDataInput, components["kernel"]);
        else
            convolution = tools::math::conv_same(components["input"], components["kernel"]);

        // // Handle dimension mismatch between convolution result and output
        // if (convolution.size() != outputDimension)
        // {
        //     // Resize or interpolate convolution result to match output dimension
        //     if (outputDimension > convolution.size())
        //     {
        //         // Upsample: simple linear interpolation or zero-padding
        //         std::vector<double> resized(outputDimension, 0.0);
        //         double scale = static_cast<double>(convolution.size()) / outputDimension;
        //
        //         for (int i = 0; i < outputDimension; i++)
        //         {
        //             const double sourceIndex = i * scale;
        //             int lowerIndex = static_cast<int>(std::floor(sourceIndex));
        //             int upperIndex = std::min(lowerIndex + 1, static_cast<int>(convolution.size() - 1));
        //             double weight = sourceIndex - lowerIndex;
        //
        //             if (lowerIndex < convolution.size())
        //             {
        //                 resized[i] = (1.0 - weight) * convolution[lowerIndex];
        //                 if (upperIndex < convolution.size() && upperIndex != lowerIndex)
        //                     resized[i] += weight * convolution[upperIndex];
        //             }
        //         }
        //         convolution = resized;
        //     }
        //     else
        //     {
        //         // Downsample: take every nth element or average
        //         std::vector<double> resized(outputDimension);
        //         double scale = static_cast<double>(convolution.size()) / outputDimension;
        //
        //         for (int i = 0; i < outputDimension; i++)
        //         {
        //             int sourceIndex = static_cast<int>(i * scale);
        //             resized[i] = convolution[sourceIndex];
        //         }
        //         convolution = resized;
        //     }
        // }

        for (int i = 0; i < components["output"].size(); i++)
            components["output"][i] = convolution[i] + parameters.amplitudeGlobal * fullSum;

	}

	std::string KernelCoupling::toString() const
	{
		std::string result = "Gauss kernel element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> KernelCoupling::clone() const
	{
		auto cloned = std::make_shared<KernelCoupling>(*this);
		return cloned;
	}

	void KernelCoupling::setParameters(const KernelCouplingParameters& gk_parameters)
	{
		parameters = gk_parameters;
		inputDimension = parameters.inputSize.value_or(commonParameters.dimensionParameters.size);
		outputDimension = parameters.outputSize.value_or(commonParameters.dimensionParameters.size);

		init();
	}

	KernelCouplingParameters KernelCoupling::getParameters() const
	{
		return parameters;
	}
}

