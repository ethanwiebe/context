#include "fileio.h"

Ref<TextBuffer> FileManager::ReadFile(const std::string& path){
	std::fstream f(path,std::ios::in);

	Ref<TextBuffer> textBuffer = MakeRef<TextBuffer>();

	std::string line;

	while (std::getline(f,line)){
		textBuffer->InsertLine(textBuffer->end(),line);
	}

	return textBuffer;
}
