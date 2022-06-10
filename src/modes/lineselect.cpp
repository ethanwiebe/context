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
	if (selectAnchor.line.index>selectCursor.line.index)
		return selectCursor;
	if (selectAnchor.line.index<selectCursor.line.index)
		return selectAnchor;
	if (selectAnchor.column>selectCursor.column)
		return selectCursor;
	return selectAnchor;
}

Cursor LineModeBase::GetSelectEndPos() const {
	if (selectAnchor.line.index>selectCursor.line.index)
		return selectAnchor;
	if (selectAnchor.line.index<selectCursor.line.index)
		return selectCursor;
	if (selectAnchor.column>selectCursor.column)
		return selectAnchor;
	return selectCursor;
}

void LineModeBase::VisualCursorDeleteSelection(VisualCursor& cursor,bool copy){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	
	if (end.line.index==(s32)(textBuffer->size()-1)&&end.column==(s32)end.line.it->size())
		MoveCursorLeft(end,1);
	
	if (copy) copiedText.clear();

	while (end.line.index>start.line.index||end.column>start.column){
		if (copy) copiedText.insert(copiedText.begin(),GetCharAt(end));
		DeleteCharAt(end);
		MoveCursorLeft(end,1);
	}
	
	if (copy) copiedText.insert(copiedText.begin(),GetCharAt(end));
	DeleteCharAt(end);

	cursor.cursor = start;
	if (start.line.index<=viewLine.index)
		viewLine = start.line; //viewLine invalidated
	SetVisualCursorColumn(cursor,cursor.cursor.column);
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

void LineModeBase::IndentSelection(){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	
	end.column = 0;
	
	
	while (end.line.index>=start.line.index){
		bool tab = textBuffer->IsTabIndented(end.line.it);
		if (tab)
			InsertCharAt(end,'\t');
		else {
			for (size_t i=0;i<Config::tabSize;++i)
				InsertCharAt(end,' ');
		}
		--end.line;
	}
	
	highlighterNeedsUpdate = true;
}

void LineModeBase::DedentSelection(){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	
	end.column = 0;
	
	while (end.line.index>=start.line.index){
		bool tab = textBuffer->IsTabIndented(end.line.it);
		if (tab){
			if (GetCharAt(end)=='\t')
				DeleteCharAt(end);
		} else {
			for (size_t i=0;i<Config::tabSize;++i){
				if (GetCharAt(end)!=' ') break;
				DeleteCharAt(end);
			}
		}
		
		--end.line;
	}
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

