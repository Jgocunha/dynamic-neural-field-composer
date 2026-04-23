#pragma once

#include "element.h"
#include "tools/math.h"
#include "tools/utils.h"


namespace dnf_composer::element
{
	/// @brief A single sparse Gaussian point-coupling between two spatial locations.
	///
	/// Models a localized projection from location @c x_i in the source field to
	/// location @c x_j in the target field, with a Gaussian spread of @c width
	/// and peak @c amplitude.
	///
	/// @ingroup elements
	struct GaussCoupling
	{
		double x_i;       ///< Source location in the input field.
		double x_j;       ///< Target location in the output field.
		double amplitude; ///< Peak coupling weight.
		double width;     ///< Gaussian spread (σ) of the coupling.

		/// @brief Construct a GaussCoupling. All parameters must be positive.
		/// @param x_i        Source location.
		/// @param x_j        Target location.
		/// @param amplitude  Peak weight (must be > 0).
		/// @param width      Gaussian σ (must be > 0).
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


	/// @brief Parameters for a sparse Gaussian field coupling (fixed projection).
	/// @ingroup elements
	struct GaussFieldCouplingParameters final : ElementSpecificParameters
	{
		ElementDimensions inputFieldDimensions; ///< Spatial dimensions of the source field.
		bool normalized;                        ///< If true, each coupling Gaussian is area-normalised.
		bool circular;                          ///< If true, convolution wraps at field boundaries.
		std::vector<GaussCoupling> couplings;   ///< List of explicit point-to-point Gaussian couplings.

		/// @brief Construct GaussFieldCoupling parameters.
		/// @param inputFieldDimensions  Source field dimensions.
		/// @param normalized            Normalise individual Gaussians (default true).
		/// @param circular              Circular boundary (default false).
		/// @param couplings             Initial coupling list (default empty).
		explicit GaussFieldCouplingParameters(const ElementDimensions& inputFieldDimensions = ElementDimensions{},
		                             bool normalized = true, bool circular = false,
		                             const std::vector<GaussCoupling>& couplings = {})
			: inputFieldDimensions(inputFieldDimensions),
			  normalized(normalized), circular(circular), couplings(couplings)
		{}

		/// @brief Append a coupling to the list.
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

	/// @brief Sparse Gaussian field coupling with a fixed, user-defined projection.
	///
	/// Unlike FieldCoupling (which learns a full weight matrix), GaussFieldCoupling
	/// uses an explicit list of @c GaussCoupling descriptors. Each descriptor projects
	/// activity from a source location @c x_i to a target location @c x_j with a
	/// Gaussian spread. This is useful for hand-crafted or evolution-derived projections
	/// where the exact connectivity is known in advance.
	///
	/// @ingroup elements
	class GaussFieldCoupling final : public Element
	{
	private:
		GaussFieldCouplingParameters parameters;
	public:
		/// @brief Construct a GaussFieldCoupling.
		/// @param elementCommonParameters  Name, label, and output field dimensions.
		/// @param gfc_parameters           Coupling parameters (source dims + coupling list).
		GaussFieldCoupling(const ElementCommonParameters& elementCommonParameters,
		                   const GaussFieldCouplingParameters& gfc_parameters);

		/// @brief Append a new point-coupling at runtime.
		/// @param coupling  The GaussCoupling to add.
		void addCoupling(const GaussCoupling& coupling);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		/// @brief Return a copy of the current coupling parameters.
		GaussFieldCouplingParameters getParameters() const;

		/// @brief Replace the coupling parameters at runtime.
		void setParameters(const GaussFieldCouplingParameters& gfc_parameters);

		/// @brief Return the spatial dimensions of the source (input) field.
		ElementDimensions getInputFieldDimensions() const;
	private:
		void updateOutput();
		void updateInputFieldDimensions();
	};

}
