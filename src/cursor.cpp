#include "cursor.h"

Cursor MakeCursorAtBufferStart(TextBuffer& buf){
	return {{buf.begin(),0},0};
}

Cursor MakeCursorAtBufferEnd(TextBuffer& buf){
	LineIndexedIterator indexIt = {--buf.end(),(s32)(buf.size()-1)};
	return {indexIt,(s32)indexIt.it->size()};
}
