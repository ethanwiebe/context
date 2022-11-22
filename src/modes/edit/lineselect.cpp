#include "linemode.h"

void LineModeBase::StartSelecting(const VisualCursor& cursor){
	selecting = true;

	selectAnchor = cursor.cursor;
	selectCursor = cursor.cursor;
}

void LineModeBase::StopSelecting(){
	selecting = false;
}

void LineModeBase::UpdateSelection(const VisualCursor& cursor){
	selectCursor = cursor.cursor;
}

Cursor LineModeBase::GetSelectStartPos() const {
	if (selectAnchor.line.index>selectCursor.line.index){
		Cursor copy = selectCursor;
		MoveCursorRight(copy,1);
		return copy;
	}
	if (selectAnchor.line.index<selectCursor.line.index)
		return selectAnchor;
	if (selectAnchor.column>selectCursor.column){
		Cursor copy = selectCursor;
		MoveCursorRight(copy,1);
		return copy;
	}
	return selectAnchor;
}

Cursor LineModeBase::GetSelectEndPos() const {
	if (selectAnchor.line.index>selectCursor.line.index)
		return selectAnchor;
	if (selectAnchor.line.index<selectCursor.line.index){
		Cursor copy = selectCursor;
		MoveCursorLeft(copy,1);
		return copy;
	}
	if (selectAnchor.column>selectCursor.column)
		return selectAnchor;
	Cursor copy = selectCursor;
	MoveCursorLeft(copy,1);
	return copy;
}

void LineModeBase::VisualCursorDeleteSelection(VisualCursor& cursor,bool copy){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	
	if (end>=start){
		if (copy) copiedText.clear();
	
		while (end>=start){
			if (copy) copiedText.insert(copiedText.begin(),GetCharAt(end));
			DeleteCharAt(end);
			if (end.line.index==0&&end.column==0) break;
			MoveCursorLeft(end,1);
		}
		
		cursor.cursor = start;
		if (start.line.index<=viewLine.index)
			viewLine = start.line; //viewLine invalidated
		SetVisualCursorColumn(cursor,cursor.cursor.column);
	}
	StopSelecting();
	ForceFinishAction();
	highlighterNeedsUpdate = true;
}

void LineModeBase::CopySelection(){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	
	if (end.line.index==(s32)(textBuffer->size()-1)&&end.column==(s32)end.line.it->size())
		MoveCursorLeft(end,1);
	
	copiedText.clear();
	
	while (end.line.index>start.line.index||end.column>start.column){
		copiedText.insert(copiedText.begin(),GetCharAt(end));
		MoveCursorLeft(end,1);
	}
	
	copiedText.insert(copiedText.begin(),GetCharAt(end));
}

void LineModeBase::CopyLinesInSelection(){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	
	copiedText.clear();
	
	while (start.line.index!=end.line.index){
		copiedText += *start.line.it;
		copiedText += '\n';
		
		++start.line;
	}
	
	copiedText += *end.line.it;
}

void LineModeBase::IndentSelection(VisualCursor& cursor){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	Cursor& affected = cursor.cursor;
	
	end.column = 0;
	size_t diff;
	while (end.line.index>=start.line.index){
		bool tab = IsTabIndented(end.line.it);
		diff = 0;
		if (tab){
			InsertCharAt(end,'\t');
			++diff;
		} else {
			for (ssize_t i=0;i<config.tabSize;++i){
				++diff;
				InsertCharAt(end,' ');
			}
		}
		if (end.line.index==selectAnchor.line.index)
			selectAnchor.column += diff;
		if (end.line.index==selectCursor.line.index)
			selectCursor.column += diff;
		if (end.line.index==affected.line.index)
			affected.column += diff;
		--end.line;
	}
	
	SetCachedX(cursor);
}

void LineModeBase::DedentSelection(VisualCursor& cursor){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	Cursor& affected = cursor.cursor;
	
	end.column = 0;
	size_t diff;
	while (end.line.index>=start.line.index){
		bool tab = IsTabIndented(end.line.it);
		diff = 0;
		if (tab){
			if (GetCharAt(end)=='\t'){
				++diff;
				DeleteCharAt(end);
			}
		} else {
			for (ssize_t i=0;i<config.tabSize;++i){
				if (GetCharAt(end)!=' ') break;
				DeleteCharAt(end);
				++diff;
			}
		}
		if (end.line.index==selectAnchor.line.index)
			selectAnchor.column -= diff;
		if (end.line.index==selectCursor.line.index)
			selectCursor.column -= diff;
		if (end.line.index==affected.line.index)
			affected.column -= diff;
		
		--end.line;
	}
	
	SetCachedX(cursor);
}

void LineModeBase::DeleteLinesInSelection(VisualCursor& cursor){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	Cursor cached;
	
	s32 count = end.line.index-start.line.index+1;
	
	while (--count>=0){
		cached = start;
		if (cached.line.index==(s32)textBuffer->size()-1&&cached.line.index!=0)
			--cached.line;
			
		DeleteLine(start);
		start = cached;
	}
	
	StopSelecting();
	cursor.cursor = start;
	if (start.line.index<=viewLine.index)
		viewLine = start.line;
	SetVisualCursorColumn(cursor,std::min(cursor.CurrentLineLen(),cursor.cachedX));
}

