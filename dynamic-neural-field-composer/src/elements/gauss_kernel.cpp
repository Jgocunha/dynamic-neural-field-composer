// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/gauss_kernel.h"


namespace dnf_composer
{
	namespace element
	{
		GaussKernel::GaussKernel(const ElementCommonParameters& elementCommonParameters, GaussKernelParameters gk_parameters)
			: Kernel(elementCommonParameters), parameters(std::move(gk_parameters))
		{
			commonParameters.identifiers.label = ElementLabel::GAUSS_KERNEL;
			components["kernel"] = std::vector<double>(commonParameters.dimensionParameters.size);
			if (parameters.outputFieldDimensions.has_value())
				components["output"].resize(parameters.outputFieldDimensions->size, 0.0);
		}

		void GaussKernel::init()
		{
			kernelRange = tools::math::computeKernelRange(parameters.width, cutOfFactor, commonParameters.dimensionParameters.size, parameters.circular);

			if (parameters.circular)
			{
				extIndex = tools::math::createExtendedIndex(commonParameters.dimensionParameters.size, kernelRange);
				components["input"].resize(extIndex.size()); 
			}
			else
			{
				extIndex = {};
				components["input"].resize(commonParameters.dimensionParameters.size);
			}

			int rangeXsize = kernelRange[0] + kernelRange[1] + 1;
			std::vector<int> rangeX(rangeXsize);
			const int startingValue = static_cast<int>(kernelRange[0]);
			std::iota(rangeX.begin(), rangeX.end(), -startingValue);
			std::vector<double> gauss(commonParameters.dimensionParameters.size);

			if (parameters.normalized)
				gauss = tools::math::gaussNorm(rangeX, 0.0, parameters.width);
			else
				gauss = tools::math::gauss(rangeX, 0.0, parameters.width);

			components["kernel"].resize(rangeX.size());
			for (int i = 0; i < components["kernel"].size(); i++)
				components["kernel"][i] = parameters.amplitude * gauss[i];
			 
			scratchConvolution.assign(commonParameters.dimensionParameters.size, 0.0);
			fullSum = 0.0;
			std::ranges::fill(components["input"], 0.0);
			if (parameters.outputFieldDimensions.has_value())
				components["output"].assign(parameters.outputFieldDimensions->size, 0.0);
			else
				std::ranges::fill(components["output"], 0.0);
		}

		void GaussKernel::step(double t, double deltaT)
		{
			updateInput();

			fullSum = std::accumulate(components["input"].begin(), components["input"].end(),
				(double)0.0);

			std::vector<double> convolution(commonParameters.dimensionParameters.size);
			const std::vector<double> subDataInput = tools::math::obtainCircularVector(extIndex, components["input"]);

			if (parameters.circular)
				convolution = tools::math::conv_valid(subDataInput, components["kernel"]);
			else
				convolution = tools::math::conv_same(components["input"], components["kernel"]);

			if (parameters.outputFieldDimensions.has_value() &&
				parameters.outputFieldDimensions->size != commonParameters.dimensionParameters.size)
			{
				for (int i = 0; i < static_cast<int>(scratchConvolution.size()); i++)
					scratchConvolution[i] = convolution[i] + parameters.amplitudeGlobal * fullSum;
				tools::math::resampleInto(scratchConvolution, components["output"]);
			}
			else
			{
				for (int i = 0; i < static_cast<int>(components["output"].size()); i++)
					components["output"][i] = convolution[i] + parameters.amplitudeGlobal * fullSum;
			}
		}

		std::string GaussKernel::toString() const
		{
			std::string result = "Gauss kernel element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> GaussKernel::clone() const
		{
			auto cloned = std::make_shared<GaussKernel>(*this);
			return cloned;
		}

		void GaussKernel::setParameters(const GaussKernelParameters& gk_parameters)
		{
			parameters = gk_parameters;
			init();
		}

		GaussKernelParameters GaussKernel::getParameters() const
		{
			return parameters;
		}
	}
}
