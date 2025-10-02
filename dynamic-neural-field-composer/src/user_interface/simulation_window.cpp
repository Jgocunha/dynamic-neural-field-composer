// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "user_interface/simulation_window.h"


namespace dnf_composer
{
	namespace user_interface
	{
		SimulationWindow::SimulationWindow(const std::shared_ptr<Simulation>& simulation)
			: simulation(simulation)
		{
		}

		void SimulationWindow::render()
		{
			if (ImGui::Begin("Simulation Control", nullptr, imgui_kit::getGlobalWindowFlags()))
			{
				renderSimulationControlButtons();
				renderSimulationProperties();
				renderAddElement();
				renderSetInteraction();
				renderRemoveElement();
				renderLogElementProperties();
				renderExportElementComponents();
			}
			ImGui::End();
		}

		void SimulationWindow::renderSimulationControlButtons() const
		{

			if (ImGui::Button("Start"))
				simulation->init();

			ImGui::SameLine();

			if (ImGui::Button("Pause"))
				simulation->pause();

			ImGui::SameLine();

			if (ImGui::Button("Resume"))
				simulation->resume();

			ImGui::SameLine();

			if (ImGui::Button("Stop"))
				simulation->close();

			// Section for running a specific number of iterations
			ImGui::Separator();

			static bool runNSteps = false;
			static int iterationCount = 1000; // Default value
			static int currentIteration = 0;
			ImGui::SetNextItemWidth(120);
			ImGui::InputInt("Iterations", &iterationCount, 1, 10);
			if (iterationCount < 1) iterationCount = 1; // Ensure positive value

			ImGui::SameLine();
			if (ImGui::Button("Run N steps"))
			{
				// Run the simulation for the specified number of iterations
				currentIteration = simulation->getT();
				if (!simulation->isInitialized())
				{
					simulation->init();
					currentIteration = 0;
				}
				simulation->resume();
				runNSteps = true;
				tools::logger::log(tools::logger::INFO, "Simulation has started running for "
					+ std::to_string(iterationCount) + " steps.");
			}

			if (runNSteps)
			{
				// simulation has finished running the specified number of iterations
				if (simulation->getT() >= iterationCount + currentIteration)
				{
					runNSteps = false;
					simulation->pause();
					tools::logger::log(tools::logger::INFO, "Simulation has finished running "
						+ std::to_string(iterationCount) + " steps.");
				}
			}

		}

		void SimulationWindow::renderSimulationProperties() const
		{
			ImGui::Separator();

			const std::string& identifier = simulation->getIdentifier();
			const double deltaT = simulation->getDeltaT();
			const double tZero = simulation->getTZero();
			const double t = simulation->getT();

			ImGui::Text("Identifier: ");
			ImGui::SameLine();
			ImGui::TextUnformatted(identifier.c_str());

			ImGui::Text("Time Step (deltaT): ");
			ImGui::SameLine();
			ImGui::Text("%.3f", deltaT);

			ImGui::Text("Start Time (tZero): ");
			ImGui::SameLine();
			ImGui::Text("%.3f", tZero);

			ImGui::Text("Current Time (t): ");
			ImGui::SameLine();
			ImGui::Text("%.3f", t);

			ImGui::Separator();
		}

		void SimulationWindow::renderAddElement() const
		{
			ImGui::PushID("add element");
			if (ImGui::CollapsingHeader("Add element"))
			{
				for (const auto& pair : element::ElementLabelToString)
					if (pair.first != element::ElementLabel::UNINITIALIZED)
						renderElementProperties(pair);
			}
			ImGui::PopID();
		}

