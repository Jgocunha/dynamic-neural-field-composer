#pragma once

#include "element.h"
#include "tools/math.h"


namespace dnf_composer
{
	struct WeightedCoupling
	{
		double x_i, x_j, w_i_j;
	};

	namespace element
	{
		struct GaussFieldCouplingParameters
		{
			int inputFieldSize = 0;
			double sigma = 0.0;
			std::vector<WeightedCoupling> couplings;

			void addCoupling(const WeightedCoupling& coupling)
			{
				couplings.emplace_back(coupling);
			}
		};

		class GaussFieldCoupling : public Element
		{
		private:
			GaussFieldCouplingParameters parameters;
		public:
			GaussFieldCoupling(const ElementCommonParameters& elementCommonParameters, const GaussFieldCouplingParameters& gfc_parameters);

			void addCoupling(const WeightedCoupling& coupling);

			void init() override;
			void step(double t, double deltaT) override;
			void close() override;
			void printParameters() override;

			GaussFieldCouplingParameters getParameters() const;

			~GaussFieldCoupling() override = default;
		private:
			void updateOutput();
		};

	}
}




