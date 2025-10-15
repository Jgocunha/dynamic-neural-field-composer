#include "simulation/simulation_file_manager.h"


namespace dnf_composer
{
    using json = nlohmann::json;

	SimulationFileManager::SimulationFileManager(const std::shared_ptr<Simulation>& simulation, const std::string& filePath)
		: simulation(simulation), filePath(filePath)
	{
        if (filePath.empty())
            this->filePath = std::string(OUTPUT_DIRECTORY) + "/simulations/";
	}

	void SimulationFileManager::saveElementsToJson() const
	{
        json elementsJson;

		for (const auto& element : simulation->getElements())
		{
            json elementJson;
            elementJson = elementToJson(element);
            elementsJson.emplace_back(elementJson);
        }

        // Write the JSON to a file
        std::ofstream file(filePath + simulation->getUniqueIdentifier() + ".json");
        if (file.is_open()) {
            file << elementsJson.dump(4); // Add indentation for readability
            log(tools::logger::INFO, "Elements saved to: " + filePath + simulation->getUniqueIdentifier() + ".json.");
        }
        else {
            log(tools::logger::ERROR, "Unable to open file to save elements: " + filePath + simulation->getUniqueIdentifier() + ".json.");
        }
	}

    void SimulationFileManager::loadElementsFromJson() const
    {
        // Open the JSON file for reading
        std::ifstream file(filePath);
        if (!file.is_open()) {
            log(tools::logger::ERROR, "Unable to open file to load elements: " + filePath + ".");
            return;
        }

        // Read the JSON content
        json elementsJson;
        try {
            file >> elementsJson;
        }
        catch (const std::exception& e) {
            log(tools::logger::ERROR, "Error reading JSON file: " + std::string(e.what()) + "");
            return;
        }

        log(tools::logger::INFO, "Elements loaded from: " + filePath);

        jsonToElements(elementsJson);

    }

