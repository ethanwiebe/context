#pragma once

#include "core.h"

//key codes
enum KeyEnum {
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

enum class Action : u16 {
	None,

	MoveLeftChar,
	MoveRightChar,
	MoveUpLine,
	MoveDownLine,

	MoveLeftMulti,
	MoveRightMulti,
	MoveUpMulti,
	MoveDownMulti,

	MoveUpPage,
	MoveDownPage,

	MoveScreenUpLine,
	MoveScreenDownLine,

	MoveLeftWord,
	MoveRightWord,

	MoveToLineStart,
	MoveToLineEnd,
	
	MoveToBufferStart,
	MoveToBufferEnd,

	InsertChar,
	InsertLine,
	InsertTab,
	
	InsertLineAtEnd,

	DeletePreviousChar,
	DeleteCurrentChar,

	DeletePreviousMulti,
	DeleteCurrentMulti,

	DeleteLine,

	UndoAction,
	RedoAction,

	ToggleSelect,

	CutSelection,
	CopySelection,
	PasteClipboard,

	Escape,

	Entry,

	NewMode,
	OpenMode,
	CloseMode,
	NextMode,
	PreviousMode,
	SaveMode,
	SaveAsMode,

	DebugAction
};

struct TextAction {
	Action action;
	
	union {
		s32 num;
		char character;
		const char* str;
	};
};

s32 CharLower(s32);
s32 IsPrintable(s32,s32);
