#pragma once

#include "core.h"

//key codes
enum class KeyEnum : s32 {
	None = 0,
	Backspace = 263,
	Tab = 9,
	Enter = 10,
	Escape = 27,
	Space = ' ',
	
	Up = 259,
	Down = 258,
	Left = 260,
	Right = 261,

	F1 = 265,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,

	PageUp = 339,
	PageDown = 338,

	Delete = 330,
	Insert = 331,
	Home = 262,
	End = 360
};

enum KeyModifier {
	None = 0,
	Shift = 1,
	Ctrl = 2,
	Alt = 4
};

struct KeyboardEvent {
	s32 key;
	s32 mod;
};

#define ACTION(x) x,

enum class Action : u16 {
	None,
	
	#include "actions.h"
};

#undef ACTION

struct TextAction {
	Action action;
	
	union {
		s32 num;
		char character;
		const char* str;
	};
};


