#pragma once

#include <map>
#include <string>
#include <format>

#include "tools/logger.h"

/// @defgroup element_parameters Element Parameters
/// @brief Parameter structs and common identity/dimension types shared by all elements.

namespace dnf_composer
{
	namespace element
	{
		/// @brief Identifies the concrete type of a simulation element.
		/// @ingroup element_parameters
		enum ElementLabel : int
		{
			UNINITIALIZED,           ///< Default / not yet assigned.
			NEURAL_FIELD,            ///< NeuralField.
			GAUSS_STIMULUS,          ///< GaussStimulus.
			BOOST_STIMULUS,          ///< BoostStimulus.
			GAUSS_KERNEL,            ///< GaussKernel.
			MEXICAN_HAT_KERNEL,      ///< MexicanHatKernel.
			OSCILLATORY_KERNEL,      ///< OscillatoryKernel.
			ASYMMETRIC_GAUSS_KERNEL, ///< AsymmetricGaussKernel.
			NORMAL_NOISE,            ///< NormalNoise.
			FIELD_COUPLING,          ///< FieldCoupling.
			GAUSS_FIELD_COUPLING,    ///< GaussFieldCoupling.
			MEMORY_TRACE,            ///< MemoryTrace.
		};

		/// @brief Maps every ElementLabel to its human-readable name.
		inline const std::map<ElementLabel, std::string> ElementLabelToString = {
			{UNINITIALIZED, "uninitialized" },
			{NEURAL_FIELD, "neural field" },
			{GAUSS_STIMULUS, "gauss stimulus" },
			{BOOST_STIMULUS, "boost stimulus" },
			{GAUSS_FIELD_COUPLING, "gauss field coupling" },
			{FIELD_COUPLING, "field coupling" },
			{GAUSS_KERNEL, "gauss kernel" },
			{MEXICAN_HAT_KERNEL, "mexican hat kernel" },
			{OSCILLATORY_KERNEL, "oscillatory kernel"},
			{ASYMMETRIC_GAUSS_KERNEL, "asymmetric gauss kernel"},
			{NORMAL_NOISE, "normal noise" },
			{MEMORY_TRACE, "memory trace" },
		};

		/// @brief Spatial discretization of a one-dimensional field.
		/// @ingroup element_parameters
		struct ElementDimensions
		{
			int x_max;  ///< Spatial extent: field runs from 0 to x_max.
			int size;   ///< Number of samples: size = x_max / d_x.
			double d_x; ///< Spatial resolution (step size between samples).

			/// @brief Construct dimensions from extent and resolution.
			/// @param x_max  Upper spatial bound (default 100).
			/// @param d_x    Resolution (default 1.0).
			ElementDimensions(int x_max = 100, double d_x = 1.0);
			bool operator==(const ElementDimensions& other) const;
			void print() const;
			std::string toString() const;
		};

		/// @brief Identity information for an element: numeric ID, name, and type label.
		/// @ingroup element_parameters
		struct ElementIdentifiers
		{
			static inline int uniqueIdentifierCounter = 0; ///< Global counter for ID assignment.
			int uniqueIdentifier;                          ///< Auto-assigned numeric ID.
			std::string uniqueName;                        ///< User-supplied name.
			ElementLabel label;                            ///< Concrete element type.

			/// @brief Construct identifiers from a type label (name will be auto-generated).
			ElementIdentifiers(ElementLabel label);

			/// @brief Construct identifiers from a user-supplied name (label defaults to UNINITIALIZED).
			ElementIdentifiers(std::string elementName);
			bool operator==(const ElementIdentifiers& other) const;
			void print() const;
			std::string toString() const;
		};

		/// @brief Bundles identity and spatial dimensions passed to every Element constructor.
		/// @ingroup element_parameters
		struct ElementCommonParameters
		{
			ElementIdentifiers identifiers;       ///< Unique ID, name, and label.
			ElementDimensions dimensionParameters; ///< Spatial discretization.

			ElementCommonParameters();

			/// @brief Construct from a type label (auto-generates name and default dimensions).
			ElementCommonParameters(ElementLabel label);

			/// @brief Construct from a user-supplied name with default dimensions (100 nodes, d_x=1).
			ElementCommonParameters(const std::string& elementName);

			/// @brief Construct from name and explicit spatial extent.
			/// @param elementName  Unique name string.
			/// @param x_max        Upper spatial bound.
			ElementCommonParameters(const std::string& elementName, int x_max);

			/// @brief Construct from name and full dimension specification.
			ElementCommonParameters(const std::string& elementName,
				const ElementDimensions& dimensionParameters);

			/// @brief Construct from pre-built identifiers and dimensions.
			ElementCommonParameters(ElementIdentifiers identifiers,
				const ElementDimensions& dimensionParameters);
			bool operator==(const ElementCommonParameters& other) const;
			void print() const;
			std::string toString() const;
		};

		/// @brief Abstract base for all element-specific parameter structs.
		///
		/// Concrete parameter structs (e.g. NeuralFieldParameters, GaussKernelParameters)
		/// inherit from this and implement @c toString().
		/// @ingroup element_parameters
		struct ElementSpecificParameters
		{
			ElementSpecificParameters() = default;
			virtual ~ElementSpecificParameters() = default;
			/// @brief Return a human-readable summary of the parameters.
			virtual std::string toString() const = 0;
			void print() const;
		};
	}
}
