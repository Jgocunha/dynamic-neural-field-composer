#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>

#include "tools/math.h"
#include "kernel.h"

//https://github.com/stevenlovegrove/Pangolin/issues/352
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace dnf_composer::element
{
	struct MexicanHatKernel2DParameters final : ElementSpecificParameters
	{
		double widthExc;
		double amplitudeExc;
		double widthInh;
		double amplitudeInh;
		double amplitudeGlobal;
		bool circular;
		bool normalized;

		explicit MexicanHatKernel2DParameters(double widthExc = 2.5, double amplitudeExc = 11.0,
		                                      double widthInh = 5.0, double amplitudeInh = 15.0,
		                                      double amplitudeGlobal = -0.1,
		                                      bool circular = true, bool normalized = true)
			: widthExc(widthExc), amplitudeExc(amplitudeExc),
			  widthInh(widthInh), amplitudeInh(amplitudeInh),
			  amplitudeGlobal(amplitudeGlobal),
			  circular(circular), normalized(normalized)
		{}

		bool operator==(const MexicanHatKernel2DParameters& other) const
		{
			constexpr double epsilon = 1e-6;
			return std::abs(widthExc - other.widthExc) < epsilon &&
			       std::abs(amplitudeExc - other.amplitudeExc) < epsilon &&
			       std::abs(widthInh - other.widthInh) < epsilon &&
			       std::abs(amplitudeInh - other.amplitudeInh) < epsilon &&
			       std::abs(amplitudeGlobal - other.amplitudeGlobal) < epsilon &&
			       circular == other.circular &&
			       normalized == other.normalized;
		}

		[[nodiscard]] std::string toString() const override
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2);
			result << "Parameters: ["
				<< "Width exc: " << widthExc << ", "
				<< "Amplitude exc: " << amplitudeExc << ", "
				<< "Width inh: " << widthInh << ", "
				<< "Amplitude inh: " << amplitudeInh << ", "
				<< "Amplitude global: " << amplitudeGlobal << ", "
				<< "Circular: " << (circular ? "true" : "false") << ", "
				<< "Normalized: " << (normalized ? "true" : "false") << "]";
			return result.str();
		}
	};

	class MexicanHatKernel2D final : public Kernel
	{
	private:
		MexicanHatKernel2DParameters parameters;
		std::array<int, 2> kernelRangeExc_x{}, kernelRangeExc_y{};
		std::array<int, 2> kernelRangeInh_x{}, kernelRangeInh_y{};
		std::vector<int> extIndexExc_x, extIndexExc_y;
		std::vector<int> extIndexInh_x, extIndexInh_y;
		std::vector<double> kernelExc_x, kernelExc_y;
		std::vector<double> kernelInh_x, kernelInh_y;
		std::vector<double> scratchTmp_;
		std::vector<double> scratchExcConv_;
		std::vector<double> scratchInhConv_;
		tools::math::Conv2dScratch<double> scratch2d_;
	public:
		MexicanHatKernel2D(const ElementCommonParameters& elementCommonParameters,
		                   const MexicanHatKernel2DParameters& parameters);

		void init() override;
		void step(double t, double deltaT) override;
		std::string toString() const override;
		std::shared_ptr<Element> clone() const override;

		void setParameters(const MexicanHatKernel2DParameters& parameters);
		MexicanHatKernel2DParameters getParameters() const;
	};
}
