#include "tools/fft_convolution.h"

#include <fftw3.h>
#include <cstring>
#include <stdexcept>
#include <atomic>
#include <mutex>

namespace dnf_composer::tools::math
{
	namespace
	{
		inline int freqCols(int size_x) { return size_x / 2 + 1; }

		std::atomic<ConvolutionMode>& modeOverrideStorage()
		{
			static std::atomic<ConvolutionMode> mode{ConvolutionMode::Auto};
			return mode;
		}

		// FFTW's planner (fftw_plan_dft_*, fftw_destroy_plan) is not reentrant
		// and must be serialized process-wide. Guards every planner call below;
		// fftw_malloc/fftw_free are plain aligned allocation and need no guard.
		std::mutex& plannerMutex()
		{
			static std::mutex m;
			return m;
		}
	}

	void setConvolutionModeOverride(ConvolutionMode mode)
	{
		modeOverrideStorage().store(mode, std::memory_order_relaxed);
	}

	ConvolutionMode convolutionModeOverride()
	{
		return modeOverrideStorage().load(std::memory_order_relaxed);
	}

	ScopedConvolutionMode::ScopedConvolutionMode(ConvolutionMode mode)
		: previous_(convolutionModeOverride())
	{
		setConvolutionModeOverride(mode);
	}

	ScopedConvolutionMode::~ScopedConvolutionMode()
	{
		setConvolutionModeOverride(previous_);
	}

	bool shouldUseSpectral2D(bool circular, int totalTaps, int size_x, int size_y)
	{
		// An FFT of the full field is inherently periodic, so spectral is only
		// DEFINED for circular boundaries. This precedes the override check on
		// purpose: ForceSpectral means "spectral wherever it is legal", never
		// "always".
		if (!circular || size_x <= 0 || size_y <= 0) return false;

		switch (convolutionModeOverride())
		{
		case ConvolutionMode::ForceDirect:   return false;
		case ConvolutionMode::ForceSpectral: return true;
		case ConvolutionMode::Auto:          break;
		}

		return totalTaps > kFFTTapThreshold &&
			size_x >= kFFTMinAxisSize && size_y >= kFFTMinAxisSize;
	}

	void SpectralConvolver2D::destroy()
	{
		std::lock_guard<std::mutex> lock(plannerMutex());
		if (forwardPlan_) fftw_destroy_plan(static_cast<fftw_plan>(forwardPlan_));
		if (inversePlan_) fftw_destroy_plan(static_cast<fftw_plan>(inversePlan_));
		if (fieldReal_)  fftw_free(fieldReal_);
		if (fieldFreq_)  fftw_free(fieldFreq_);
		if (kernelFreq_) fftw_free(kernelFreq_);
		if (resultReal_) fftw_free(resultReal_);
		forwardPlan_ = inversePlan_ = nullptr;
		fieldReal_ = fieldFreq_ = kernelFreq_ = resultReal_ = nullptr;
		size_x_ = size_y_ = 0;
	}

	SpectralConvolver2D::~SpectralConvolver2D()
	{
		destroy();
	}

	void SpectralConvolver2D::copyFrom(const SpectralConvolver2D& other)
	{
		if (other.size_x_ == 0 || other.size_y_ == 0)
			return;

		init(other.size_x_, other.size_y_);
		if (other.kernelFreq_)
		{
			const std::size_t bytes =
				static_cast<std::size_t>(size_y_) * freqCols(size_x_) * sizeof(fftw_complex);
			std::memcpy(kernelFreq_, other.kernelFreq_, bytes);
		}
	}

	SpectralConvolver2D::SpectralConvolver2D(const SpectralConvolver2D& other)
	{
		copyFrom(other);
	}

	SpectralConvolver2D& SpectralConvolver2D::operator=(const SpectralConvolver2D& other)
	{
		if (this != &other)
		{
			destroy();
			copyFrom(other);
		}
		return *this;
	}

	SpectralConvolver2D::SpectralConvolver2D(SpectralConvolver2D&& other) noexcept
		: size_x_(other.size_x_), size_y_(other.size_y_),
		  fieldReal_(other.fieldReal_), fieldFreq_(other.fieldFreq_),
		  kernelFreq_(other.kernelFreq_), resultReal_(other.resultReal_),
		  forwardPlan_(other.forwardPlan_), inversePlan_(other.inversePlan_)
	{
		other.size_x_ = other.size_y_ = 0;
		other.fieldReal_ = other.fieldFreq_ = other.kernelFreq_ = other.resultReal_ = nullptr;
		other.forwardPlan_ = other.inversePlan_ = nullptr;
	}

	SpectralConvolver2D& SpectralConvolver2D::operator=(SpectralConvolver2D&& other) noexcept
	{
		if (this != &other)
		{
			destroy();
			size_x_ = other.size_x_; size_y_ = other.size_y_;
			fieldReal_ = other.fieldReal_; fieldFreq_ = other.fieldFreq_;
			kernelFreq_ = other.kernelFreq_; resultReal_ = other.resultReal_;
			forwardPlan_ = other.forwardPlan_; inversePlan_ = other.inversePlan_;
			other.size_x_ = other.size_y_ = 0;
			other.fieldReal_ = other.fieldFreq_ = other.kernelFreq_ = other.resultReal_ = nullptr;
			other.forwardPlan_ = other.inversePlan_ = nullptr;
		}
		return *this;
	}

