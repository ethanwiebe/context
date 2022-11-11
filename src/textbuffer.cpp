#include "textbuffer.h"

#include <sstream>

TextBuffer::TextBuffer(const char* c){
	lines = {};
	auto it = lines.begin();
	std::stringstream ss(c);
	std::string line;
	while (std::getline(ss,line)){
		lines.insert(it,line);
	}
	if (lines.size()==0)
		lines.insert(it,"");
}


TextBuffer::TextBuffer(const TextBuffer& t) : lines(t.lines){}

TextBuffer& TextBuffer::operator=(const TextBuffer& t){
	lines = t.lines;
	return *this;
}

void TextBuffer::clear() noexcept {
	lines.clear();
}

void TextBuffer::InsertLine(LineIterator it,const std::string& s){
	lines.insert(it,s);
}

void TextBuffer::InsertLineAfter(LineIterator it,const std::string& s){
	lines.insert(++it,s);
}

void TextBuffer::BackDeleteLine(LineIterator it){
	auto prev = it;
	--prev;
	*prev += *it;
	lines.erase(it);
}

void TextBuffer::ForwardDeleteLine(LineIterator it){
	auto next = it;
	++next;

	*it += *next;
	lines.erase(next);
}

void TextBuffer::SetLine(LineIterator it,const std::string& s){
	*it = s;
}