    json SimulationFileManager::elementToJson(const std::shared_ptr<element::Element>& element)
    {
        json elementJson;

        // Get common parameters from the Element
        const element::ElementCommonParameters& commonParams = element->getElementCommonParameters();

        // Add common parameters to the JSON object
        elementJson["uniqueName"] = commonParams.identifiers.uniqueName;
        elementJson["label"] = { commonParams.identifiers.label, element::ElementLabelToString.at(commonParams.identifiers.label) };
        elementJson["x_max"] = commonParams.dimensionParameters.x_max;
        elementJson["d_x"] = commonParams.dimensionParameters.d_x;

        // Add interactions to the JSON object
        const std::unordered_map<std::shared_ptr<element::Element>, std::string> inputs = element->getInputsAndComponents();
        if(!(inputs.empty()))
        {
            for (const auto& [key, component] : inputs)
            {
                const auto inputUniqueName = key->getUniqueName();
                const auto inputComponent = component;
                elementJson["inputs"] += {inputUniqueName, inputComponent};
            }
        }
        else
        {
            elementJson["inputs"] = {};
        }

        // Add element specific parameters to JSON object
        switch (commonParams.identifiers.label)
        {
        case element::NEURAL_FIELD:
        {
            const auto neuralField = std::dynamic_pointer_cast<element::NeuralField>(element);
            const auto neuralFieldParameters = neuralField->getParameters();
            const auto activationFunctionType = neuralFieldParameters.activationFunction->type;
            elementJson["tau"] = neuralFieldParameters.tau;
            elementJson["restingLevel"] = neuralFieldParameters.startingRestingLevel;

            switch (activationFunctionType) {
            case element::ActivationFunctionType::HEAVISIDE:
            {
	            if (const auto heavisideActivationFunction = dynamic_cast<const element::HeavisideFunction*>(neuralFieldParameters.activationFunction.get())) {
                    elementJson["activationFunction"] = {
                        {"type", "heaviside"},
                        {"x_shift", heavisideActivationFunction->getXShift()}
                    };
                }
            }
            break;
            case element::ActivationFunctionType::SIGMOID:
            {
	            if (const auto sigmoidActivationFunction = dynamic_cast<const element::SigmoidFunction*>(neuralFieldParameters.activationFunction.get())) {
                    elementJson["activationFunction"] = {
                        {"type", "sigmoid"},
                        {"x_shift", sigmoidActivationFunction->getXShift()},
                        {"steepness", sigmoidActivationFunction->getSteepness()},
                    };
                }
            }
            break;
            }
        }
        break;
        case element::GAUSS_KERNEL:
        {
            const auto kernel = std::dynamic_pointer_cast<element::GaussKernel>(element);
            const auto kernelParameters = kernel->getParameters();
            elementJson["width"] = kernelParameters.width;
            elementJson["amplitude"] = kernelParameters.amplitude;
            elementJson["amplitudeGlobal"] = kernelParameters.amplitudeGlobal;
            elementJson["circular"] = kernelParameters.circular;
            elementJson["normalized"] = kernelParameters.normalized;
        }
        break;
        case element::MEXICAN_HAT_KERNEL:
        {
            const auto kernel = std::dynamic_pointer_cast<element::MexicanHatKernel>(element);
            const auto kernelParameters = kernel->getParameters();
            elementJson["widthExc"] = kernelParameters.widthExc;
            elementJson["amplitudeExc"] = kernelParameters.amplitudeExc;
            elementJson["widthInh"] = kernelParameters.widthInh;
            elementJson["amplitudeInh"] = kernelParameters.amplitudeInh;
            elementJson["amplitudeGlobal"] = kernelParameters.amplitudeGlobal;
            elementJson["circular"] = kernelParameters.circular;
            elementJson["normalized"] = kernelParameters.normalized;
        }
        break;
        case element::NORMAL_NOISE:
        {
            const auto normalNoise = std::dynamic_pointer_cast<element::NormalNoise>(element);
            const auto normalNoiseParameters = normalNoise->getParameters();
            elementJson["amplitude"] = normalNoiseParameters.amplitude;
        }
        break;
        case element::GAUSS_STIMULUS:
        {
            const auto gaussStimulus = std::dynamic_pointer_cast<element::GaussStimulus>(element);
            const auto gaussStimulusParameters = gaussStimulus->getParameters();
            elementJson["amplitude"] = gaussStimulusParameters.amplitude;
            elementJson["width"] = gaussStimulusParameters.width;
            elementJson["position"] = gaussStimulusParameters.position;
            elementJson["circular"] = gaussStimulusParameters.circular;
            elementJson["normalized"] = gaussStimulusParameters.normalized;
        }
        break;
        case element::FIELD_COUPLING:
        {
            const auto fieldCoupling = std::dynamic_pointer_cast<element::FieldCoupling>(element);
            const auto fieldCouplingParameters = fieldCoupling->getParameters();
            elementJson["learningRate"] = fieldCouplingParameters.learningRate;
            elementJson["learningRule"] = fieldCouplingParameters.learningRule;
            elementJson["scalar"] = fieldCouplingParameters.scalar;
            elementJson["input_x_max"] = fieldCouplingParameters.inputFieldDimensions.x_max;
            elementJson["input_d_x"] = fieldCouplingParameters.inputFieldDimensions.d_x;
        }
        break;
        case element::GAUSS_FIELD_COUPLING:
        {
            const auto gaussFieldCoupling = std::dynamic_pointer_cast<element::GaussFieldCoupling>(element);
            const auto gaussFieldCouplingParameters = gaussFieldCoupling->getParameters();
            elementJson["circular"] = gaussFieldCouplingParameters.circular;
            elementJson["normalized"] = gaussFieldCouplingParameters.normalized;
            elementJson["input_x_max"] = gaussFieldCouplingParameters.inputFieldDimensions.x_max;
            elementJson["input_d_x"] = gaussFieldCouplingParameters.inputFieldDimensions.d_x;
            for (const auto& coupling : gaussFieldCouplingParameters.couplings)
				elementJson["couplings"] += {coupling.x_i, coupling.x_j, coupling.amplitude, coupling.width};
        }
        break;
        case element::OSCILLATORY_KERNEL:
	        {
		        const auto oscillatoryKernel = std::dynamic_pointer_cast<element::OscillatoryKernel>(element);
		        const auto oscillatoryKernelParameters = oscillatoryKernel->getParameters();
		        elementJson["amplitude"] = oscillatoryKernelParameters.amplitude;
		        elementJson["decay"] = oscillatoryKernelParameters.decay;
		        elementJson["zeroCrossings"] = oscillatoryKernelParameters.zeroCrossings;
                elementJson["amplitudeGlobal"] = oscillatoryKernelParameters.amplitudeGlobal;
		        elementJson["circular"] = oscillatoryKernelParameters.circular;
		        elementJson["normalized"] = oscillatoryKernelParameters.normalized;
	        }
            break;
        default: 
        case element::UNINITIALIZED:
            tools::logger::log(tools::logger::ERROR, "Element label not recognized.");
            break;
        }

        return elementJson;
    }

