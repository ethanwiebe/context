#include "os.h"

#include <fstream>

void ChooseBestPrefix(
		const std::vector<std::string>& choices,
		std::string& best){
	
	best.clear();
	size_t lowest = std::string::npos;
	for (const auto& choice : choices){
		if (best.empty()){
			best = choice;
			lowest = choice.size();
		} else {
			for (size_t i=0;i<best.size();++i){
				if (i-1==lowest) break;
				if (choice[i]!=best[i]){
					lowest = i-1;
					best = choice.substr(0,i);
					break;
				}
			}
		}
	}
}

void CrunchHomePrefix(std::string& path,const std::string& homePrefix){
	if (!path.starts_with(homePrefix)) return;
	path = '~'+path.substr(homePrefix.size());
}

void OSInterface::AutocompletePath(std::string& path) const {
	std::string home = GetHomePath();
	
	bool crunch = false;
	if (path.starts_with("~/")){
		path = home+path.substr(1);
		crunch = true;
	}
	
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
	
	ChooseBestPrefix(validPrefixes,closest);
	if (!closest.empty())
		path = closest;
		
	if (crunch){
		CrunchHomePrefix(path,home);
	}
}

#include <thread>
#include <chrono>

s64 OSInterface::GetTime() const {
	return std::chrono::duration_cast<std::chrono::milliseconds,s64>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}

void OSInterface::Sleep(s64 f) const {
	std::this_thread::sleep_for(std::chrono::milliseconds(f));
}
