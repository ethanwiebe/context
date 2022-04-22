#pragma once

#include "core.h"

#include <vector>

struct TextCell {
	u8 c;
	u8 fg;
	u8 bg;
};

class TextScreen {
	s32 width,height;
	
	std::vector<TextCell> cells;

public:

	s32 GetWidth() const;
	s32 GetHeight() const;

	void SetSize(s32,s32);

	std::vector<TextCell>::iterator begin();
	std::vector<TextCell>::iterator end();

	size_t size() const noexcept;

	TextCell operator[](size_t) const;
	TextCell& operator[](size_t);
};

