#include "simulation/simulation_file_manager.h"


namespace dnf_composer
{
    using json = nlohmann::json;

	SimulationFileManager::SimulationFileManager(const std::shared_ptr<Simulation>& simulation, const std::string& filePath)
		: simulation(simulation), filePath(filePath)
	{
        if (filePath.empty())
            this->filePath = tools::utils::getResourceRoot() + "/data/";
	}

	void SimulationFileManager::saveElementsToJson() const
	{
        const std::filesystem::path simDir = std::filesystem::path(filePath) / simulation->getUniqueIdentifier();
        std::filesystem::create_directories(simDir);

        // Write FieldCoupling weights into the sim-specific directory before saving JSON.
        for (const auto& el : simulation->getElements())
        {
            if (const auto fc = std::dynamic_pointer_cast<element::FieldCoupling>(el))
            {
                fc->setWeightsDirectory(simDir.string());
                fc->writeWeights();
            }
        }

        json elementsJson = json::array();
		for (const auto& element : simulation->getElements())
            elementsJson.emplace_back(elementToJson(element));

        json root;
        root["identifier"] = simulation->getUniqueIdentifier();
        root["deltaT"]     = simulation->getDeltaT();
        root["elements"]   = elementsJson;

        const std::string path = (simDir / (simulation->getUniqueIdentifier() + ".dnf")).string();
        std::ofstream file(path);
        if (file.is_open()) {
            file << root.dump(4);
            log(tools::logger::INFO, "Simulation saved to: " + path);
        }
        else {
            log(tools::logger::ERROR, "Unable to open file to save simulation: " + path);
        }
	}

    void SimulationFileManager::loadElementsFromJson() const
    {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            log(tools::logger::ERROR, "Unable to open file to load simulation: " + filePath + ".");
            return;
        }

        json root;
        try {
            file >> root;
        }
        catch (const std::exception& e) {
            log(tools::logger::ERROR, "Error reading JSON file: " + std::string(e.what()));
            return;
        }

        // Backwards-compatible: old format is a bare array of elements.
        // New format is an object with metadata + "elements" array.
        json elementsJson;
        if (root.is_array())
        {
            elementsJson = root;
        }
        else if (root.is_object())
        {
            const json& elems = root.contains("elements") ? root["elements"] : json::array();
            if (!elems.is_array())
            {
                log(tools::logger::ERROR, "Invalid simulation file: \"elements\" is not an array: " + filePath);
                return;
            }
            elementsJson = elems;

            if (root.contains("identifier") && root["identifier"].is_string())
                simulation->setUniqueIdentifier(root["identifier"].get<std::string>());
            else if (root.contains("identifier"))
                log(tools::logger::ERROR, "Invalid simulation file: \"identifier\" is not a string: " + filePath);

            if (root.contains("deltaT") && root["deltaT"].is_number())
            {
                const double dt = root["deltaT"].get<double>();
                if (std::isfinite(dt) && dt > 0.0)
                    simulation->setDeltaT(dt);
                else
                    log(tools::logger::ERROR, "Invalid simulation file: \"deltaT\" is not a valid positive number: " + filePath);
            }
            else if (root.contains("deltaT"))
                log(tools::logger::ERROR, "Invalid simulation file: \"deltaT\" is not a number: " + filePath);
        }
        else
        {
            log(tools::logger::ERROR, "Invalid simulation file: unexpected JSON root type: " + filePath);
            return;
        }

        log(tools::logger::INFO, "Simulation loaded from: " + filePath);
        jsonToElements(elementsJson);

