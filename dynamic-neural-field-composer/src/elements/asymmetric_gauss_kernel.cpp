#include "elements/asymmetric_gauss_kernel.h"


namespace dnf_composer
{
	namespace element
	{
		AsymmetricGaussKernel::AsymmetricGaussKernel(const ElementCommonParameters& elementCommonParameters, AsymmetricGaussKernelParameters agk_parameters)
			: Kernel(elementCommonParameters), parameters(std::move(agk_parameters))
		{
			commonParameters.identifiers.label = ElementLabel::ASYMMETRIC_GAUSS_KERNEL;
			components["kernel"] = std::vector<double>(commonParameters.dimensionParameters.size);
		}

		void AsymmetricGaussKernel::init()
		{
            // Compute kernel range
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

            // Generate the Gaussian kernel
            int rangeXsize = kernelRange[0] + kernelRange[1] + 1;
            std::vector<int> rangeX(rangeXsize);
            const int startingValue = static_cast<int>(kernelRange[0]);
            std::iota(rangeX.begin(), rangeX.end(), -startingValue);

            // Generate Gaussian and derivative
			gauss = std::vector<double>(commonParameters.dimensionParameters.size);
			gaussDerivative = std::vector<double>(commonParameters.dimensionParameters.size);

            if (parameters.normalized)
            {
                gauss = tools::math::gaussNorm(rangeX, 0.0, parameters.width);
                gaussDerivative = tools::math::gaussDerivativeNorm(rangeX, 0.0, parameters.width, parameters.amplitude); 
            }
            else
            {
                gauss = tools::math::gauss(rangeX, 0.0, parameters.width);
                gaussDerivative = tools::math::gaussDerivative(rangeX, 0.0, parameters.width, parameters.amplitude);
            }

            // Combine Gaussian with its derivative for asymmetry
            components["kernel"].resize(rangeX.size());
            for (int i = 0; i < components["kernel"].size(); i++)
            {
                components["kernel"][i] = parameters.amplitude * gauss[i] + parameters.timeShift * gaussDerivative[i];
            }

            scratchExtended.assign(extIndex.empty() ? 0 : extIndex.size(), 0.0);
            scratchConvolution.assign(commonParameters.dimensionParameters.size, 0.0);
            fullSum = 0.0;
            std::ranges::fill(components["input"], 0.0);
            std::ranges::fill(components["output"], 0.0);
		}

		void AsymmetricGaussKernel::step(double t, double deltaT)
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

		std::string AsymmetricGaussKernel::toString() const
		{
			std::string result = "Asymmetric gauss kernel element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> AsymmetricGaussKernel::clone() const
		{
			auto cloned = std::make_shared<AsymmetricGaussKernel>(*this);
			return cloned;
		}

		void AsymmetricGaussKernel::setParameters(const AsymmetricGaussKernelParameters& agk_parameters)
		{
			parameters = agk_parameters;
			init();
		}

		AsymmetricGaussKernelParameters AsymmetricGaussKernel::getParameters() const
		{
			return parameters;
		}
	}
}
