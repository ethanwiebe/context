#include "logger.h"

Logger::Logger(){
#ifndef NDEBUG
	logFile = std::fstream("c.log",std::ios::app|std::ios::out);
#endif
}

Logger::~Logger(){
#ifndef NDEBUG
	logFile.close();
#endif
}

Logger logger;

