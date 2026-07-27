#pragma once

#include <cstddef>

// Runtime CPU-feature dispatch for the AVX2+FMA convolution kernel — mirrors how
// OpenCV/FFTW select their SIMD code path at runtime rather than at compile time,
// so a single dnf-composer binary runs correctly on any x86-64 CPU instead of
// requiring AVX2 to be present at all (see TRADE_OFF_CAVEATS.md in the benchmark
// repo for why this matters).
//
// conv_valid_into_avx2_f64 is defined in simd_dispatch_avx2.cpp, the ONLY
// translation unit in this library compiled with AVX2+FMA enabled; every other
// file (including this header's caller, tools/math.h) is compiled at the
// toolchain's default baseline. Calling this function is only safe after
// avx2_fma_available() has returned true — the function boundary itself is
// ABI-safe to declare/call from baseline code (it passes only pointers and
// ints, no vector registers), but executing its body on a CPU without AVX2+FMA
// would fault.

namespace dnf_composer::tools::math::detail
{
	// Detects AVX2 + FMA + OS YMM-state support once and caches the result.
	// Safe to call from a baseline-compiled translation unit.
	bool avx2_fma_available();

	// Bit-identical to the AVX2 branch previously inlined in conv_valid_into<double>:
	// out[i] = sum_{m=0..M-1} kr[m] * mx[i+m], for i in [0, n), with the symmetric-
	// kernel folding optimization applied when kr is a palindrome. `o` must have at
	// least n elements; `kr` has M elements; `mx` must be readable up to index
	// n + M - 2.
	void conv_valid_into_avx2_f64(const double* kr, int M, const double* mx, double* o, int n);

	// Vectorized double-precision logistic sigmoid:
	//   out[i] = 1 / (1 + exp(clamp(-s * (in[i] - xs), -88, 88)))
	// Same denormal-avoidance clamp as the scalar SigmoidFunction::apply (see
	// activation_function.cpp) — the exponential itself uses a Cephes-style
	// rational-polynomial range-reduction (~1e-15 relative accuracy), far inside
	// the ~1e-7 the project's 1e-4 field-dynamics gate can absorb since the
	// sigmoid is a pointwise map, not an accumulated reduction. `out` and `in`
	// may alias (element-wise, read-before-write per lane). Safe to call only
	// after avx2_fma_available() has returned true.
	void sigmoid_avx2_f64(const double* in, double* out, std::size_t n, double s, double xs);
}
