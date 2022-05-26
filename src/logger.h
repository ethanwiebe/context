#pragma once

#include "core.h"

#include <fstream>

class Logger {
public:
	Logger();
	~Logger();

	std::fstream logFile;

	Logger& operator<<(char c){
#ifndef NDEBUG
		logFile << c;
		logFile.flush();
#endif
		return *this;
	}

	Logger& operator<<(s32 i){
#ifndef NDEBUG
		logFile << i;
		logFile.flush();
#endif
		return *this;
	}

	Logger& operator<<(s64 i){
#ifndef NDEBUG
		logFile << i;
		logFile.flush();
#endif
		return *this;
	}

	Logger& operator<<(const char* c){
#ifndef NDEBUG
		logFile << c;
		logFile.flush();
#endif
		return *this;
	}

	Logger& operator<<(const std::string& s){
#ifndef NDEBUG
		logFile << s;
		logFile.flush();
#endif
		return *this;
	}

	Logger& operator<<(std::string_view sv){
#ifndef NDEBUG
		logFile << sv;
		logFile.flush();
#endif
		return *this;
	}

};

extern Logger logger;
