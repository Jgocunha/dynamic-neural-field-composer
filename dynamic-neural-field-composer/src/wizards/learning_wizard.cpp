#include "./wizards/learning_wizard.h"



namespace dnf_composer
{
    LearningWizard::LearningWizard(const std::shared_ptr<Simulation>& simulation, const std::string& fieldCouplingUniqueId)
        :simulation(simulation)
    {
        setFieldCoupling(fieldCouplingUniqueId);
        setNeuralFieldPre();
        setNeuralFieldPost();
        fieldCoupling->resetWeights();

        //const std::string pathPrefix = std::string(OUTPUT_DIRECTORY) + "/inter-field-synaptic-connections/" + fieldCoupling->getUniqueName() + "_";
        const std::string pathPrefix = std::string(OUTPUT_DIRECTORY) + "/" + fieldCoupling->getUniqueName() + "_";
        pathToFieldActivationPre = pathPrefix + neuralFieldPre->getUniqueName() + ".txt";
        pathToFieldActivationPost = pathPrefix + neuralFieldPost->getUniqueName() + ".txt";
    }

    void LearningWizard::setDataFilePath(const std::string& filePath)
    {
        //const std::string pathPrefix = filePath + "/inter-field-synaptic-connections/" + fieldCoupling->getUniqueName() + "_";
        const std::string pathPrefix = filePath + "/" + fieldCoupling->getUniqueName() + "_";
        pathToFieldActivationPre = pathPrefix + neuralFieldPre->getUniqueName() + ".txt";
        pathToFieldActivationPost = pathPrefix + neuralFieldPost->getUniqueName() + ".txt";
    }

    void LearningWizard::setTargetPeakLocationsForNeuralFieldPre(const std::vector<std::vector<double>>& targetPeakLocationsForNeuralFieldPre)
    {
        this->targetPeakLocationsForNeuralFieldPre = targetPeakLocationsForNeuralFieldPre;
    }

    void LearningWizard::setTargetPeakLocationsForNeuralFieldPost(const std::vector<std::vector<double>>& targetPeakLocationsForNeuralFieldPost)
    {
        this->targetPeakLocationsForNeuralFieldPost = targetPeakLocationsForNeuralFieldPost;
    }

