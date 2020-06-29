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

// command line argument parser
void parse_args(int argc, char** argv, int& thread_count, std::vector<std::filesystem::path>& paths)
{
	for (int i = 1; i < argc; i++)
	{
		if (argv[i] == std::string("-t"))
			thread_count = std::stoi(argv[i + 1]);
		if (argv[i] == std::string("-p"))
			paths.push_back(argv[i + 1]);
	}
}


int main(int argc, char** argv)
{
	timer timer("time spent: ");
	int thread_count = 0;
	std::vector<std::filesystem::path> dirs;

	parse_args(argc, argv, thread_count, dirs);

	// check if all command line arguments were entered correctly
	if (dirs.size() == 0)
	{
		std::cerr << "could not get paths to directories from command line arguments. exiting..." << std::endl;
		return 0;
	}
	if ((2 > thread_count) || (thread_count > std::thread::hardware_concurrency()))
	{
		std::cerr << "invalid thread count or could not get one. valid thread count is in range from 2 to "
			<< std::thread::hardware_concurrency() << ". exiting..." << std::endl;
		return 0;
	}

	return 0;
}