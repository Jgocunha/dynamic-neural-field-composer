#pragma once

#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <functional>

namespace dnf_composer::tools::utils
{
	// Returns the runtime install prefix (parent of bin/).
	// Falls back to the compile-time PROJECT_DIR in dev builds.
	/// @brief Locate the runtime resource root (e.g. for shaders, fonts, presets).
	/// @return The runtime install prefix (parent of `bin/`), or the compile-time
	///         `PROJECT_DIR` when running from a dev build.
	std::string getResourceRoot();

	/// @brief Count the number of lines in a text file.
	/// @param filename Path to the file to read.
	/// @return Number of lines in the file.
	int countNumOfLinesInFile(const std::string& filename);

	/// @brief Write a vector of doubles to a file, one value per line.
	/// @param vector   Values to write.
	/// @param filename Path to the file to write.
	/// @return True on success, false if the file could not be opened.
	bool saveVectorToFile(const std::vector<double>& vector, const std::string& filename);

	/// @brief Replace every forward slash in a string with a backslash.
	/// @param str Input string.
	/// @return Copy of @p str with `/` replaced by `\`.
	std::string replaceForwardSlashesWithBackslashes(const std::string& str);

	/// @brief Resize a 2D matrix, growing or shrinking both dimensions.
	/// @tparam T Element type.
	/// @param matrix    Matrix to resize, in place.
	/// @param newRowSize Number of rows after resizing.
	/// @param newColSize Number of columns after resizing.
	template <typename T>
	void resizeMatrix(std::vector<std::vector<T>>& matrix, int newRowSize, int newColSize)
	{
		matrix.resize(newRowSize);
		for (int i = 0; i < newRowSize; i++) {
			matrix[i].resize(newColSize);
}
	}

	/// @brief Draw a uniformly distributed random number in [min, max].
	/// @tparam T Numeric type of the bounds and result.
	/// @param min Lower bound.
	/// @param max Upper bound.
	/// @return A random value in [@p min, @p max].
	template <typename T>
	T generateRandomNumber(const T& min, const T& max)
	{
		// Seed the random number generator with a random device
		std::random_device rd;
		std::mt19937 gen(rd());
		// Create a uniform distribution from 1 to 2 (inclusive)
		std::uniform_real_distribution<> dis(min, max);
		// Generate a random integer between 1 and 2
		T randomNum = dis(gen);
		return randomNum;
	}

	/// @brief Fill every element of a 2D matrix with a uniformly distributed random value.
	/// @tparam T Element type.
	/// @param matrix   Matrix to fill, in place.
	/// @param minRange Lower bound of the random range.
	/// @param maxRange Upper bound of the random range.
	template <typename T>
	void fillMatrixWithRandomValues(std::vector<std::vector<T>>& matrix, double minRange = -1.0, double maxRange = 1.0) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(minRange, maxRange);
		for (auto& row : matrix) {
			for (auto& element : row) {
				element = dis(gen);
}
}
	}

	/// @brief Portable wrapper around the platform's thread-safe localtime conversion.
	/// @param time   Time to convert.
	/// @param result Broken-down time, written on success.
	/// @return True on success, false if the conversion failed.
	inline bool safe_localtime(const std::time_t* time, std::tm* result)
	{
#ifdef _WIN32
		return localtime_s(result, time) == 0;
#else
		return localtime_r(time, result) != nullptr;
#endif
	}

	/// @brief Query the current process's resident memory usage.
	/// @return Resident memory usage in megabytes.
	float getProcessMemoryMb();

	/// @brief Run a step and turn any failure into a message.
	///
	/// Intended for call sites (e.g. the element-creation forms in the ImGui render
	/// loop) where an exception thrown from inside a frame unwinds straight out of
	/// the render loop and terminates the application, so the call is funnelled
	/// through here instead.
	///
	/// @param createAndAdd  The step to run.
	/// @return An empty string if @p createAndAdd completed; otherwise a message
	///         describing the failure. `dnf_composer::Exception` messages already
	///         name the element and the error code, so they are passed through as-is.
	[[nodiscard]] std::string describeElementCreationFailure(const std::function<void()>& createAndAdd);
}