    void LearningWizard::simulateAssociation()
    {
        static constexpr int timeSteps = 25; // this value is hardcoded and depends on the fields timescale...

        for (int i = 0; i < static_cast<int>(targetPeakLocationsForNeuralFieldPre.size()); i++)
        {
            // Create Gaussian stimuli in the input field
            for (int j = 0; j < targetPeakLocationsForNeuralFieldPre[i].size(); j++)
            {
                auto kernel = std::dynamic_pointer_cast<dnf_composer::element::GaussKernel>(simulation->getElement("per - per"));
                auto kernel_width = kernel->getParameters().width;
                auto kernel_amplitude = kernel->getParameters().amplitude;
                gaussStimulusParameters.amplitude = kernel_amplitude;
                gaussStimulusParameters.width = kernel_width;
                gaussStimulusParameters.normalized = false;
                gaussStimulusParameters.circular = true;

                const std::string stimulusName = "Input Gaussian Stimulus " + std::to_string(i + 1) + std::to_string(j + 1);
                const element::ElementIdentifiers stimulusIdentifiers{ stimulusName };

                element::ElementSpatialDimensionParameters stimulusDimensions{ neuralFieldPre->getMaxSpatialDimension(), neuralFieldPre->getStepSize() };
                element::ElementCommonParameters commonParameters{ stimulusIdentifiers, stimulusDimensions };

                gaussStimulusParameters.position = targetPeakLocationsForNeuralFieldPre[i][j];
                std::shared_ptr<element::GaussStimulus> stimulus = std::make_shared<element::GaussStimulus>(commonParameters, gaussStimulusParameters);

                simulation->addElement(stimulus);
                neuralFieldPre->addInput(stimulus);

                simulation->init();
                fieldCoupling->resetWeights();
                for (int k = 0; k < timeSteps; k++)
                    simulation->step();
            }

            // Create Gaussian stimuli in the output field
            for (size_t j = 0; j < targetPeakLocationsForNeuralFieldPost[i].size(); j++)
            {
                auto kernel = std::dynamic_pointer_cast<dnf_composer::element::GaussKernel>(simulation->getElement("out - out"));
                auto kernel_width = kernel->getParameters().width;
                auto kernel_amplitude = kernel->getParameters().amplitude;
                gaussStimulusParameters.amplitude = kernel_amplitude;
                gaussStimulusParameters.width = kernel_width;

                const std::string stimulusName = "Output Gaussian Stimulus " + std::to_string(i + 1) + std::to_string(j + 1);
                const element::ElementIdentifiers stimulusIdentifiers{ stimulusName };

                element::ElementSpatialDimensionParameters stimulusDimensions{ neuralFieldPost->getMaxSpatialDimension(), neuralFieldPost->getStepSize() };
                element::ElementCommonParameters commonParameters{ stimulusIdentifiers, stimulusDimensions };

                gaussStimulusParameters.position = targetPeakLocationsForNeuralFieldPost[i][j];
                std::shared_ptr<element::GaussStimulus> stimulus = std::make_shared<element::GaussStimulus>(commonParameters, gaussStimulusParameters);

                simulation->addElement(stimulus);
                neuralFieldPost->addInput(stimulus);

                simulation->init();
                fieldCoupling->resetWeights();

                for (int k = 0; k < timeSteps; k++)
                    simulation->step();
            }

            // Remove gaussian stimuli from the input field
            // This works if the field is self-sustained, but if it is not, the field will return to its resting level
            for (size_t j = 0; j < targetPeakLocationsForNeuralFieldPre[i].size(); j++)
            {
                std::string stimulusName = "Input Gaussian Stimulus " + std::to_string(i + 1) + std::to_string(j + 1);
                simulation->removeElement(stimulusName);
            }

            // Wait for the input field to settle again
            for (int k = 0; k < timeSteps; k++)
                simulation->step();


            std::vector<double>* input = simulation->getComponentPtr(neuralFieldPre->getUniqueName(), "activation");
            std::vector<double>* output = simulation->getComponentPtr(neuralFieldPost->getUniqueName(), "activation");

            // normalize data (remove resting level and normalize between -1 and 1))
            *input = normalizeFieldActivation(*input);
            *output = normalizeFieldActivation(*output);
            
            // save data
            saveFieldActivation(input, pathToFieldActivationPre);
            saveFieldActivation(output, pathToFieldActivationPost);

            // Remove gaussian stimuli from the output field
            for (size_t j = 0; j < targetPeakLocationsForNeuralFieldPost[i].size(); j++)
            {
                std::string stimulusName = "Output Gaussian Stimulus " + std::to_string(i + 1) + std::to_string(j + 1);
                simulation->removeElement(stimulusName);
            }

            // restart simulation
            simulation->init();
        }
    }

    // this generates a weighted matrix WITHOUT 0 value weights
    std::vector<double> LearningWizard::normalizeFieldActivation(std::vector<double>& vec)
    {
        for (double& val : vec)
            if (val < 0.00001)
                val = 0;

        constexpr double safetyFactor = 0.000001;
        // Find the minimum and maximum values in the vector
        const double maxVal = *std::max_element(vec.begin(), vec.end()) + safetyFactor;
        const double minVal = *std::min_element(vec.begin(), vec.end()) - safetyFactor;

        // Normalize the vector
        std::vector<double> normalizedVec;
        for (double& val : vec)
        {
            if (val != 0.0)
            {
                //val = (val - minVal) / (maxVal - minVal) * 2.0 - 1.0;
                val = (val - minVal) / (maxVal - minVal);
            }

            normalizedVec.push_back(val);
        }

        return normalizedVec;
    }

    // this generates a weighted matrix WITH 0 value weights
    //std::vector<double> LearningWizard::normalizeFieldActivation(std::vector<double>& vec, const double& restingLevel)
    //{
    //    // this removes the resting level
    //    // the code works without this  
    //    //for (double& val : vec)
    //    //    val += restingLevel;

    //    for (double& val : vec)
    //        if (val < 0.01)
    //            val = 0;

    //    constexpr int safetyFactor = 0;
    //    // Find the minimum and maximum values in the vector
    //    const double maxVal = *std::max_element(vec.begin(), vec.end()) + safetyFactor;
    //    const double minVal = *std::min_element(vec.begin(), vec.end()) - safetyFactor;

