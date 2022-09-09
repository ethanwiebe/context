#pragma once

#include "core.h"

#include <string>

struct Message {
	std::string msg;
	bool displayed;
	
	inline void Set(std::string&& s){
		msg = s;
		displayed = false;
	}
	
	inline void Mark(){
		displayed = true;
	}
	
	inline void Clear(){
		if (displayed)
			msg.clear();
	}
	
	inline bool Empty() const {
		return msg.empty();
	}
};
