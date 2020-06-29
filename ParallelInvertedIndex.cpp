#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>

#include <filesystem>

#include <algorithm>

#include <map>
#include <set>
#include <vector>

#include <mutex>
#include <thread>

#include <chrono>

std::mutex index_mtx;

// scoped timer with message
struct timer
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<float> duration;
	std::string message;
	timer(std::string msg)
	{
		message = msg;
		start = std::chrono::high_resolution_clock::now();
	}
	~timer()
	{
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		std::cout << message << duration.count() << "s" << std::endl;
	}
};

int main(int argc, char** argv)
{
	timer timer("time spent: ");

	return 0;
}