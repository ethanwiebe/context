#pragma once

#include "core.h"

struct Color {
	u8 r,g,b;

	bool operator==(Color other){
		return r==other.r && g==other.g && b==other.b;
	}
};

extern Color ColorBlack;
extern Color ColorWhite;
extern Color ColorRed;
extern Color ColorGreen;
extern Color ColorYellow;
extern Color ColorBlue;
extern Color ColorCyan;
extern Color ColorMagenta;
extern Color ColorOrange;

extern Color ColorBackgroundDark;
extern Color ColorBackgroundLight;
extern Color ColorForegroundDark;
extern Color ColorForegroundLight;

void SetColors();

enum StyleFlag : u8 {
	NoFlag = 0,
	Bold = 1,
	Underline = 2,
	AlternateCharacterSet = 4
};

struct TextStyle {
	Color fg;
	Color bg;
	u8 flags;

	TextStyle operator|(u8 f){
		return {fg,bg,(u8)(flags|f)};
	}
	
	bool operator==(const TextStyle& t){
		return fg==t.fg && bg==t.bg && flags==t.flags;
	}
};

extern TextStyle defaultStyle;
extern TextStyle cursorStyle;
extern TextStyle lineNumberStyle;
extern TextStyle blankLineStyle;
extern TextStyle lineDrawingStyle;
extern TextStyle errorStyle;
extern TextStyle statementStyle;
extern TextStyle typeStyle;
extern TextStyle funcStyle;
extern TextStyle commentStyle;
extern TextStyle barStyle;
extern TextStyle highlightStyle;
extern TextStyle highlightStyle2;
