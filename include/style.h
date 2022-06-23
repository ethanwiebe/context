#pragma once

#include "core.h"

#include <map>

struct Color {
	u8 r,g,b;

	bool operator==(Color other){
		return r==other.r && g==other.g && b==other.b;
	}
};

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

struct StyleSet {
	TextStyle textStyle,cursorStyle,emptyLineStyle,barStyle,
			lineNumberStyle,errorStyle,highlightStyle,highlightSelectStyle,
			statementStyle,typeStyle,funcStyle,stringStyle,numberStyle,
			directiveStyle,commentStyle;
};

extern std::map<u64,StyleSet> gStyleMap;

extern const StyleSet defaultStyleSet;

#define STYLE(x) extern TextStyle x ## Style;
#include "stylenames.h"
#undef STYLE

void LoadStyle();
void SaveStyle();

