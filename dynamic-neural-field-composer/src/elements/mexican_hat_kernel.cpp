// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/mexican_hat_kernel.h"

namespace dnf_composer
{
	namespace element
	{
		MexicanHatKernel::MexicanHatKernel(const ElementCommonParameters& elementCommonParameters, const MexicanHatKernelParameters& mhk_parameters)
			: Kernel(elementCommonParameters), parameters(mhk_parameters)
		{
			commonParameters.identifiers.label = ElementLabel::MEXICAN_HAT_KERNEL;
		}

		void MexicanHatKernel::init()
		{
			const double maxSigma = std::max((parameters.amplitudeExc != 0.0) ? parameters.sigmaExc : 0,
				(parameters.amplitudeInh != 0.0) ? parameters.sigmaInh : 0);
			kernelRange = tools::math::computeKernelRange(maxSigma, cutOfFactor, commonParameters.dimensionParameters.size, circular);

			if (circular)
				extIndex = tools::math::createExtendedIndex(commonParameters.dimensionParameters.size, kernelRange);
			else
				extIndex = {};


			int rangeXsize = kernelRange[0] + kernelRange[1] + 1;
			std::vector<int> rangeX(rangeXsize);
			const int startingValue = static_cast<int>(kernelRange[0]);
			std::iota(rangeX.begin(), rangeX.end(), -startingValue);
			std::vector<double> gaussExc(commonParameters.dimensionParameters.size);
			std::vector<double> gaussInh(commonParameters.dimensionParameters.size);
			if (normalized)
			{
				gaussExc = tools::math::gaussNorm(rangeX, 0.0, parameters.sigmaExc);
				gaussInh = tools::math::gaussNorm(rangeX, 0.0, parameters.sigmaInh);
			}
			else
			{

				gaussExc = tools::math::gauss(rangeX, 0.0, parameters.sigmaExc);
				gaussInh = tools::math::gauss(rangeX, 0.0, parameters.sigmaInh);
			}

			components["kernel"].resize(rangeX.size());
			for (int i = 0; i < components["kernel"].size(); i++)
				components["kernel"][i] = parameters.amplitudeExc * gaussExc[i] - parameters.amplitudeInh * gaussInh[i];

			std::ranges::fill(components["input"], 0.0);
		}

		void MexicanHatKernel::step(double t, double deltaT)
		{
			updateInput();

			std::vector<double> convolution(commonParameters.dimensionParameters.size);
			const std::vector<double> subDataInput = tools::math::obtainCircularVector(extIndex, components["input"]);

			if (circular)
				convolution = tools::math::conv_valid(subDataInput, components["kernel"]);
			else
				convolution = tools::math::conv(components["input"], components["kernel"]);

			for (int i = 0; i < components["output"].size(); i++)
				components["output"][i] = convolution[i];
		}

		void MexicanHatKernel::close()
		{
		}

		void MexicanHatKernel::printParameters()
		{
			printCommonParameters();

			std::ostringstream logStream;

			logStream << "Logging specific element parameters" << std::endl;
			logStream << "AmplitudeExc: " << parameters.amplitudeExc << std::endl;
			logStream << "SigmaExc: " << parameters.sigmaExc << std::endl;
			logStream << "AmplitudeInh: " << parameters.amplitudeInh << std::endl;
			logStream << "SigmaInh: " << parameters.sigmaInh << std::endl;
			logStream << "CutOffFactor: " << cutOfFactor << std::endl;
			logStream << "Normalized: " << normalized << std::endl;

			log(tools::logger::LogLevel::INFO, logStream.str());
		}

		std::shared_ptr<Element> MexicanHatKernel::clone() const
		{
			auto cloned = std::make_shared<MexicanHatKernel>(*this);
			// If there are deep copy specifics that the copy constructor doesn't handle, do them here.
			return cloned;
		}

		void MexicanHatKernel::setParameters(const MexicanHatKernelParameters& mhk_parameters)
		{
			parameters = mhk_parameters;
			updateParameters();
		}

		MexicanHatKernelParameters MexicanHatKernel::getParameters() const
		{
			return parameters;
		}

		void MexicanHatKernel::updateParameters()
		{
			init();
		}
	}
}