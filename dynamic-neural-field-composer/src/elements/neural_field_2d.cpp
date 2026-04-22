// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/neural_field_2d.h"


namespace dnf_composer::element
{
	NeuralField2D::NeuralField2D(const ElementCommonParameters& elementCommonParameters,
		const NeuralField2DParameters& parameters)
		: Element(elementCommonParameters), parameters(parameters)
	{
		commonParameters.identifiers.label = ElementLabel::NEURAL_FIELD_2D;
		components["activation"] = std::vector<double>(commonParameters.dimensionParameters.size);
		components["resting level"] = std::vector<double>(commonParameters.dimensionParameters.size);
	}

	void NeuralField2D::init()
	{
		std::ranges::fill(components["activation"], parameters.startingRestingLevel);
		std::ranges::fill(components["resting level"], parameters.startingRestingLevel);
		std::ranges::fill(components["input"], 0.0);
		std::ranges::fill(components["output"], 0.0);
		calculateOutput();
	}

	void NeuralField2D::step(double t, double deltaT)
	{
		updateInput();
		calculateActivation(deltaT);
		calculateOutput();
	}

	void NeuralField2D::calculateActivation(double deltaT)
	{
		for (int i = 0; i < commonParameters.dimensionParameters.size; ++i)
		{
			components["activation"][i] += deltaT / parameters.tau *
				(-components["activation"][i] + components["resting level"][i] + components["input"][i]);
		}
	}

	void NeuralField2D::calculateOutput()
	{
		if (parameters.activationFunction)
			components["output"] = parameters.activationFunction->operator()(components["activation"]);
		else
		{
			SigmoidFunction sigmoid(0.0, 10.0);
			components["output"] = sigmoid(components["activation"]);
		}
	}

	std::string NeuralField2D::toString() const
	{
		std::string result = "Neural field 2D element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}

	std::shared_ptr<Element> NeuralField2D::clone() const
	{
		return std::make_shared<NeuralField2D>(*this);
	}

	void NeuralField2D::setParameters(const NeuralField2DParameters& neuralField2DParameters)
	{
		parameters = neuralField2DParameters;
		init();
	}

	NeuralField2DParameters NeuralField2D::getParameters() const
	{
		return parameters;
	}
}

