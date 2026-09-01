#include "simulation/simulation_file_manager.h"

#include <cmath>
#include <format>
#include <limits>
#include <unordered_set>
#include <vector>


namespace dnf_composer
{
    using json = nlohmann::json;

    namespace
    {
        // True when `value` can size an axis: a positive, finite, whole number small
        // enough to survive the get<int>() the loader performs a few lines later. That
        // last part is not pedantry -- converting an out-of-range double to int is
        // undefined behaviour, so the range has to be checked BEFORE the conversion,
        // not by catching something afterwards.
        // Mirrors the ElementDimensions extent contract (element_parameters.cpp).
        [[nodiscard]] bool isValidAxisExtent(const json& value)
        {
            if (!value.is_number()) {
                return false;
}
            const double extent = value.get<double>();
            return std::isfinite(extent)
                && extent > 0.0
                && extent <= static_cast<double>(std::numeric_limits<int>::max())
                && extent == std::floor(extent);
        }

        // True when `value` can be an axis step: a positive, finite number.
        [[nodiscard]] bool isValidAxisSpacing(const json& value)
        {
            if (!value.is_number()) {
                return false;
}
            const double spacing = value.get<double>();
            return std::isfinite(spacing) && spacing > 0.0;
        }
    }

	SimulationFileManager::SimulationFileManager(const std::shared_ptr<Simulation>& simulation, const std::string& filePath)
		: simulation(simulation), filePath(filePath)
	{
        if (filePath.empty()) {
            this->filePath = tools::utils::getResourceRoot() + "/data/";
}
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
		for (const auto& element : simulation->getElements()) {
            elementsJson.emplace_back(elementToJson(element));
}

        json root;
        root["formatVersion"] = kCurrentFormatVersion;
        root["identifier"] = simulation->getUniqueIdentifier();
        root["deltaT"]     = simulation->getDeltaT();
        root["elements"]   = elementsJson;

        const std::string path = (simDir / (simulation->getUniqueIdentifier() + ".dnf")).string();
        std::ofstream file(path);
        if (file.is_open()) {
            file << root.dump(4);
            log(tools::logger::INFO, std::format("Simulation saved to: {}", path));
        }
        else {
            log(tools::logger::ERROR, std::format("Unable to open file to save simulation: {}", path));
        }
	}

    bool SimulationFileManager::isReadableFormatVersion(const json& root) const
    {
        // No "formatVersion" key means the file predates the field: implicit version 0,
        // which is the object layout this loader has always accepted. Files already on
        // users' disks land here, so this must stay silent and accepting.
        if (!root.contains("formatVersion")) {
            return true;
        }

        const json& declared = root["formatVersion"];
        if (!declared.is_number_integer() || declared.get<long long>() < 0)
        {
            log(tools::logger::ERROR, std::format(
                "Invalid simulation file: \"formatVersion\" must be a whole number >= 0: {}", filePath));
            return false;
        }

        const long long version = declared.get<long long>();
        if (version > kCurrentFormatVersion)
        {
            // Best-effort rather than refusal: the element schema has only ever grown, and
            // jsonToElements() already skips what it does not recognise, so a partial load
            // is far more useful to a user than an empty simulation.
            log(tools::logger::WARNING, std::format(
                "Simulation file {} declares format version {}, but this build of dnf-composer only "
                "knows version {} - this file was written by a newer version of dnf-composer. "
                "Loading it anyway; anything the newer format added will be ignored.",
                filePath, version, kCurrentFormatVersion));
        }

        return true;
    }

