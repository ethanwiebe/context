#pragma once

#include "core.h"

#include <functional>

struct AsyncContext {
	std::function<void()> func;
	float preDelay;
	bool updateAfter;
};

struct AsyncData {
	size_t index;
	bool canceled;
	bool updateAfter;
};
