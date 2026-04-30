#include "tools/profiling.h"


namespace dnf_composer
{
	namespace tools
	{
		namespace profiling
		{
			Timer::Timer(std::string signature, std::ostream& outStream)
				: startTimepoint(std::chrono::high_resolution_clock::now()),
					signature(std::move(signature)),
					outStream(outStream)
			{}

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
				outStream << "Signature: " << signature
					<< " Duration (us): " << duration
				    //<< " Duration (ms): " << static_cast<double>(duration.count()) * 0.001
					<< std::endl;
			}
		}
	}
}
