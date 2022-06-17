#ifdef _WIN32

#include "os_windows.h"

#include <windows.h>
#include <shlwapi.h>
#include <locale>
#include <codecvt>
#include <string_view>
#include <fstream>
#include <cstdio>
#include <filesystem>

bool WindowsOSImpl::PathExists(std::string_view path) const {
	std::string copied = std::string(path);

	return PathFileExists(copied.c_str());
}

bool WindowsOSImpl::PathIsFile(std::string_view path) const {
	std::string copied = std::string(path);

	return !PathIsDirectory(copied.c_str());
}

bool WindowsOSImpl::FileIsReadable(std::string_view path) const {
	std::string temp = std::string(path);
	std::ifstream file(temp.c_str());
	return file.good();
}

bool WindowsOSImpl::FileIsWritable(std::string_view path) const {
	bool good;
	bool exists = PathExists(path);
	{
		std::string temp = std::string(path);
		std::ofstream file(temp.c_str(),std::ios::out|std::ios::app);
		good = file.good();
	}
	if (!exists)
		std::remove(path.data());

	return good;
}

//s64 WindowsOSImpl::GetModifyTime(std::string_view) const {
	
//}

void WindowsOSImpl::ListDir(const std::string& path,std::vector<std::string>& entries) const {
	const std::filesystem::path fspath{path};
	
	for (auto const& dirEntry : std::filesystem::directory_iterator{fspath}){
		entries.push_back(dirEntry.path().filename().string());
    }
}

bool WindowsOSImpl::PathsAreSame(std::string_view p1,std::string_view p2) const {
	const std::filesystem::path path1{p1};
	const std::filesystem::path path2{p2};
	
	std::error_code dummy;
	return std::filesystem::equivalent(path1,path2,dummy);
}

bool WindowsOSImpl::ReadFileIntoTextBuffer(std::string_view path,Ref<TextBuffer> textBuffer) const {
	textBuffer->clear();

	std::string temp = std::string(path);
	std::fstream file(temp.c_str(),std::ios::in);
	if (!file.good()) return false;

	std::string line;
	auto it = textBuffer->begin();
	
	char r;
	while (file.get(r)){
		if (r=='\n'){
			textBuffer->InsertLine(it,line);
			line.clear();
		} else {
			line += r;
		}
	}
	
	textBuffer->InsertLine(it,line);
	
	return true;
}

bool WindowsOSImpl::WriteTextBufferIntoFile(std::string_view path,Ref<TextBuffer> textBuffer) const {
	std::string temp = std::string(path);
	std::fstream file(temp.c_str(),std::ios::out);
	if (!file.good()) return false;

	auto end = --textBuffer->end();
	for (auto it=textBuffer->begin();it!=end;++it){
		file << *it << "\n";
	}
	
	file << *end;

	return true;
}

#endif