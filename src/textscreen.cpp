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

void TextScreen::RenderString(s32 x,s32 y,std::string_view s){
	for (const auto& c : s){
		cells[y*width + x++] = TextCell(c,0,0);
	}
}
