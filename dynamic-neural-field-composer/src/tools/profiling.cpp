// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "tools/profiling.h"


namespace dnf_composer
{
	namespace tools
	{
		namespace profiling
		{
			Timer::Timer(std::string sig)
				: startTimepoint(std::chrono::high_resolution_clock::now()), sig(std::move(sig))
			{
			}

			Timer::~Timer()
			{
				stop();
			}

			void Timer::stop() const
			{
				auto endTimepoint = std::chrono::high_resolution_clock::now();
				const auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTimepoint).time_since_epoch();
				const auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch();
				const auto duration = end - start;
				std::cout << "Signature: " << sig << " Duration: " << duration << std::endl;
			}
		}
	}
}

