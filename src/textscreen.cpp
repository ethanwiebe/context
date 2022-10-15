#include "textscreen.h"

s32 TextScreen::GetWidth() const {
	return width;
}

s32 TextScreen::GetHeight() const {
	return height;
}

void TextScreen::SetSize(s32 w,s32 h){
	width = w;
	height = h;

	cells.resize(w*h);
}

TextCell TextScreen::GetAt(s32 x,s32 y) const {
	if (x<0||x>=width||y<0||y>=height) return TextCell(' ',textStyle);
	
	return cells[y*width+x];
}

void TextScreen::SetAt(s32 x,s32 y,TextCell t){
	if (x<0||x>=width||y<0||y>=height) return;

	cells[y*width+x] = t;
}

std::vector<TextCell>::iterator TextScreen::begin(){
	return cells.begin();
}

std::vector<TextCell>::iterator TextScreen::end(){
	return cells.end();
}

size_t TextScreen::size() const noexcept {
	return cells.size();
}

TextCell TextScreen::operator[](size_t i) const {
	return cells[i];
}

TextCell& TextScreen::operator[](size_t i){
	return cells[i];
}

void TextScreen::RenderString(s32 x,s32 y,std::string_view s,TextStyle style){
	size_t index;
	if (y>=height) return;
	for (const auto& c : s){
		index = y*width + x++;
		if (index>cells.size()-1) break;
		if (x>=width) break;
		
		if (c<' ')
			cells[index] = TextCell(' ',style);
		else
			cells[index] = TextCell(c,style);
	}
}

#include <cstring>

void TextScreen::FastBlit(const TextScreen& other,s32 y){
	constexpr auto cellSize = sizeof(TextCell);
	
	s32 realH = std::min(height-y,other.height);
	
	memcpy(&cells[y*width],&other.cells[0],width*realH*cellSize);
}

void TextScreen::Blit(const TextScreen& other,s32 x,s32 y){
	if (other.width==width&&x==0){
		return FastBlit(other,y);
	}
	
	constexpr auto cellSize = sizeof(TextCell);
	s32 realW = std::min(width-x,other.width);
	if (realW<=0) return;
	
	s32 realH = std::min(height-y,other.height);
	if (realH<=0) return;
	
	for (s32 j=0;j<realH;++j){
		memcpy(&cells[(y+j)*width+x],&other.cells[j*width],realW*cellSize);
	}
}
