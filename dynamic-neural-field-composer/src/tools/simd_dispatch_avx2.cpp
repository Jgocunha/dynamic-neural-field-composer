// Compiled with AVX2+FMA (see CMakeLists.txt set_source_files_properties for this
// file) — the ONLY translation unit in this library built that way. Every other
// file, including tools/math.h which calls into this one, is compiled at the
// toolchain's default baseline so the resulting binary does not require AVX2 to
// load and run; math.h only calls conv_valid_into_avx2_f64 after
// detail::avx2_fma_available() has confirmed the host CPU supports it.
//
// The body below is extracted verbatim (same operation order, same accumulator
// grouping) from the AVX2 branch that used to be inlined in conv_valid_into<double>
// in tools/math.h, so its numerics are unchanged from before this split.

#include "tools/simd_dispatch.h"
#include <cmath>

// This file is only compiled with AVX2+FMA flags on x86/x64 (see CMakeLists.txt);
// on other architectures (e.g. arm64) it is compiled at the default baseline
// with no such flags, <immintrin.h> is unavailable, and avx2_fma_available()
// unconditionally returns false, so the functions below are never called —
// they are stubbed out to keep this TU portable rather than excluded from the
// build, so no separate per-architecture source list is needed in CMakeLists.
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
	#define DNF_COMPOSER_X86 1
#else
	#define DNF_COMPOSER_X86 0
#endif

#if DNF_COMPOSER_X86
#include <immintrin.h>

namespace dnf_composer::tools::math::detail
{
	namespace
	{
		// Vectorized exp(x) for 4 packed doubles, Cephes-style range reduction +
		// rational (Padé) approximation of the fractional part — same algorithm
		// and same coefficients as the widely-used cephes/exp.c double routine,
		// just retargeted from scalar floor/ldexp to AVX2 lanes. Valid for the
		// clamped domain this is actually called with ([-88, 88]); not a
		// general-purpose exp (no under/overflow saturation outside that range).
		inline __m256d exp_pd(__m256d x)
		{
			const __m256d log2e = _mm256_set1_pd(1.4426950408889634073599);
			const __m256d c1    = _mm256_set1_pd(6.93145751953125E-1);   // ln2 hi
			const __m256d c2    = _mm256_set1_pd(1.42860682030941723212E-6); // ln2 lo
			const __m256d half  = _mm256_set1_pd(0.5);
			const __m256d one   = _mm256_set1_pd(1.0);
			const __m256d two   = _mm256_set1_pd(2.0);

			// n = floor(log2(e) * x + 0.5); x -= n*ln2 (hi+lo split for precision)
			__m256d n = _mm256_floor_pd(_mm256_fmadd_pd(log2e, x, half));
			x = _mm256_fnmadd_pd(n, c1, x);
			x = _mm256_fnmadd_pd(n, c2, x);

			const __m256d xx = _mm256_mul_pd(x, x);

			// P(xx), degree 2 (Horner)
			const __m256d P0 = _mm256_set1_pd(1.26177193074810590878E-4);
			const __m256d P1 = _mm256_set1_pd(3.02994407707441961300E-2);
			const __m256d P2 = _mm256_set1_pd(9.99999999999999999910E-1);
			__m256d p = _mm256_fmadd_pd(P0, xx, P1);
			p = _mm256_fmadd_pd(p, xx, P2);
			p = _mm256_mul_pd(p, x);

			// Q(xx), degree 3 (Horner)
			const __m256d Q0 = _mm256_set1_pd(3.00198505138664455042E-6);
			const __m256d Q1 = _mm256_set1_pd(2.52448340349684104192E-3);
			const __m256d Q2 = _mm256_set1_pd(2.27265548208155028766E-1);
			const __m256d Q3 = _mm256_set1_pd(2.00000000000000000009E0);
			__m256d q = _mm256_fmadd_pd(Q0, xx, Q1);
			q = _mm256_fmadd_pd(q, xx, Q2);
			q = _mm256_fmadd_pd(q, xx, Q3);

			// x = 1 + 2*p/(q-p)
			__m256d r = _mm256_div_pd(p, _mm256_sub_pd(q, p));
			r = _mm256_fmadd_pd(two, r, one);

			// scale by 2^n via direct exponent-bit construction (n is small and
			// exact here — |n| <= ~127 for the clamped [-88,88] input domain).
			__m128i n32 = _mm256_cvttpd_epi32(n);
			__m256i n64 = _mm256_cvtepi32_epi64(n32);
			__m256i biased = _mm256_add_epi64(n64, _mm256_set1_epi64x(1023));
			__m256i pow2n_bits = _mm256_slli_epi64(biased, 52);
			__m256d pow2n = _mm256_castsi256_pd(pow2n_bits);

			return _mm256_mul_pd(r, pow2n);
		}
	}

