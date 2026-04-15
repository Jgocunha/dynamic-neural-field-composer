#pragma once

#include <string>

#include "tools/logger.h"
#include "tools/math.h"


namespace dnf_composer::element
{
	enum ActivationFunctionType : int
	{
		SIGMOID,
		HEAVISIDE,
	};

	struct ActivationFunction
	{
		ActivationFunctionType type;
		ActivationFunction() = default;
		ActivationFunction(const ActivationFunction&) = default;
		ActivationFunction& operator=(const ActivationFunction&) = delete;
		virtual std::vector<double> operator()(const std::vector<double>& input) = 0;
		[[nodiscard]] virtual std::unique_ptr<ActivationFunction> clone() const = 0;
		[[nodiscard]] virtual std::string toString() const = 0;
		virtual void print() const = 0;
		virtual ~ActivationFunction() = default;

	};

	struct SigmoidFunction final : public ActivationFunction
	{
		double x_shift, steepness;

		SigmoidFunction(const SigmoidFunction&) = default;
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

	struct HeavisideFunction final : public ActivationFunction
	{
		double x_shift;

		HeavisideFunction(const HeavisideFunction&) = default;
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