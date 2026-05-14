#include "elements/mexican_hat_kernel.h"

namespace dnf_composer
{
	namespace element
	{
		MexicanHatKernel::MexicanHatKernel(const ElementCommonParameters& elementCommonParameters, MexicanHatKernelParameters mhk_parameters)
			: Kernel(elementCommonParameters), parameters(std::move(mhk_parameters))
		{
			commonParameters.identifiers.label = ElementLabel::MEXICAN_HAT_KERNEL;
			if (parameters.outputFieldDimensions.has_value())
				components["output"].resize(parameters.outputFieldDimensions->size, 0.0);
		}

		void MexicanHatKernel::init()
		{
			const double maxWidth = std::max((parameters.amplitudeExc != 0.0) ? parameters.widthExc : 0,
				(parameters.amplitudeInh != 0.0) ? parameters.widthInh : 0);
			kernelRange = tools::math::computeKernelRange(maxWidth, cutOfFactor,
				commonParameters.dimensionParameters.size, parameters.circular);

			if (parameters.circular)
				extIndex = tools::math::createExtendedIndex(commonParameters.dimensionParameters.size, kernelRange);
			else
				extIndex = {};


			int rangeXsize = kernelRange[0] + kernelRange[1] + 1;
			std::vector<int> rangeX(rangeXsize);
			const int startingValue = static_cast<int>(kernelRange[0]);
			std::iota(rangeX.begin(), rangeX.end(), -startingValue);
			std::vector<double> gaussExc(commonParameters.dimensionParameters.size);
			std::vector<double> gaussInh(commonParameters.dimensionParameters.size);
			if (parameters.normalized)
			{
				gaussExc = tools::math::gaussNorm(rangeX, 0.0, parameters.widthExc);
				gaussInh = tools::math::gaussNorm(rangeX, 0.0, parameters.widthInh);
			}
			else
			{

				gaussExc = tools::math::gauss(rangeX, 0.0, parameters.widthExc);
				gaussInh = tools::math::gauss(rangeX, 0.0, parameters.widthInh);
			}

			components["kernel"].resize(rangeX.size());
			for (int i = 0; i < components["kernel"].size(); i++)
				components["kernel"][i] = parameters.amplitudeExc * gaussExc[i] -
				parameters.amplitudeInh * gaussInh[i];

			scratchExtended.assign(extIndex.empty() ? 0 : extIndex.size(), 0.0);
			scratchConvolution.assign(commonParameters.dimensionParameters.size, 0.0);
			if (parameters.outputFieldDimensions.has_value() &&
				parameters.outputFieldDimensions->size != commonParameters.dimensionParameters.size)
				scratchResample_.assign(commonParameters.dimensionParameters.size, 0.0);
			else
				scratchResample_.clear();
			fullSum = 0.0;
			std::ranges::fill(components["input"], 0.0);
			if (parameters.outputFieldDimensions.has_value())
				components["output"].assign(parameters.outputFieldDimensions->size, 0.0);
			else
				std::ranges::fill(components["output"], 0.0);
		}

		void MexicanHatKernel::step(double t, double deltaT)
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
			if (!scratchResample_.empty())
			{
				for (int i = 0; i < static_cast<int>(scratchConvolution.size()); i++)
					scratchResample_[i] = scratchConvolution[i] + globalOffset;
				tools::math::resampleInto(scratchResample_, components["output"]);
			}
			else
			{
				auto& out = components["output"];
				for (int i = 0; i < static_cast<int>(out.size()); i++)
					out[i] = scratchConvolution[i] + globalOffset;
			}
		}

		std::string MexicanHatKernel::toString() const
		{
			std::string result = "Mexican hat kernel element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> MexicanHatKernel::clone() const
		{
			auto cloned = std::make_shared<MexicanHatKernel>(*this);
			return cloned;
		}

		void MexicanHatKernel::setParameters(const MexicanHatKernelParameters& mhk_parameters)
		{
			parameters = mhk_parameters;
			init();
		}

		MexicanHatKernelParameters MexicanHatKernel::getParameters() const
		{
			return parameters;
		}
	}
}