#pragma once

#include "core.h"

#include <vector>
#include <string_view>

enum class ArgType : u8 {
	Any,
	String,
	Number
};

struct CArg {
	ArgType type;
	const char* name;
	const char* def = nullptr;
};

struct Command {
	const char* name;
	const std::vector<CArg> args;
};

extern const size_t gCommandCount;
extern const Command gCommands[];

bool GetCommandFromName(std::string_view,const Command**);
size_t GetReqArgCount(const Command&);

