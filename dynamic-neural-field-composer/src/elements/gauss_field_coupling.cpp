// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/gauss_field_coupling.h"

namespace dnf_composer
{
	namespace element
	{
		GaussFieldCoupling::GaussFieldCoupling(const ElementCommonParameters& elementCommonParameters, const GaussFieldCouplingParameters& gfc_parameters)
			: Element(elementCommonParameters), parameters(gfc_parameters)
		{

			if (gfc_parameters.inputFieldSize <= 0)
				throw Exception(ErrorCode::ELEM_SIZE_NOT_ALLOWED, commonParameters.identifiers.uniqueName);

			commonParameters.identifiers.label = ElementLabel::GAUSS_FIELD_COUPLING;
			components["input"] = std::vector<double>(gfc_parameters.inputFieldSize);
		}

		void GaussFieldCoupling::init()
		{
			std::ranges::fill(components["output"], 0.0);
		}

		void GaussFieldCoupling::step(double t, double deltaT)
		{
			updateInput();
			updateOutput();
		}

		void GaussFieldCoupling::close()
		{
		}

		void GaussFieldCoupling::printParameters()
		{
			printCommonParameters();

			std::ostringstream logStream;

			logStream << "Logging specific element parameters" << std::endl;
			logStream << "Input Field Size: " << parameters.inputFieldSize << std::endl;
			logStream << "Sigma: " << parameters.sigma << std::endl;

			logStream << "Couplings: ";
			for (const auto& coupling : parameters.couplings)
			{
				logStream << "x_i: " << coupling.x_i << ", x_j: " << coupling.x_j << ", w_i_j: " << coupling.w_i_j << " | ";
			}

			logStream << std::endl;

			log(tools::logger::LogLevel::INFO, logStream.str());
		}

		std::shared_ptr<Element> GaussFieldCoupling::clone() const
		{
			auto cloned = std::make_shared<GaussFieldCoupling>(*this);
			// If there are deep copy specifics that the copy constructor doesn't handle, do them here.
			return cloned;
		}

		void GaussFieldCoupling::updateOutput()
		{
			std::vector<double> summedGaussians(commonParameters.dimensionParameters.size);
			std::ranges::fill(summedGaussians, 0.0);

			for (const auto& coupling : parameters.couplings)
			{
				const auto activationAtx_i = components["input"][static_cast<int>(coupling.x_i)];
				if (activationAtx_i > 0.0)
				{
					std::vector<double> gauss = tools::math::circularGauss(commonParameters.dimensionParameters.size, parameters.sigma, coupling.x_j);
					for (auto& element : gauss)
						element *= coupling.w_i_j * activationAtx_i * element;

					for (int i = 0; i < commonParameters.dimensionParameters.size; i++)
						summedGaussians[i] += gauss[i];
				}
			}

			components["output"] = std::move(summedGaussians);
		}

		void GaussFieldCoupling::addCoupling(const WeightedCoupling& coupling)
		{
			parameters.couplings.emplace_back(coupling);
		}

		GaussFieldCouplingParameters GaussFieldCoupling::getParameters() const
		{
			return parameters;
		}
	}
}