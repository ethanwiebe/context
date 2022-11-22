#pragma once

#include "core.h"

#include <string>
#include <deque>

struct MessageQueue {
	std::deque<std::string> msgs;
	
	inline void Push(std::string&& s){
		msgs.push_back(s);
	}
	
	inline std::string Front() const {
		if (!Empty()) return msgs.front();
		return {};
	}
	
	inline std::string Pop(){
		std::string top = Front();
		msgs.pop_front();
		return top;
	}
	
	inline void Clear(){
		msgs.clear();
	}
	
	inline bool Empty() const {
		return msgs.empty();
	}
};