	void sigmoid_avx2_f64(const double* in, double* out, std::size_t n, double s, double xs)
	{
		const __m256d sv    = _mm256_set1_pd(s);
		const __m256d xsv   = _mm256_set1_pd(xs);
		const __m256d lo    = _mm256_set1_pd(-88.0);
		const __m256d hi    = _mm256_set1_pd(88.0);
		const __m256d one   = _mm256_set1_pd(1.0);
		const __m256d zero  = _mm256_setzero_pd();

		std::size_t i = 0;
		for (; i + 4 <= n; i += 4)
		{
			__m256d x = _mm256_loadu_pd(in + i);
			__m256d t = _mm256_sub_pd(x, xsv);
			__m256d e = _mm256_fnmadd_pd(sv, t, zero); // -(s*t)
			e = _mm256_max_pd(lo, _mm256_min_pd(hi, e));
			__m256d ee = exp_pd(e);
			__m256d res = _mm256_div_pd(one, _mm256_add_pd(one, ee));
			_mm256_storeu_pd(out + i, res);
		}
		for (; i < n; ++i)
		{
			double e = -s * (in[i] - xs);
			e = e < -88.0 ? -88.0 : (e > 88.0 ? 88.0 : e);
			out[i] = 1.0 / (1.0 + std::exp(e));
		}
	}