    void SimulationFileManager::jsonToElements(const json& jsonElements) const
    {
         //Iterate over elements in the JSON and reconstruct them
	    for (const auto& elementJson : jsonElements) 
        {
	        // Parse common parameters
	        const std::string uniqueName = elementJson["uniqueName"];
	        const std::tuple<element::ElementLabel, std::string> label = elementJson["label"];
	        const int x_max = elementJson["x_max"];
	        const double d_x = elementJson["d_x"];

	        switch (get<0>(label))
	    	{
	        case element::NEURAL_FIELD: 
	            {
		            // Parse specific parameters for neural field
		            const double tau = elementJson["tau"];
		            const double restingLevel = elementJson["restingLevel"];

		            // Check the activation function type and parameters
		            auto activationFunctionJson = elementJson["activationFunction"];
		            std::unique_ptr<element::ActivationFunction> activationFunction;
		            if (!activationFunctionJson.is_null()) {
		                std::string activationFunctionType = activationFunctionJson["type"];
		                if (activationFunctionType == "heaviside") {
		                    double x_shift = activationFunctionJson["x_shift"];
		                    activationFunction = std::make_unique<element::HeavisideFunction>(x_shift);
		                }
		                else if (activationFunctionType == "sigmoid") {
		                    double x_shift = activationFunctionJson["x_shift"];
		                    double steepness = activationFunctionJson["steepness"];
		                    activationFunction = std::make_unique<element::SigmoidFunction>(x_shift, steepness);
		                }
		            }
		            // Reconstruct neural field element
		            auto neuralField = std::make_shared<element::NeuralField>(
		                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
		                element::NeuralFieldParameters(tau, restingLevel, *activationFunction)
		            );
		            // Add the reconstructed element to the simulation
		            simulation->addElement(neuralField);
		        }
	        	break;
            case element::NORMAL_NOISE:
            {
                const double amplitude = elementJson["amplitude"];

                auto normalNoise = std::make_shared<element::NormalNoise>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::NormalNoiseParameters(amplitude)
                );
                simulation->addElement(normalNoise);
            }
            break;
	        case element::GAUSS_KERNEL:
            {
                const double amplitude = elementJson["amplitude"];
                const double width = elementJson["width"];
                const bool circular = elementJson["circular"];
                const bool normalized = elementJson["normalized"];
                const double amplitudeGlobal = elementJson["amplitudeGlobal"];

                auto kernel = std::make_shared<element::GaussKernel>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::GaussKernelParameters(width, amplitude, amplitudeGlobal, circular, normalized)
                );
                simulation->addElement(kernel);
            }
            break;
	        case element::MEXICAN_HAT_KERNEL:
            {
                const double amplitudeExc = elementJson["amplitudeExc"];
                const double widthExc = elementJson["widthExc"];
                const double amplitudeInh = elementJson["amplitudeInh"];
                const double widthInh = elementJson["widthInh"];
                const double amplitudeGlobal = elementJson["amplitudeGlobal"];
                const bool circular = elementJson["circular"];
                const bool normalized = elementJson["normalized"];

                auto kernel = std::make_shared<element::MexicanHatKernel>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::MexicanHatKernelParameters(widthExc, amplitudeExc, widthInh, amplitudeInh, amplitudeGlobal, circular, normalized)
                );
                simulation->addElement(kernel);
            }
            break;
	        case element::GAUSS_STIMULUS:
            {
                const double amplitude = elementJson["amplitude"];
                const double width = elementJson["width"];
                const double position = elementJson["position"];
                const bool circular = elementJson["circular"];
                const bool normalized = elementJson["normalized"];

                auto stimulus = std::make_shared<element::GaussStimulus>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::GaussStimulusParameters(width, amplitude, position, circular, normalized)
                );
                simulation->addElement(stimulus);
            }
            break;
	        case element::FIELD_COUPLING:
            {
                const double learningRate = elementJson["learningRate"];
                const LearningRule learningRule = elementJson["learningRule"];
                const double scalar = elementJson["scalar"];
                const int input_x_max = elementJson["input_x_max"];
                const double input_d_x = elementJson["input_d_x"];
                auto coupling = std::make_shared<element::FieldCoupling>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::FieldCouplingParameters(element::ElementDimensions(input_x_max, input_d_x), learningRule, scalar, learningRate)
                );
                simulation->addElement(coupling);
            }
            break;
	        case element::GAUSS_FIELD_COUPLING:
            {
				const bool circular = elementJson["circular"];
                const bool normalized = elementJson["normalized"];
                const int input_x_max = elementJson["input_x_max"];
                const double input_d_x = elementJson["input_d_x"];

                std::vector<element::GaussCoupling> couplings;
                couplings.reserve(elementJson["couplings"].size());
                for (const auto& coupling : elementJson["couplings"])
                {
	                const double x_i = coupling[0];
					const double x_j = coupling[1];
                    const double amp = coupling[2];
                    const double width = coupling[3];
					auto gc = element::GaussCoupling(x_i, x_j, amp, width);
					couplings.push_back(gc);
				}

                auto coupling = std::make_shared<element::GaussFieldCoupling>(
					element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::GaussFieldCouplingParameters(element::ElementDimensions(input_x_max, input_d_x), normalized, circular, couplings)
				);
                simulation->addElement(coupling);
            }
            break;
	        case element::OSCILLATORY_KERNEL:
		        {
			        const double decay = elementJson["decay"];
			        const double zeroCrossings = elementJson["zeroCrossings"];
			        const double amplitude = elementJson["amplitude"];
                    const double amplitudeGlobal = elementJson["amplitudeGlobal"];
			        const bool circular = elementJson["circular"];
			        const bool normalized = elementJson["normalized"];

                    auto kernel = std::make_shared<element::OscillatoryKernel>(
				        element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
				        element::OscillatoryKernelParameters(amplitude, decay, zeroCrossings, amplitudeGlobal, circular, normalized)
			        );
			        simulation->addElement(kernel);
		        }
            break;
	        default:
	        case element::UNINITIALIZED:
                tools::logger::log(tools::logger::ERROR, "Element label not recognized.");
            break;
	        }
    }

	    // Iterate to create interactions
	    for (const auto& elementJson : jsonElements)
	    {
	        const std::string uniqueName = elementJson["uniqueName"];
	        const auto& inputsJson = elementJson["inputs"];

            if(!inputsJson.empty())
            {
                // Iterate over each inner array
                for (const auto& input : inputsJson)
                {
                    // Extract component and keyUniqueName
                    const std::string& keyUniqueName = input[0];
                    const std::string& component = input[1];

                    simulation->createInteraction(keyUniqueName, component, uniqueName);
                }
            }

	    }

    }

}
