#include "command.h"

#include <cstring>

const size_t gCommandCount = 13;

const Command gCommands[gCommandCount] = {
	{
		"open", 
		{
			{ArgType::String,"path"}, 
			{ArgType::String,"mode","edit"}
		}
	},
	{
		"save",
		{}
	},
	{
		"saveAs",
		{
			{ArgType::String,"path"}
		}
	},
	{
		"var",
		{
			{ArgType::String, "varName"},
			{ArgType::Any, "value"}
		}
	},
	{
		"modeVar",
		{
			{ArgType::String, "varName"},
			{ArgType::Any, "value"}
		}
	},
	{
		"bind",
		{
			{ArgType::String, "actionName"},
			{ArgType::Any, "binds..."}
		}
	},
	{
		"style",
		{
			{ArgType::String, "styleName"},
			{ArgType::String, "fgColor"},
			{ArgType::String, "bgColor"},
			{ArgType::String, "opts", ""}
		}
	},
	{
		"source",
		{
			{ArgType::String, "path"}
		}
	},
	{
		"proc",
		{
			{ArgType::String, "procName"}
		}
	},
	{
		"end",
		{}
	},
	{
		"run",
		{
			{ArgType::String, "procName"}
		}
	},
	{
		"ext",
		{
			{ArgType::String, "extension"},
			{ArgType::String, "modeName"},
			{ArgType::String, "procName",""}
		}
	},
	{
		"modeHook",
		{
			{ArgType::String, "modeName"},
			{ArgType::String, "procName"}
		}
	}
};

bool GetCommandFromName(std::string_view name,const Command** cmd){
	for (size_t i=0;i<gCommandCount;++i){
		if (gCommands[i].name==name){
			*cmd = &gCommands[i];
			return true;
		}
	}
	return false;
}

size_t GetReqArgCount(const Command& cmd){
	size_t c = 0;
	for (const auto& arg : cmd.args){
		if (arg.def==nullptr) ++c;
	}
	return c;
}

