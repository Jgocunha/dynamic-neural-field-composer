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
			NORMAL_NOISE,
			FIELD_COUPLING,
			GAUSS_FIELD_COUPLING,
			LATERAL_INTERACTIONS
		};

		inline const std::map<ElementLabel, std::string> ElementLabelToString = {
			{NEURAL_FIELD, "neural field" },
			{GAUSS_STIMULUS, "gauss stimulus" },
			{GAUSS_FIELD_COUPLING, "gauss field coupling" },
			{FIELD_COUPLING, "field coupling" },
			{GAUSS_KERNEL, "gauss kernel" },
			{MEXICAN_HAT_KERNEL, "mexican hat kernel" },
			{NORMAL_NOISE, "normal noise" },
			{LATERAL_INTERACTIONS, "lateral interactions" },
		};

		struct ElementSpatialDimensionParameters
		{
			int x_max, size;
			double d_x;

			ElementSpatialDimensionParameters(int x_max = 100, double d_x = 1.0);
			void print() const;
		};

		struct ElementIdentifiers
		{
			static inline int uniqueIdentifierCounter = 0;
			int uniqueIdentifier;
			std::string uniqueName;
			ElementLabel label;

			ElementIdentifiers();
			ElementIdentifiers(std::string elementName);
			void print() const;
		};

		struct ElementCommonParameters
		{
			ElementIdentifiers identifiers;
			ElementSpatialDimensionParameters dimensionParameters;

			ElementCommonParameters();
			ElementCommonParameters(const std::string& elementName);
			ElementCommonParameters(const std::string& elementName, int x_max);
			ElementCommonParameters(const std::string& elementName, const ElementSpatialDimensionParameters& dimensionParameters);
			ElementCommonParameters(ElementIdentifiers identifiers, const ElementSpatialDimensionParameters& dimensionParameters);
			void print() const;
		};

		struct ElementSpecificParameters
		{
			// This struct is intentionally left empty.
			// Derived structs are expected to define specific member variables
			// and functions relevant to their respective elements.
			ElementSpecificParameters() = default;
			// Declare a virtual destructor
			virtual ~ElementSpecificParameters() = default;
		};
	}
}