        // Point FieldCoupling elements to their weights in the same directory as the JSON file.
        const std::string simDir = std::filesystem::path(filePath).parent_path().string();
        for (const auto& el : simulation->getElements())
        {
            if (const auto fc = std::dynamic_pointer_cast<element::FieldCoupling>(el))
            {
                fc->setWeightsDirectory(simDir);
                fc->tryReadWeights();
            }
        }
    }

    static element::ElementLabel elementLabelFromString(const std::string& s)
    {
        for (const auto& [k, v] : element::ElementLabelToString)
            if (v == s) return k;
        return element::UNINITIALIZED;
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
        elementJson["y_max"] = commonParams.dimensionParameters.y_max;
        elementJson["d_y"] = commonParams.dimensionParameters.d_y;

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
            case element::ActivationFunctionType::ABSSIGMOID:
            {
                const auto absSigmoidFn = dynamic_cast<const element::AbsSigmoidFunction*>(neuralFieldParameters.activationFunction.get());
                if (absSigmoidFn) {
                    elementJson["activationFunction"] = {
                        {"type", "abs_sigmoid"},
                        {"x_shift", absSigmoidFn->getXShift()},
                        {"beta", absSigmoidFn->getBeta()},
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
        case element::CORRELATED_NORMAL_NOISE:
        {
            const auto cnn = std::dynamic_pointer_cast<element::CorrelatedNormalNoise>(element);
            const auto p = cnn->getParameters();
            elementJson["amplitude"] = p.amplitude;
            elementJson["width"] = p.width;
            elementJson["circular"] = p.circular;
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
            elementJson["couplings"] = json::array();
            for (const auto& coupling : gaussFieldCouplingParameters.couplings)
                elementJson["couplings"].push_back(json::array({coupling.x_i, coupling.x_j, coupling.amplitude, coupling.width}));
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
        case element::ASYMMETRIC_GAUSS_KERNEL:
        {
            const auto kernel = std::dynamic_pointer_cast<element::AsymmetricGaussKernel>(element);
            const auto p = kernel->getParameters();
            elementJson["width"]           = p.width;
            elementJson["amplitude"]       = p.amplitude;
            elementJson["amplitudeGlobal"] = p.amplitudeGlobal;
            elementJson["timeShift"]       = p.timeShift;
            elementJson["circular"]        = p.circular;
            elementJson["normalized"]      = p.normalized;
        }
        break;
        case element::BOOST_STIMULUS:
        {
            const auto boostStimulus = std::dynamic_pointer_cast<element::BoostStimulus>(element);
            const auto boostStimulusParameters = boostStimulus->getParameters();
            elementJson["amplitude"] = boostStimulusParameters.amplitude;
            elementJson["isActive"] = boostStimulusParameters.isActive;
        }
        break;
        case element::MEMORY_TRACE:
        {
            const auto memoryTrace = std::dynamic_pointer_cast<element::MemoryTrace>(element);
            const auto memoryTraceParameters = memoryTrace->getParameters();
            elementJson["tauBuild"]  = memoryTraceParameters.tauBuild;
            elementJson["tauDecay"]  = memoryTraceParameters.tauDecay;
            elementJson["threshold"] = memoryTraceParameters.threshold;
        }
        break;
        case element::NEURAL_FIELD_2D:
        {
            const auto nf = std::dynamic_pointer_cast<element::NeuralField2D>(element);
            const auto p  = nf->getParameters();
            const auto activationFunctionType = p.activationFunction->type;
            elementJson["tau"] = p.tau;
            elementJson["restingLevel"] = p.startingRestingLevel;

            switch (activationFunctionType) {
            case element::ActivationFunctionType::HEAVISIDE:
            {
                if (const auto* fn = dynamic_cast<const element::HeavisideFunction*>(p.activationFunction.get())) {
                    elementJson["activationFunction"] = {
                        {"type", "heaviside"},
                        {"x_shift", fn->getXShift()}
                    };
                }
            }
            break;
            case element::ActivationFunctionType::SIGMOID:
            {
                if (const auto* fn = dynamic_cast<const element::SigmoidFunction*>(p.activationFunction.get())) {
                    elementJson["activationFunction"] = {
                        {"type", "sigmoid"},
                        {"x_shift", fn->getXShift()},
                        {"steepness", fn->getSteepness()},
                    };
                }
            }
            break;
            }
        }
        break;
        case element::GAUSS_STIMULUS_2D:
        {
            const auto gs = std::dynamic_pointer_cast<element::GaussStimulus2D>(element);
            const auto p  = gs->getParameters();
            elementJson["width"]      = p.width;
            elementJson["amplitude"]  = p.amplitude;
            elementJson["position_x"] = p.position_x;
            elementJson["position_y"] = p.position_y;
            elementJson["circular"]   = p.circular;
            elementJson["normalized"] = p.normalized;
        }
        break;
        case element::GAUSS_KERNEL_2D:
        {
            const auto gk = std::dynamic_pointer_cast<element::GaussKernel2D>(element);
            const auto p  = gk->getParameters();
            elementJson["width"]           = p.width;
            elementJson["amplitude"]       = p.amplitude;
            elementJson["amplitudeGlobal"] = p.amplitudeGlobal;
            elementJson["circular"]        = p.circular;
            elementJson["normalized"]      = p.normalized;
        }
        break;
        case element::MEXICAN_HAT_KERNEL_2D:
        {
            const auto mh = std::dynamic_pointer_cast<element::MexicanHatKernel2D>(element);
            const auto p  = mh->getParameters();
            elementJson["widthExc"]        = p.widthExc;
            elementJson["amplitudeExc"]    = p.amplitudeExc;
            elementJson["widthInh"]        = p.widthInh;
            elementJson["amplitudeInh"]    = p.amplitudeInh;
            elementJson["amplitudeGlobal"] = p.amplitudeGlobal;
            elementJson["circular"]        = p.circular;
            elementJson["normalized"]      = p.normalized;
        }
        break;
        case element::NORMAL_NOISE_2D:
        {
            const auto nn = std::dynamic_pointer_cast<element::NormalNoise2D>(element);
            elementJson["amplitude"] = nn->getParameters().amplitude;
        }
        break;
        case element::OSCILLATORY_KERNEL_2D:
        {
            const auto ok = std::dynamic_pointer_cast<element::OscillatoryKernel2D>(element);
            const auto p  = ok->getParameters();
            elementJson["amplitude"]       = p.amplitude;
            elementJson["decay"]           = p.decay;
            elementJson["zeroCrossings"]   = p.zeroCrossings;
            elementJson["amplitudeGlobal"] = p.amplitudeGlobal;
            elementJson["circular"]        = p.circular;
            elementJson["normalized"]      = p.normalized;
        }
        break;
        case element::TIMED_GAUSS_STIMULUS:
        {
            const auto tgs = std::dynamic_pointer_cast<element::TimedGaussStimulus>(element);
            const auto p   = tgs->getParameters();
            elementJson["width"]     = p.width;
            elementJson["amplitude"] = p.amplitude;
            elementJson["position"]  = p.position;
            elementJson["circular"]  = p.circular;
            elementJson["normalized"]= p.normalized;
            json onTimesJson = json::array();
            for (const auto& [start, end] : p.onTimes)
                onTimesJson.push_back({start, end});
            elementJson["onTimes"] = onTimesJson;
        }
        break;
        case element::TIMED_GAUSS_STIMULUS_2D:
        {
            const auto tgs = std::dynamic_pointer_cast<element::TimedGaussStimulus2D>(element);
            const auto p   = tgs->getParameters();
            elementJson["width"]      = p.width;
            elementJson["amplitude"]  = p.amplitude;
            elementJson["position_x"] = p.position_x;
            elementJson["position_y"] = p.position_y;
            elementJson["circular"]   = p.circular;
            elementJson["normalized"] = p.normalized;
            json onTimesJson = json::array();
            for (const auto& [start, end] : p.onTimes)
                onTimesJson.push_back({start, end});
            elementJson["onTimes"] = onTimesJson;
        }
        break;
        case element::BOOST_STIMULUS_2D:
        {
            const auto bs = std::dynamic_pointer_cast<element::BoostStimulus2D>(element);
            const auto p  = bs->getParameters();
            elementJson["amplitude"] = p.amplitude;
            elementJson["isActive"]  = p.isActive;
        }
        break;
        case element::CORRELATED_NORMAL_NOISE_2D:
        {
            const auto cnn = std::dynamic_pointer_cast<element::CorrelatedNormalNoise2D>(element);
            const auto p   = cnn->getParameters();
            elementJson["amplitude"] = p.amplitude;
            elementJson["width"]     = p.width;
            elementJson["circular"]  = p.circular;
        }
        break;
        case element::ASYMMETRIC_GAUSS_KERNEL_2D:
        {
            const auto agk = std::dynamic_pointer_cast<element::AsymmetricGaussKernel2D>(element);
            const auto p   = agk->getParameters();
            elementJson["width"]           = p.width;
            elementJson["amplitude"]       = p.amplitude;
            elementJson["amplitudeGlobal"] = p.amplitudeGlobal;
            elementJson["timeShift_x"]     = p.timeShift_x;
            elementJson["timeShift_y"]     = p.timeShift_y;
            elementJson["circular"]        = p.circular;
            elementJson["normalized"]      = p.normalized;
        }
        break;
        case element::MEMORY_TRACE_2D:
        {
            const auto mt = std::dynamic_pointer_cast<element::MemoryTrace2D>(element);
            const auto p  = mt->getParameters();
            elementJson["tauBuild"]  = p.tauBuild;
            elementJson["tauDecay"]  = p.tauDecay;
            elementJson["threshold"] = p.threshold;
        }
        break;
        case element::RESIZE:
        {
            const auto rz = std::dynamic_pointer_cast<element::Resize>(element);
            const auto p  = rz->getParameters();
            elementJson["method"]      = static_cast<int>(p.method);
            elementJson["input_x_max"] = p.inputDimensions.x_max;
            elementJson["input_d_x"]   = p.inputDimensions.d_x;
        }
        break;
        case element::RESIZE_2D:
        {
            const auto rz = std::dynamic_pointer_cast<element::Resize2D>(element);
            const auto p  = rz->getParameters();
            elementJson["method"]      = static_cast<int>(p.method);
            elementJson["input_x_max"] = p.inputDimensions.x_max;
            elementJson["input_d_x"]   = p.inputDimensions.d_x;
            elementJson["input_y_max"] = p.inputDimensions.y_max;
            elementJson["input_d_y"]   = p.inputDimensions.d_y;
        }
        break;
        case element::COLLAPSE:
        {
            const auto cl = std::dynamic_pointer_cast<element::Collapse>(element);
            const auto p  = cl->getParameters();
            elementJson["compression"] = static_cast<int>(p.compression);
            elementJson["keepAxis"]    = static_cast<int>(p.keepAxis);
            elementJson["input_x_max"] = p.inputDimensions.x_max;
            elementJson["input_d_x"]   = p.inputDimensions.d_x;
            elementJson["input_y_max"] = p.inputDimensions.y_max;
            elementJson["input_d_y"]   = p.inputDimensions.d_y;
        }
        break;
        case element::EXPAND:
        {
            const auto ex = std::dynamic_pointer_cast<element::Expand>(element);
            const auto p  = ex->getParameters();
            elementJson["broadcastProfileAxis"] = static_cast<int>(p.broadcastProfileAxis);
            elementJson["input_x_max"]          = p.inputDimensions.x_max;
            elementJson["input_d_x"]            = p.inputDimensions.d_x;
        }
        break;
        case element::UNSUPERVISED_FIELD_COUPLING:
        {
            const auto ufc = std::dynamic_pointer_cast<element::UnsupervisedFieldCoupling>(element);
            const auto p   = ufc->getParameters();
            elementJson["learningRate"]  = p.learningRate;
            elementJson["learningRule"]  = p.learningRule;
            elementJson["scalar"]        = p.scalar;
            elementJson["input_x_max"]   = p.inputFieldDimensions.x_max;
            elementJson["input_d_x"]     = p.inputFieldDimensions.d_x;
        }
        break;
        case element::SUPERVISED_FIELD_COUPLING:
        {
            const auto sfc = std::dynamic_pointer_cast<element::SupervisedFieldCoupling>(element);
            const auto p   = sfc->getParameters();
            elementJson["learningRate"]  = p.learningRate;
            elementJson["scalar"]        = p.scalar;
            elementJson["input_x_max"]   = p.inputFieldDimensions.x_max;
            elementJson["input_d_x"]     = p.inputFieldDimensions.d_x;
            const auto refSrc = sfc->getReferenceSource();
            elementJson["reference_source_name"] = refSrc ? refSrc->getUniqueName() : "";
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
	        const std::string labelStr = elementJson["label"][1].get<std::string>();
	        const element::ElementLabel elementLabel = elementLabelFromString(labelStr);
	        const int x_max = elementJson["x_max"];
	        const double d_x = elementJson["d_x"];
	        const int y_max = elementJson.contains("y_max") ? elementJson["y_max"].get<int>() : 1;
	        const double d_y = elementJson.contains("d_y") ? elementJson["d_y"].get<double>() : 1.0;

	        switch (elementLabel)
	    	{
	        case element::NEURAL_FIELD:
	            {
		            const double tau = elementJson["tau"];
		            const double restingLevel = elementJson["restingLevel"];

		            auto activationFunctionJson = elementJson["activationFunction"];
		            std::unique_ptr<element::ActivationFunction> activationFunction;
		            if (!activationFunctionJson.is_null()) {
		                std::string activationFunctionType = activationFunctionJson["type"];
		                if (activationFunctionType == "heaviside") {
		                    const double x_shift = activationFunctionJson["x_shift"];
		                    activationFunction = std::make_unique<element::HeavisideFunction>(x_shift);
		                }
		                else if (activationFunctionType == "sigmoid") {
		                    const double x_shift = activationFunctionJson["x_shift"];
		                    const double steepness = activationFunctionJson["steepness"];
		                    activationFunction = std::make_unique<element::SigmoidFunction>(x_shift, steepness);
		                }
		                else if (activationFunctionType == "abs_sigmoid") {
		                    const double x_shift = activationFunctionJson["x_shift"];
		                    const double beta = activationFunctionJson["beta"];
		                    activationFunction = std::make_unique<element::AbsSigmoidFunction>(x_shift, beta);
		                }
		            }
		            if (!activationFunction)
		                activationFunction = std::make_unique<element::SigmoidFunction>(0.0, 10.0);

		            auto neuralField = std::make_shared<element::NeuralField>(
		                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
		                element::NeuralFieldParameters(tau, restingLevel, *activationFunction)
		            );
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
            case element::CORRELATED_NORMAL_NOISE:
            {
                const double amplitude = elementJson["amplitude"];
                const double width = elementJson["width"];
                const bool circular = elementJson["circular"];

                auto cnn = std::make_shared<element::CorrelatedNormalNoise>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::CorrelatedNormalNoiseParameters(amplitude, width, circular)
                );
                simulation->addElement(cnn);
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
                if (elementJson.contains("couplings") && elementJson["couplings"].is_array())
                {
                    couplings.reserve(elementJson["couplings"].size());
                    for (const auto& coupling : elementJson["couplings"])
                    {
	                    const double x_i = coupling[0];
					    const double x_j = coupling[1];
                        const double amp = coupling[2];
                        const double width = coupling[3];
					    couplings.push_back(element::GaussCoupling(x_i, x_j, amp, width));
				    }
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
        case element::ASYMMETRIC_GAUSS_KERNEL:
        {
            const double width           = elementJson["width"];
            const double amplitude       = elementJson["amplitude"];
            const double amplitudeGlobal = elementJson["amplitudeGlobal"];
            const double timeShift       = elementJson["timeShift"];
            const bool   circular        = elementJson["circular"];
            const bool   normalized      = elementJson["normalized"];

            auto kernel = std::make_shared<element::AsymmetricGaussKernel>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::AsymmetricGaussKernelParameters(width, amplitude, amplitudeGlobal, timeShift, circular, normalized)
            );
            simulation->addElement(kernel);
        }
        break;
        case element::BOOST_STIMULUS:
        {
            const double amplitude = elementJson["amplitude"];
            const bool isActive = elementJson["isActive"];

            auto boostStimulus = std::make_shared<element::BoostStimulus>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::BoostStimulusParameters(amplitude, isActive)
            );
            simulation->addElement(boostStimulus);
        }
        break;
        case element::MEMORY_TRACE:
        {
            const double tauBuild  = elementJson["tauBuild"];
            const double tauDecay  = elementJson["tauDecay"];
            const double threshold = elementJson["threshold"];

            auto memoryTrace = std::make_shared<element::MemoryTrace>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::MemoryTraceParameters(tauBuild, tauDecay, threshold)
            );
            simulation->addElement(memoryTrace);
        }
        break;
        case element::NEURAL_FIELD_2D:
        {
            const double tau = elementJson["tau"];
            const double restingLevel = elementJson["restingLevel"];

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
            auto nf = std::make_shared<element::NeuralField2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::NeuralField2DParameters(tau, restingLevel, *activationFunction)
            );
            simulation->addElement(nf);
        }
        break;
        case element::GAUSS_STIMULUS_2D:
        {
            const double width      = elementJson["width"];
            const double amplitude  = elementJson["amplitude"];
            const double position_x = elementJson["position_x"];
            const double position_y = elementJson["position_y"];
            const bool circular     = elementJson["circular"];
            const bool normalized   = elementJson["normalized"];

            auto gs = std::make_shared<element::GaussStimulus2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::GaussStimulus2DParameters(width, amplitude, position_x, position_y, circular, normalized)
            );
            simulation->addElement(gs);
        }
        break;
        case element::GAUSS_KERNEL_2D:
        {
            const double width           = elementJson["width"];
            const double amplitude       = elementJson["amplitude"];
            const double amplitudeGlobal = elementJson["amplitudeGlobal"];
            const bool circular          = elementJson["circular"];
            const bool normalized        = elementJson["normalized"];

            auto gk = std::make_shared<element::GaussKernel2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::GaussKernel2DParameters(width, amplitude, amplitudeGlobal, circular, normalized)
            );
            simulation->addElement(gk);
        }
        break;
        case element::MEXICAN_HAT_KERNEL_2D:
        {
            const double widthExc        = elementJson["widthExc"];
            const double amplitudeExc    = elementJson["amplitudeExc"];
            const double widthInh        = elementJson["widthInh"];
            const double amplitudeInh    = elementJson["amplitudeInh"];
            const double amplitudeGlobal = elementJson["amplitudeGlobal"];
            const bool circular          = elementJson["circular"];
            const bool normalized        = elementJson["normalized"];

            auto mh = std::make_shared<element::MexicanHatKernel2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::MexicanHatKernel2DParameters(widthExc, amplitudeExc, widthInh, amplitudeInh, amplitudeGlobal, circular, normalized)
            );
            simulation->addElement(mh);
        }
        break;
        case element::NORMAL_NOISE_2D:
        {
            const double amplitude = elementJson["amplitude"];

            auto nn = std::make_shared<element::NormalNoise2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::NormalNoise2DParameters(amplitude)
            );
            simulation->addElement(nn);
        }
        break;
        case element::OSCILLATORY_KERNEL_2D:
        {
            const double amplitude       = elementJson["amplitude"];
            const double decay           = elementJson["decay"];
            const double zeroCrossings   = elementJson["zeroCrossings"];
            const double amplitudeGlobal = elementJson["amplitudeGlobal"];
            const bool circular          = elementJson["circular"];
            const bool normalized        = elementJson["normalized"];

            auto ok = std::make_shared<element::OscillatoryKernel2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::OscillatoryKernel2DParameters(amplitude, decay, zeroCrossings, amplitudeGlobal, circular, normalized)
            );
            simulation->addElement(ok);
        }
        break;
        case element::TIMED_GAUSS_STIMULUS:
        {
            const double width     = elementJson["width"];
            const double amplitude = elementJson["amplitude"];
            const double position  = elementJson["position"];
            const bool circular    = elementJson["circular"];
            const bool normalized  = elementJson["normalized"];
            std::vector<std::pair<double, double>> onTimes;
            if (elementJson.contains("onTimes") && elementJson["onTimes"].is_array())
                for (const auto& pair : elementJson["onTimes"])
                    onTimes.emplace_back(pair[0].get<double>(), pair[1].get<double>());

            auto tgs = std::make_shared<element::TimedGaussStimulus>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::TimedGaussStimulusParameters(width, amplitude, position, std::move(onTimes), circular, normalized)
            );
            simulation->addElement(tgs);
        }
        break;
        case element::TIMED_GAUSS_STIMULUS_2D:
        {
            const double width      = elementJson["width"];
            const double amplitude  = elementJson["amplitude"];
            const double position_x = elementJson["position_x"];
            const double position_y = elementJson["position_y"];
            const bool circular     = elementJson["circular"];
            const bool normalized   = elementJson["normalized"];
            std::vector<std::pair<double, double>> onTimes;
            if (elementJson.contains("onTimes") && elementJson["onTimes"].is_array())
                for (const auto& pair : elementJson["onTimes"])
                    onTimes.emplace_back(pair[0].get<double>(), pair[1].get<double>());

            auto tgs = std::make_shared<element::TimedGaussStimulus2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::TimedGaussStimulus2DParameters(width, amplitude, position_x, position_y, std::move(onTimes), circular, normalized)
            );
            simulation->addElement(tgs);
        }
        break;
        case element::BOOST_STIMULUS_2D:
        {
            const double amplitude = elementJson["amplitude"];
            const bool isActive    = elementJson["isActive"];

            auto bs = std::make_shared<element::BoostStimulus2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::BoostStimulus2DParameters(amplitude, isActive)
            );
            simulation->addElement(bs);
        }
        break;
        case element::CORRELATED_NORMAL_NOISE_2D:
        {
            const double amplitude = elementJson["amplitude"];
            const double width     = elementJson["width"];
            const bool circular    = elementJson["circular"];

            auto cnn = std::make_shared<element::CorrelatedNormalNoise2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::CorrelatedNormalNoise2DParameters(amplitude, width, circular)
            );
            simulation->addElement(cnn);
        }
        break;
        case element::ASYMMETRIC_GAUSS_KERNEL_2D:
        {
            const double width           = elementJson["width"];
            const double amplitude       = elementJson["amplitude"];
            const double amplitudeGlobal = elementJson["amplitudeGlobal"];
            const double timeShift_x     = elementJson["timeShift_x"];
            const double timeShift_y     = elementJson["timeShift_y"];
            const bool circular          = elementJson["circular"];
            const bool normalized        = elementJson["normalized"];

            auto agk = std::make_shared<element::AsymmetricGaussKernel2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::AsymmetricGaussKernel2DParameters(width, amplitude, amplitudeGlobal, timeShift_x, timeShift_y, circular, normalized)
            );
            simulation->addElement(agk);
        }
        break;
        case element::MEMORY_TRACE_2D:
        {
            const double tauBuild  = elementJson["tauBuild"];
            const double tauDecay  = elementJson["tauDecay"];
            const double threshold = elementJson["threshold"];

            auto mt = std::make_shared<element::MemoryTrace2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::MemoryTrace2DParameters(tauBuild, tauDecay, threshold)
            );
            simulation->addElement(mt);
        }
        break;
        case element::RESIZE:
        {
            const auto method = static_cast<element::InterpolationMethod>(
                elementJson.contains("method") ? elementJson["method"].get<int>() : 0);
            // Tolerate older/hand-edited files missing the input-dimension keys: fall
            // back to the element's own dims rather than throwing and aborting the load.
            const int input_x_max  = elementJson.contains("input_x_max") ? elementJson["input_x_max"].get<int>() : x_max;
            const double input_d_x = elementJson.contains("input_d_x") ? elementJson["input_d_x"].get<double>() : d_x;

            auto rz = std::make_shared<element::Resize>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::ResizeParameters(method, element::ElementDimensions(input_x_max, input_d_x))
            );
            simulation->addElement(rz);
        }
        break;
        case element::RESIZE_2D:
        {
            const auto method = static_cast<element::InterpolationMethod>(
                elementJson.contains("method") ? elementJson["method"].get<int>() : 0);
            const int input_x_max  = elementJson.contains("input_x_max") ? elementJson["input_x_max"].get<int>() : x_max;
            const double input_d_x = elementJson.contains("input_d_x") ? elementJson["input_d_x"].get<double>() : d_x;
            const int input_y_max  = elementJson.contains("input_y_max") ? elementJson["input_y_max"].get<int>() : y_max;
            const double input_d_y = elementJson.contains("input_d_y") ? elementJson["input_d_y"].get<double>() : d_y;

            auto rz = std::make_shared<element::Resize2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::Resize2DParameters(method, element::ElementDimensions(input_x_max, input_y_max, input_d_x, input_d_y))
            );
            simulation->addElement(rz);
        }
        break;
        case element::COLLAPSE:
        {
            const auto compression = static_cast<element::CompressionType>(
                elementJson.contains("compression") ? elementJson["compression"].get<int>() : 0);
            const auto keepAxis = static_cast<element::ProjectionAxis>(
                elementJson.contains("keepAxis") ? elementJson["keepAxis"].get<int>() : 0);
            // Collapse input is 2D; fall back to the output dims if keys are missing.
            const int input_x_max  = elementJson.contains("input_x_max") ? elementJson["input_x_max"].get<int>() : x_max;
            const double input_d_x = elementJson.contains("input_d_x") ? elementJson["input_d_x"].get<double>() : d_x;
            const int input_y_max  = elementJson.contains("input_y_max") ? elementJson["input_y_max"].get<int>() : y_max;
            const double input_d_y = elementJson.contains("input_d_y") ? elementJson["input_d_y"].get<double>() : d_y;

            auto cl = std::make_shared<element::Collapse>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::CollapseParameters(compression, keepAxis, element::ElementDimensions(input_x_max, input_y_max, input_d_x, input_d_y))
            );
            simulation->addElement(cl);
        }
        break;
        case element::EXPAND:
        {
            const auto broadcastProfileAxis = static_cast<element::ProjectionAxis>(
                elementJson.contains("broadcastProfileAxis") ? elementJson["broadcastProfileAxis"].get<int>() : 0);
            const int input_x_max  = elementJson.contains("input_x_max") ? elementJson["input_x_max"].get<int>() : x_max;
            const double input_d_x = elementJson.contains("input_d_x") ? elementJson["input_d_x"].get<double>() : d_x;

            auto ex = std::make_shared<element::Expand>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::ExpandParameters(broadcastProfileAxis, element::ElementDimensions(input_x_max, input_d_x))
            );
            simulation->addElement(ex);
        }
        break;
        case element::UNSUPERVISED_FIELD_COUPLING:
        {
            const double learningRate = elementJson["learningRate"];
            const LearningRule learningRule = elementJson["learningRule"];
            const double scalar = elementJson["scalar"];
            const int input_x_max = elementJson["input_x_max"];
            const double input_d_x = elementJson["input_d_x"];
            auto ufc = std::make_shared<element::UnsupervisedFieldCoupling>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::UnsupervisedFieldCouplingParameters(element::ElementDimensions(input_x_max, input_d_x), learningRule, scalar, learningRate)
            );
            simulation->addElement(ufc);
        }
        break;
        case element::SUPERVISED_FIELD_COUPLING:
        {
            const double learningRate = elementJson["learningRate"];
            const double scalar = elementJson["scalar"];
            const int input_x_max = elementJson["input_x_max"];
            const double input_d_x = elementJson["input_d_x"];
            auto sfc = std::make_shared<element::SupervisedFieldCoupling>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::SupervisedFieldCouplingParameters(element::ElementDimensions(input_x_max, input_d_x), scalar, learningRate)
            );
            simulation->addElement(sfc);
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

                    // Skip interactions whose endpoints were not created (e.g. an
                    // element with an unrecognized/unsupported label was dropped above).
                    // Wiring to a missing element would corrupt the loaded graph.
                    if (!simulation->getElement(keyUniqueName) || !simulation->getElement(uniqueName))
                    {
                        log(tools::logger::WARNING, "Skipping interaction '" + keyUniqueName
                            + "' -> '" + uniqueName + "': one or both elements were not loaded.");
                        continue;
                    }

                    simulation->createInteraction(keyUniqueName, component, uniqueName);
                }
            }

            // Wire reference source for SupervisedFieldCoupling
            const auto label = elementLabelFromString(elementJson["label"][1].get<std::string>());
            if (label == element::SUPERVISED_FIELD_COUPLING && elementJson.contains("reference_source_name"))
            {
                const std::string refName = elementJson["reference_source_name"];
                if (!refName.empty())
                {
                    const auto sfc = std::dynamic_pointer_cast<element::SupervisedFieldCoupling>(simulation->getElement(uniqueName));
                    const auto refSrc = simulation->getElement(refName);
                    if (sfc && refSrc)
                        sfc->addInput(refSrc, "reference");
                    else
                        log(tools::logger::WARNING, "Skipping reference source '" + refName
                            + "' for '" + uniqueName + "': one or both elements were not loaded.");
                }
            }
	    }

    }

}
