#pragma once

#include "core.h"

struct Color {
	u8 r,g,b;

	bool operator==(Color other){
		return r==other.r && g==other.g && b==other.b;
	}
};

const Color ColorBlack = {0,0,0};
const Color ColorWhite = {255,255,255};
const Color ColorRed = {255,0,0};
const Color ColorBlue = {0,0,255};
const Color ColorCyan = {0,255,255};

struct TextStyle {
	Color fg;
	Color bg;
	bool bold;
};

const TextStyle defaultStyle = {ColorWhite,ColorBlack,false};
const TextStyle cursorStyle = {ColorBlack,ColorWhite,false};
const TextStyle lineNumberStyle = {ColorBlue,ColorBlack,false};
const TextStyle blankLineStyle = {ColorCyan,ColorBlack,false};


