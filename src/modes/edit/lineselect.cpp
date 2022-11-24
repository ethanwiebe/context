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
	if (selectAnchor.line.index>selectCursor.line.index){
		Cursor copy = selectAnchor;
		MoveCursorLeft(copy,1);
		return copy;
	}
	if (selectAnchor.line.index<selectCursor.line.index){
		Cursor copy = selectCursor;
		MoveCursorLeft(copy,1);
		return copy;
	}
	if (selectAnchor.column>selectCursor.column){
		Cursor copy = selectAnchor;
		MoveCursorLeft(copy,1);
		return copy;
	}
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

// fixes not grabbing the last line in selection if
// the selection column happens to be column 0
void LineModeBase::SelectEndCursorFix(Cursor& end) const {
	if (end.column==(s32)end.line.it->size()&&end.line.index!=(s32)(textBuffer->size()-1)){
		MoveCursorRight(end,1);
	}
}

void LineModeBase::CopyLinesInSelection(){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	SelectEndCursorFix(end);
	
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
	SelectEndCursorFix(end);
	
	Cursor& affected = cursor.cursor;
	
	LineDiffInfo info = {};
	LineDiff lineDiff = {};
	
	end.column = 0;
	size_t diff;
	while (end.line.index>=start.line.index){
		lineDiff.location = end.line;
		lineDiff.before = *end.line.it;
		diff = 0;
		bool tab = IsTabIndented(end.line.it);
		if (tab){
			diff = 1;
			end.line.it->insert(0,1,'\t');
		} else {
			diff = config.tabSize;
			end.line.it->insert(0,config.tabSize,' ');
		}
		if (end.line.index==selectAnchor.line.index)
			selectAnchor.column += diff;
		if (end.line.index==selectCursor.line.index)
			selectCursor.column += diff;
		if (end.line.index==affected.line.index)
			affected.column += diff;
		lineDiff.after = *end.line.it;
		info.push_back(lineDiff);
		--end.line;
	}
	
	PushLineReplacementAction(std::move(info));
	SetModified();
	highlighterNeedsUpdate = true;
	SetCachedX(cursor);
}

void LineModeBase::DedentSelection(VisualCursor& cursor){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	SelectEndCursorFix(end);
	
	Cursor& affected = cursor.cursor;
	
	LineDiffInfo info = {};
	LineDiff lineDiff = {};
	
	end.column = 0;
	s32 diff;
	while (end.line.index>=start.line.index){
		if (end.line.it->empty()){
			--end.line;
			continue;
		}
		char front = end.line.it->front();
		if (front!=' '&&front!='\t'){
			--end.line;
			continue;
		}
		
		lineDiff.location = end.line;
		lineDiff.before = *end.line.it;
		diff = 0;
		if (front=='\t'){
			diff = 1;
			end.line.it->erase(0,1);
		} else {
			for (ssize_t i=0;i<config.tabSize;++i){
				if (end.line.it->front()!=' ') break;
				end.line.it->erase(0,1);
				++diff;
			}
		}
		if (end.line.index==selectAnchor.line.index)
			selectAnchor.column = std::max(selectAnchor.column-diff,0);
		if (end.line.index==selectCursor.line.index)
			selectCursor.column = std::max(selectCursor.column-diff,0);
		if (end.line.index==affected.line.index)
			affected.column = std::max(affected.column-diff,0);
		
		lineDiff.after = *end.line.it;
		info.push_back(lineDiff);
		--end.line;
	}
	
	if (!info.empty()){
		PushLineReplacementAction(std::move(info));
		SetModified();
		highlighterNeedsUpdate = true;
	}
	SetCachedX(cursor);
}

void LineModeBase::DeleteLinesInSelection(VisualCursor& cursor){
	Cursor start = GetSelectStartPos();
	Cursor end = GetSelectEndPos();
	SelectEndCursorFix(end);
	
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