    bool SimulationFileManager::extractElementsAndMetadata(const json& root, json& elementsJson) const
    {
        // Version 0, shape A: the oldest format is a bare array of elements, with nowhere
        // to declare a version. Detected by shape, exactly as before.
        if (root.is_array())
        {
            elementsJson = root;
            return true;
        }
        if (!root.is_object())
        {
            log(tools::logger::ERROR, std::format("Invalid simulation file: unexpected JSON root type: {}", filePath));
            return false;
        }

        // Object roots: version 0 (no "formatVersion") and version 1 read identically,
        // so the branch here is only about rejecting a malformed version and warning
        // about a future one. A real migration would fork the parse below.
        if (!isReadableFormatVersion(root)) {
            return false;
        }

        const json& elems = root.contains("elements") ? root["elements"] : json::array();
        if (!elems.is_array())
        {
            log(tools::logger::ERROR, std::format("Invalid simulation file: \"elements\" is not an array: {}", filePath));
            return false;
        }
        elementsJson = elems;

        // Metadata is best-effort: a bad "identifier" or "deltaT" is reported and skipped,
        // leaving the simulation's own value in place, rather than failing the whole load.
        if (root.contains("identifier") && root["identifier"].is_string()) {
            simulation->setUniqueIdentifier(root["identifier"].get<std::string>());
        } else if (root.contains("identifier")) {
            log(tools::logger::ERROR, std::format("Invalid simulation file: \"identifier\" is not a string: {}", filePath));
}

        if (root.contains("deltaT") && root["deltaT"].is_number())
        {
            const double dt = root["deltaT"].get<double>();
            if (std::isfinite(dt) && dt > 0.0) {
                simulation->setDeltaT(dt);
            } else {
                log(tools::logger::ERROR, std::format("Invalid simulation file: \"deltaT\" is not a valid positive number: {}", filePath));
}
        }
        else if (root.contains("deltaT")) {
            log(tools::logger::ERROR, std::format("Invalid simulation file: \"deltaT\" is not a number: {}", filePath));
}

        return true;
    }

    bool SimulationFileManager::buildElementsOrRollBack(const json& elementsJson) const
    {
        // Last-resort guard around element construction. The pre-check in jsonToElements()
        // rejects the malformed inputs it can name, but the element constructors enforce
        // contracts it deliberately does not re-derive -- notably the samples-per-axis
        // ceiling that ElementDimensions applies to x_max/d_x. Without this guard an
        // Exception from any constructor would unwind straight out of
        // loadElementsFromJson() and, in the GUI, out of the render loop (issue #146).
        std::unordered_set<std::string> preExistingNames;
        for (const auto& el : simulation->getElements()) {
            preExistingNames.insert(el->getUniqueName());
}

        try
        {
            // A false return means the up-front validation rejected the file. That happens
            // before any element is added, so there is nothing to roll back -- but the load
            // still failed, and the caller must not go on to report success.
            if (!jsonToElements(elementsJson)) {
                return false;
            }
        }
        catch (const std::exception& e)
        {
            // Drop what this load added so a bad file never leaves a half-built
            // simulation behind. Anything the caller already held is left alone,
            // since loading appends rather than replaces.
            std::vector<std::string> addedNames;
            for (const auto& el : simulation->getElements())
            {
                if (!preExistingNames.contains(el->getUniqueName())) {
                    addedNames.push_back(el->getUniqueName());
}
            }
            for (const auto& name : addedNames) {
                simulation->removeElement(name);
}
            log(tools::logger::ERROR, std::format(
                "Invalid simulation file: could not build the elements of {} ({}) - load aborted.",
                filePath, e.what()));
            return false;
        }

        return true;
    }

    bool SimulationFileManager::willFileLoadSuccessfully() const
    {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            log(tools::logger::ERROR, std::format("Unable to open file to load simulation: {}.", filePath));
            return false;
        }

        json root;
        try {
            file >> root;
        }
        catch (const std::exception& e) {
            log(tools::logger::ERROR, std::format("Error reading JSON file: {}", e.what()));
            return false;
        }

        // A bare array has nowhere to carry "formatVersion" and is always readable at the
        // root level (element-by-element validation happens later, in loadElementsFromJson()).
        if (root.is_array()) {
            return true;
        }
        if (!root.is_object())
        {
            log(tools::logger::ERROR, std::format("Invalid simulation file: unexpected JSON root type: {}", filePath));
            return false;
        }

