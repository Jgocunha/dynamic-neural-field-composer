#include "elements/neural_field.h"


namespace dnf_composer
{
	namespace element
	{
		void NeuralFieldBump::print() const
		{
			const std::string str = toString();
			dnf_composer::tools::logger::log(dnf_composer::tools::logger::LogLevel::INFO, str);
		}

		void NeuralFieldState::print() const
		{
			const std::string str = toString();
			dnf_composer::tools::logger::log(dnf_composer::tools::logger::LogLevel::INFO, str);
		}
		

		NeuralField::NeuralField(const ElementCommonParameters& elementCommonParameters, 
			const NeuralFieldParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::NEURAL_FIELD;
			components["activation"] = std::vector<double>(commonParameters.dimensionParameters.size);
			components["resting level"] = std::vector<double>(commonParameters.dimensionParameters.size);
		}

		void NeuralField::init()
		{
			std::ranges::fill(components["activation"], parameters.startingRestingLevel);
			std::ranges::fill(components["input"], 0.0);
			std::ranges::fill(components["output"], 0.0);
			std::ranges::fill(components["resting level"], parameters.startingRestingLevel);

			act_  = components["activation"].data();
			inp_  = components["input"].data();
			rest_ = components["resting level"].data();

			calculateOutput();
		}

		void NeuralField::step(double t, double deltaT)
		{
			updateInput();
			calculateActivation(t, deltaT);
			calculateOutput();
			if (computeStateMetrics_)
				updateState(deltaT);
		}

		void NeuralField::setParameters(const NeuralFieldParameters& neuralFieldParameters)
		{
			parameters = neuralFieldParameters;
			init();
		}

		NeuralFieldParameters NeuralField::getParameters() const
		{
			return parameters;
		}

		bool NeuralField::isStable() const
		{
			if (state.stable)
				return true;
			return false;
		}

		std::shared_ptr<Kernel> NeuralField::getSelfExcitationKernel() const
		{
			for (const auto& input : inputs)
			{
				if (input.first->getLabel() == ElementLabel::GAUSS_KERNEL ||
					input.first->getLabel() == ElementLabel::MEXICAN_HAT_KERNEL)
				{
					auto kernel = std::dynamic_pointer_cast<Kernel>(input.first);
					for (const auto& kernelInput : kernel->getInputs())
					{
						if (kernelInput->getUniqueName() == commonParameters.identifiers.uniqueName)
							return kernel;
					}
				}
			}
			return nullptr;
		}

		std::string NeuralField::toString() const
		{
			std::string result = "Neural field element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString() + '\n';
			result += state.toString();
			return result;
		}

		std::shared_ptr<Element> NeuralField::clone() const
		{
			auto cloned = std::make_shared<NeuralField>(*this);
			return cloned;
		}

		void NeuralField::calculateActivation(double t, double deltaT)
		{
			const double dtOverTau = deltaT / parameters.tau;
			const int sz = commonParameters.dimensionParameters.size;
			for (int i = 0; i < sz; ++i)
				act_[i] += dtOverTau * (-act_[i] + rest_[i] + inp_[i]);
		}

		void NeuralField::calculateOutput()
		{
			parameters.activationFunction->apply(components["activation"], components["output"]);
		}

		void NeuralField::updateState(double deltaT)
		{
			const std::size_t n = static_cast<std::size_t>(commonParameters.dimensionParameters.size);

			// One pass over act_: compute sum, sumSq (for norm), min, max simultaneously.
			// Replaces 5 separate O(N) passes that each also did a string hash-map lookup.
			double sum = 0.0, sumSq = 0.0;
			double vmin = act_[0], vmax = act_[0];
			for (std::size_t i = 0; i < n; ++i)
			{
				const double v = act_[i];
				sum   += v;
				sumSq += v * v;
				if (v < vmin) vmin = v;
				if (v > vmax) vmax = v;
			}
			const double norm = std::sqrt(sumSq);
			const double avg  = sum / static_cast<double>(n);

			state.lowestActivation  = vmin;
			state.highestActivation = vmax;

			state.stable =
				std::abs(sum  - state.previousActivationSum)  < state.thresholdForStability &&
				std::abs(avg  - state.previousActivationAvg)  < state.thresholdForStability &&
				std::abs(norm - state.previousActivationNorm) < state.thresholdForStability;

			state.previousActivationSum  = sum;
			state.previousActivationAvg  = avg;
			state.previousActivationNorm = norm;

			updateBumps(deltaT);
		}

