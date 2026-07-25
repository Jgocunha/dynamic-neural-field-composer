#include "element_parameters/element_parameters.h"

#include <cmath>

#include "exceptions/exception.h"

namespace dnf_composer::element
{
	namespace
	{
		// Upper bound on samples-per-axis and on the total sample count. This is not a
		// precision issue: it exists so that a bad/unvalidated dimension (negative, zero,
		// or absurdly large due to a mismatched constructor overload) can never silently
		// flow into a buffer allocation/wiring path and trigger an overrun or an
		// out-of-memory abort. Any legitimate field is many orders of magnitude below this.
		constexpr int kMaxAxisSamples = 1'000'000'000;

		// Validates (extent, spacing) and returns the resulting sample count. Throws
		// dnf_composer::Exception instead of letting a non-positive/non-finite input
		// (e.g. spacing == 0, negative extent) size a downstream buffer.
		[[nodiscard]] int toValidatedSampleCount(const int extent, const double spacing)
		{
			if (extent <= 0)
			{
				throw Exception("ElementDimensions: extent must be positive, got " + std::to_string(extent) + '.');
			}
			if (!std::isfinite(spacing) || spacing <= 0.0)
			{
				throw Exception("ElementDimensions: step size (d_x/d_y) must be a positive, finite value, got " +
					std::to_string(spacing) + '.');
			}

			const auto samples = static_cast<long long>(std::llround(static_cast<double>(extent) / spacing));
			if (samples <= 0 || samples > kMaxAxisSamples)
			{
				throw Exception("ElementDimensions: computed sample count " + std::to_string(samples) +
					" (extent " + std::to_string(extent) + " / step " + std::to_string(spacing) +
					") is outside the safe range 1.." + std::to_string(kMaxAxisSamples) +
					"; refusing to size a field/buffer from it.");
			}
			return static_cast<int>(samples);
		}

		// Combines two already-validated axis sample counts into a total, guarding against
		// the int overflow that size_x * size_y could otherwise produce for a 2D field.
		[[nodiscard]] int toValidatedTotalSampleCount(const int size_x, const int size_y)
		{
			const auto total = static_cast<long long>(size_x) * static_cast<long long>(size_y);
			if (total <= 0 || total > kMaxAxisSamples)
			{
				throw Exception("ElementDimensions: total sample count " + std::to_string(total) +
					" (size_x=" + std::to_string(size_x) + ", size_y=" + std::to_string(size_y) +
					") is outside the safe range 1.." + std::to_string(kMaxAxisSamples) +
					"; refusing to size a field/buffer from it.");
			}
			return static_cast<int>(total);
		}
	}

	ElementDimensions::ElementDimensions(const int dimensionality)
		: dimensionality(dimensionality), x_max(100), y_max(1), d_x(1.0), d_y(1.0)
	{
		if(dimensionality == 2)
		{
			y_max = 100;
		}
		else if (dimensionality != 1)
		{
			throw Exception("ElementDimensions(" + std::to_string(dimensionality) +
				"): invalid dimensionality (must be 1 or 2). Note the two constructors differ by one "
				"argument and mean different things: ElementDimensions{N} selects the field "
				"dimensionality (1 or 2), while ElementDimensions{N, d_x} builds a 1D field of length "
				"N. If you intended a field of size " + std::to_string(dimensionality) +
				", use ElementDimensions{" + std::to_string(dimensionality) + ", 1.0} instead.");
		}

		x_max = 100;

		d_x = 1.0;
		d_y = 1.0;

		size_x = toValidatedSampleCount(x_max, d_x);
		size_y = toValidatedSampleCount(y_max, d_y);
		size = toValidatedTotalSampleCount(size_x, size_y);
	}

	ElementDimensions::ElementDimensions(const int x_max, const double d_x)
		: dimensionality(1), x_max(x_max), y_max(1), d_x(d_x), d_y(1.0),
		  size_x(toValidatedSampleCount(x_max, d_x)), size_y(1),
		  size(size_x)
	{}

	ElementDimensions::ElementDimensions(const int x_max, const int y_max, const double d_x, const double d_y)
		: dimensionality(2), x_max(x_max), y_max(y_max), d_x(d_x), d_y(d_y),
		  size_x(toValidatedSampleCount(x_max, d_x)),
		  size_y(toValidatedSampleCount(y_max, d_y)),
		  size(toValidatedTotalSampleCount(size_x, size_y))
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
		if (size_y > 1) {
			result << ", y_max: " << y_max << ", d_y: " << d_y;
}
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
		: identifiers(label) 
	{
	}

	ElementCommonParameters::ElementCommonParameters(const std::string& elementName)
		: identifiers(elementName) 
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
