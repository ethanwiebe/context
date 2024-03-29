#pragma once

#include "core.h"
#include "style.h"

#include <vector>

struct TextCell {
	u32 c;
	TextStyle style;
};

class TextScreen {
	s32 width,height;
	std::vector<TextCell> cells;

	void FastBlit(const TextScreen& other,s32 y);
public:
	s32 GetWidth() const;
	s32 GetHeight() const;

	void SetSize(s32,s32);

	TextCell GetAt(s32,s32) const;
	void SetAt(s32,s32,TextCell);

	std::vector<TextCell>::iterator begin();
	std::vector<TextCell>::iterator end();
	
	size_t size() const noexcept;

	TextCell operator[](size_t) const;
	TextCell& operator[](size_t);

	void RenderString(s32 x,s32 y,std::string_view s,TextStyle = textStyle);
	
	void Blit(const TextScreen& other,s32 x,s32 y);
};

