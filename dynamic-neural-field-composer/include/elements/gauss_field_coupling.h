#pragma once

#include "element.h"
#include "tools/math.h"
#include "tools/utils.h"


namespace dnf_composer::element
{
	struct GaussCoupling
	{
		double x_i, x_j, amplitude, width;

		GaussCoupling(const double x_i, const double x_j, const double amplitude, const double width)
			: x_i(x_i), x_j(x_j), amplitude(amplitude), width(width)
		{
			if (x_i <= 0.0 || x_j <= 0.0 || amplitude <= 0.0 || width <= 0.0)
				throw Exception(ErrorCode::ELEM_INVALID_PARAMETER, "GaussCoupling");
		}

		bool operator==(const GaussCoupling& other) const
		{
			constexpr double epsilon = 1e-6;

			return std::abs(x_i - other.x_i) < epsilon &&
				std::abs(x_j - other.x_j) < epsilon &&
				std::abs(amplitude - other.amplitude) < epsilon &&
				std::abs(width - other.width) < epsilon;
		}

		[[nodiscard]] std::string toString() const
		{
			std::string result = "Gauss coupling [";
			result += "x_i: " + std::format("{:.2f}", x_i) + " ";
			result += "x_j: " + std::format("{:.2f}", x_j) + " ";
			result += "a: " + std::format("{:.2f}", amplitude) + " ";
			result += "w: " + std::format("{:.2f}", width) + "]\n";
			return result;
		}
	};


	struct GaussFieldCouplingParameters final : ElementSpecificParameters
	{
		ElementDimensions inputFieldDimensions;
		bool normalized;
		bool circular;
		std::vector<GaussCoupling> couplings;

		explicit GaussFieldCouplingParameters(const ElementDimensions& inputFieldDimensions = ElementDimensions{},
		                             bool normalized = true, bool circular = false,
		                             const std::vector<GaussCoupling>& couplings = {})
			: inputFieldDimensions(inputFieldDimensions),
			  normalized(normalized), circular(circular), couplings(couplings)
		{}

		void addCoupling(const GaussCoupling& coupling)
		{
			couplings.emplace_back(coupling);
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "Circular: " << (circular ? "true" : "false") << ", "
				<< "Normalized: " << (normalized ? "true" : "false") << ", "
				<< "Input field dimensions: " + inputFieldDimensions.toString() << "]\n";

			for (const auto& coupling : couplings)
				result << coupling.toString();

			return result.str();
		}
	};

	class GaussFieldCoupling final : public Element
	{
	private:
		GaussFieldCouplingParameters parameters;
	public:
		GaussFieldCoupling(const ElementCommonParameters& elementCommonParameters,
		                   const GaussFieldCouplingParameters& gfc_parameters);

		void addCoupling(const GaussCoupling& coupling);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		GaussFieldCouplingParameters getParameters() const;
		void setParameters(const GaussFieldCouplingParameters& gfc_parameters);
		ElementDimensions getInputFieldDimensions() const;
	private:
		void updateOutput();
		void updateInputFieldDimensions();
	};

}




