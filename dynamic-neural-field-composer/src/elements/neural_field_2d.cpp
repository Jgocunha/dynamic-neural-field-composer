#include "elements/neural_field_2d.h"


namespace dnf_composer::element
{
	NeuralField2D::NeuralField2D(const ElementCommonParameters& elementCommonParameters,
		const NeuralField2DParameters& parameters)
		: Element(elementCommonParameters), parameters(parameters)
	{
		commonParameters.identifiers.label = ElementLabel::NEURAL_FIELD_2D;
		components["activation"]    = std::vector<double>(commonParameters.dimensionParameters.size);
		components["resting level"] = std::vector<double>(commonParameters.dimensionParameters.size);
	}

	void NeuralField2D::init()
	{
		std::ranges::fill(components["activation"],    parameters.startingRestingLevel);
		std::ranges::fill(components["resting level"], parameters.startingRestingLevel);
		std::ranges::fill(components["input"],  0.0);
		std::ranges::fill(components["output"], 0.0);

		act_  = components["activation"].data();
		inp_  = components["input"].data();
		rest_ = components["resting level"].data();

		calculateOutput();
	}

	void NeuralField2D::step(double t, double deltaT)
	{
		updateInput();
		calculateActivation(t, deltaT);
		calculateOutput();
		if (computeStateMetrics_)
			updateState(deltaT);
	}

	void NeuralField2D::calculateActivation(double /*t*/, double deltaT)
	{
		const double dtOverTau = deltaT / parameters.tau;
		const int sz = commonParameters.dimensionParameters.size;
		for (int i = 0; i < sz; ++i)
			act_[i] += dtOverTau * (-act_[i] + rest_[i] + inp_[i]);
	}

	void NeuralField2D::calculateOutput()
	{
		parameters.activationFunction->apply(components["activation"], components["output"]);
	}

	void NeuralField2D::updateState(double deltaT)
	{
		const std::size_t n = static_cast<std::size_t>(commonParameters.dimensionParameters.size);
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

	void NeuralField2D::updateBumps(double deltaT)
	{
		const int    size_x = commonParameters.dimensionParameters.size_x;
		const int    size_y = commonParameters.dimensionParameters.size_y;
		const double d_x    = commonParameters.dimensionParameters.d_x;
		const double d_y    = commonParameters.dimensionParameters.d_y;
		constexpr double threshold = 0.00001;

		prevBumps_.swap(state.bumps);
		state.bumps.clear();

		if (visited_.size() != static_cast<std::size_t>(size_x * size_y))
			visited_.assign(size_x * size_y, 0);
		else
			std::fill(visited_.begin(), visited_.end(), 0);
		std::vector<char>& visited = visited_;

		for (int xi = 0; xi < size_x; ++xi)
		{
			for (int yi = 0; yi < size_y; ++yi)
			{
				const int idx = yi * size_x + xi;
				if (act_[idx] <= threshold || visited[idx])
					continue;

				NeuralField2DBump bump;
				double sumX = 0.0, sumY = 0.0, sumAct = 0.0;
				int cellCount = 0;

				std::queue<int> q;
				q.push(idx);
				visited[idx] = true;

				while (!q.empty())
				{
					const int curr = q.front(); q.pop();
					const int cx = curr % size_x;
					const int cy = curr / size_x;
					const double a = act_[curr];

					bump.amplitude = std::max(bump.amplitude, a);
					sumX   += (cx + 1) * d_x * a;
					sumY   += (cy + 1) * d_y * a;
					sumAct += a;
					++cellCount;

					const int nx[4] = { cx - 1, cx + 1, cx,     cx     };
					const int ny[4] = { cy,     cy,     cy - 1, cy + 1 };
					for (int k = 0; k < 4; ++k)
					{
						if (nx[k] < 0 || nx[k] >= size_x || ny[k] < 0 || ny[k] >= size_y)
							continue;
						const int nIdx = ny[k] * size_x + nx[k];
						if (!visited[nIdx] && act_[nIdx] > threshold)
						{
							visited[nIdx] = true;
							q.push(nIdx);
						}
					}
				}

				if (sumAct > 0.0)
				{
					bump.centroid_x = sumX / sumAct;
					bump.centroid_y = sumY / sumAct;
				}
				bump.area = static_cast<double>(cellCount) * d_x * d_y;

				constexpr double epsilon = 2.0;
				const auto it = std::find_if(prevBumps_.begin(), prevBumps_.end(),
					[&bump](const NeuralField2DBump& prev) {
						return std::hypot(bump.centroid_x - prev.centroid_x,
						                  bump.centroid_y - prev.centroid_y) < epsilon;
					});
				if (it != prevBumps_.end())
				{
					bump.velocity_x = (bump.centroid_x - it->centroid_x) / deltaT;
					bump.velocity_y = (bump.centroid_y - it->centroid_y) / deltaT;
				}

				state.bumps.push_back(bump);
			}
		}
	}

	std::string NeuralField2D::toString() const
	{
		std::string result = "Neural field 2D element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString() + '\n';
		result += state.toString();
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
