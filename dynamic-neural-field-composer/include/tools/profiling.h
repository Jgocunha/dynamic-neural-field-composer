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

	/// @brief Scoped timer that prints its elapsed lifetime when destroyed.
	class Timer
	{
	public:
		/// @brief Start timing. The elapsed time is reported when the timer is destroyed.
		/// @param signature Label printed alongside the elapsed time.
		/// @param outStream Stream the elapsed time is written to.
		Timer(std::string signature = "something that takes time", std::ostream& outStream = std::cout);
		~Timer();
	private:
		void stop() const;
	
		std::chrono::time_point<std::chrono::high_resolution_clock> startTimepoint;
		std::string signature;
		std::ostream& outStream;
	};
}



