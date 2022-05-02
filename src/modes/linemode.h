#pragma once

#include "mode.h"
#include "../keybind.h"
#include "../config.h"
#include "../textbuffer.h"
#include "../tui.h"
#include "../logger.h"

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
	Ref<TextBuffer> textBuffer;
	std::string bufferPath;

	IndexedIterator viewLine;
	s32 screenSubline;
	std::vector<TextCursor> cursors;
	s32 lineWidth,innerHeight;

public:
	LineModeBase(ContextEditor* ctx);
	void InitIterators();

	TextScreen GetTextScreen(s32,s32) override;
	std::string_view GetBufferName() override;

	bool OpenAction(const OSInterface& os,std::string_view path) override;
	bool SaveAction(const OSInterface& os) override;
	void SetPath(const OSInterface&,std::string_view) override;
	
	bool HasSavePath() override;

	void MoveScreenDown(s32,bool = true);
	void MoveScreenUp(s32,bool = false);
	void MoveScreenToCursor(TextCursor&);
	void LockScreenToCursor(TextCursor&);

	void MoveCursorDown(TextCursor&,s32);
	void MoveCursorUp(TextCursor&,s32);
	void SetCursorColumn(TextCursor&,s32);
};

