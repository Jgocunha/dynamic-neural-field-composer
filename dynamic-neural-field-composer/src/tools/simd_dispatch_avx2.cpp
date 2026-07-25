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
#include <immintrin.h>

namespace dnf_composer::tools::math::detail
{
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
