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

#include <chrono>


// Scoped timer with message
struct Timer
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<float> duration;
	std::string message;
	Timer(std::string msg)
	{
		message = msg;
		start = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		std::cout << message << duration.count() << "s" << std::endl;
	}
};

// command line argument parser
void parse_args(int argc, char** argv, std::vector<std::filesystem::path>& paths, bool& filter_used, std::string& regex_filter)
{
	for (int i = 1; i < argc; i++)
	{
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
void build_index(std::vector<std::filesystem::path>& files, std::map<std::wstring, std::map<int, std::wstring>>& index)
{
	std::wstring word;
	int word_pos;

	for (int i = 0; i < files.size(); i++)
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
				index[word][i].append(std::to_wstring(word_pos) + L" ");
				word_pos++;
			}
		}
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

int main(int argc, char** argv)
{
	Timer timer("Time spent: ");
	std::vector<std::filesystem::path> dirs;

	bool filter_used = false;
	std::string regex_filter;

	parse_args(argc, argv, dirs, filter_used, regex_filter);

	// check if all command line arguments were entered correctly
	if (dirs.size() == 0)
	{
		std::cerr << "Could not get paths to directories from command line arguments. Exiting..." << std::endl;
		return 0;
	}

	std::vector<std::filesystem::path> files;

	build_document_table(dirs, files, filter_used, regex_filter);

	std::map<std::wstring, std::map<int, std::wstring>> index;

	build_index(files, index);

	print_index(index);

	std::cout << "Index size: " << index.size() << std::endl;

	return 0;
}