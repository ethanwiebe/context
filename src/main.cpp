#include "context.h"

int main(int argc,char** argv){
	std::string file = {};
	if (argc==2){
		file = {argv[1]};
	}

	ContextEditor ct(file);
	return 0;
}

