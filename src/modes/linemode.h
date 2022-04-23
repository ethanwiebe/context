#pragma once

#include "mode.h"
#include "../keybind.h"
#include "../config.h"

#include <vector>

struct TextCursor {
	IndexedIterator line;
	s32 visualLine;
	s32 column;
	s32 subline;

	TextCursor(LineIterator it) : line(it), visualLine(0), column(0), subline(0) {
		
	}

	inline s32 CurrentLineLen() const noexcept {
		return line.it->size();
	}

	void SetLineFromVisualLine(IndexedIterator viewLine,s32 screenSubline,s32 w);
};


class LineModeBase : public ModeBase {
protected:
	IndexedIterator viewLine;
	s32 screenSubline;
	std::vector<TextCursor> cursors;
	s32 lineWidth;

public:
	void ProcessKeyboardEvent(KeyboardEvent*) override;
	TextScreen GetTextScreen(s32,s32) override;

	virtual void ProcessTextAction(TextAction) = 0;

	void MoveCursorDown(TextCursor&,s32);
	void MoveCursorUp(TextCursor&,s32);
	void SetCursorColumn(TextCursor&,s32);
};

