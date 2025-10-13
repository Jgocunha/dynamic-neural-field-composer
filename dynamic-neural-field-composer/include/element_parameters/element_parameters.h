#pragma once

#include <map>
#include <string>
#include <format>

#include "tools/logger.h"

namespace dnf_composer
{
	namespace element
	{
		enum ElementLabel : int
		{
			UNINITIALIZED,
			NEURAL_FIELD,
			GAUSS_STIMULUS,
			GAUSS_KERNEL,
			MEXICAN_HAT_KERNEL,
			OSCILLATORY_KERNEL,
			ASYMMETRIC_GAUSS_KERNEL,
			NORMAL_NOISE,
			FIELD_COUPLING,
			GAUSS_FIELD_COUPLING,
			KERNEL_COUPLING,
		};

		inline const std::map<ElementLabel, std::string> ElementLabelToString = {
			{UNINITIALIZED, "uninitialized" },
			{NEURAL_FIELD, "neural field" },
			{GAUSS_STIMULUS, "gauss stimulus" },
			{GAUSS_FIELD_COUPLING, "gauss field coupling" },
			{FIELD_COUPLING, "field coupling" },
			{GAUSS_KERNEL, "gauss kernel" },
			{MEXICAN_HAT_KERNEL, "mexican hat kernel" },
			{OSCILLATORY_KERNEL, "oscillatory kernel"},
			{ASYMMETRIC_GAUSS_KERNEL, "asymmetric gauss kernel"},
			{NORMAL_NOISE, "normal noise" },
		};

		struct ElementDimensions
		{
			int x_max, size;
			double d_x;

			ElementDimensions(int x_max = 100, double d_x = 1.0);
			bool operator==(const ElementDimensions& other) const;
			void print() const;
			std::string toString() const;
		};

		struct ElementIdentifiers
		{
			static inline int uniqueIdentifierCounter = 0;
			int uniqueIdentifier;
			std::string uniqueName;
			ElementLabel label;

			ElementIdentifiers(ElementLabel label);
			ElementIdentifiers(std::string elementName);
			bool operator==(const ElementIdentifiers& other) const;
			void print() const;
			std::string toString() const;
		};

		struct ElementCommonParameters
		{
			ElementIdentifiers identifiers;
			ElementDimensions dimensionParameters;

			ElementCommonParameters();
			ElementCommonParameters(ElementLabel label);
			ElementCommonParameters(const std::string& elementName);
			ElementCommonParameters(const std::string& elementName, int x_max);
			ElementCommonParameters(const std::string& elementName,
				const ElementDimensions& dimensionParameters);
			ElementCommonParameters(ElementIdentifiers identifiers, 
				const ElementDimensions& dimensionParameters);
			bool operator==(const ElementCommonParameters& other) const;
			void print() const;
			std::string toString() const;
		};

		struct ElementSpecificParameters
		{
			ElementSpecificParameters() = default;
			virtual ~ElementSpecificParameters() = default;
			//virtual bool operator==(const ElementSpecificParameters& other) const = 0;
			virtual std::string toString() const = 0;
			void print() const;
		};
	}
}
