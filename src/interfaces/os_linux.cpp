#ifdef __linux__

#include "os_linux.h"

#include <dirent.h>
#include <vector>

bool LinuxOSImpl::PathExists(std::string_view path) const {
	struct stat buf;
	std::string temp = std::string(path);
	return stat(temp.c_str(),&buf)==0;
}

bool LinuxOSImpl::PathIsFile(std::string_view path) const {
	struct stat buf;
	std::string temp = std::string(path);
	if (stat(temp.c_str(),&buf)!=0)
		return false;

	return !S_ISDIR(buf.st_mode);
}

bool LinuxOSImpl::FileIsReadable(std::string_view path) const {
	std::string temp = std::string(path);
	std::ifstream file(temp.c_str());
	return file.good();
}

bool LinuxOSImpl::FileIsWritable(std::string_view path) const {
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

void LinuxOSImpl::ListDir(const std::string& path,std::vector<std::string>& entries){
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir(path.c_str()))!=NULL){
		while ((ent = readdir(dir))!=NULL){
			entries.emplace_back(ent->d_name);
		}
		closedir(dir);
	}
}

s64 LinuxOSImpl::GetModifyTime(std::string_view path) const {
	struct stat64 buf;
	std::string temp = std::string(path);
	if (stat64(temp.c_str(),&buf)!=0)
		return 0;
	
	return buf.st_mtim.tv_sec;
}


bool LinuxOSImpl::ReadFileIntoTextBuffer(std::string_view path,Ref<TextBuffer> textBuffer) const {
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

bool LinuxOSImpl::WriteTextBufferIntoFile(std::string_view path,Ref<TextBuffer> textBuffer) const {
	std::string temp = std::string(path);
	std::fstream file(temp.c_str(),std::ios::out);
	if (!file.good()) return false;

	for (const auto& line : *textBuffer){
		file << line << "\n";
	}

	return true;
}

#endif