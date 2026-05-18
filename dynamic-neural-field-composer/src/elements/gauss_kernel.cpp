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
			 
			scratchExtended.assign(extIndex.empty() ? 0 : extIndex.size(), 0.0);
			scratchConvolution.assign(commonParameters.dimensionParameters.size, 0.0);
			fullSum = 0.0;
			std::ranges::fill(components["input"], 0.0);
			std::ranges::fill(components["output"], 0.0);
		}

		void GaussKernel::step(double t, double deltaT)
		{
			updateInput();

			const auto& inp = components["input"];
			fullSum = std::accumulate(inp.begin(), inp.begin() + commonParameters.dimensionParameters.size,
				0.0);

			if (parameters.circular) {
				tools::math::obtainCircularVector_into(scratchExtended, extIndex, inp);
				tools::math::conv_valid_into(scratchConvolution, scratchExtended, components["kernel"]);
			} else {
				tools::math::conv_same_into(scratchConvolution, inp, components["kernel"]);
			}

			const double globalOffset = parameters.amplitudeGlobal * fullSum;
			auto& out = components["output"];
			for (int i = 0; i < static_cast<int>(out.size()); i++)
				out[i] = scratchConvolution[i] + globalOffset;
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