#include "tools/utils.h"

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
	std::string getResourceRoot()
	{
		static const std::string cached = []() -> std::string
		{
			std::filesystem::path exeDir;
#ifdef _WIN32
			char buf[MAX_PATH];
			GetModuleFileNameA(nullptr, buf, MAX_PATH);
			exeDir = std::filesystem::path(buf).parent_path();
#elif defined(__APPLE__)
			char buf[PATH_MAX] = {};
			uint32_t size = sizeof(buf);
			if (_NSGetExecutablePath(buf, &size) != 0) {
				// buf was too small; size now holds the required length
				std::string dynbuf(size, '\0');
				if (_NSGetExecutablePath(dynbuf.data(), &size) != 0)
					return std::string(PROJECT_DIR);
				exeDir = std::filesystem::path(dynbuf).parent_path();
			} else {
				exeDir = std::filesystem::path(buf).parent_path();
			}
#else
			char buf[PATH_MAX] = {};
			const ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
			if (len <= 0)
				return std::string(PROJECT_DIR);
			buf[len] = '\0';
			exeDir = std::filesystem::path(buf).parent_path();
#endif
			const auto parent = std::filesystem::weakly_canonical(exeDir / "..");
			if (std::filesystem::exists(parent / "resources"))
				return parent.string();
			return std::string(PROJECT_DIR);
		}();
		return cached;
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
			for (auto& element : vector)
				file << element << " ";
			file.close();
			return true;
		}
		return false;
	}

	std::string replaceForwardSlashesWithBackslashes(const std::string& str)
	{
		std::string adjustedStr = str;
		size_t pos;
		while ((pos = adjustedStr.find('/')) != std::string::npos)
			adjustedStr.replace(pos, 1, "\\");
		return adjustedStr;
	}

	float getProcessMemoryMb()
	{
#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS pmc{};
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
			return static_cast<float>(pmc.WorkingSetSize) / (1024.0f * 1024.0f);
		return 0.0f;
#elif defined(__APPLE__)
		task_vm_info_data_t info{};
		mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
		if (task_info(mach_task_self(), TASK_VM_INFO,
		              reinterpret_cast<task_info_t>(&info), &count) == KERN_SUCCESS)
			return static_cast<float>(info.phys_footprint) / (1024.0f * 1024.0f);
		return 0.0f;
#else
		std::ifstream f("/proc/self/status");
		std::string line;
		while (std::getline(f, line))
			if (line.rfind("VmRSS:", 0) == 0)
			{
				long kb = 0;
				sscanf(line.c_str(), "VmRSS: %ld kB", &kb);
				return static_cast<float>(kb) / 1024.0f;
			}
		return 0.0f;
#endif
	}

}

