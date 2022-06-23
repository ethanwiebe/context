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

		cells[index] = TextCell(c,style);
	}
}