    //    // Normalize the vector
    //    std::vector<double> normalizedVec;
    //    for (double& val : vec)
    //    {
    //        if (val != 0.0)
    //        {
    //            //val = (val - minVal) / (maxVal - minVal) * 2.0 - 1.0;
    //            val = (val - minVal) / (maxVal - minVal);
    //        }

    //        normalizedVec.push_back(val);
    //    }

    //    return normalizedVec;
    //}


    void LearningWizard::saveFieldActivation(const std::vector<double>* fieldActivation, const std::string& filename)
    {
        std::ofstream file(filename, std::ios::app); // Open file in append mode
        if (file.is_open())
        {
            for (const auto& element : (*fieldActivation))
                file << element << " ";  // Write element to file separated by a space
            file << '\n';
            file.close();
        }
        else
        {
            const std::string message = "Failed to save data to " + filename + ".";
            log(tools::logger::LogLevel::ERROR, message);
        }
    }

    std::vector<double> LearningWizard::readFieldActivation(const std::string& filename, int line)
    {
        std::ifstream file(filename);
        std::vector<double> data;

        if (file.is_open())
        {
            std::string lineData;
            int currentLine = 0;

            // Read lines from the file until the desired line is reached
            while (std::getline(file, lineData) && currentLine < line)
                currentLine++;

            if (currentLine == line)
            {
                std::istringstream iss(lineData);
                double value;
                while (iss >> value)
                    data.push_back(value);
            }
            else
            {
                const std::string message = "Error training the field coupling weights. "
                    "Line " + std::to_string(static_cast<int>(line)) + " not found in " + filename + ".";
                log(tools::logger::LogLevel::ERROR, message);
            }
            file.close();
        }
        else
        {
            const std::string message = "Failed to open file " + filename + ".";
            log(tools::logger::LogLevel::ERROR, message);
        }

        return data;
    }

    void LearningWizard::trainWeights(const int iterations) const
    {
        // check how much lines "temp_input.txt", and "temp_output.txt" have
        const int numLinesInput = tools::utils::countNumOfLinesInFile(pathToFieldActivationPre);
        const int numLinesOutput = tools::utils::countNumOfLinesInFile(pathToFieldActivationPost);

        if (numLinesInput != numLinesOutput)
        {
            const std::string message = "Error training the field coupling weights. The files " + pathToFieldActivationPre + " and " + pathToFieldActivationPost + " have a different number of lines.";
            log(tools::logger::LogLevel::ERROR, message);
        }

        // read data and update weights
        int lineCount = 0;
        std::vector<double> input = std::vector<double>(neuralFieldPre->getSize());
        std::vector<double> output = std::vector<double>(neuralFieldPost->getSize());
        for (int i = 0; i < iterations; i++)
        {
            input = readFieldActivation(pathToFieldActivationPre, lineCount);
            output = readFieldActivation(pathToFieldActivationPost, lineCount);
            fieldCoupling->updateWeights(input, output);
            lineCount = (lineCount + 1) % (numLinesInput);
        }
        //fieldCoupling->saveWeights();
    }

    void LearningWizard::setFieldCoupling(const std::string& fieldCouplingUniqueId)
    {
        const std::shared_ptr<element::Element> fieldCouplingElement = simulation->getElement(fieldCouplingUniqueId);
        fieldCoupling = std::dynamic_pointer_cast<element::FieldCoupling>(fieldCouplingElement);
    }

    void LearningWizard::setNeuralFieldPre()
    {
        const std::shared_ptr<element::Element> inputElement = fieldCoupling->getInputs().at(0);
        neuralFieldPre = std::dynamic_pointer_cast<element::NeuralField>(inputElement);
    }

    void LearningWizard::setNeuralFieldPost()
    {
        const std::shared_ptr<element::Element> outputElement = simulation->getElementsThatHaveSpecifiedElementAsInput(fieldCoupling->getUniqueName()).at(0);
        neuralFieldPost = std::dynamic_pointer_cast<element::NeuralField>(outputElement);
    }

    void LearningWizard::saveWeights() const
    {
        fieldCoupling->saveWeights();
    }

    void LearningWizard::clearTargetPeakLocationsFromFiles() const
    {
        std::ofstream file(pathToFieldActivationPre);
        file.close();
        file.open(pathToFieldActivationPost);
        file.close();
    }
}