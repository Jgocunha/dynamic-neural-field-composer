// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "elements/field_projection.h"

#include "tools/math.h"

namespace dnf_composer::element
{
	FieldProjectionParameters::FieldProjectionParameters(const int projectionAxis,
		const FieldProjectionCompression compressionType,
		const FieldProjectionDirection   direction)
		: projectionAxis(projectionAxis), compressionType(compressionType), direction(direction)
	{
	}

	std::string FieldProjectionParameters::toString() const
	{
		const char* axisStr  = (projectionAxis == 0) ? "0 (keep X)" : "1 (keep Y)";
		const char* dirStr   = (direction == FieldProjectionDirection::COMPRESS) ? "Compress 2D→1D" : "Expand 1D→2D";
		const char* compStr  = "Sum";
		if (compressionType == FieldProjectionCompression::AVERAGE) compStr = "Average";
		else if (compressionType == FieldProjectionCompression::MAXIMUM) compStr = "Maximum";
		else if (compressionType == FieldProjectionCompression::MINIMUM) compStr = "Minimum";
		return "Direction: " + std::string(dirStr) +
			"\nProjection axis: " + axisStr +
			"\nCompression: " + compStr;
	}

	FieldProjection::FieldProjection(const ElementCommonParameters& common,
		FieldProjectionParameters params)
		: Element(common), parameters(std::move(params))
	{
		commonParameters.identifiers.label = ElementLabel::FIELD_PROJECTION;
		resizeComponents();
	}

	void FieldProjection::addInput(const std::shared_ptr<Element>& inputElement,
	                               const std::string& inputComponent)
	{
		if (parameters.direction == FieldProjectionDirection::COMPRESS)
		{
			// Auto-adapt dimensions from the upstream 2D element
			const auto upDims = inputElement->getElementCommonParameters().dimensionParameters;
			commonParameters.dimensionParameters = upDims;
			const int projectedSize = (parameters.projectionAxis == 0) ? upDims.size_x : upDims.size_y;
			components["input"].assign(upDims.size, 0.0);
			components["output"].assign(projectedSize, 0.0);
		}
		Element::addInput(inputElement, inputComponent);
	}

	void FieldProjection::resizeComponents()
	{
		const int sx = commonParameters.dimensionParameters.size_x;
		const int sy = commonParameters.dimensionParameters.size_y;
		const int projectedSize = (parameters.projectionAxis == 0) ? sx : sy;

		if (parameters.direction == FieldProjectionDirection::COMPRESS)
		{
			// input: 2D (sx*sy) — already set by base; output: 1D
			components["output"].assign(projectedSize, 0.0);
		}
		else // EXPAND
		{
			// input: 1D (projectedSize) so addInput accepts a 1D upstream; output: 2D
			components["input"].assign(projectedSize, 0.0);
			components["output"].assign(sx * sy, 0.0);
		}
	}

	void FieldProjection::init()
	{
		resizeComponents();
		std::ranges::fill(components["input"], 0.0);
		std::ranges::fill(components["output"], 0.0);
	}

	void FieldProjection::step(double t, double deltaT)
	{
		updateInput();
		const int sx = commonParameters.dimensionParameters.size_x;
		const int sy = commonParameters.dimensionParameters.size_y;
		const auto& inp = components["input"];
		auto& out = components["output"];

		if (parameters.direction == FieldProjectionDirection::EXPAND)
		{
			if (parameters.projectionAxis == 0)
			{
				for (int xi = 0; xi < sx; ++xi)
					for (int yi = 0; yi < sy; ++yi)
						out[xi * sy + yi] = inp[xi];
			}
			else
			{
				for (int xi = 0; xi < sx; ++xi)
					for (int yi = 0; yi < sy; ++yi)
						out[xi * sy + yi] = inp[yi];
			}
			return;
		}

		if (parameters.projectionAxis == 0)
		{
			for (int xi = 0; xi < sx; ++xi)
			{
				double acc = (parameters.compressionType == FieldProjectionCompression::MINIMUM)
					? std::numeric_limits<double>::max()
					: (parameters.compressionType == FieldProjectionCompression::MAXIMUM)
					  ? std::numeric_limits<double>::lowest()
					  : 0.0;
				for (int yi = 0; yi < sy; ++yi)
				{
					const double v = inp[xi * sy + yi];
					switch (parameters.compressionType)
					{
					case FieldProjectionCompression::SUM:
					case FieldProjectionCompression::AVERAGE: acc += v; break;
					case FieldProjectionCompression::MAXIMUM: acc = std::max(acc, v); break;
					case FieldProjectionCompression::MINIMUM: acc = std::min(acc, v); break;
					}
				}
				out[xi] = (parameters.compressionType == FieldProjectionCompression::AVERAGE)
					? acc / sy : acc;
			}
		}
		else
		{
			if (parameters.compressionType == FieldProjectionCompression::MINIMUM)
				std::ranges::fill(out, std::numeric_limits<double>::max());
			else if (parameters.compressionType == FieldProjectionCompression::MAXIMUM)
				std::ranges::fill(out, std::numeric_limits<double>::lowest());
			else
				std::ranges::fill(out, 0.0);

			for (int xi = 0; xi < sx; ++xi)
				for (int yi = 0; yi < sy; ++yi)
				{
					const double v = inp[xi * sy + yi];
					switch (parameters.compressionType)
					{
					case FieldProjectionCompression::SUM:
					case FieldProjectionCompression::AVERAGE: out[yi] += v; break;
					case FieldProjectionCompression::MAXIMUM: out[yi] = std::max(out[yi], v); break;
					case FieldProjectionCompression::MINIMUM: out[yi] = std::min(out[yi], v); break;
					}
				}

			if (parameters.compressionType == FieldProjectionCompression::AVERAGE)
				for (auto& v : out) v /= sx;
		}
	}

	void FieldProjection::setParameters(const FieldProjectionParameters& params)
	{
		parameters = params;
		init();
	}

	FieldProjectionParameters FieldProjection::getParameters() const
	{
		return parameters;
	}

	std::shared_ptr<Element> FieldProjection::clone() const
	{
		return std::make_shared<FieldProjection>(*this);
	}

	std::string FieldProjection::toString() const
	{
		std::string result = "Field projection element\n";
		result += commonParameters.toString() + '\n';
		result += parameters.toString();
		return result;
	}
}
