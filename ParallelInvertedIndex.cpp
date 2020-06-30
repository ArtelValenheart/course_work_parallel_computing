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
void parse_args(int argc, char** argv, int& thread_count, std::vector<std::filesystem::path>& paths, bool& filter_used, std::string& regex_filter)
{
	for (int i = 1; i < argc; i++)
	{
		if (argv[i] == std::string("-t"))
			thread_count = std::stoi(argv[i + 1]);
		if (argv[i] == std::string("-p"))
			paths.push_back(argv[i + 1]);
		if (argv[i] == std::string("-r"))
		{
			filter_used = true;
			regex_filter = argv[i + 1];
		}
	}
}

// removes all html tags and then removes all characters except for english alphatnumerical ones and returns false if text is empty
bool format_text(std::wstring& text)
{
	text = std::regex_replace(std::regex_replace(text, std::wregex(L"<([^>]*)>"), L""), std::wregex(L"[(\(\))]|[^(a-zA-Z\d )]"), L"");
	transform(text.begin(), text.end(), text.begin(), [](wchar_t c) {return towlower(c); });
	return text.compare(L"") != 0;
}

// builds index with word position in each document
void build_index(std::vector<std::filesystem::path>& files, std::map<std::wstring, std::map<int, std::wstring>>& index, int& thread_count, int id)
{
	std::wstring word;
	int word_pos;

	for (int i = id; i < files.size(); i += thread_count)
	{
		// opening files
		std::wifstream file(files[i]);

		// reading the file into string
		std::wstring file_content((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());

		// clead html and other symbols
		if (!format_text(file_content))
			continue;

		// input into subindex
		std::wistringstream wsstream(file_content);
		word_pos = 0;
		while (wsstream >> word)
		{
			if (word.compare(L"") != 0)
			{
				std::lock_guard<std::mutex> lk(index_mtx);
				index[word][i].append(std::to_wstring(word_pos) + L" ");
				word_pos++;
			}

		}
	}
}

void print_index(std::map<std::wstring, std::map<int, std::wstring>>& index)
{
	std::wofstream myfile("index.txt");

	for (auto[word, doc] : index)
	{
		myfile << word << L":" << std::endl;

		for (auto[doc_id, doc_positions] : doc)
			myfile << doc_id << L": " << doc_positions << std::endl;
		myfile << std::endl;
	}
}

void build_document_table(std::vector<std::filesystem::path>& dirs, std::vector<std::filesystem::path>& files, bool& filter_used, std::string& regex_filter)
{
	std::ofstream indexed_docs("indexed_docs.txt");

	// assign an id for each document and put paths in 'indexed_docs.txt'. 
	int doc_id = 0;
	if (!filter_used)
	{
		for (const auto& dir : dirs)
		{
			for (const auto& file : std::filesystem::directory_iterator(dir))
			{
				if (file.is_regular_file())
				{
					files.push_back(file);
					indexed_docs << doc_id++ << " : " << file.path().string() << std::endl;
				}
			}
		}
	}
	else
	{
		for (const auto& dir : dirs)
		{
			for (const auto& file : std::filesystem::directory_iterator(dir))
			{
				if (file.is_regular_file() && std::regex_match(file.path().string(), std::regex(regex_filter)))
				{
					files.push_back(file);
					indexed_docs << doc_id++ << " : " << file.path().string() << std::endl;
				}
			}
		}
	}
}

int main(int argc, char** argv)
{
	timer timer("time spent: ");
	int thread_count = 0;
	std::vector<std::filesystem::path> dirs;

	bool filter_used = false;
	std::string regex_filter;

	parse_args(argc, argv, thread_count, dirs, filter_used, regex_filter);

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

	std::vector<std::filesystem::path> files;

	build_document_table(dirs, files, filter_used, regex_filter);

	std::map<std::wstring, std::map<int, std::wstring>> index;

	// if number of files is less than specified thread count, then change thread count to be one thread per file
	if (files.size() < thread_count)
		thread_count = files.size();
	std::vector<std::thread> threads;
	for (int i = 1; i < thread_count; i++)
		threads.emplace_back(build_index, ref(files), std::ref(index), std::ref(thread_count), i);

	build_index(files, index, thread_count, 0);

	// wait for completion
	for (int i = 0; i < thread_count - 1; i++)
		threads[i].join();

	print_index(index);

	std::cout << "index size: " << index.size() << std::endl;

	return 0;
}