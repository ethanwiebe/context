#include "logger.h"

Logger::Logger(){
	logFile = std::fstream("c.log",std::ios::app|std::ios::out);
}

Logger::~Logger(){
	logFile.close();
}

Logger logger;