		void NeuralField::updateBumps(double deltaT)
		{
			prevBumps_.swap(state.bumps); // O(1): reuse existing allocation, no heap alloc
			state.bumps.clear();

			constexpr double activationThreshold = 0.00001;
			const int sz = commonParameters.dimensionParameters.size;
			bool inBump = false;
			NeuralFieldBump currentBump(0, 0, 0, 0, 0);

			for (int i = 0; i < sz; ++i)
			{
				const double activation = act_[i];
				if (activation > activationThreshold && !inBump)
				{
					inBump = true;
					currentBump = NeuralFieldBump();
					currentBump.startPosition = (i + 1) * commonParameters.dimensionParameters.d_x;
					currentBump.amplitude = activation;
					currentBump.width = 1;
				}
				else if (activation > activationThreshold && inBump)
				{
					currentBump.amplitude = std::max(currentBump.amplitude, activation);
					currentBump.width++;
				}
				else if (inBump)
				{
					currentBump.width -= 1;
					currentBump.width *= commonParameters.dimensionParameters.d_x;
					currentBump.endPosition = i * commonParameters.dimensionParameters.d_x;
					currentBump.centroid = ((currentBump.startPosition + currentBump.endPosition) / 2);

					static constexpr double epsilon = 2.0;
					const auto it = std::find_if(prevBumps_.begin(), prevBumps_.end(),
						[&currentBump](const NeuralFieldBump& bump) {
							return std::abs(bump.centroid - currentBump.centroid) < epsilon;
						});
					if (it != prevBumps_.end())
					{
						currentBump.velocity = (currentBump.centroid - it->centroid) / deltaT;
						currentBump.acceleration = (currentBump.velocity - it->velocity) / deltaT;
					}
					else
					{
						currentBump.velocity = 0.0;
						currentBump.acceleration = 0.0;
					}

					state.bumps.push_back(currentBump);
					inBump = false;
				}
			}

			if (inBump)
			{
				// Bump reached the last index without dropping below threshold — finalize it.
				currentBump.width -= 1;
				currentBump.width *= commonParameters.dimensionParameters.d_x;
				currentBump.endPosition = sz * commonParameters.dimensionParameters.d_x;
				currentBump.centroid = (currentBump.startPosition + currentBump.endPosition) / 2.0;

				static constexpr double epsilon = 2.0;
				const auto it = std::find_if(prevBumps_.begin(), prevBumps_.end(),
					[&currentBump](const NeuralFieldBump& bump) {
						return std::abs(bump.centroid - currentBump.centroid) < epsilon;
					});
				if (it != prevBumps_.end())
				{
					currentBump.velocity     = (currentBump.centroid - it->centroid) / deltaT;
					currentBump.acceleration = (currentBump.velocity  - it->velocity)  / deltaT;
				}
				else
				{
					currentBump.velocity     = 0.0;
					currentBump.acceleration = 0.0;
				}

				state.bumps.push_back(currentBump);
			}

			// Check if the first and last bumps are connected (wrap-around)
			if (!state.bumps.empty() && act_[0] > activationThreshold && act_[sz - 1] > activationThreshold)
			{
				const auto& firstBump = state.bumps.front();
				const auto& lastBump = state.bumps.back();

				if (&firstBump != &lastBump)
				{
					NeuralFieldBump newBump;
					newBump.startPosition = lastBump.startPosition;
					newBump.endPosition = firstBump.endPosition;
					newBump.amplitude = std::max(firstBump.amplitude, lastBump.amplitude);
					newBump.width = commonParameters.dimensionParameters.x_max -
						(newBump.startPosition - newBump.endPosition);
					newBump.centroid = fmod(
						((newBump.startPosition + newBump.endPosition +
							commonParameters.dimensionParameters.x_max) / 2.0),
						commonParameters.dimensionParameters.x_max);

					static constexpr double epsilon = 2.0;
					const auto it = std::find_if(prevBumps_.begin(), prevBumps_.end(),
						[&newBump](const NeuralFieldBump& bump) {
							return std::abs(bump.centroid - newBump.centroid) < epsilon;
						});
					if (it != prevBumps_.end())
					{
						newBump.velocity = (newBump.centroid - it->centroid) / deltaT;
						newBump.acceleration = (newBump.velocity - it->velocity) / deltaT;
					}
					else
					{
						newBump.velocity = 0.0;
						newBump.acceleration = 0.0;
					}

					state.bumps.pop_back();
					state.bumps.erase(state.bumps.begin());
					state.bumps.push_back(newBump);
				}
			}
		}
	}
}