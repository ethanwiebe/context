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
	const char* def = "";
};

struct Command {
	const char* name;
	const std::vector<CArg> args;
};

const Command gCommands[] = {
	{
		"open", 
		{
			{ArgType::String,"path"}, 
			{ArgType::String,"mode","edit"}
		}
	},
	{
		"saveas",
		{
			{ArgType::String,"path"}
		}
	},
	{
		"set",
		{
			{ArgType::String, "varName"},
			{ArgType::Any, "value"}
		}
	}
};

const size_t gCommandCount = 3;

bool GetCommandFromName(std::string_view,const Command**);
size_t GetReqArgCount(const Command&);

