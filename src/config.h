#pragma once

#include "core.h"

#include "message.h"
#include "modes/mode.h"

#include <string>
#include <string_view>
#include <errno.h>

struct Config {
	std::string style;
	bool sleepy;
	s64 tabBarWidth;
};

extern Config gConfig;

inline bool ParseBool(bool& var,std::string_view val){
	if (val=="true"||val=="1")
		var = true;
	else if (val=="false"||val=="0")
		var = false;
	else
		return false;
	
	return true;
}

inline bool ParseInt(s64& var,std::string_view val){
	if (val.empty()) return false;
	
	errno = 0;
	ssize_t n = strtol(val.data(),NULL,10);
	if (errno==ERANGE)
		return false;
	
	var = n;
	return true;
}

inline bool ParsePositiveInt(s64& var,std::string_view val){
	if (val.empty()) return false;
	
	errno = 0;
	ssize_t n = strtol(val.data(),NULL,10);
	if (errno==ERANGE)
		return false;
	if (n<=0)
		return false;
	
	var = n;
	return true;
}

inline bool ParseNonNegativeInt(s64& var,std::string_view val){
	if (val.empty()) return false;
	
	errno = 0;
	ssize_t n = strtol(val.data(),NULL,10);
	if (errno==ERANGE)
		return false;
	if (n<0)
		return false;
	
	var = n;
	return true;
}

bool FindAndSetConfigVar(ModeIndex modeIndex,const std::string& name,const TokenVector& tokens,Message& error);

