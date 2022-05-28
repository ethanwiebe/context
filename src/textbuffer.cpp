#include "textbuffer.h"

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

s32 TextBuffer::GetIndentationAt(LineIterator it,s32 size){
	s32 count = 0;
	s32 spaceCount = 0;
	char first = (*it)[0];

	if (first=='\t'){
		for (char c : *it){
			if (c!='\t') break;
			++count;
		}
	} else if (first==' '){
		for (char c : *it){
			if (c!=' ') break;
			++spaceCount;
			if (spaceCount==size){
				spaceCount = 0;
				++count;
			}
		}
	}
	return count;
}

bool TextBuffer::IsTabIndented(LineIterator it){
	char first = (*it)[0];
	return first=='\t';
}

s32 ToNextMultiple(s32 x,s32 d,s32 w){
	s32 xproper = x%w;
	return x + (d-xproper%d);
}

void UpdateXI(const std::string& str,s32& x,s32& i,s32 width){
	if (str[i]=='\t'){
		x = std::min(ToNextMultiple(x,Config::tabSize,width),ToNextMultiple(x,width,width));
		++i;
	} else if (str[i]&128){ //utf8 handling
		++i;
		++x;
		/*if ((str[i]>>5)==6){
			x++;
			i += 2;
		} else if ((str[i]>>4)==14){
			x++;
			i += 3;
		} else if ((str[i]>>3)==30){
			x++;
			i += 4;
		}*/
	} else {
		++x;
		++i;
	}
}

s32 GetXPosOfIndex(const std::string& str,s32 index,s32 width){
	s32 x = 0;
	s32 strLen = str.size();
	for (s32 i=0;i<index;){
		UpdateXI(str,x,i,width);

		if (i>strLen) break;
	}

	return x;
}

s32 GetIndexOfXPos(const std::string& str,s32 x,s32 width){
	s32 index = 0;
	s32 strLen = str.size();
	for (s32 i=0;i<x;){
		UpdateXI(str,i,index,width);

		if (index>strLen) break;
	}

	return index;
}
