#pragma once

#include <map>
#include <string>
#include <format>

#include "tools/logger.h"

namespace dnf_composer::element
{
	enum ElementLabel : int
	{
		UNINITIALIZED,
		NEURAL_FIELD,
		GAUSS_STIMULUS,
		BOOST_STIMULUS,
		GAUSS_KERNEL,
		MEXICAN_HAT_KERNEL,
		OSCILLATORY_KERNEL,
		ASYMMETRIC_GAUSS_KERNEL,
		NORMAL_NOISE,
		CORRELATED_NORMAL_NOISE,
		FIELD_COUPLING,
		GAUSS_FIELD_COUPLING,
		MEMORY_TRACE,

		NEURAL_FIELD_2D,
		GAUSS_STIMULUS_2D,
		GAUSS_KERNEL_2D,
		MEXICAN_HAT_KERNEL_2D,
		NORMAL_NOISE_2D,
		OSCILLATORY_KERNEL_2D,
		TIMED_GAUSS_STIMULUS,
		TIMED_GAUSS_STIMULUS_2D,
		BOOST_STIMULUS_2D,
		CORRELATED_NORMAL_NOISE_2D,
		ASYMMETRIC_GAUSS_KERNEL_2D,
		MEMORY_TRACE_2D,

		RESIZE,
		RESIZE_2D,

		COLLAPSE,
		EXPAND,
	};

	inline const std::map<ElementLabel, std::string> ElementLabelToString = {
		{UNINITIALIZED, "uninitialized" },
		{NEURAL_FIELD, "neural field" },
		{GAUSS_STIMULUS, "gauss stimulus" },
		{BOOST_STIMULUS, "boost stimulus" },
		{GAUSS_KERNEL, "gauss kernel" },
		{MEXICAN_HAT_KERNEL, "mexican hat kernel" },
		{OSCILLATORY_KERNEL, "oscillatory kernel"},
		{ASYMMETRIC_GAUSS_KERNEL, "asymmetric gauss kernel"},
		{NORMAL_NOISE, "normal noise" },
		{CORRELATED_NORMAL_NOISE, "correlated normal noise"},
		{GAUSS_FIELD_COUPLING, "gauss field coupling" },
		{FIELD_COUPLING, "field coupling" },
		{MEMORY_TRACE, "memory trace" },

		{NEURAL_FIELD_2D, "neural field 2d" },
		{GAUSS_STIMULUS_2D, "gauss stimulus 2d" },
		{GAUSS_KERNEL_2D, "gauss kernel 2d" },
		{MEXICAN_HAT_KERNEL_2D, "mexican hat kernel 2d" },
		{NORMAL_NOISE_2D, "normal noise 2d" },
		{OSCILLATORY_KERNEL_2D, "oscillatory kernel 2d" },
		{TIMED_GAUSS_STIMULUS, "timed gauss stimulus" },
		{TIMED_GAUSS_STIMULUS_2D, "timed gauss stimulus 2d" },
		{BOOST_STIMULUS_2D, "boost stimulus 2d" },
		{CORRELATED_NORMAL_NOISE_2D, "correlated normal noise 2d" },
		{ASYMMETRIC_GAUSS_KERNEL_2D, "asymmetric gauss kernel 2d" },
		{MEMORY_TRACE_2D, "memory trace 2d" },

		{RESIZE, "resize" },
		{RESIZE_2D, "resize 2d" },

		{COLLAPSE, "collapse" },
		{EXPAND, "expand" },
	};

	struct ElementDimensions
	{
		int dimensionality; // 1 or 2 (D)
		int x_max, y_max;
		double d_x, d_y;
		int size_x, size_y, size;  // size = size_x * size_y

		explicit ElementDimensions(int dimensionality = 1);
		explicit ElementDimensions(int x_max, double d_x); // 1D
		explicit ElementDimensions(int x_max, int y_max, double d_x, double d_y); // 2D
		bool operator==(const ElementDimensions& other) const;
		void print() const;
		[[nodiscard]] std::string toString() const;
	};

	struct ElementIdentifiers
	{
		static inline int uniqueIdentifierCounter = 0;
		int uniqueIdentifier;
		std::string uniqueName;
		ElementLabel label;

		explicit ElementIdentifiers(ElementLabel label);
		explicit ElementIdentifiers(std::string elementName);
		bool operator==(const ElementIdentifiers& other) const;
		void print() const;
		[[nodiscard]] std::string toString() const;
	};

	struct ElementCommonParameters
	{
		ElementIdentifiers identifiers;
		ElementDimensions dimensionParameters;

		ElementCommonParameters();
		explicit ElementCommonParameters(ElementLabel label);
		explicit ElementCommonParameters(const std::string& elementName);
		ElementCommonParameters(const std::string& elementName, int x_max);
		ElementCommonParameters(const std::string& elementName,
		                        const ElementDimensions& dimensionParameters);
		ElementCommonParameters(ElementIdentifiers identifiers,
		                        const ElementDimensions& dimensionParameters);
		bool operator==(const ElementCommonParameters& other) const;
		void print() const;
		[[nodiscard]] std::string toString() const;
	};

	struct ElementSpecificParameters
	{
		ElementSpecificParameters() = default;
		virtual ~ElementSpecificParameters() = default;
		[[nodiscard]] virtual std::string toString() const = 0;
		void print() const;
	};
}
