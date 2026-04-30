#include "exceptions/exception.h"

namespace dnf_composer
{

	Exception::Exception(std::string message)
		: errorCode(ErrorCode::OK), errorMessage(std::move(message)), errorElement(), errorComponent(), errorIndex(0)
	{}

	Exception::Exception(const ErrorCode errorCode)
		: errorCode(errorCode), errorComponent(), errorIndex(0)
	{
		errorMessage = getErrorMessage();
	}

	Exception::Exception(const ErrorCode errorCode, std::string errorElement)
		: errorCode(errorCode), errorElement(std::move(errorElement)), errorComponent(), errorIndex(0)
	{
		errorMessage = getErrorMessage();
	}

	Exception::Exception(const ErrorCode errorCode, int errorIndex)
		: errorCode(errorCode), errorElement(), errorComponent()
	{
		this->errorIndex = errorIndex;
		errorMessage = getErrorMessage();
	}

	Exception::Exception(const ErrorCode errorCode, std::string errorElement, std::string errorComponent)
		: errorCode(errorCode), errorElement(std::move(errorElement)), errorComponent(std::move(errorComponent)), errorIndex(0)
	{
		errorMessage = getErrorMessage();
	}

	const char* Exception::what() const noexcept
	{
		return errorMessage.c_str();
	}

	ErrorCode Exception::getErrorCode() const
	{
		return errorCode;
	}

	std::string Exception::getErrorMessage() const
	{
		switch (errorCode)
		{
		case ErrorCode::OK:
			return "Should not get here.";
		case ErrorCode::APP_CONSTRUCTION:
			return "Failed to construct Application.";
		case ErrorCode::APP_INIT:
			return "Failed to init?() Application.";
		case ErrorCode::APP_STEP:
			return "Failed to step() Application.";
		case ErrorCode::APP_CLOSE:
			return "Failed to close() Application.";
		case ErrorCode::SIM_ELEM_NOT_FOUND:
			return "Element with id " + errorElement + " not found.";
		case ErrorCode::SIM_RUNTIME_LESS_THAN_ZERO:
			return "runTime should be greater than 0. Currently " + std::to_string(errorIndex) + ".";
		case ErrorCode::SIM_ELEM_INDEX:
			return "Element index out of range, in elements. Currently " + std::to_string(errorIndex) + ".";
		case ErrorCode::ELEM_COMP_NOT_FOUND:
			return "Component '" + errorComponent + "' not found for element with id '" + errorElement + "'.";
		case ErrorCode::ELEM_INPUT_IS_NULL:
			return "Input element is null.";
		case ErrorCode::ELEM_INPUT_ALREADY_EXISTS:
			return "Input element with the same name already exists.";
		case ErrorCode::SIM_ELEM_ALREADY_EXISTS:
			return "Element with id " + errorElement + " already exists.";
		case ErrorCode::ELEM_INPUT_NOT_FOUND:
			return "Input element with name " + errorElement + " not found.";
		case ErrorCode::ELEM_INPUT_SIZE_MISMATCH:
			return "Input element with name " + errorElement + " has a size mismatch.";
		case ErrorCode::ELEM_SIZE_NOT_ALLOWED:
			return "For now element (" + errorElement + ") cannot be resized as it can be potentially damaging for the simulation.";
		case ErrorCode::ELEM_RENAME_NOT_ALLOWED:
			return "For now element (" + errorElement + ") cannot be renamed as it can be potentially damaging for the simulation.";
		case ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE:
			return "Gaussian stimulus (" + errorElement + ") position is out of range, must be between 0 and size.";
		case ErrorCode::GAUSS_STIMULUS_SUM_MISMATCH:
			return "Size mismatch when summing gauss stimulus (" + errorElement + ").";
		case ErrorCode::VIS_DATA_NOT_FOUND:
			return "Failed to retrieve data for plotting (" + errorElement + errorComponent + ").";
		case ErrorCode::VIS_INVALID_SIM:
			return "Invalid simulation pointer.";
		case ErrorCode::VIS_INVALID_PLOTTING_INDEX:
			return "Invalid plotting index.";
		case ErrorCode::ELEM_INVALID_PARAMETER:
			return "Invalid parameter for element (" + errorElement + ").";
		case ErrorCode::ELEM_INVALID_SIZE:
			return "Invalid size for element (" + errorElement + ").";
		case ErrorCode::SIM_INVALID_PARAMETER:
			return "Invalid parameter for simulation.";
		case ErrorCode::LOG_LOCAL_TIME_ERROR:
			return "Error trying to obtain local time for error.";
		case ErrorCode::PLOT_INVALID_VIS_OBJ:
			return "You tried to create a plot without a visualization object.";
		case ErrorCode::APP_INVALID_SIM:
			return "Invalid simulation object.";
		case ErrorCode::APP_INVALID_VIS:
			return "Invalid visualization object.";
		case ErrorCode::APP_VIS_SIM_MISMATCH:
			return "Visualization object does not point to the same Simulation object as Application.";
		default:
			return "An error was detected but no suitable response was found!";
		}
	}
}