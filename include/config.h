#pragma once

#include "core.h"

#include <string>
#include <string_view>

struct Config {
	std::string style;
	bool sleepy;
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
