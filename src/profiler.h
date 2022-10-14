#pragma once

#include <chrono>
#include <source_location>
#include <map>
#include <string>

struct ProfilerInfo {
	std::chrono::microseconds time;
};

typedef std::map<std::string,ProfilerInfo> ProfilerMap;

extern ProfilerMap gProfilerMap;

struct ProfileThis {
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::string funcName;
	
	ProfileThis(const std::source_location& loc = std::source_location::current()){
		funcName = loc.function_name();
		
		start = std::chrono::steady_clock::now();
	}
	
	~ProfileThis(){
		std::chrono::microseconds duration = 
			std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::steady_clock::now()-start
			);
		
		gProfilerMap[funcName] = {duration};
	}
};
