#include "fileio.h"

Ref<TextFile> FileManager::ReadFile(const std::string& path){
	std::fstream f(path,std::ios::in);

	Ref<TextFile> textFile = MakeRef<TextFile>();

	std::string line;

	while (std::getline(f,line)){
		textFile->InsertLine(textFile->end(),line);
	}

	return textFile;
}
