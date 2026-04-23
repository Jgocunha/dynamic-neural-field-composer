#pragma once

#include <string>

#include "tools/logger.h"
#include "tools/math.h"

/// @defgroup activation_functions Activation Functions
/// @brief Pointwise nonlinearities applied to neural field activation.
/// @ingroup elements

namespace dnf_composer::element
{
	enum ActivationFunctionType : int
	{
		SIGMOID,    ///< Logistic sigmoid.
		HEAVISIDE,  ///< Binary step function.
	};

	/// @brief Abstract base for activation functions applied to a neural field.
	///
	/// Subclasses implement @c operator() to transform an activation vector
	/// element-wise into an output (field "output" component).
	/// @ingroup activation_functions
	struct ActivationFunction
	{
		ActivationFunctionType type; ///< Concrete function type.
		ActivationFunction() = default;
		ActivationFunction(const ActivationFunction&) = default;
		ActivationFunction& operator=(const ActivationFunction&) = delete;

		/// @brief Apply the activation function to @p input and return the result.
		virtual std::vector<double> operator()(const std::vector<double>& input) = 0;

		[[nodiscard]] virtual std::unique_ptr<ActivationFunction> clone() const = 0;
		[[nodiscard]] virtual std::string toString() const = 0;

		virtual void print() const = 0;
		virtual ~ActivationFunction() = default;
	};

	/// @brief Logistic sigmoid: output = 1 / (1 + exp(-steepness * (x - x_shift))).
	/// @ingroup activation_functions
	struct SigmoidFunction final : public ActivationFunction
	{
		double x_shift;   ///< Inflection point of the sigmoid.
		double steepness; ///< Slope at the inflection point.

		SigmoidFunction(const SigmoidFunction&) = default;

		/// @brief Construct a sigmoid with the given inflection point and slope.
		/// @param x_shift    Activation value at the inflection (threshold).
		/// @param steepness  Slope parameter (larger = steeper transition).
		SigmoidFunction(double x_shift, double steepness);

		std::vector<double> operator()(const std::vector<double>& input) override;
		bool operator==(const SigmoidFunction& other) const;
		[[nodiscard]] std::unique_ptr<ActivationFunction> clone() const override;
		[[nodiscard]] std::string toString() const override;
		void print() const override;

		[[nodiscard]] double getSteepness() const;
		[[nodiscard]] double getXShift() const;

		~SigmoidFunction() override = default;
	};

	/// @brief Heaviside step function: output = (x >= x_shift) ? 1 : 0.
	/// @ingroup activation_functions
	struct HeavisideFunction final : public ActivationFunction
	{
		double x_shift; ///< Threshold; activation at or above this value maps to 1.

		HeavisideFunction(const HeavisideFunction&) = default;

		/// @brief Construct a Heaviside function with the given threshold.
		/// @param x_shift  Activation threshold.
		explicit HeavisideFunction(double x_shift);

		std::vector<double> operator()(const std::vector<double>& input) override;
		bool operator==(const HeavisideFunction& other) const;
		[[nodiscard]] std::unique_ptr<ActivationFunction> clone() const override;
		[[nodiscard]] std::string toString() const override;
		void print() const override;

		[[nodiscard]] double getXShift() const;

		~HeavisideFunction() override = default;
	};
}
