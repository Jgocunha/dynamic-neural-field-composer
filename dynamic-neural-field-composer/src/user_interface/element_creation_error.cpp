#include "user_interface/element_creation_error.h"

#include <exception>

#include "exceptions/exception.h"
#include "tools/logger.h"

namespace dnf_composer::user_interface
{
	std::string describeElementCreationFailure(const std::function<void()>& createAndAdd)
	{
		try
		{
			createAndAdd();
			return {};
		}
		catch (const Exception& ex)
		{
			// Already carries the element name and the ErrorCode.
			tools::logger::log(tools::logger::ERROR, std::string("Could not add element: ") + ex.what());
			return ex.what();
		}
		catch (const std::exception& ex)
		{
			tools::logger::log(tools::logger::ERROR, std::string("Could not add element: ") + ex.what());
			return ex.what();
		}
		catch (...)
		{
			tools::logger::log(tools::logger::ERROR, "Could not add element: unknown error.");
			return "Unknown error.";
		}
	}
}
