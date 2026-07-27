#include "elements/activation_function.h"
#include "tools/simd_dispatch.h"


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

	void SigmoidFunction::apply(const std::vector<double>& input, std::vector<double>& out) const
	{
		// Full double-precision logistic sigmoid: 1/(1+exp(-s(x-xs))). The exponent is
		// clamped to [-88, 88] — not the wider [-708, 708] double-exp range. At the
		// saturated tail the sigmoid is already 0/1 to ~1e-38, so the clamp is a numeric
		// no-op, but it keeps the smallest output near 1e-38 (a normal double) instead of
		// ~1e-308: the latter underflows to a DENORMAL when multiplied by the kernel
		// weights in the downstream convolution, and each denormal costs a microcode
		// assist (measured: >50x slower on resting-cell-dominated fields). Clamping to
		// [-88,88] keeps every value a normal double, no FTZ flush needed.
		const double s  = steepness;
		const double xs = x_shift;
		const std::size_t n = input.size();

		// Runtime-dispatched AVX2+FMA path (see simd_dispatch.h) — same clamp,
		// same formula, ~1e-15-accurate vectorized exp; falls back to the scalar
		// loop below on pre-AVX2 hosts.
		if (tools::math::detail::avx2_fma_available())
		{
			tools::math::detail::sigmoid_avx2_f64(input.data(), out.data(), n, s, xs);
			return;
		}

		for (std::size_t i = 0; i < n; ++i)
		{
			double e = -s * (input[i] - xs);
			e = e < -88.0 ? -88.0 : (e > 88.0 ? 88.0 : e);
			out[i] = 1.0 / (1.0 + std::exp(e));
		}
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

	void HeavisideFunction::apply(const std::vector<double>& input, std::vector<double>& out) const
	{
		for (std::size_t i = 0; i < input.size(); ++i) {
			out[i] = (input[i] > x_shift) ? 1.0 : 0.0;
}
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

	void AbsSigmoidFunction::apply(const std::vector<double>& input, std::vector<double>& out) const
	{
		for (std::size_t i = 0; i < input.size(); ++i) {
			const double diff = input[i] - x_shift;
			out[i] = 0.5 * (1.0 + beta * diff / (1.0 + beta * std::abs(diff)));
		}
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
