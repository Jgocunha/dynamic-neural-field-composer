// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "tools/utils.h"

#ifdef _WIN32
#  include <windows.h>
#else
#  include <climits>
#  include <unistd.h>
#endif

namespace dnf_composer::tools::utils
{
	std::string getResourceRoot()
	{
		std::filesystem::path exeDir;
#ifdef _WIN32
		char buf[MAX_PATH];
		GetModuleFileNameA(nullptr, buf, MAX_PATH);
		exeDir = std::filesystem::path(buf).parent_path();
#else
		char buf[PATH_MAX] = {};
		const ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
		if (len <= 0)
			return std::string(PROJECT_DIR);
		buf[len] = '\0';
		exeDir = std::filesystem::path(buf).parent_path();
#endif
		const auto candidate = exeDir / ".." / "resources";
		if (std::filesystem::exists(candidate))
			return std::filesystem::weakly_canonical(exeDir / "..").string();
		return std::string(PROJECT_DIR);
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

}


