#pragma once

#include <vector>

// FFTW-backed circular 2D convolution, for kernels wide enough that the
// direct/separable path (tools/math.h conv2d_separable_into) is no longer the
// cheaper option (see kFFTTapThreshold in mexican_hat_kernel_2d.cpp for the
// dispatch rule). Only supports CIRCULAR boundaries — an FFT of the full field
// natively computes a periodic convolution, which is exactly what
// `circular = true` means; non-circular kernels keep using the direct path.
//
// FFTW types (fftw_plan, fftw_complex, ...) are intentionally kept out of this
// header — every other translation unit in the library, including the element
// that owns a SpectralConvolver2D, stays decoupled from the FFTW3 headers, the
// same way tools/simd_dispatch.h keeps <immintrin.h> local to its own .cpp.
namespace dnf_composer::tools::math
{
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
		// uses. Safe to call again to change size; destroys any previous plan.
		void init(int size_x, int size_y);

		// Sets the frequency-domain kernel from a real, size_x*size_y spatial
		// kernel that has already been "wrap embedded" — i.e. kernel[0,0] holds
		// the zero-offset tap and positive/negative offsets are placed at
		// [offset] / [size - offset] respectively (see embedWrappedKernel2D in
		// mexican_hat_kernel_2d.cpp). Runs one forward transform and folds FFTW's
		// 1/(size_x*size_y) normalization into the stored spectrum so apply()
		// needs no separate scaling pass. Must be called after init().
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
