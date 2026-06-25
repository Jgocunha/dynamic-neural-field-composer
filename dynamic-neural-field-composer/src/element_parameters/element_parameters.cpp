#include "element_parameters/element_parameters.h"

namespace dnf_composer::element
{
	ElementDimensions::ElementDimensions(const int dimensionality)
		: dimensionality(dimensionality), x_max(100), y_max(1), d_x(1.0), d_y(1.0)
	{
		if(dimensionality == 2)
		{
			y_max = 100;
		}	
		else if (dimensionality != 1)
		{
			const std::string logMessage = "Element dimensionality '" + std::to_string(dimensionality) +
										"' is invalid. Defaulting to {1D, 100, 1.0}.";
			log(tools::logger::LogLevel::ERROR, logMessage);
			this->dimensionality = 1;
		}
	
		x_max = 100;

		d_x = 1.0;
		d_y = 1.0;

		size_x = static_cast<int>(std::round(x_max / d_x));
		size_y = static_cast<int>(std::round(y_max / d_y));
		size = static_cast<int>(std::round(x_max / d_x)) * static_cast<int>(std::round(y_max / d_y));
	}

	ElementDimensions::ElementDimensions(const int x_max, const double d_x)
		: dimensionality(1), x_max(x_max), y_max(1), d_x(d_x), d_y(1.0),
		  size_x(static_cast<int>(std::round(x_max / d_x))), size_y(1),
		  size(static_cast<int>(std::round(x_max / d_x)))
	{}

	ElementDimensions::ElementDimensions(const int x_max, const int y_max, const double d_x, const double d_y)
		: dimensionality(2), x_max(x_max), y_max(y_max), d_x(d_x), d_y(d_y),
		  size_x(static_cast<int>(std::round(x_max / d_x))),
		  size_y(static_cast<int>(std::round(y_max / d_y))),
		  size(static_cast<int>(std::round(x_max / d_x)) * static_cast<int>(std::round(y_max / d_y)))
	{}

	bool ElementDimensions::operator==(const ElementDimensions& other) const
	{
		constexpr double epsilon = 1e-6;
		return x_max == other.x_max && y_max == other.y_max &&
		       std::abs(d_x - other.d_x) < epsilon && std::abs(d_y - other.d_y) < epsilon;
	}

	void ElementDimensions::print() const
	{
		tools::logger::log(tools::logger::LogLevel::INFO, toString());
	}

	std::string ElementDimensions::toString() const
	{
		std::ostringstream result;
		result << std::fixed << std::setprecision(2);
		result << "Dimensionality: " << dimensionality;
		result << ", Dimensions: ["
				<< "x_max: " << x_max << ", "
				<< "d_x: " << d_x;
		if (size_y > 1)
			result << ", y_max: " << y_max << ", d_y: " << d_y;
		result << ", Samples: " << size << "]";
		return result.str();
	}

	ElementIdentifiers::ElementIdentifiers(const ElementLabel label)
		: uniqueIdentifier(uniqueIdentifierCounter.fetch_add(1, std::memory_order_relaxed)), label(label)
	{
		uniqueName = "Element " + ElementLabelToString.at(label) + " " + std::to_string(uniqueIdentifier);
	}

	ElementIdentifiers::ElementIdentifiers(std::string elementName)
		: uniqueIdentifier(uniqueIdentifierCounter.fetch_add(1, std::memory_order_relaxed)), uniqueName(std::move(elementName)),
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
		: identifiers(ElementLabel::UNINITIALIZED), dimensionParameters(100, 1.0)
	{
	}

	ElementCommonParameters::ElementCommonParameters(const ElementLabel label)
		: identifiers(label), dimensionParameters()
	{
	}

	ElementCommonParameters::ElementCommonParameters(const std::string& elementName)
		: identifiers(elementName), dimensionParameters()
	{}

	ElementCommonParameters::ElementCommonParameters(const std::string& elementName, int x_max)
		: identifiers(elementName), dimensionParameters(x_max, 1.0)
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
