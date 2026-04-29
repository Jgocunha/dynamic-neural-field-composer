// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/gauss_field_coupling.h"

namespace dnf_composer
{
	namespace element
	{
		GaussFieldCoupling::GaussFieldCoupling(const ElementCommonParameters& elementCommonParameters, 
			const GaussFieldCouplingParameters& gfc_parameters)
			: Element(elementCommonParameters), parameters(gfc_parameters)
		{
			commonParameters.identifiers.label = ElementLabel::GAUSS_FIELD_COUPLING;
			components["input"] = std::vector<double>(parameters.inputFieldDimensions.size);
			components["output"] = std::vector<double>(commonParameters.dimensionParameters.size);
			components["weights"] = std::vector<double>(components.at("input").size() * components.at("output").size());
		}

		void GaussFieldCoupling::init()
		{
			updateInputFieldDimensions();

			std::ranges::fill(components["input"], 0);
			std::ranges::fill(components["output"], 0);
			std::ranges::fill(components["weights"], 0);

			const unsigned int cols = static_cast<int>(components["output"].size());
			const unsigned int rows = static_cast<int>(components["input"].size());

			for (unsigned int i = 0; i < cols; i++)
			{
				std::vector<double> col;
				col.reserve(rows);
				for (unsigned int j = 0; j < rows; j++)
				{
					double value = 0.0;
					for (const auto& coupling : parameters.couplings)
					{
						double amplitude = coupling.amplitude;
						if (parameters.normalized)	
							amplitude /= sqrt(2 * std::numbers::pi * std::pow(coupling.width, 2));
						if (parameters.circular)
							value += tools::math::gaussian_2d_periodic(j, i,
								coupling.x_i/parameters.inputFieldDimensions.d_x, coupling.x_j/commonParameters.dimensionParameters.d_x,
								coupling.width, amplitude,
								rows, cols);
						else
							value += tools::math::gaussian_2d(j, i,
								coupling.x_i / parameters.inputFieldDimensions.d_x, coupling.x_j / commonParameters.dimensionParameters.d_x,
								coupling.width, coupling.width, amplitude);
					}
					const size_t index = j * cols + i;
					components["weights"][index] = value;
				}
			}
		}

		void GaussFieldCoupling::step(double t, double deltaT)
		{
			updateInput();
			updateOutput();
		}

		std::string GaussFieldCoupling::toString() const
		{
			std::string result = "Gauss field coupling element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> GaussFieldCoupling::clone() const
		{
			auto cloned = std::make_shared<GaussFieldCoupling>(*this);
			return cloned;
		}

		void GaussFieldCoupling::updateOutput()
		{
			components["output"] = std::vector<double>(components["output"].size(), 0);

			for (size_t i = 0; i < components["output"].size(); i++)
			{
				for (size_t j = 0; j < components["input"].size(); j++)
				{
					const size_t index = j * components["output"].size() + i;
					components["output"][i] += components["weights"][index] * components["input"][j];
				}
			}
		}

		void GaussFieldCoupling::addCoupling(const GaussCoupling& coupling)
		{
			parameters.couplings.emplace_back(coupling);
		}

		GaussFieldCouplingParameters GaussFieldCoupling::getParameters() const
		{
			return parameters;
		}

		void GaussFieldCoupling::changeDimensions(const ElementDimensions& newDimensions)
		{
			commonParameters.dimensionParameters = newDimensions;
			const int inputSize = static_cast<int>(components["input"].size());
			components["output"].assign(newDimensions.size, 0.0);
			components["weights"].assign(inputSize * newDimensions.size, 0.0);
			init();
		}

		void GaussFieldCoupling::changeInputDimensions(const ElementDimensions& newInputDimensions)
		{
			parameters.inputFieldDimensions = newInputDimensions;
			const int outputSize = static_cast<int>(components["output"].size());
			components["input"].assign(newInputDimensions.size, 0.0);
			components["weights"].assign(newInputDimensions.size * outputSize, 0.0);
			init();
		}

		void GaussFieldCoupling::setParameters(const GaussFieldCouplingParameters& gfc_parameters)
		{
			parameters = gfc_parameters;
			init();
		}

		ElementDimensions GaussFieldCoupling::getInputFieldDimensions() const
		{
			return parameters.inputFieldDimensions;
		}

		void GaussFieldCoupling::updateInputFieldDimensions()
		{
			if (inputs.size() != 1)
			{
				const std::string logMessage = "Incorrect number of inputs for gauss field coupling '"
					+ commonParameters.identifiers.uniqueName + "'. Should be 1, is " + std::to_string(inputs.size()) + ".";
				log(tools::logger::LogLevel::WARNING, logMessage);
				return;
			}

			if (inputs.begin()->first->getLabel() != ElementLabel::NEURAL_FIELD)
			{
				const std::string logMessage = "Incorrect input type for field coupling '"
					+ commonParameters.identifiers.uniqueName + "'. Should be a neural field, is " + ElementLabelToString.at(inputs.begin()->first->getLabel()) + ".";
				log(tools::logger::LogLevel::WARNING, logMessage);
				return;
			}

			const std::shared_ptr<Element> input = inputs.begin()->first;
			parameters.inputFieldDimensions = input->getElementCommonParameters().dimensionParameters;
		}
	}
}