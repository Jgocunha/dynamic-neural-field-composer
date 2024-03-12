// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com


#include "elements/lateral_interactions.h"

namespace dnf_composer
{
	namespace element
	{
		LateralInteractions::LateralInteractions(const ElementCommonParameters& elementCommonParameters, const LateralInteractionsParameters& li_parameters)
			: Kernel(elementCommonParameters), parameters(li_parameters)
		{
			commonParameters.identifiers.label = ElementLabel::LATERAL_INTERACTIONS;
		}

		void LateralInteractions::init()
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

			fullSum = 0;
			std::ranges::fill(components["input"], 0.0);
		}

		void LateralInteractions::step(double t, double deltaT)
		{
			updateInput();

			fullSum = std::accumulate(components["input"].begin(), components["input"].end(), (double)0.0);

			std::vector<double> convolution(commonParameters.dimensionParameters.size);
			const std::vector<double> subDataInput = tools::math::obtainCircularVector(extIndex, components["input"]);

			if (circular)
				convolution = tools::math::conv_valid(subDataInput, components["kernel"]);
			else
				convolution = tools::math::conv(components["input"], components["kernel"]);

			for (int i = 0; i < components["output"].size(); i++)
				components["output"][i] = convolution[i] + parameters.amplitudeGlobal * fullSum;
		}

		void LateralInteractions::close()
		{
		}

		void LateralInteractions::printParameters()
		{
			printCommonParameters();

			std::ostringstream logStream;

			logStream << "Logging specific element parameters" << std::endl;
			logStream << "AmplitudeExc: " << parameters.amplitudeExc << std::endl;
			logStream << "SigmaExc: " << parameters.sigmaExc << std::endl;
			logStream << "AmplitudeInh: " << parameters.amplitudeInh << std::endl;
			logStream << "SigmaInh: " << parameters.sigmaInh << std::endl;
			logStream << "AmplitudeGlobal: " << parameters.amplitudeGlobal << std::endl;
			logStream << "CutOffFactor: " << cutOfFactor << std::endl;
			logStream << "Normalized: " << normalized << std::endl;

			log(tools::logger::LogLevel::INFO, logStream.str());
		}

		std::shared_ptr<Element> LateralInteractions::clone() const
		{
			auto cloned = std::make_shared<LateralInteractions>(*this);
			// If there are deep copy specifics that the copy constructor doesn't handle, do them here.
			return cloned;
		}

		void LateralInteractions::setParameters(const LateralInteractionsParameters& li_parameters)
		{
			parameters = li_parameters;
			updateParameters();
		}

		LateralInteractionsParameters LateralInteractions::getParameters() const
		{
			return parameters;
		}

		void LateralInteractions::updateParameters()
		{
			init();
		}
	}
}