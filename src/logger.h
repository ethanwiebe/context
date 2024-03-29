#pragma once

#include "core.h"

#include <fstream>

class Logger {
public:
	Logger();
	~Logger();

	std::fstream logFile;

	Logger& operator<<(char c){
		logFile << c;
		logFile.flush();
		return *this;
	}

	Logger& operator<<(s32 i){
		logFile << i;
		logFile.flush();
		return *this;
	}

	Logger& operator<<(s64 i){
		logFile << i;
		logFile.flush();
		return *this;
	}

	Logger& operator<<(const char* c){
		logFile << c;
		logFile.flush();
		return *this;
	}

	Logger& operator<<(const std::string& s){
		logFile << s;
		logFile.flush();
		return *this;
	}

	Logger& operator<<(std::string_view sv){
		logFile << sv;
		logFile.flush();
		return *this;
	}

};

extern Logger logger;

#ifdef NDEBUG
#define LOG(x) ;
#else
#define LOG(x) logger << x << '\n'
#endif
