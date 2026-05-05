#pragma once

#include <string>

#include "tools/logger.h"
#include "tools/math.h"

/// @defgroup activation_functions Activation Functions
/// @brief Pointwise nonlinearities applied to neural field activation.
/// @ingroup elements

namespace dnf_composer::element
{
	/// @brief Identifies the concrete activation function type.
	///
	/// Cross-framework naming:
	/// | dnf-composer      | cedar             | cosivina  |
	/// |-------------------|-------------------|-----------|
	/// | SIGMOID           | ExpSigmoid        | sigmoid   |
	/// | ABSSIGMOID        | AbsSigmoid        | —         |
	/// | HEAVISIDE         | HeavisideSigmoid  | —         |
	/// @ingroup activation_functions
	enum ActivationFunctionType : int
	{
		SIGMOID,     ///< Logistic (exponential) sigmoid.
		HEAVISIDE,   ///< Binary step function.
		ABSSIGMOID,  ///< Rational (absolute-value) sigmoid approximation — cedar's default.
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

	/// @brief Logistic (exponential) sigmoid.
	///
	/// Formula: @f$ \sigma(x) = \frac{1}{1 + e^{-\mathrm{steepness}(x - x\_shift)}} @f$
	///
	/// **Cross-framework equivalence (formula-identical):**
	/// | Framework   | Type / call                               |
	/// |-------------|-------------------------------------------|
	/// | dnf-composer | `SigmoidFunction(x_shift, steepness)`    |
	/// | cedar        | `ExpSigmoid(beta=steepness, theta=x_shift)` |
	/// | cosivina     | `sigmoid(beta=steepness, x0=x_shift)`     |
	///
	/// All three use the same formula. Only the parameter names differ.
	/// @ingroup activation_functions
	struct SigmoidFunction final : public ActivationFunction
	{
		double x_shift;   ///< Inflection point of the sigmoid (cedar: theta; cosivina: x0).
		double steepness; ///< Slope at the inflection point (cedar/cosivina: beta).

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

	/// @brief Heaviside step function.
	///
	/// Formula: @f$ \sigma(x) = \begin{cases} 1 & x > x\_shift \\ 0 & x \le x\_shift \end{cases} @f$
	///
	/// **Cross-framework equivalence:**
	/// | Framework   | Type / call                          | Note                          |
	/// |-------------|--------------------------------------|-------------------------------|
	/// | dnf-composer | `HeavisideFunction(x_shift)`        | maps 1 when x > x_shift       |
	/// | cedar        | `HeavisideSigmoid(theta=x_shift)`   | identical formula (strict >)  |
	/// | cosivina     | no built-in equivalent              | —                             |
	/// @ingroup activation_functions
	struct HeavisideFunction final : public ActivationFunction
	{
		double x_shift; ///< Threshold; activation strictly above this value maps to 1.

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

	/// @brief Rational (absolute-value) sigmoid — cedar's default activation function.
	///
	/// Formula: @f$ \sigma(x) = \frac{1}{2}\left(1 + \frac{\beta(x - x\_shift)}{1 + \beta|x - x\_shift|}\right) @f$
	///
	/// This is computationally cheaper than SigmoidFunction because it avoids `std::exp`.
	/// At @f$\beta \ge 20@f$ the two functions are numerically indistinguishable
	/// (max pointwise error < 0.001).
	///
	/// **Cross-framework equivalence:**
	/// | Framework   | Type / call                                  | Exact? |
	/// |-------------|----------------------------------------------|--------|
	/// | dnf-composer | `AbsSigmoidFunction(x_shift, beta)`         | ✓      |
	/// | cedar        | `AbsSigmoid(beta=beta, theta=x_shift)`      | ✓      |
	/// | cosivina     | no built-in equivalent                      | —      |
	///
	/// **Approximate equivalence to ExpSigmoid:** Use `SigmoidFunction(x_shift, beta)` or
	/// cosivina `sigmoid(beta=beta, x0=x_shift)` — identical for @f$\beta \ge 20@f$.
	///
	/// @note Cedar uses AbsSigmoid with @f$\beta = 100@f$ by default (near-Heaviside).
	///       Published cedar parameter sets assume this steep nonlinearity.
	/// @ingroup activation_functions
	struct AbsSigmoidFunction final : public ActivationFunction
	{
		double x_shift; ///< Inflection point / threshold (cedar: theta).
		double beta;    ///< Steepness parameter — cedar/cosivina naming (larger = steeper).

		AbsSigmoidFunction(const AbsSigmoidFunction&) = default;

		/// @brief Construct an AbsSigmoid with the given threshold and steepness.
		/// @param x_shift  Inflection point of the sigmoid.
		/// @param beta     Slope parameter.
		AbsSigmoidFunction(double x_shift, double beta);

		std::vector<double> operator()(const std::vector<double>& input) override;
		bool operator==(const AbsSigmoidFunction& other) const;
		[[nodiscard]] std::unique_ptr<ActivationFunction> clone() const override;
		[[nodiscard]] std::string toString() const override;
		void print() const override;

		[[nodiscard]] double getBeta() const;
		[[nodiscard]] double getXShift() const;

		~AbsSigmoidFunction() override = default;
	};
}
