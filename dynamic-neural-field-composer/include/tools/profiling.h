#pragma once

#include <chrono>
#include <string>
#include <iostream>
#include <mutex>



namespace dnf_composer::tools::profiling
{
	// to use the timer class:
	// void your_function()
	// {
	//      {
	//          timer t;
	//          const int a = 1 + 5;
	//          std::cout << "My integer is: " << a << std::endl;
	//		}
	// }

	class Timer
	{
	public:
		Timer(std::string signature = "something that takes time", std::ostream& outStream = std::cout);
		~Timer();
	private:
		void stop() const;
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> startTimepoint;
		std::string signature;
		std::ostream& outStream;
	};
}



