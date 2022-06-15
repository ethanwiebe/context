#include "context.h"
#include "platform.h"

int main(int argc,char** argv){
	std::string file = {};
	if (argc==2){
		file = {argv[1]};
	#ifdef _WIN32
		FixWindowsPath(file);
	#endif
	}

	ContextEditor ct(file);
	return 0;
}

