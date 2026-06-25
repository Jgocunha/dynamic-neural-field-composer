#include "elements/normal_noise.h"

namespace dnf_composer
{
	namespace element
	{
		NormalNoise::NormalNoise(const ElementCommonParameters& elementCommonParameters, NormalNoiseParameters parameters)
			: Element(elementCommonParameters), parameters(std::move(parameters))
		{
			 commonParameters.identifiers.label = ElementLabel::NORMAL_NOISE;
		}

		void NormalNoise::init()
		{
			std::ranges::fill(components["output"], 0.0);
		}

		void NormalNoise::step(double t, double deltaT)
		{
			// Zero amplitude => output is identically zero; skip the per-step RNG
			// draw and allocation entirely (behaviour identical, output stays zero).
			if (parameters.amplitude == 0.0)
			{
				std::ranges::fill(components["output"], 0.0);
				return;
			}

			// Fill the output with standard-normal samples in place (no temporary
			// vector), then scale by amplitude/sqrt(dt) in the same pass.
			std::vector<double>& out = components["output"];
			const int n = commonParameters.dimensionParameters.size;
			tools::math::fillNormal(out.data(), static_cast<std::size_t>(n));
			const double scale = parameters.amplitude / std::sqrt(deltaT);
			for (int i = 0; i < n; i++)
				out[i] *= scale;
		}

		std::shared_ptr<Element> NormalNoise::clone() const
		{
			auto cloned = std::make_shared<NormalNoise>(*this);
			return cloned;
		}

		std::string NormalNoise::toString() const
		{
			std::string result;
			result += "Normal noise element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		void NormalNoise::setParameters(NormalNoiseParameters normalNoiseParameters)
		{
			parameters = std::move(normalNoiseParameters);
		}

		NormalNoiseParameters NormalNoise::getParameters() const
		{
			return parameters;
		}
	}
}