        return isReadableFormatVersion(root);
    }

    void SimulationFileManager::loadElementsFromJson() const
    {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            log(tools::logger::ERROR, std::format("Unable to open file to load simulation: {}.", filePath));
            return;
        }

        json root;
        try {
            file >> root;
        }
        catch (const std::exception& e) {
            log(tools::logger::ERROR, std::format("Error reading JSON file: {}", e.what()));
            return;
        }

        // extractElementsAndMetadata() applies "identifier" and "deltaT" as a side effect,
        // but the load is not committed until every element is built. Capture them first
        // so a failed build can put them back: rolling back only the elements would leave
        // the simulation renamed and re-timed while holding none of that file's elements,
        // a combination that came from no file at all. Only this scope sees both steps,
        // so the restore belongs here rather than inside either one.
        const std::string previousIdentifier = simulation->getUniqueIdentifier();
        const double previousDeltaT = simulation->getDeltaT();

        json elementsJson;
        if (!extractElementsAndMetadata(root, elementsJson)) {
            return;
        }

        if (!buildElementsOrRollBack(elementsJson)) {
            simulation->setUniqueIdentifier(previousIdentifier);
            simulation->setDeltaT(previousDeltaT);
            return;
        }

        log(tools::logger::INFO, std::format("Simulation loaded from: {}", filePath));

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
        for (const auto& [k, v] : element::ElementLabelToString) {
            if (v == s) { return k;
}
}
        return element::UNINITIALIZED;
    }

    // NOLINTNEXTLINE(readability-function-cognitive-complexity) - one branch per element type for JSON serialization; splitting would scatter a single lookup table across files
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
	            if (const auto *const heavisideActivationFunction = dynamic_cast<const element::HeavisideFunction*>(neuralFieldParameters.activationFunction.get())) {
                    elementJson["activationFunction"] = {
                        {"type", "heaviside"},
                        {"x_shift", heavisideActivationFunction->getXShift()}
                    };
                }
            }
            break;
            case element::ActivationFunctionType::SIGMOID:
            {
	            if (const auto *const sigmoidActivationFunction = dynamic_cast<const element::SigmoidFunction*>(neuralFieldParameters.activationFunction.get())) {
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
                const auto *const absSigmoidFn = dynamic_cast<const element::AbsSigmoidFunction*>(neuralFieldParameters.activationFunction.get());
                if (absSigmoidFn != nullptr) {
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
            elementJson["decayRate"] = fieldCouplingParameters.decayRate;
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
            for (const auto& coupling : gaussFieldCouplingParameters.couplings) {
                elementJson["couplings"].push_back(json::array({coupling.x_i, coupling.x_j, coupling.amplitude, coupling.width}));
}
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
        case element::STIMULUS_SUM:
            // No parameters to serialize.
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
            for (const auto& [start, end] : p.onTimes) {
                onTimesJson.push_back({start, end});
}
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
            for (const auto& [start, end] : p.onTimes) {
                onTimesJson.push_back({start, end});
}
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
        default:
        case element::UNINITIALIZED:
            tools::logger::log(tools::logger::ERROR, "Element label not recognized.");
            break;
        }

        return elementJson;
    }

    // NOLINTNEXTLINE(readability-function-cognitive-complexity) - one branch per element type for JSON deserialization; mirrors elementToJson's structure
    bool SimulationFileManager::jsonToElements(const json& jsonElements) const
    {
        // Validate every element's required common fields up front, before any element
        // is constructed or added to the live simulation. A malformed entry anywhere in
        // the file aborts the whole load with a descriptive error instead of throwing
        // mid-loop and leaving the simulation half-loaded.
        for (size_t i = 0; i < jsonElements.size(); ++i)
        {
            const json& elementJson = jsonElements[i];
            const std::string elementRef = (elementJson.contains("uniqueName") && elementJson["uniqueName"].is_string())
                ? "'" + elementJson["uniqueName"].get<std::string>() + "'"
                : "at index " + std::to_string(i);

            if (!elementJson.contains("uniqueName") || !elementJson["uniqueName"].is_string())
            {
                log(tools::logger::ERROR, "Invalid simulation file: element " + elementRef
                    + R"( is missing a valid "uniqueName": )" + filePath);
                return false;
            }
            if (!elementJson.contains("label") || !elementJson["label"].is_array()
                || elementJson["label"].size() != 2 || !elementJson["label"][1].is_string())
            {
                log(tools::logger::ERROR, "Invalid simulation file: element " + elementRef
                    + R"( has a missing or malformed "label" (expected a 2-element array): )" + filePath);
                return false;
            }
            if (!elementJson.contains("x_max") || !elementJson["x_max"].is_number()
                || !elementJson.contains("d_x") || !elementJson["d_x"].is_number())
            {
                log(tools::logger::ERROR, "Invalid simulation file: element " + elementRef
                    + R"( is missing a valid "x_max" or "d_x": )" + filePath);
                return false;
            }
            if (!isValidAxisExtent(elementJson["x_max"]) || !isValidAxisSpacing(elementJson["d_x"]))
            {
                log(tools::logger::ERROR, "Invalid simulation file: element " + elementRef
                    + R"( has an invalid "x_max" or "d_x" ("x_max" must be a whole number > 0 that )"
                    + R"(fits in an int, "d_x" a finite number > 0): )" + filePath);
                return false;
            }
            // The y axis gets the same treatment as the x axis (issue #146). Both keys
            // are optional -- an older or 1D-only file omits them and defaults to 1 --
            // but when present they must be valid, or ElementDimensions throws from
            // deeper inside the load instead of reporting the file as malformed here.
            if (elementJson.contains("y_max") || elementJson.contains("d_y"))
            {
                const bool yMaxValid = !elementJson.contains("y_max")
                    || isValidAxisExtent(elementJson["y_max"]);
                const bool dYValid = !elementJson.contains("d_y")
                    || isValidAxisSpacing(elementJson["d_y"]);
                if (!yMaxValid || !dYValid)
                {
                    log(tools::logger::ERROR, "Invalid simulation file: element " + elementRef
                        + R"( has an invalid "y_max" or "d_y" ("y_max" must be a whole number > 0 that )"
                        + R"(fits in an int, "d_y" a finite number > 0): )" + filePath);
                    return false;
                }
            }
        }

        // Track names already loaded so duplicate uniqueNames in the file are rejected
        // (mirrors the guard in Simulation::addElement). Used by both passes below.
        std::unordered_set<std::string> seenNames;

         //Iterate over elements in the JSON and reconstruct them
	    for (const auto& elementJson : jsonElements)
        {
	        // Parse common parameters. uniqueName/label/x_max/d_x are all read with at()
	        // rather than the const operator[] used elsewhere before this PR -- the
	        // up-front pre-check above already guarantees these four are present and
	        // well-formed for every element in jsonElements, so at() never actually
	        // throws here in practice, but it keeps this read locally self-evidently
	        // safe instead of relying on that cross-function invariant continuing to
	        // hold (see #163: the same const operator[] pattern on the type-specific
	        // fields below was undefined behavior on a missing key).
	        const std::string uniqueName = elementJson.at("uniqueName");

	        // Reject duplicate element names: keep the first occurrence, skip the rest
	        // (and their interactions in the second pass) with a clear error.
	        if (!seenNames.insert(uniqueName).second)
	        {
	            log(tools::logger::LogLevel::ERROR, std::format("Duplicate element name '{}' in file - skipping this element.", uniqueName));
	            continue;
	        }

	        const std::string labelStr = elementJson.at("label").at(1).get<std::string>();
	        const element::ElementLabel elementLabel = elementLabelFromString(labelStr);
	        const int x_max = elementJson.at("x_max");
	        const double d_x = elementJson.at("d_x");
	        const int y_max = elementJson.contains("y_max") ? elementJson["y_max"].get<int>() : 1;
	        const double d_y = elementJson.contains("d_y") ? elementJson["d_y"].get<double>() : 1.0;

	        switch (elementLabel)
	    	{
	        case element::NEURAL_FIELD:
	            {
		            const double tau = elementJson.at("tau");
		            const double restingLevel = elementJson.at("restingLevel");

		            // "activationFunction" is genuinely optional -- a file that omits it
		            // falls back to the default SigmoidFunction(0.0, 10.0) below. Every
		            // other key in this switch is required, so it is the only branch
		            // guarded with contains() instead of at().
		            std::unique_ptr<element::ActivationFunction> activationFunction;
		            if (elementJson.contains("activationFunction") && !elementJson["activationFunction"].is_null()) {
		                const auto& activationFunctionJson = elementJson["activationFunction"];
		                const std::string activationFunctionType = activationFunctionJson.at("type");
		                if (activationFunctionType == "heaviside") {
		                    const double x_shift = activationFunctionJson.at("x_shift");
		                    activationFunction = std::make_unique<element::HeavisideFunction>(x_shift);
		                }
		                else if (activationFunctionType == "sigmoid") {
		                    const double x_shift = activationFunctionJson.at("x_shift");
		                    const double steepness = activationFunctionJson.at("steepness");
		                    activationFunction = std::make_unique<element::SigmoidFunction>(x_shift, steepness);
		                }
		                else if (activationFunctionType == "abs_sigmoid") {
		                    const double x_shift = activationFunctionJson.at("x_shift");
		                    const double beta = activationFunctionJson.at("beta");
		                    activationFunction = std::make_unique<element::AbsSigmoidFunction>(x_shift, beta);
		                }
		            }
		            if (!activationFunction) {
		                activationFunction = std::make_unique<element::SigmoidFunction>(0.0, 10.0);
}

		            auto neuralField = std::make_shared<element::NeuralField>(
		                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
		                element::NeuralFieldParameters(tau, restingLevel, *activationFunction)
		            );
		            simulation->addElement(neuralField);
		        }
	        	break;
            case element::NORMAL_NOISE:
            {
                const double amplitude = elementJson.at("amplitude");

                auto normalNoise = std::make_shared<element::NormalNoise>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::NormalNoiseParameters(amplitude)
                );
                simulation->addElement(normalNoise);
            }
            break;
            case element::CORRELATED_NORMAL_NOISE:
            {
                const double amplitude = elementJson.at("amplitude");
                const double width = elementJson.at("width");
                const bool circular = elementJson.at("circular");

                auto cnn = std::make_shared<element::CorrelatedNormalNoise>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::CorrelatedNormalNoiseParameters(amplitude, width, circular)
                );
                simulation->addElement(cnn);
            }
            break;
	        case element::GAUSS_KERNEL:
            {
                const double amplitude = elementJson.at("amplitude");
                const double width = elementJson.at("width");
                const bool circular = elementJson.at("circular");
                const bool normalized = elementJson.at("normalized");
                const double amplitudeGlobal = elementJson.at("amplitudeGlobal");

                auto kernel = std::make_shared<element::GaussKernel>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::GaussKernelParameters(width, amplitude, amplitudeGlobal, circular, normalized)
                );
                simulation->addElement(kernel);
            }
            break;
	        case element::MEXICAN_HAT_KERNEL:
            {
                const double amplitudeExc = elementJson.at("amplitudeExc");
                const double widthExc = elementJson.at("widthExc");
                const double amplitudeInh = elementJson.at("amplitudeInh");
                const double widthInh = elementJson.at("widthInh");
                const double amplitudeGlobal = elementJson.at("amplitudeGlobal");
                const bool circular = elementJson.at("circular");
                const bool normalized = elementJson.at("normalized");

                auto kernel = std::make_shared<element::MexicanHatKernel>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::MexicanHatKernelParameters(widthExc, amplitudeExc, widthInh, amplitudeInh, amplitudeGlobal, circular, normalized)
                );
                simulation->addElement(kernel);
            }
            break;
	        case element::GAUSS_STIMULUS:
            {
                const double amplitude = elementJson.at("amplitude");
                const double width = elementJson.at("width");
                const double position = elementJson.at("position");
                const bool circular = elementJson.at("circular");
                const bool normalized = elementJson.at("normalized");

                auto stimulus = std::make_shared<element::GaussStimulus>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::GaussStimulusParameters(width, amplitude, position, circular, normalized)
                );
                simulation->addElement(stimulus);
            }
            break;
	        case element::FIELD_COUPLING:
            {
                const double learningRate = elementJson.at("learningRate");
                const LearningRule learningRule = elementJson.at("learningRule");
                const double scalar = elementJson.at("scalar");
                // decayRate is genuinely optional (unlike the keys above): files saved
                // before decayRate existed don't have this key at all, and must still
                // load (at 0.0, i.e. decay disabled) rather than throw.
                const double decayRate = elementJson.value("decayRate", 0.0);
                // Unlike the keys above, the input field's own dimensions have a genuine
                // default: FieldCouplingParameters defaults inputFieldDimensions to
                // ElementDimensions{} (x_max 100, d_x 1.0) when none is supplied, so an
                // older file that omits these two keys still loads with that default
                // rather than being rejected as malformed. The fallback is all-or-nothing:
                // a file supplying only one of the two keys still gets the full default
                // pair rather than a hybrid of the supplied value and the other default,
                // since a partial ElementDimensions is not a state ElementDimensions'
                // own constructors can produce either.
                const element::ElementDimensions defaultInputDimensions{};
                const bool hasInputDims = elementJson.contains("input_x_max") && elementJson.contains("input_d_x");
                const int input_x_max = hasInputDims
                    ? elementJson["input_x_max"].get<int>() : defaultInputDimensions.x_max;
                const double input_d_x = hasInputDims
                    ? elementJson["input_d_x"].get<double>() : defaultInputDimensions.d_x;
                auto coupling = std::make_shared<element::FieldCoupling>(
                    element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                    element::FieldCouplingParameters(element::ElementDimensions(input_x_max, input_d_x), learningRule, scalar, learningRate, decayRate)
                );
                simulation->addElement(coupling);
            }
            break;
	        case element::GAUSS_FIELD_COUPLING:
            {
				const bool circular = elementJson.at("circular");
                const bool normalized = elementJson.at("normalized");
                // Same genuine, all-or-nothing default as FIELD_COUPLING above:
                // GaussFieldCouplingParameters defaults inputFieldDimensions to
                // ElementDimensions{} (x_max 100, d_x 1.0) unless both keys are present.
                const element::ElementDimensions defaultInputDimensions{};
                const bool hasInputDims = elementJson.contains("input_x_max") && elementJson.contains("input_d_x");
                const int input_x_max = hasInputDims
                    ? elementJson["input_x_max"].get<int>() : defaultInputDimensions.x_max;
                const double input_d_x = hasInputDims
                    ? elementJson["input_d_x"].get<double>() : defaultInputDimensions.d_x;

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
					    couplings.emplace_back(x_i, x_j, amp, width);
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
			        const double decay = elementJson.at("decay");
			        const double zeroCrossings = elementJson.at("zeroCrossings");
			        const double amplitude = elementJson.at("amplitude");
                    const double amplitudeGlobal = elementJson.at("amplitudeGlobal");
			        const bool circular = elementJson.at("circular");
			        const bool normalized = elementJson.at("normalized");

                    auto kernel = std::make_shared<element::OscillatoryKernel>(
				        element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
				        element::OscillatoryKernelParameters(amplitude, decay, zeroCrossings, amplitudeGlobal, circular, normalized)
			        );
			        simulation->addElement(kernel);
		        }
            break;
        case element::ASYMMETRIC_GAUSS_KERNEL:
        {
            const double width           = elementJson.at("width");
            const double amplitude       = elementJson.at("amplitude");
            const double amplitudeGlobal = elementJson.at("amplitudeGlobal");
            const double timeShift       = elementJson.at("timeShift");
            const bool   circular        = elementJson.at("circular");
            const bool   normalized      = elementJson.at("normalized");

            auto kernel = std::make_shared<element::AsymmetricGaussKernel>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::AsymmetricGaussKernelParameters(width, amplitude, amplitudeGlobal, timeShift, circular, normalized)
            );
            simulation->addElement(kernel);
        }
        break;
        case element::BOOST_STIMULUS:
        {
            const double amplitude = elementJson.at("amplitude");
            const bool isActive = elementJson.at("isActive");

            auto boostStimulus = std::make_shared<element::BoostStimulus>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::BoostStimulusParameters(amplitude, isActive)
            );
            simulation->addElement(boostStimulus);
        }
        break;
        case element::MEMORY_TRACE:
        {
            const double tauBuild  = elementJson.at("tauBuild");
            const double tauDecay  = elementJson.at("tauDecay");
            const double threshold = elementJson.at("threshold");

            auto memoryTrace = std::make_shared<element::MemoryTrace>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::MemoryTraceParameters(tauBuild, tauDecay, threshold)
            );
            simulation->addElement(memoryTrace);
        }
        break;
        case element::STIMULUS_SUM:
        {
            auto stimulusSum = std::make_shared<element::StimulusSum>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::StimulusSumParameters()
            );
            simulation->addElement(stimulusSum);
        }
        break;
        case element::NEURAL_FIELD_2D:
        {
            const double tau = elementJson.at("tau");
            const double restingLevel = elementJson.at("restingLevel");

            // Same optional-with-default treatment as NEURAL_FIELD above.
            std::unique_ptr<element::ActivationFunction> activationFunction;
            if (elementJson.contains("activationFunction") && !elementJson["activationFunction"].is_null()) {
                const auto& activationFunctionJson = elementJson["activationFunction"];
                const std::string activationFunctionType = activationFunctionJson.at("type");
                if (activationFunctionType == "heaviside") {
                    const double x_shift = activationFunctionJson.at("x_shift");
                    activationFunction = std::make_unique<element::HeavisideFunction>(x_shift);
                }
                else if (activationFunctionType == "sigmoid") {
                    const double x_shift = activationFunctionJson.at("x_shift");
                    const double steepness = activationFunctionJson.at("steepness");
                    activationFunction = std::make_unique<element::SigmoidFunction>(x_shift, steepness);
                }
                else if (activationFunctionType == "abs_sigmoid") {
                    const double x_shift = activationFunctionJson.at("x_shift");
                    const double beta = activationFunctionJson.at("beta");
                    activationFunction = std::make_unique<element::AbsSigmoidFunction>(x_shift, beta);
                }
            }
            if (!activationFunction) {
                activationFunction = std::make_unique<element::SigmoidFunction>(0.0, 10.0);
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
            const double width      = elementJson.at("width");
            const double amplitude  = elementJson.at("amplitude");
            const double position_x = elementJson.at("position_x");
            const double position_y = elementJson.at("position_y");
            const bool circular     = elementJson.at("circular");
            const bool normalized   = elementJson.at("normalized");

            auto gs = std::make_shared<element::GaussStimulus2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::GaussStimulus2DParameters(width, amplitude, position_x, position_y, circular, normalized)
            );
            simulation->addElement(gs);
        }
        break;
        case element::GAUSS_KERNEL_2D:
        {
            const double width           = elementJson.at("width");
            const double amplitude       = elementJson.at("amplitude");
            const double amplitudeGlobal = elementJson.at("amplitudeGlobal");
            const bool circular          = elementJson.at("circular");
            const bool normalized        = elementJson.at("normalized");

            auto gk = std::make_shared<element::GaussKernel2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::GaussKernel2DParameters(width, amplitude, amplitudeGlobal, circular, normalized)
            );
            simulation->addElement(gk);
        }
        break;
        case element::MEXICAN_HAT_KERNEL_2D:
        {
            const double widthExc        = elementJson.at("widthExc");
            const double amplitudeExc    = elementJson.at("amplitudeExc");
            const double widthInh        = elementJson.at("widthInh");
            const double amplitudeInh    = elementJson.at("amplitudeInh");
            const double amplitudeGlobal = elementJson.at("amplitudeGlobal");
            const bool circular          = elementJson.at("circular");
            const bool normalized        = elementJson.at("normalized");

            auto mh = std::make_shared<element::MexicanHatKernel2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::MexicanHatKernel2DParameters(widthExc, amplitudeExc, widthInh, amplitudeInh, amplitudeGlobal, circular, normalized)
            );
            simulation->addElement(mh);
        }
        break;
        case element::NORMAL_NOISE_2D:
        {
            const double amplitude = elementJson.at("amplitude");

            auto nn = std::make_shared<element::NormalNoise2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::NormalNoise2DParameters(amplitude)
            );
            simulation->addElement(nn);
        }
        break;
        case element::OSCILLATORY_KERNEL_2D:
        {
            const double amplitude       = elementJson.at("amplitude");
            const double decay           = elementJson.at("decay");
            const double zeroCrossings   = elementJson.at("zeroCrossings");
            const double amplitudeGlobal = elementJson.at("amplitudeGlobal");
            const bool circular          = elementJson.at("circular");
            const bool normalized        = elementJson.at("normalized");

            auto ok = std::make_shared<element::OscillatoryKernel2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::OscillatoryKernel2DParameters(amplitude, decay, zeroCrossings, amplitudeGlobal, circular, normalized)
            );
            simulation->addElement(ok);
        }
        break;
        case element::TIMED_GAUSS_STIMULUS:
        {
            const double width     = elementJson.at("width");
            const double amplitude = elementJson.at("amplitude");
            const double position  = elementJson.at("position");
            const bool circular    = elementJson.at("circular");
            const bool normalized  = elementJson.at("normalized");
            std::vector<std::pair<double, double>> onTimes;
            if (elementJson.contains("onTimes") && elementJson["onTimes"].is_array()) {
                for (const auto& pair : elementJson["onTimes"]) {
                    onTimes.emplace_back(pair[0].get<double>(), pair[1].get<double>());
}
}

            auto tgs = std::make_shared<element::TimedGaussStimulus>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, d_x)),
                element::TimedGaussStimulusParameters(width, amplitude, position, std::move(onTimes), circular, normalized)
            );
            simulation->addElement(tgs);
        }
        break;
        case element::TIMED_GAUSS_STIMULUS_2D:
        {
            const double width      = elementJson.at("width");
            const double amplitude  = elementJson.at("amplitude");
            const double position_x = elementJson.at("position_x");
            const double position_y = elementJson.at("position_y");
            const bool circular     = elementJson.at("circular");
            const bool normalized   = elementJson.at("normalized");
            std::vector<std::pair<double, double>> onTimes;
            if (elementJson.contains("onTimes") && elementJson["onTimes"].is_array()) {
                for (const auto& pair : elementJson["onTimes"]) {
                    onTimes.emplace_back(pair[0].get<double>(), pair[1].get<double>());
}
}

            auto tgs = std::make_shared<element::TimedGaussStimulus2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::TimedGaussStimulus2DParameters(width, amplitude, position_x, position_y, std::move(onTimes), circular, normalized)
            );
            simulation->addElement(tgs);
        }
        break;
        case element::BOOST_STIMULUS_2D:
        {
            const double amplitude = elementJson.at("amplitude");
            const bool isActive    = elementJson.at("isActive");

            auto bs = std::make_shared<element::BoostStimulus2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::BoostStimulus2DParameters(amplitude, isActive)
            );
            simulation->addElement(bs);
        }
        break;
        case element::CORRELATED_NORMAL_NOISE_2D:
        {
            const double amplitude = elementJson.at("amplitude");
            const double width     = elementJson.at("width");
            const bool circular    = elementJson.at("circular");

            auto cnn = std::make_shared<element::CorrelatedNormalNoise2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::CorrelatedNormalNoise2DParameters(amplitude, width, circular)
            );
            simulation->addElement(cnn);
        }
        break;
        case element::ASYMMETRIC_GAUSS_KERNEL_2D:
        {
            const double width           = elementJson.at("width");
            const double amplitude       = elementJson.at("amplitude");
            const double amplitudeGlobal = elementJson.at("amplitudeGlobal");
            const double timeShift_x     = elementJson.at("timeShift_x");
            const double timeShift_y     = elementJson.at("timeShift_y");
            const bool circular          = elementJson.at("circular");
            const bool normalized        = elementJson.at("normalized");

            auto agk = std::make_shared<element::AsymmetricGaussKernel2D>(
                element::ElementCommonParameters(uniqueName, element::ElementDimensions(x_max, y_max, d_x, d_y)),
                element::AsymmetricGaussKernel2DParameters(width, amplitude, amplitudeGlobal, timeShift_x, timeShift_y, circular, normalized)
            );
            simulation->addElement(agk);
        }
        break;
        case element::MEMORY_TRACE_2D:
        {
            const double tauBuild  = elementJson.at("tauBuild");
            const double tauDecay  = elementJson.at("tauDecay");
            const double threshold = elementJson.at("threshold");

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
	    default:
	    case element::UNINITIALIZED:
            tools::logger::log(tools::logger::ERROR, "Element label not recognized.");
        break;
	    }
    }

	    // Iterate to create interactions
	    std::unordered_set<std::string> wiredNames;
	    for (const auto& elementJson : jsonElements)
	    {
	        const std::string uniqueName = elementJson.at("uniqueName");

	        // Skip the interactions of a duplicate entry: only the first occurrence of a
	        // name was loaded, so wiring a later duplicate's inputs would attach them to
	        // the wrong (first) element.
	        if (!wiredNames.insert(uniqueName).second) {
	            continue;
}

	        // "inputs" is not covered by the pre-check above (only uniqueName/label/
	        // x_max/d_x/y_max/d_y are), so this one is a genuine required-key check,
	        // not just a defensive at() -- a hand-edited file that omits "inputs"
	        // entirely is rejected here instead of reading past the end of the object.
	        const auto& inputsJson = elementJson.at("inputs");

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
                        const std::string message = std::format("Skipping interaction '{}' -> '{}': one or both elements were not loaded.",
                            keyUniqueName, uniqueName);
                        log(tools::logger::WARNING, message);
                        continue;
                    }

                    simulation->createInteraction(keyUniqueName, component, uniqueName);
                }
            }

	    }

        return true;
    }

}
