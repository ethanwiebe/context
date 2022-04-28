#pragma once

#include "mode.h"
#include "../keybind.h"
#include "../config.h"
#include "../textbuffer.h"

#include <vector>

struct TextCursor {
	IndexedIterator line;
	s32 visualLine;
	s32 column;
	s32 subline;

	TextCursor(LineIterator it) : line(it), visualLine(0), column(0), subline(0) {}

	inline s32 CurrentLineLen() const noexcept {
		return (*line).size();
	}

	//void SetLineFromVisualLine(IndexedIterator viewLine,s32 screenSubline,s32 w);
	void SetVisualLineFromLine(IndexedIterator viewLine,s32 screenSubline,s32 w,s32 h);
};


class LineModeBase : public ModeBase {
protected:
	Ref<TextBuffer> file;

	IndexedIterator viewLine;
	s32 screenSubline;
	std::vector<TextCursor> cursors;
	s32 lineWidth,innerHeight;

public:
	void ProcessKeyboardEvent(KeyboardEvent*) override;
	TextScreen GetTextScreen(s32,s32) override;

	virtual void ProcessTextAction(TextAction) = 0;

	void MoveScreenDown(s32);
	void MoveScreenUp(s32,bool = false);
	void MoveScreenToCursor(TextCursor&);
	void LockScreenToCursor(TextCursor&);

	void MoveCursorDown(TextCursor&,s32);
	void MoveCursorUp(TextCursor&,s32);
	void SetCursorColumn(TextCursor&,s32);
};

