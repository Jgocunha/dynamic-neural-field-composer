// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/normal_noise_2d.h"

namespace dnf_composer
{
	namespace element
	{
		NormalNoise2D::NormalNoise2D(const ElementCommonParameters& elementCommonParameters,
			const NormalNoise2DParameters& parameters)
			: Element(elementCommonParameters), parameters(parameters)
		{
			commonParameters.identifiers.label = ElementLabel::NORMAL_NOISE_2D;
		}

		void NormalNoise2D::init()
		{
			std::ranges::fill(components["output"], 0.0);
		}

		void NormalNoise2D::step(double t, double deltaT)
		{
			// Zero amplitude => output is identically zero; skip the per-step RNG
			// draw and allocation entirely (the output was already zeroed in init()
			// and is only ever written here).
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
			for (int i = 0; i < n; ++i)
				out[i] *= scale;
		}

		std::string NormalNoise2D::toString() const
		{
			std::string result = "Normal noise 2D element\n";
			result += commonParameters.toString() + '\n';
			result += parameters.toString();
			return result;
		}

		std::shared_ptr<Element> NormalNoise2D::clone() const
		{
			return std::make_shared<NormalNoise2D>(*this);
		}

		void NormalNoise2D::setParameters(const NormalNoise2DParameters& p)
		{
			parameters = p;
		}

		NormalNoise2DParameters NormalNoise2D::getParameters() const
		{
			return parameters;
		}
	}
}
