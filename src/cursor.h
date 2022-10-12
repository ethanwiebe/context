#pragma once

#include "core.h"

#include "textbuffer.h"

struct Cursor {
	LineIndexedIterator line;
	s32 column;
	
	inline bool operator==(const Cursor& b) const noexcept {
		return line.index==b.line.index&&column==b.column;
	}
	
	inline bool operator>=(const Cursor& b) const noexcept {
		if (line.index>b.line.index) return true;
		else if (line.index<b.line.index) return false;
		
		return column>=b.column;
	}
};

Cursor MakeCursorAtBufferStart(TextBuffer&);
Cursor MakeCursorAtBufferEnd(TextBuffer&);
