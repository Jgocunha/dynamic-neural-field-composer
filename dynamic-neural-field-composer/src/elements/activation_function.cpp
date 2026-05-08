#include "elements/activation_function.h"


namespace dnf_composer::element
{
	SigmoidFunction::SigmoidFunction(const double x_shift, const double steepness)
	: x_shift(x_shift), steepness(steepness)
	{
		type = ActivationFunctionType::SIGMOID;
	}

	std::vector<double> SigmoidFunction::operator()(const std::vector<double>& input)
	{
		return tools::math::sigmoid(input, steepness, x_shift);
	}

	bool SigmoidFunction::operator==(const SigmoidFunction& other) const
	{
		return x_shift == other.x_shift && steepness == other.steepness;
	}

	std::unique_ptr<ActivationFunction> SigmoidFunction::clone() const
	{
		return std::make_unique<SigmoidFunction>(*this);
	}

	std::string SigmoidFunction::toString() const
	{
		std::string result = "SigmoidFunction(";
		std::ostringstream stream_x_shift;
		stream_x_shift << std::fixed << std::setprecision(2) << x_shift;
		result += "x_shift = " + stream_x_shift.str() + ", ";
		std::ostringstream stream_steepness;
		stream_steepness << std::fixed << std::setprecision(2) << steepness;
		result += "steepness = " + stream_steepness.str() + ")";
		return result;
	}

	void SigmoidFunction::print() const
	{
		const std::string result = toString();
		tools::logger::log(tools::logger::LogLevel::INFO, result);
	}

	double SigmoidFunction::getSteepness() const
	{
		return steepness;
	}

	double SigmoidFunction::getXShift() const
	{
		return x_shift;
	}

	HeavisideFunction::HeavisideFunction(double x_shift)
	: x_shift(x_shift)
	{
		type = ActivationFunctionType::HEAVISIDE;
	}

	std::vector<double> HeavisideFunction::operator()(const std::vector<double>& input)
	{
		return tools::math::heaviside(input, x_shift);
	}

	bool HeavisideFunction::operator==(const HeavisideFunction& other) const
	{
		return x_shift == other.x_shift;
	}

	std::unique_ptr<ActivationFunction> HeavisideFunction::clone() const
	{
		return std::make_unique<HeavisideFunction>(*this);
	}

	std::string HeavisideFunction::toString() const
	{
		std::string result = "HeavisideFunction(";
		std::ostringstream stream;
		stream << std::fixed << std::setprecision(2) << x_shift;
		result += "x_shift = " + stream.str() + ")";
		return result;
	}

	void HeavisideFunction::print() const
	{
		const std::string result = toString();
		tools::logger::log(tools::logger::LogLevel::INFO, result);
	}

	double HeavisideFunction::getXShift() const
	{
		return x_shift;
	}

	AbsSigmoidFunction::AbsSigmoidFunction(const double x_shift, const double beta)
		: x_shift(x_shift), beta(beta)
	{
		type = ActivationFunctionType::ABSSIGMOID;
	}

	std::vector<double> AbsSigmoidFunction::operator()(const std::vector<double>& input)
	{
		return tools::math::absSigmoid(input, beta, x_shift);
	}

	bool AbsSigmoidFunction::operator==(const AbsSigmoidFunction& other) const
	{
		return x_shift == other.x_shift && beta == other.beta;
	}

	std::unique_ptr<ActivationFunction> AbsSigmoidFunction::clone() const
	{
		return std::make_unique<AbsSigmoidFunction>(*this);
	}

	std::string AbsSigmoidFunction::toString() const
	{
		std::string result = "AbsSigmoidFunction(";
		std::ostringstream stream_x_shift;
		stream_x_shift << std::fixed << std::setprecision(2) << x_shift;
		result += "x_shift = " + stream_x_shift.str() + ", ";
		std::ostringstream stream_beta;
		stream_beta << std::fixed << std::setprecision(2) << beta;
		result += "beta = " + stream_beta.str() + ")";
		return result;
	}

	void AbsSigmoidFunction::print() const
	{
		const std::string result = toString();
		tools::logger::log(tools::logger::LogLevel::INFO, result);
	}

	double AbsSigmoidFunction::getBeta() const
	{
		return beta;
	}

	double AbsSigmoidFunction::getXShift() const
	{
		return x_shift;
	}
}