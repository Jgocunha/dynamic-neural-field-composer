#include "element_parameters/element_parameters.h"

namespace dnf_composer
{
	namespace element
	{
		ElementDimensions::ElementDimensions(int x_max, double d_x)
			: x_max(x_max), size(static_cast<int>(std::round(x_max / d_x))), d_x(d_x)
		{}

		bool ElementDimensions::operator==(const ElementDimensions& other) const
		{
			constexpr double epsilon = 1e-6;
			return x_max == other.x_max && std::abs(d_x - other.d_x) < epsilon;
		}

		void ElementDimensions::print() const
		{
			tools::logger::log(tools::logger::LogLevel::INFO, toString());
		}

		std::string ElementDimensions::toString() const
		{
			std::ostringstream result;
			result << std::fixed << std::setprecision(2); 
			result << "Dimensions: ["
				<< "x_max: " << x_max << ", "
				<< "d_x: " << d_x << ", "
				<< "Samples: " << size
				<< "]";
			return result.str();
		}

		ElementIdentifiers::ElementIdentifiers(ElementLabel label)
			: uniqueIdentifier(uniqueIdentifierCounter++), label(label)
		{
			uniqueName = "Element " + ElementLabelToString.at(label) + " " + std::to_string(uniqueIdentifier);
		}

		ElementIdentifiers::ElementIdentifiers(std::string elementName)
			: uniqueIdentifier(uniqueIdentifierCounter++), uniqueName(std::move(elementName)),
				label(ElementLabel::UNINITIALIZED)
		{}

		bool ElementIdentifiers::operator==(const ElementIdentifiers& other) const
		{
			return uniqueIdentifier == other.uniqueIdentifier && uniqueName == other.uniqueName &&
				label == other.label;
		}

		void ElementIdentifiers::print() const
		{
			tools::logger::log(tools::logger::LogLevel::INFO, toString());
		}

		std::string ElementIdentifiers::toString() const
		{
			std::ostringstream result;
			result << "Identifiers: ["
				<< "ID: " << uniqueIdentifier << ", "
				<< "Name: " << uniqueName << ", "
				<< "Type: " << ElementLabelToString.at(label)
				<< "]";
			return result.str();
		}

		ElementCommonParameters::ElementCommonParameters()
			: identifiers(ElementLabel::UNINITIALIZED), dimensionParameters()
		{
		}

		ElementCommonParameters::ElementCommonParameters(ElementLabel label)
			: identifiers(label), dimensionParameters()
		{
		}

		ElementCommonParameters::ElementCommonParameters(const std::string& elementName)
			: identifiers(elementName), dimensionParameters()
		{}

		ElementCommonParameters::ElementCommonParameters(const std::string& elementName, int x_max)
			: identifiers(elementName), dimensionParameters(x_max)
		{}

		ElementCommonParameters::ElementCommonParameters(const std::string& elementName, 
			const ElementDimensions& dimensionParameters)
			: identifiers(elementName), dimensionParameters(dimensionParameters)
		{}

		ElementCommonParameters::ElementCommonParameters(ElementIdentifiers identifiers, 
			const ElementDimensions& dimensionParameters)
			: identifiers(std::move(identifiers)), dimensionParameters(dimensionParameters)
		{}

		bool ElementCommonParameters::operator==(const ElementCommonParameters& other) const
		{
			return identifiers == other.identifiers && dimensionParameters == other.dimensionParameters;
		}

		void ElementCommonParameters::print() const
		{
			tools::logger::log(tools::logger::LogLevel::INFO, toString());
		}

		std::string ElementCommonParameters::toString() const
		{
			std::string result;
			result += "Common parameters {";
			result += "  " + identifiers.toString();
			result += dimensionParameters.toString() + "}";
			return result;
		}

		void ElementSpecificParameters::print() const
		{
			log(tools::logger::LogLevel::INFO, toString());
		}

	}
}