#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>

#include "kernel.h"
#include "tools/math.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif


namespace dnf_composer
{
	namespace element
	{
		struct MexicanHatKernelParameters : ElementSpecificParameters
		{
			double widthExc;
			double amplitudeExc;
			double widthInh;
			double amplitudeInh;
			double amplitudeGlobal;
			bool circular;
			bool normalized;
			std::optional<ElementDimensions> outputFieldDimensions;

			MexicanHatKernelParameters(double widthExc = 2.5, double amplitudeExc = 11.0,
				double widthInh = 5.0, double amplitudeInh = 15.0,
				double amplitudeGlobal = -0.1,
				bool circular = true, bool normalized = true,
				std::optional<ElementDimensions> outputDims = std::nullopt)
				: widthExc(widthExc), amplitudeExc(amplitudeExc),
				widthInh(widthInh), amplitudeInh(amplitudeInh),
				amplitudeGlobal(amplitudeGlobal),
				circular(circular), normalized(normalized),
				outputFieldDimensions(outputDims)
			{}

			bool operator==(const MexicanHatKernelParameters& other) const
			{
				constexpr double epsilon = 1e-6;

				return std::abs(widthExc - other.widthExc) < epsilon &&
					std::abs(amplitudeExc - other.amplitudeExc) < epsilon &&
					std::abs(widthInh - other.widthInh) < epsilon &&
					std::abs(amplitudeInh - other.amplitudeInh) < epsilon &&
					std::abs(amplitudeGlobal - other.amplitudeGlobal) < epsilon &&
					circular == other.circular &&
					normalized == other.normalized &&
					outputFieldDimensions == other.outputFieldDimensions;
			}

			std::string toString() const override
			{
				std::ostringstream result;
				result << std::fixed << std::setprecision(2);
				result << "Parameters: ["
					<< "Width exc.: " << widthExc << ", "
					<< "Amplitude exc.: " << amplitudeExc << ", "
					<< "Width inh.: " << widthInh << ", "
					<< "Amplitude inh.: " << amplitudeInh << ", "
					<< "Amplitude glob.: " << amplitudeGlobal << ", "
					<< "Circular: " << (circular ? "true" : "false") << ", "
					<< "Normalized: " << (normalized ? "true" : "false");
				if (outputFieldDimensions.has_value())
					result << ", Output size: " << outputFieldDimensions->size;
				result << "]";
				return result.str();
			}
		};

		class MexicanHatKernel : public Kernel
		{
		private:
			MexicanHatKernelParameters parameters;
		public:
			MexicanHatKernel(const ElementCommonParameters& elementCommonParameters,
				MexicanHatKernelParameters mhk_parameters);

			void init() override;
			void step(double t, double deltaT) override;
			std::string toString() const override;
			std::shared_ptr<Element> clone() const override;

			void setParameters(const MexicanHatKernelParameters& mhk_parameters);
			MexicanHatKernelParameters getParameters() const;
		};
	}
}