		void SimulationWindow::renderSetInteraction() const
		{
			ImGui::PushID("set interaction");
			if (ImGui::CollapsingHeader("Set interactions between elements"))
			{
				for (const auto& element : simulation->getElements())
				{
					const auto elementId = element->getUniqueName();
					if (ImGui::TreeNode(elementId.c_str()))
					{
						static std::string selectedElementId{};
						static int currentElementIdx = 0;

						// Section 1: Add New Input
						ImGui::Text("Select the element you want to define as input");
						if (ImGui::BeginListBox("##Element list available as inputs"))
						{
							for (const auto& other_element : simulation->getElements())
							{
								std::string inputElementId = other_element->getUniqueName();
								const bool isSelected = (currentElementIdx == other_element->getUniqueIdentifier());
								if (ImGui::Selectable(inputElementId.c_str(), isSelected))
								{
									selectedElementId = inputElementId;
									currentElementIdx = other_element->getUniqueIdentifier();
								}

								if (isSelected)
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndListBox();
						}

						if (ImGui::Button("Add", { 100.0f, 30.0f }))
						{
							auto input = simulation->getElement(selectedElementId);
							element->addInput(input);
							simulation->init();
						}

						// Section 2: Show Existing Connections
						ImGui::Separator();
						ImGui::Text("Currently connected elements:");
						const auto& inputs = element->getInputs();
						if (inputs.empty())
						{
							ImGui::Text("No connections.");
						}
						else
						{
							for (size_t i = 0; i < inputs.size(); ++i)
							{
								const auto& connectedElement = inputs[i];
								ImGui::BulletText("%s", connectedElement->getUniqueName().c_str());

								// Add a "Remove" button for each connection
								ImGui::SameLine();
								std::string buttonLabel = "Remove##" + std::to_string(i);
								if (ImGui::Button(buttonLabel.c_str()))
								{
									element->removeInput(connectedElement->getUniqueIdentifier());
									simulation->init();
								}
							}
						}
						ImGui::Separator();

						ImGui::TreePop();
					}
				}
			}
			ImGui::PopID();
		}

		void SimulationWindow::renderRemoveElement() const
		{
			ImGui::PushID("remove element");
			if (ImGui::CollapsingHeader("Remove elements from simulation"))
			{
				for (const auto& element : simulation->getElements())
				{
					const auto elementId = element->getUniqueName();
					if (ImGui::TreeNode(elementId.c_str()))
					{
						if (ImGui::Button("Remove", { 100.0f, 30.0f }))
						{
							simulation->removeElement(elementId);
							simulation->init();
						}
						ImGui::TreePop();
					}
				}
			}
			ImGui::PopID();
		}

		void SimulationWindow::renderElementProperties(const std::pair<int, std::string>& elementId) const
		{
			if (ImGui::TreeNode(elementId.second.c_str()))
			{
				switch (elementId.first)
				{
				case element::ElementLabel::NEURAL_FIELD:
					addElementNeuralField();
					break;
				case element::ElementLabel::GAUSS_STIMULUS:
					addElementGaussStimulus();
					break;
				case element::ElementLabel::FIELD_COUPLING:
					addElementFieldCoupling();
					break;
				case element::ElementLabel::GAUSS_KERNEL:
					addElementGaussKernel();
					break;
				case element::ElementLabel::MEXICAN_HAT_KERNEL:
					addElementMexicanHatKernel();
					break;
				case element::ElementLabel::NORMAL_NOISE:
					addElementNormalNoise();
					break;
				case element::ElementLabel::GAUSS_FIELD_COUPLING:
					addElementGaussFieldCoupling();
					break;
				default:
					log(tools::logger::LogLevel::ERROR, "There is a missing element in the TreeNode in simulation window.");
					break;
				}
				ImGui::TreePop();
			}
		}

		void SimulationWindow::renderLogElementProperties() const
		{
			ImGui::PushID("log element parameters");
			if (ImGui::CollapsingHeader("Log element parameters"))
			{
				for (const auto& element : simulation->getElements())
				{
					const auto elementId = element->getUniqueName();
					if (ImGui::TreeNode(elementId.c_str()))
					{
						if (ImGui::Button("Log", { 100.0f, 30.0f }))
												{
							element->print();
						}
						ImGui::TreePop();
					}
				}
			}
			ImGui::PopID();
		}

		void SimulationWindow::renderExportElementComponents() const
		{
			ImGui::PushID("export element components");
			if (ImGui::CollapsingHeader("Export element components"))
			{
				for (const auto& element : simulation->getElements())
				{
					const auto elementId = element->getUniqueName();
					if (ImGui::TreeNode(elementId.c_str()))
					{
						for (const auto& componentName : element->getComponentList())
						{
							if (ImGui::TreeNode(componentName.c_str()))
							{
								if (ImGui::Button("Export", { 100.0f, 30.0f }))
								{
									simulation->exportComponentToFile(elementId, componentName);
								}
								ImGui::TreePop();
							}
						}
						ImGui::TreePop();
					}
				}
			}
			ImGui::PopID();
		}

		void SimulationWindow::addElementNeuralField() const
		{
			ImGui::PushID("neural field");
			static char id[CHAR_SIZE] = "neural field u";
			ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
			static int x_max = 100;
			ImGui::InputInt("x_max", &x_max, 1.0, 10.0);
			static double d_x = 1.0;
			ImGui::InputDouble("d_x", &d_x, 0.1, 0.5, "%.2f");
			static double tau = 25;
			ImGui::InputDouble("tau", &tau, 1.0f, 10.0f, "%.2f");
			static double sigmoidSteepness = 5.0f;
			ImGui::InputDouble("sigmoid steepness", &sigmoidSteepness, 1.0f, 10.0f, "%.2f");
			static double restingLevel = -10.0f;
			ImGui::InputDouble("resting level", &restingLevel, 1.0f, 10.0f, "%.2f");

			if (ImGui::Button("Add", { 100.0f, 30.0f }))
			{
				const element::SigmoidFunction activationFunction{ 0, sigmoidSteepness };
				const element::NeuralFieldParameters nfp = { tau, restingLevel, activationFunction };

				const element::ElementIdentifiers neuralFieldIdentifiers{id};
				const element::ElementDimensions neuralFieldDimensions{ x_max, d_x };
				const element::ElementCommonParameters commonParameters{ neuralFieldIdentifiers, neuralFieldDimensions };

				const std::shared_ptr<element::NeuralField> neuralField(new element::NeuralField(commonParameters, nfp));
				simulation->addElement(neuralField);
			}
			ImGui::PopID();
		}

		void SimulationWindow::addElementGaussStimulus() const
		{
			ImGui::PushID("gauss stimulus");
			static char id[CHAR_SIZE] = "gauss stimulus a";
			ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
			static int x_max = 100;
			ImGui::InputInt("x_max", &x_max, 1.0, 10.0);
			static double d_x = 1.0;
			ImGui::InputDouble("d_x", &d_x, 0.1, 0.5, "%.2f");
			static double sigma = 5;
			ImGui::InputDouble("sigma", &sigma, 1.0f, 10.0f, "%.2f");
			static double amplitude = 20;
			ImGui::InputDouble("amplitude", &amplitude, 1.0f, 10.0f, "%.2f");
			static double position = 50;
			ImGui::InputDouble("position", &position, 1.0f, 10.0f, "%.2f");
			static bool normalized = false;
			ImGui::Checkbox("normalized", &normalized);
			static bool circular = false;
			ImGui::Checkbox("circular", &circular);

			if (ImGui::Button("Add", { 100.0f, 30.0f }))
			{
				const element::GaussStimulusParameters gsp = { sigma, amplitude, position, circular, normalized};
				const element::ElementDimensions dimensions{ x_max, d_x };
				const std::shared_ptr<element::GaussStimulus> gaussStimulus(new element::GaussStimulus ({id, dimensions}, gsp));
				simulation->addElement(gaussStimulus);
			}
			ImGui::PopID();
		}

		void SimulationWindow::addElementNormalNoise() const
		{
			ImGui::PushID("normal noise");
			static char id[CHAR_SIZE] = "normal noise a";
			ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
			static int x_max = 100;
			ImGui::InputInt("x_max", &x_max, 1.0, 10.0);
			static double d_x = 1.0;
			ImGui::InputDouble("d_x", &d_x, 0.1, 0.5, "%.2f");
			static double amplitude = 0.01;
			ImGui::InputDouble("amplitude", &amplitude, 0.01f, 1.0f, "%.2f");

			if (ImGui::Button("Add", { 100.0f, 30.0f }))
			{
				const element::NormalNoiseParameters nnp = { amplitude };
				const element::ElementDimensions dimensions{ x_max, d_x };
				const std::shared_ptr<element::NormalNoise> normalNoise( new element::NormalNoise({ id, dimensions}, nnp));
				const element::GaussKernelParameters gkp = { 0.25, 0.2 };
				const std::shared_ptr<element::GaussKernel> gaussKernelNormalNoise(new element::GaussKernel({ std::string(id) + " gauss kernel", dimensions }, gkp));
				simulation->addElement(normalNoise);
				simulation->addElement(gaussKernelNormalNoise);
				simulation->createInteraction(id, "output", std::string(id) + " gauss kernel");
				simulation->createInteraction(std::string(id) + " gauss kernel", "output", id);
			}
			ImGui::PopID();
		}

		void SimulationWindow::addElementFieldCoupling() const
		{
			ImGui::PushID("field coupling");
			static char id[CHAR_SIZE] = "field coupling u -> v";
			ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
			static int x_max = 100;
			ImGui::InputInt("output x_max", &x_max, 1.0, 10.0);
			static double d_x = 1.0;
			ImGui::InputDouble("output d_x", &d_x, 0.1, 0.5, "%.2f");
			static int in_x_max = 100;
			ImGui::InputInt("input x_max", &in_x_max, 1.0, 10.0);
			static double in_d_x = 1.0;
			ImGui::InputDouble("input d_x", &in_d_x, 0.1, 0.5, "%.2f");
			static LearningRule learningRule = LearningRule::HEBB;
			if (ImGui::BeginCombo("learning rule", LearningRuleToString.at(learningRule).c_str()))
			{
				for (size_t i = 0; i < LearningRuleToString.size(); ++i)
				{
					const char* name = LearningRuleToString.at(static_cast<LearningRule>(i)).c_str();
					if (ImGui::Selectable(name, learningRule == static_cast<LearningRule>(i)))
					{
						learningRule = static_cast<LearningRule>(i);
					}
				}
				ImGui::EndCombo();
			}
			static double scalar = 1.0;
			ImGui::InputDouble("scalar", &scalar, 0.1f, 1.0f, "%.2f");
			static double learningRate = 0.01;
			ImGui::InputDouble("learning rate", &learningRate, 0.01f, 0.1f, "%.2f");

			if(ImGui::Button("Add", { 100.0f, 30.0f }))
			{
				const element::FieldCouplingParameters fcp = { {in_x_max, in_d_x}, learningRule, scalar, learningRate };
				const element::ElementDimensions dimensions{ x_max, d_x };
				const std::shared_ptr<element::FieldCoupling> fieldCoupling(new element::FieldCoupling({ id, dimensions }, fcp));
				simulation->addElement(fieldCoupling);
			}
			ImGui::PopID();
		}

		void SimulationWindow::addElementGaussFieldCoupling() const
		{
			ImGui::PushID("gauss field coupling");
			static char id[CHAR_SIZE] = "gauss field coupling u -> v";
			ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
			static int x_max = 100;
			ImGui::InputInt("output x_max", &x_max, 1.0, 10.0);
			static double d_x = 1.0;
			ImGui::InputDouble("output d_x", &d_x, 0.1, 0.5, "%.2f");
			static int in_x_max = 100;
			ImGui::InputInt("input x_max", &in_x_max, 1.0, 10.0);
			static double in_d_x = 1.0;
			ImGui::InputDouble("input d_x", &in_d_x, 0.1, 0.5, "%.2f");
			static double x_i = 1;
			ImGui::InputDouble("x_i", &x_i, 1.0f, 10.0f, "%.2f");
			static double x_j = 1;
			ImGui::InputDouble("x_j", &x_j, 1.0f, 10.0f, "%.2f");
			static double amplitude = 5;
			ImGui::InputDouble("amplitude", &amplitude, 1.0f, 10.0f, "%.2f");
			static double width = 5;
			ImGui::InputDouble("width", &width, 1.0f, 10.0f, "%.2f");
			static bool normalized = true;
			ImGui::Checkbox("normalized", &normalized);
			static bool circular = false;
			ImGui::Checkbox("circular", &circular);

			if (ImGui::Button("Add", { 100.0f, 30.0f }))
			{
				const element::GaussFieldCouplingParameters gfcp = { {in_x_max, in_d_x}, normalized, circular, {{x_i, x_j, amplitude, width}} };
				const element::ElementDimensions dimensions{ x_max, d_x };
				const std::shared_ptr<element::GaussFieldCoupling> gaussCoupling(new element::GaussFieldCoupling({ id, dimensions }, gfcp));
				simulation->addElement(gaussCoupling);
			}
			ImGui::PopID();
		}

		void SimulationWindow::addElementGaussKernel() const
		{
			ImGui::PushID("gauss kernel");
			static char id[CHAR_SIZE] = "gauss kernel u -> u";
			ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
			static int x_max = 100;
			ImGui::InputInt("x_max", &x_max, 1.0, 10.0);
			static double d_x = 1.0;
			ImGui::InputDouble("d_x", &d_x, 0.1, 0.5, "%.2f");
			static double sigma = 20;
			ImGui::InputDouble("sigma", &sigma, 1.0f, 10.0f, "%.2f");
			static double amplitude = 2;
			ImGui::InputDouble("amplitude", &amplitude, 1.0f, 10.0f, "%.2f");
			static double amplitudeGlobal = -0.01;
			ImGui::InputDouble("amplitudeGlobal", &amplitudeGlobal, -0.01f, -0.1f, "%.2f");
			static bool normalized = true;
			ImGui::Checkbox("normalized", &normalized);
			static bool circular = false;
			ImGui::Checkbox("circular", &circular);

			if (ImGui::Button("Add", { 100.0f, 30.0f }))
			{
				const element::GaussKernelParameters gkp = { sigma, amplitude, amplitudeGlobal, circular, normalized};
				const element::ElementDimensions dimensions{ x_max, d_x };
				const std::shared_ptr<element::GaussKernel> gaussKernel(new element::GaussKernel({ id, dimensions }, gkp));
				simulation->addElement(gaussKernel);
			}
			ImGui::PopID();
		}

		void SimulationWindow::addElementMexicanHatKernel() const
		{
			ImGui::PushID("mexican hat kernel");
			static char id[CHAR_SIZE] = "mexican hat kernel u -> u";
			ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
			static int x_max = 100;
			ImGui::InputInt("x_max", &x_max, 1.0, 10.0);
			static double d_x = 1.0;
			ImGui::InputDouble("d_x", &d_x, 0.1, 0.5, "%.2f");
			static double sigmaExc = 5;
			ImGui::InputDouble("sigmaExc", &sigmaExc, 1.0f, 10.0f, "%.2f");
			static double amplitudeExc = 15;
			ImGui::InputDouble("amplitudeExc", &amplitudeExc, 1.0f, 10.0f, "%.2f");
			static double sigmaInh = 10;
			ImGui::InputDouble("sigmaInh", &sigmaInh, 1.0f, 10.0f, "%.2f");
			static double amplitudeInh = 15;
			ImGui::InputDouble("amplitudeInh", &amplitudeInh, 1.0f, 10.0f, "%.2f");
			static double amplitudeGlobal = -0.01;
			ImGui::InputDouble("amplitudeGlobal", &amplitudeGlobal, -0.01f, -0.1f, "%.2f");
			static bool normalized = true;
			ImGui::Checkbox("normalized", &normalized);
			static bool circular = false;
			ImGui::Checkbox("circular", &circular);


			if (ImGui::Button("Add", { 100.0f, 30.0f }))
			{
				const element::MexicanHatKernelParameters mhkp = { sigmaExc, amplitudeExc, sigmaInh, amplitudeInh , amplitudeGlobal, circular, normalized};
				const element::ElementDimensions dimensions{ x_max, d_x };
				const std::shared_ptr<element::MexicanHatKernel> mexicanHatKernel(new element::MexicanHatKernel({ id, dimensions }, mhkp));
				simulation->addElement(mexicanHatKernel);
			}
			ImGui::PopID();
		}

	}
}