	void SpectralConvolver2D::init(int size_x, int size_y)
	{
		// Same geometry and live plans -> nothing to re-plan. Checking the plan
		// pointers (not just the sizes) matters: a default-constructed or
		// moved-from object has size_x_ == size_y_ == 0, and init(0,0) must not
		// be short-circuited into a no-op. NOTE this preserves kernelFreq_,
		// which is exactly why every caller must still call setKernel() after
		// init() whenever the taps may have changed: see setKernel()'s comment.
		if (size_x == size_x_ && size_y == size_y_ && forwardPlan_ && inversePlan_)
			return;

		destroy();
		size_x_ = size_x;
		size_y_ = size_y;

		const std::size_t realCount = static_cast<std::size_t>(size_x) * size_y;
		const std::size_t freqCount = static_cast<std::size_t>(size_y) * freqCols(size_x);

		fieldReal_  = fftw_malloc(sizeof(double) * realCount);
		resultReal_ = fftw_malloc(sizeof(double) * realCount);
		fieldFreq_  = fftw_malloc(sizeof(fftw_complex) * freqCount);
		kernelFreq_ = fftw_malloc(sizeof(fftw_complex) * freqCount);
		if (!fieldReal_ || !resultReal_ || !fieldFreq_ || !kernelFreq_)
			throw std::bad_alloc();

		// FFTW_ESTIMATE (a heuristic, no timing trials) rather than
		// FFTW_MEASURE: every element's setParameters() re-runs init(), and the
		// UI calls setParameters() on every frame a width slider is dragged
		// (no IsItemDeactivatedAfterEdit() gate), so with five elements able to
		// plan, FFTW_MEASURE's timing trials would otherwise stall the UI for
		// tens of milliseconds per novel size, repeatedly, during a drag.
		// ESTIMATE also does not clobber its input buffers, so — unlike
		// MEASURE — planning here does not require fieldReal_/fieldFreq_ to
		// already hold anything meaningful.
		std::lock_guard<std::mutex> lock(plannerMutex());
		forwardPlan_ = fftw_plan_dft_r2c_2d(
			size_y, size_x,
			static_cast<double*>(fieldReal_),
			static_cast<fftw_complex*>(fieldFreq_),
			FFTW_ESTIMATE);
		inversePlan_ = fftw_plan_dft_c2r_2d(
			size_y, size_x,
			static_cast<fftw_complex*>(fieldFreq_),
			static_cast<double*>(resultReal_),
			FFTW_ESTIMATE);
	}

	void SpectralConvolver2D::setKernel(const std::vector<double>& wrappedKernelReal)
	{
		auto* real = static_cast<double*>(fieldReal_);
		std::memcpy(real, wrappedKernelReal.data(), wrappedKernelReal.size() * sizeof(double));
		fftw_execute(static_cast<fftw_plan>(forwardPlan_));

		// Fold FFTW's unnormalized-transform convention (forward+inverse scales
		// by size_x*size_y) into the stored kernel spectrum once here, so every
		// subsequent apply() needs no separate normalization pass.
		const double norm = 1.0 / (static_cast<double>(size_x_) * size_y_);
		const std::size_t freqCount = static_cast<std::size_t>(size_y_) * freqCols(size_x_);
		auto* kf = static_cast<fftw_complex*>(kernelFreq_);
		auto* ff = static_cast<fftw_complex*>(fieldFreq_);
		for (std::size_t i = 0; i < freqCount; ++i)
		{
			kf[i][0] = ff[i][0] * norm;
			kf[i][1] = ff[i][1] * norm;
		}
	}

	void SpectralConvolver2D::apply(const double* field, double* out)
	{
		auto* real = static_cast<double*>(fieldReal_);
		std::memcpy(real, field, static_cast<std::size_t>(size_x_) * size_y_ * sizeof(double));
		fftw_execute(static_cast<fftw_plan>(forwardPlan_));

		auto* ff = static_cast<fftw_complex*>(fieldFreq_);
		auto* kf = static_cast<fftw_complex*>(kernelFreq_);
		const std::size_t freqCount = static_cast<std::size_t>(size_y_) * freqCols(size_x_);
		for (std::size_t i = 0; i < freqCount; ++i)
		{
			const double re = ff[i][0] * kf[i][0] - ff[i][1] * kf[i][1];
			const double im = ff[i][0] * kf[i][1] + ff[i][1] * kf[i][0];
			ff[i][0] = re;
			ff[i][1] = im;
		}

		fftw_execute(static_cast<fftw_plan>(inversePlan_));
		std::memcpy(out, resultReal_, static_cast<std::size_t>(size_x_) * size_y_ * sizeof(double));
	}
}
