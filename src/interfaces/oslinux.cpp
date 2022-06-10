#include "oslinux.h"

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

void ListDir(const std::string& path,std::vector<std::string>& entries){
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir(path.c_str()))!=NULL){
		while ((ent = readdir(dir))!=NULL){
			entries.emplace_back(ent->d_name);
		}
		closedir(dir);
	}
}

void ChooseBestPrefix(
		const std::vector<std::string>& choices,
		std::string& best){
	
	best = {};
	size_t lowest = std::string::npos;
	for (const auto& choice : choices){
		if (best.empty()){
			best = choice;
			lowest = choice.size();
		} else {
			for (size_t i=0;i<best.size();++i){
				if (i==lowest) break;
				if (choice[i]!=best[i]){
					lowest = i-1;
					best = choice.substr(0,i);
					break;
				}
			}
		}
	}
}

void LinuxOSImpl::AutocompletePath(std::string& path) const {
	std::vector<std::string> entries = {};
	std::string pathPrefix;
	
	size_t index = path.find_last_of('/');
	if (index==std::string::npos)
		pathPrefix = "./";
	else
		pathPrefix = path.substr(0,index+1);
	
	ListDir(pathPrefix,entries);
	
	std::string build;
	std::string closest;
	std::vector<std::string> validPrefixes;
	
	for (const auto& entry : entries){
		build = {};
		if (pathPrefix.compare("./")!=0||path.starts_with("./"))
			build += pathPrefix;
		build += entry;
		if (build.starts_with(path)){
			validPrefixes.push_back(build);
		}
	}
	
	ChooseBestPrefix(validPrefixes,path);
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
