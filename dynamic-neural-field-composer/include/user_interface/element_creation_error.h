#pragma once

#include <functional>
#include <string>

namespace dnf_composer::user_interface
{
	/// @brief Run an element-creation step and turn any failure into a message.
	///
	/// The element-creation forms run inside the ImGui render loop, and the library
	/// now reports invalid input by throwing (`ElementDimensions`, the `Element`
	/// base constructor, `ElementFactory`). An exception thrown from inside a frame
	/// unwinds straight out of the render loop and terminates the application, so
	/// every creation call site funnels through here instead.
	///
	/// @param createAndAdd  Builds the element and adds it to the simulation.
	/// @return An empty string if @p createAndAdd completed; otherwise a message
	///         describing the failure, suitable for display in the creation form.
	///         `dnf_composer::Exception` messages already name the element and the
	///         error code, so they are passed through as-is.
	///
	/// @note This is intentionally free of any ImGui dependency so it can be unit
	///       tested without a window or a render loop.
	[[nodiscard]] std::string describeElementCreationFailure(const std::function<void()>& createAndAdd);
}
