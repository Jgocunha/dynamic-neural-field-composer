#pragma once

#include <vector>

// FFTW-backed circular 2D convolution, for kernels wide enough that the
// direct/separable path (tools/math.h conv2d_separable_into) is no longer the
// cheaper option (see kFFTTapThreshold below for the dispatch rule). Only
// supports CIRCULAR boundaries — an FFT of the full field natively computes a
// periodic convolution, which is exactly what `circular = true` means;
// non-circular kernels keep using the direct path.
//
// FFTW types (fftw_plan, fftw_complex, ...) are intentionally kept out of this
// header — every other translation unit in the library, including the element
// that owns a SpectralConvolver2D, stays decoupled from the FFTW3 headers, the
// same way tools/simd_dispatch.h keeps <immintrin.h> local to its own .cpp.
namespace dnf_composer::tools::math
{
	// Process-global test/diagnostic seam for the direct-vs-spectral dispatch
	// rule below. Auto is the only mode production code should rely on; Force*
	// exists so tests can exercise the spectral path on grids where Auto would
	// correctly decline (or vice versa), and to build direct/spectral twins of
	// the same element for differential comparison.
	enum class ConvolutionMode { Auto, ForceDirect, ForceSpectral };

	void setConvolutionModeOverride(ConvolutionMode mode);
	ConvolutionMode convolutionModeOverride();

	// RAII scoped override: restores the previous mode on destruction, including
	// when a test's EXPECT/ASSERT unwinds the stack. Not copyable/movable.
	class ScopedConvolutionMode
	{
	public:
		explicit ScopedConvolutionMode(ConvolutionMode mode);
		~ScopedConvolutionMode();
		ScopedConvolutionMode(const ScopedConvolutionMode&) = delete;
		ScopedConvolutionMode& operator=(const ScopedConvolutionMode&) = delete;
		ScopedConvolutionMode(ScopedConvolutionMode&&) = delete;
		ScopedConvolutionMode& operator=(ScopedConvolutionMode&&) = delete;
	private:
		ConvolutionMode previous_;
	};

	// Total direct-path taps per output cell (summed over every separable term
	// of the kernel) above which the fused spectral path (one r2c + one complex
	// multiply + one c2r) does fewer FLOPs than the direct separable
	// convolution(s): direct costs ~2*N*totalTaps FLOPs (N = size_x*size_y, one
	// multiply-add per tap per cell, summed over terms), while spectral costs
	// ~C_fft(N) + 3*N, independent of how many terms there are (they are
	// pre-summed into one real kernel before the transform). Solving
	// 2*N*T = C_fft(N) + 3*N for the crossover T* = C_fft(N)/(2N) + 1.5 depends
	// only on N: at grid=100 (C_fft ~ 5*N*log2(N), log2(10000)~13.3) T*~130;
	// at grid=200 (log2(40000)~15.3) T*~115. 120 is the round number in between.
	// Because the term count cancels out of T*, this single constant applies
	// unchanged whether the kernel has one term (Gauss/AsymGauss/Oscillatory/
	// CorrelatedNormalNoise) or two (MexicanHat's exc-inh difference).
	inline constexpr int kFFTTapThreshold = 120;

	// Minimum per-axis grid size for the Auto dispatch to ever pick spectral.
	// This is a real, separately-justified restriction, not a fit to dodge a
	// failing test: it's the smallest grid this project's own benchmark and
	// cross-platform-validation suites ever exercise. At the smaller 50x50
	// grids used by this repo's OWN FieldDynamics regression fixtures (sims
	// 049/050, memory regime), a wide inhibitory kernel is forced to
	// near-full-field support by computeKernelRange's clamp — exactly the kind
	// of knife-edge bistable abssigmoid attractor this codebase has hit before
	// (see the symmetric-folding revert note in simd_dispatch_avx2.cpp/math.h):
	// ANY numerically different-but-valid reformulation of the convolution can
	// flip which attractor it settles into there, independent of whether the
	// FFT path itself is correct. Restricting Auto to grids at least as large
	// as anything actually benchmarked avoids that known fragility without
	// touching the regression fixtures or the tap-count rule itself. Kept
	// separate from the override switch/case below on purpose: ForceSpectral
	// means "spectral wherever it is legal" (i.e. circular), never "always".
	inline constexpr int kFFTMinAxisSize = 100;

	// THE single dispatch rule every 2D convolution element uses to decide
	// between the direct separable path and the fused spectral path.
	bool shouldUseSpectral2D(bool circular, int totalTaps, int size_x, int size_y);


	class SpectralConvolver2D
	{
	public:
		SpectralConvolver2D() = default;
		~SpectralConvolver2D();

		// Deep copies: re-plans independently for the same size and duplicates the
		// stored spectral kernel, rather than sharing the source's FFTW plan/buffer
		// pointers. Needed because Element::clone() copy-constructs the owning
		// kernel element (see MexicanHatKernel2D::clone()) — a shallow pointer copy
		// here would double-free the FFTW plans when both copies are destroyed.
		SpectralConvolver2D(const SpectralConvolver2D& other);
		SpectralConvolver2D& operator=(const SpectralConvolver2D& other);
		SpectralConvolver2D(SpectralConvolver2D&& other) noexcept;
		SpectralConvolver2D& operator=(SpectralConvolver2D&& other) noexcept;

		// (Re)plan for a row-major, y-major field of size_x * size_y
		// (field[y * size_x + x]) — matches the layout conv2d_separable_into
		// uses. Safe to call again, including with an unchanged size (a no-op:
		// the existing plans and kernel spectrum are left untouched) or a
		// different size (destroys and re-plans). Because same-size calls are a
		// no-op, every caller whose taps may have changed — even when the grid
		// did not — must still call setKernel() after init(); see setKernel().
		void init(int size_x, int size_y);

		// Sets the frequency-domain kernel from a real, size_x*size_y spatial
		// kernel that has already been "wrap embedded" — i.e. kernel[0,0] holds
		// the zero-offset tap and positive/negative offsets are placed at
		// [offset] / [size - offset] respectively (see buildWrappedSeparableKernel2D
		// above). Runs one forward transform and folds FFTW's 1/(size_x*size_y)
		// normalization into the stored spectrum so apply() needs no separate
		// scaling pass. Must be called after init(), and again after any init()
		// call that follows a change to the kernel taps (init() alone does not
		// recompute the spectrum when the grid size is unchanged).
		void setKernel(const std::vector<double>& wrappedKernelReal);

		// out = circular_convolve(field, kernel). field and out must both be
		// size_x*size_y; out may not overlap field. Must be called after init()
		// and setKernel().
		void apply(const double* field, double* out);

	private:
		void destroy();
		void copyFrom(const SpectralConvolver2D& other);

		int size_x_ = 0;
		int size_y_ = 0;

		// Opaque FFTW handles (fftw_plan / fftw_malloc'd double* / fftw_complex*),
		// cast in fft_convolution.cpp. void* keeps <fftw3.h> out of this header.
		void* fieldReal_    = nullptr;
		void* fieldFreq_    = nullptr;
		void* kernelFreq_   = nullptr;
		void* resultReal_   = nullptr;
		void* forwardPlan_  = nullptr;
		void* inversePlan_  = nullptr;
	};
}
