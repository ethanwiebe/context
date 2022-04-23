#include "textfile.h"

IndexedIterator& IndexedIterator::operator+=(s32 n){
	index += n;
	while (n--){
		it++;
	}

	return *this;
}

IndexedIterator& IndexedIterator::operator-=(s32 n){
	index -= n;
	while (n--){
		it--;
	}

	return *this;
}

IndexedIterator& IndexedIterator::operator++(){
	++index;
	++it;

	return *this;
}

IndexedIterator& IndexedIterator::operator--(){
	--index;
	--it;

	return *this;
}


LineIterator TextFile::GetLineIterator(size_t s) noexcept {
	auto it = lines.begin();
	while (s--){ //TODO: use begin and end for optimization
		it++;
	}

	return it;
}

void TextFile::InsertLine(LineIterator it,const std::string& s){
	lines.insert(it,s);
}

void TextFile::InsertLineAfter(LineIterator it,const std::string& s){
	lines.insert(++it,s);
}

void TextFile::BackDeleteLine(LineIterator it){
	auto prev = it;
	--prev;
	*prev += *it;
	lines.erase(it);
}

void TextFile::ForwardDeleteLine(LineIterator it){
	auto next = it;
	++next;

	*it += *next;
	lines.erase(next);
}

void TextFile::SetLine(LineIterator it,const std::string& s){
	*it = s;
}

LineIterator TextFile::begin() noexcept {
	return lines.begin();
}

LineIterator TextFile::end() noexcept {
	return lines.end();
}

size_t TextFile::size() const noexcept {
	return lines.size();
}

s32 ToNextMultiple(s32 x,s32 d,s32 w){
	s32 xproper = x%w;
	return x + (d-xproper%d);
}

void UpdateXI(const std::string& str,s32& x,s32& i,s32 width){
	if (str[i]=='\t'){
		x = std::min(ToNextMultiple(x,Config::tabSize,width),ToNextMultiple(x,width,width));
		i++;
	} else if (str[i]&128){ //utf8 handling
		if ((str[i]>>5)==6){
			x++;
			i += 2;
		} else if ((str[i]>>4)==14){
			x++;
			i += 3;
		} else if ((str[i]>>3)==30){
			x++;
			i += 4;
		}
	} else {
		x++;
		i++;
	}
}

s32 GetXPosOfIndex(const std::string& str,s32 index,s32 width){
	s32 x = 0;
	s32 strLen = str.size();
	for (s32 i=0;i<index;){
		UpdateXI(str,x,i,width);

		if (index>strLen) break;
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
