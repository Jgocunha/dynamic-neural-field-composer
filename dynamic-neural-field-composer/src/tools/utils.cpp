#include "tools/utils.h"

#include <array>
#include <mutex>

#ifdef _WIN32
#  include <windows.h>
#  include <psapi.h>
#elif defined(__APPLE__)
#  include <mach-o/dyld.h>
#  include <mach/mach.h>
#  include <climits>
#else
#  include <climits>
#  include <unistd.h>
#endif

namespace dnf_composer::tools::utils
{
	namespace
	{
		std::once_flag resourceRootOnce;
		std::string resourceRootCache;

		// PROJECT_DIR is only defined when the project is configured with
		// DNF_COMPOSER_DEV_FALLBACK_PATHS=ON (the default -- see CMakeLists.txt).
		// A release/packaging build can disable it so no build-machine path is
		// compiled into the binary at all (#126); computeResourceRoot() then
		// simply has no fallback to offer if resources/ isn't found next to the
		// executable, which should never happen for a properly installed build.
		std::string devFallbackPath()
		{
#ifdef PROJECT_DIR
			return PROJECT_DIR;
#else
			return {};
#endif
		}
	}

	std::string resolveResourceRoot(const std::filesystem::path& exeDir, const std::string& devFallback)
	{
		const auto parent = std::filesystem::weakly_canonical(exeDir / "..");
		if (std::filesystem::exists(parent / "resources")) {
			return parent.string();
		}
		return devFallback;
	}

	namespace
	{
		std::string computeResourceRoot()
		{
			std::filesystem::path exeDir;
#ifdef _WIN32
			std::array<char, MAX_PATH> buf{};
			GetModuleFileNameA(nullptr, buf.data(), MAX_PATH);
			exeDir = std::filesystem::path(buf.data()).parent_path();
#elif defined(__APPLE__)
			std::array<char, PATH_MAX> buf{};
			uint32_t size = sizeof(buf);
			if (_NSGetExecutablePath(buf.data(), &size) != 0) {
				// buf was too small; size now holds the required length
				std::string dynbuf(size, '\0');
				if (_NSGetExecutablePath(dynbuf.data(), &size) != 0) {
					return devFallbackPath();
				}
				exeDir = std::filesystem::path(dynbuf).parent_path();
			} else {
				exeDir = std::filesystem::path(buf.data()).parent_path();
			}
#else
			std::array<char, PATH_MAX> buf{};
			const ssize_t len = readlink("/proc/self/exe", buf.data(), buf.size() - 1);
			if (len <= 0) {
				return devFallbackPath();
			}
			buf.at(len) = '\0';
			exeDir = std::filesystem::path(buf.data()).parent_path();
#endif
			return resolveResourceRoot(exeDir, devFallbackPath());
		}
	}

	std::string getResourceRoot()
	{
		std::call_once(resourceRootOnce, [] { resourceRootCache = computeResourceRoot(); });
		return resourceRootCache;
	}

	std::string getOutputDirectory()
	{
		return getResourceRoot() + "/data";
	}

	int countNumOfLinesInFile(const std::string& filename)
	{
		std::ifstream file(filename);
		if (file.is_open()) {
			int lineCount = 0;
			std::string line;
			while (std::getline(file, line)) {
				lineCount++;
			}
			file.close();
			return lineCount;
		}

		return -1; // Return -1 to indicate an error
	}

	bool saveVectorToFile(const std::vector<double>& vector, const std::string& filename)
	{
		std::ofstream file(filename);
		if (file.is_open())
		{
			for (const auto& element : vector) {
				file << element << " ";
}
			file.close();
			return true;
		}
		return false;
	}

	std::string replaceForwardSlashesWithBackslashes(const std::string& str)
	{
		std::string adjustedStr = str;
		size_t pos;
		while ((pos = adjustedStr.find('/')) != std::string::npos) {
			adjustedStr.replace(pos, 1, "\\");
}
		return adjustedStr;
	}

	float getProcessMemoryMb()
	{
#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS pmc{};
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
			return static_cast<float>(pmc.WorkingSetSize) / (1024.0F * 1024.0F);
}
		return 0.0F;
#elif defined(__APPLE__)
		task_vm_info_data_t info{};
		mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
		if (task_info(mach_task_self(), TASK_VM_INFO,
		              reinterpret_cast<task_info_t>(&info), &count) == KERN_SUCCESS) {
			return static_cast<float>(info.phys_footprint) / (1024.0F * 1024.0F);
		}
		return 0.0F;
#else
		std::ifstream f("/proc/self/status");
		std::string line;
		while (std::getline(f, line)) {
			if (line.starts_with("VmRSS:"))
			{
				long kb = 0;
				sscanf(line.c_str(), "VmRSS: %ld kB", &kb);
				return static_cast<float>(kb) / 1024.0F;
			}
		}
		return 0.0F;
#endif
	}

}

