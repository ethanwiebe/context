#pragma once

#include "core.h"

#include "textbuffer.h"

struct Cursor {
	LineIndexedIterator line;
	s32 column;
};

Cursor MakeCursorAtBufferStart(TextBuffer&);
Cursor MakeCursorAtBufferEnd(TextBuffer&);
