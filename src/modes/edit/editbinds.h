#pragma once

#include "core.h"
#include "../../keybind.h"

#define ACTION(x) x,

enum class EditAction : u16 {
	None,
	
	#include "editactions.h"
};

#undef ACTION

struct TextAction {
	EditAction action;
	
	union {
		s32 num;
		char character;
		const char* str;
	};
};

TextAction GetTextActionFromKey(KeyEnum,KeyModifier,s64,bool);