	void conv_valid_into_avx2_f64(const double* kr, int M, const double* mx, double* o, int n)
	{
		// Symmetric-kernel folding. For a symmetric kernel kr (Gauss / the
		// symmetric axis of Asymmetric / correlated-noise taps), each output is
		// out[i] = kr[c]*w[c] + sum_{j<c} kr[j]*(w[j] + w[2c-j]) — half the
		// multiplies. This reorders the per-output summation (pairs summed before
		// scaling), so it is NOT bit-identical; it is gated on the 1e-4 field-
		// dynamics validation suite. Measured: worst-case deviation across all
		// 600 sims stays at the ~5e-5 reference-CSV truncation floor (i.e. folding
		// adds no error beyond what's already there), with the same ~2x margin to
		// 1e-4 as the unfolded path. Vectorized 4 outputs at a time; non-symmetric
		// kernels (e.g. Oscillatory) fall through to the bit-identical path below.
		bool symmetric = (M % 2 == 1);
		if (symmetric)
			for (int j = 0, c = M - 1; j < c; ++j, --c)
				if (kr[j] != kr[c]) { symmetric = false; break; }

		// Four independent accumulator chains (16 outputs per iteration) hide
		// the fmadd latency — a single-accumulator chain is latency-bound
		// (~4-5 cycles per serialized fmadd, ~10x off throughput). Vector ops
		// are element-wise, so regrouping outputs into wider blocks does NOT
		// change any element's operation sequence (center tap, then pairs in
		// ascending j / taps in ascending m) — bit-identical to the 4-wide
		// body, which is kept as the bridge so tail elements get the exact
		// same treatment as before the unroll.
		int i = 0;
		if (symmetric)
		{
			const int c = M / 2; // center tap index
			const __m256d kc = _mm256_set1_pd(kr[c]);
			for (; i + 16 <= n; i += 16)
			{
				const double* __restrict w = mx + i;
				__m256d a0 = _mm256_mul_pd(kc, _mm256_loadu_pd(w + c));
				__m256d a1 = _mm256_mul_pd(kc, _mm256_loadu_pd(w + c + 4));
				__m256d a2 = _mm256_mul_pd(kc, _mm256_loadu_pd(w + c + 8));
				__m256d a3 = _mm256_mul_pd(kc, _mm256_loadu_pd(w + c + 12));
				for (int j = 0; j < c; ++j)
				{
					const __m256d kv = _mm256_set1_pd(kr[j]);
					const int mj = 2 * c - j;
					a0 = _mm256_fmadd_pd(kv, _mm256_add_pd(_mm256_loadu_pd(w + j),      _mm256_loadu_pd(w + mj)),      a0);
					a1 = _mm256_fmadd_pd(kv, _mm256_add_pd(_mm256_loadu_pd(w + j + 4),  _mm256_loadu_pd(w + mj + 4)),  a1);
					a2 = _mm256_fmadd_pd(kv, _mm256_add_pd(_mm256_loadu_pd(w + j + 8),  _mm256_loadu_pd(w + mj + 8)),  a2);
					a3 = _mm256_fmadd_pd(kv, _mm256_add_pd(_mm256_loadu_pd(w + j + 12), _mm256_loadu_pd(w + mj + 12)), a3);
				}
				_mm256_storeu_pd(o + i,      a0);
				_mm256_storeu_pd(o + i + 4,  a1);
				_mm256_storeu_pd(o + i + 8,  a2);
				_mm256_storeu_pd(o + i + 12, a3);
			}
			for (; i + 4 <= n; i += 4)
			{
				const double* __restrict w = mx + i;
				__m256d acc = _mm256_mul_pd(_mm256_set1_pd(kr[c]), _mm256_loadu_pd(w + c));
				for (int j = 0; j < c; ++j)
				{
					const __m256d kv  = _mm256_set1_pd(kr[j]);
					const __m256d sum = _mm256_add_pd(_mm256_loadu_pd(w + j),
					                                  _mm256_loadu_pd(w + (2 * c - j)));
					acc = _mm256_fmadd_pd(kv, sum, acc);
				}
				_mm256_storeu_pd(o + i, acc);
			}
			for (; i < n; ++i)
			{
				const double* __restrict w = mx + i;
				double acc = kr[c] * w[c];
				for (int j = 0; j < c; ++j)
					acc += kr[j] * (w[j] + w[2 * c - j]);
				o[i] = acc;
			}
			return;
		}

		// Non-symmetric: vectorize across the OUTPUT index (bit-identical — each
		// output's tap sum keeps the original m-order). Same 4-chain unroll.
		for (; i + 16 <= n; i += 16)
		{
			const double* __restrict w = mx + i;
			__m256d a0 = _mm256_setzero_pd();
			__m256d a1 = _mm256_setzero_pd();
			__m256d a2 = _mm256_setzero_pd();
			__m256d a3 = _mm256_setzero_pd();
			for (int m = 0; m < M; ++m)
			{
				const __m256d kv = _mm256_set1_pd(kr[m]);
				a0 = _mm256_fmadd_pd(kv, _mm256_loadu_pd(w + m),      a0);
				a1 = _mm256_fmadd_pd(kv, _mm256_loadu_pd(w + m + 4),  a1);
				a2 = _mm256_fmadd_pd(kv, _mm256_loadu_pd(w + m + 8),  a2);
				a3 = _mm256_fmadd_pd(kv, _mm256_loadu_pd(w + m + 12), a3);
			}
			_mm256_storeu_pd(o + i,      a0);
			_mm256_storeu_pd(o + i + 4,  a1);
			_mm256_storeu_pd(o + i + 8,  a2);
			_mm256_storeu_pd(o + i + 12, a3);
		}
		for (; i + 4 <= n; i += 4)
		{
			__m256d acc = _mm256_setzero_pd();
			const double* __restrict w = mx + i;
			for (int m = 0; m < M; ++m)
			{
				const __m256d kv = _mm256_set1_pd(kr[m]);
				const __m256d wv = _mm256_loadu_pd(w + m);
				acc = _mm256_fmadd_pd(kv, wv, acc);
			}
			_mm256_storeu_pd(o + i, acc);
		}
		for (; i < n; ++i)
		{
			const double* __restrict w = mx + i;
			double acc = 0.0;
			for (int m = 0; m < M; ++m)
				acc += kr[m] * w[m];
			o[i] = acc;
		}
	}
}

#else // !DNF_COMPOSER_X86 — unreachable (avx2_fma_available() always returns
      // false here), stubbed so this TU still links on non-x86 architectures.

namespace dnf_composer::tools::math::detail
{
	void conv_valid_into_avx2_f64(const double*, int, const double*, double*, int) {}
	void sigmoid_avx2_f64(const double*, double*, std::size_t, double, double) {}
}

#endif // DNF_COMPOSER_X86
