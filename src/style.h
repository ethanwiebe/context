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
	u8 flags;
};

enum StyleFlag : u8 {
	NoFlag = 0,
	Bold = 1,
	Underline = 2,
	AlternateCharacterSet = 4
};

const TextStyle defaultStyle = {ColorWhite,ColorBlack,StyleFlag::NoFlag};
const TextStyle cursorStyle = {ColorBlack,ColorWhite,StyleFlag::NoFlag};
const TextStyle lineNumberStyle = {ColorBlue,ColorBlack,StyleFlag::NoFlag};
const TextStyle blankLineStyle = {ColorCyan,ColorBlack,StyleFlag::NoFlag};
const TextStyle lineDrawingStyle = {ColorWhite,ColorBlack,StyleFlag::AlternateCharacterSet};
const TextStyle errorStyle = {ColorRed,ColorBlack,StyleFlag::NoFlag};

