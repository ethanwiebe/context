#ifdef _WIN32

#include "os_windows.h"

#include <windows.h>
#include <shlwapi.h>
#include <locale>
#include <codecvt>
#include <string_view>
#include <fstream>
#include <cstdio>

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

//void WindowsOSImpl::AutocompletePath(std::string&) const override;

bool WindowsOSImpl::ReadFileIntoTextBuffer(std::string_view path,Ref<TextBuffer> textBuffer) const {
	textBuffer->clear();

	std::string temp = std::string(path);
	std::fstream file(temp.c_str(),std::ios::in);
	if (!file.good()) return false;

	std::string line;
	auto it = textBuffer->begin();
	while (std::getline(file,line)){
		textBuffer->InsertLine(it,line);
	}
	
	return true;
}

bool WindowsOSImpl::WriteTextBufferIntoFile(std::string_view path,Ref<TextBuffer> textBuffer) const {
	std::string temp = std::string(path);
	std::fstream file(temp.c_str(),std::ios::out);
	if (!file.good()) return false;

	for (const auto& line : *textBuffer){
		file << line << "\n";
	}

	return true;
}

#endif