#include "elements/activation_function.h"


namespace dnf_composer::element
{
#if DNFC_HAVE_AVX2
	namespace
	{
		// Vectorized expf over 8 floats. Cephes single-precision exp: range-reduce
		// e = k*ln2 + r (|r| <= ln2/2), then a degree-6 minimax polynomial for
		// exp(r), and scale by 2^k via integer exponent assembly. Max relative error
		// ~1e-7 (well under the 1e-4 field-dynamics tolerance). Inputs are assumed
		// pre-clamped to [-88, 88] by the caller, so 2^k stays in float range.
		inline __m256 exp256_ps(__m256 x)
		{
			const __m256 LOG2EF = _mm256_set1_ps(1.44269504088896341f);
			const __m256 C1     = _mm256_set1_ps(0.693359375f);
			const __m256 C2     = _mm256_set1_ps(-2.12194440e-4f);
			const __m256 half   = _mm256_set1_ps(0.5f);
			const __m256 one    = _mm256_set1_ps(1.0f);

			// k = round(x * log2(e))
			__m256 fx = _mm256_fmadd_ps(x, LOG2EF, half);
			fx = _mm256_floor_ps(fx);
			// r = x - k*ln2  (ln2 split into C1+C2 for extra precision)
			__m256 r = _mm256_fnmadd_ps(fx, C1, x);
			r = _mm256_fnmadd_ps(fx, C2, r);

			// polynomial approximation of exp(r) on the reduced range
			__m256 p = _mm256_set1_ps(1.9875691500e-4f);
			p = _mm256_fmadd_ps(p, r, _mm256_set1_ps(1.3981999507e-3f));
			p = _mm256_fmadd_ps(p, r, _mm256_set1_ps(8.3334519073e-3f));
			p = _mm256_fmadd_ps(p, r, _mm256_set1_ps(4.1665795894e-2f));
			p = _mm256_fmadd_ps(p, r, _mm256_set1_ps(1.6666665459e-1f));
			p = _mm256_fmadd_ps(p, r, _mm256_set1_ps(5.0000001201e-1f));
			__m256 r2 = _mm256_mul_ps(r, r);
			p = _mm256_fmadd_ps(p, r2, r);
			p = _mm256_add_ps(p, one);

			// scale by 2^k: build the float 2^k from the integer exponent field
			__m256i k  = _mm256_cvttps_epi32(fx);
			k = _mm256_add_epi32(k, _mm256_set1_epi32(0x7f));
			k = _mm256_slli_epi32(k, 23);
			__m256 pow2k = _mm256_castsi256_ps(k);
			return _mm256_mul_ps(p, pow2k);
		}
	}
#endif

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
		const float s  = static_cast<float>(steepness);
		const float xs = static_cast<float>(x_shift);
		const std::size_t n = input.size();
		std::size_t i = 0;

#if DNFC_HAVE_AVX2
		// Vectorized path: 8 cells at a time. Same formula as the scalar fallback
		// (clamp the exponent to [-88,88] then 1/(1+exp(e))) but exp via the SIMD
		// Cephes approximation. This is an elementwise MAP — no reduction/summation
		// order to preserve — so it only needs to stay within the 1e-4 field-dynamics
		// tolerance (validated). exp256_ps is ~1e-7 accurate.
		const __m256 sv   = _mm256_set1_ps(s);
		const __m256 xsv  = _mm256_set1_ps(xs);
		const __m256 lo   = _mm256_set1_ps(-88.0f);
		const __m256 hi   = _mm256_set1_ps(88.0f);
		const __m256 one  = _mm256_set1_ps(1.0f);
		alignas(32) float buf[8];
		for (; i + 8 <= n; i += 8)
		{
			// gather 8 doubles -> 8 floats (two 4-wide cvt + pack)
			__m256d d0 = _mm256_loadu_pd(&input[i]);
			__m256d d1 = _mm256_loadu_pd(&input[i + 4]);
			__m256 x = _mm256_set_m128(_mm256_cvtpd_ps(d1), _mm256_cvtpd_ps(d0));
			__m256 e = _mm256_mul_ps(_mm256_sub_ps(xsv, x), sv); // -s*(x-xs) = s*(xs-x)
			e = _mm256_min_ps(_mm256_max_ps(e, lo), hi);
			__m256 r = _mm256_div_ps(one, _mm256_add_ps(one, exp256_ps(e)));
			_mm256_store_ps(buf, r);
			for (int j = 0; j < 8; ++j) out[i + j] = static_cast<double>(buf[j]);
		}
#endif
		for (; i < n; ++i)
		{
			const float x = static_cast<float>(input[i]);
			// Clamp the exponent to the float exp() range. Beyond ~±88 the result
			// has already saturated to 0/1 in float, so this is numerically a no-op,
			// but it avoids the (much slower) overflow/inf path in std::exp — which
			// dominates the field step when the steepness is large (e.g. 100).
			float e = -s * (x - xs);
			e = e < -88.0f ? -88.0f : (e > 88.0f ? 88.0f : e);
			out[i] = static_cast<double>(1.0f / (1.0f + std::exp(e)));
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
		for (std::size_t i = 0; i < input.size(); ++i)
			out[i] = (input[i] > x_shift) ? 1.0 : 0.0;
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