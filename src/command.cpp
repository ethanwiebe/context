#include "command.h"

#include <cstring>

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
		if (strlen(arg.def)==0) ++c;
	}
	return c;
}

