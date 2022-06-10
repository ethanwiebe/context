#include "linemode.h"

void LineModeBase::VisualCursorInsertLine(VisualCursor& cursor){
	InsertCharAt(cursor.cursor,'\n');
	s32 indentLevel = textBuffer->GetIndentationAt(cursor.cursor.line.it,Config::tabSize);
	bool tabs = textBuffer->IsTabIndented(cursor.cursor.line.it);
	MoveCursorRight(cursor.cursor,1);
	if (Config::autoIndent){
		if (tabs){
			while (--indentLevel>=0){
				InsertCharAt(cursor.cursor,'\t');
				MoveVisualCursorRight(cursor,1);
			}
		} else {
			s32 spaceCount = Config::tabSize*indentLevel;
			while (--spaceCount>=0){
				InsertCharAt(cursor.cursor,' ');
				MoveVisualCursorRight(cursor,1);
			}
		}
	}
}

void LineModeBase::DeleteLine(Cursor cursor){
	cursor.column = 0;
	DeleteCharCountAt(cursor,cursor.line.it->size());
	if (cursor.line.index!=(s32)textBuffer->size()-1){
		DeleteCharAt(cursor);
	} else if (cursor.line.index!=0){
		MoveCursorLeft(cursor,1);
		DeleteCharAt(cursor);
	}
}

void LineModeBase::VisualCursorDeleteLine(VisualCursor& cursor){
	Cursor cachedCursor = cursor.cursor;
	if (cursor.cursor.line.index>=(s32)textBuffer->size()-1&&
			cursor.cursor.line.index!=0)
		--cursor.cursor.line;
		
	DeleteLine(cachedCursor);
	
	s32 maxLine = cursor.CurrentLineLen();
	SetVisualCursorColumn(cursor,std::min(cursor.cachedX,maxLine));
	ForceFinishAction();
}

void LineModeBase::DeleteCharAt(Cursor cursor,bool undoable){
	if (cursor.line.index==(s32)textBuffer->size()-1 &&
			cursor.column==(s32)cursor.line.it->size())
		return;

	char deletedChar;
	deletedChar = GetCharAt(cursor);

	if (undoable)
		PushDeletionAction(cursor,deletedChar);

	if (deletedChar=='\n'){
		textBuffer->ForwardDeleteLine(cursor.line.it);
	} else {
		cursor.line.it->erase(cursor.column,1);
	}
	
	SetModified();
}

void LineModeBase::DeleteCharCountAt(Cursor cursor,s32 count){
	while (--count>=0){
		DeleteCharAt(cursor);
	}
}

void LineModeBase::VisualCursorDeletePreviousChar(VisualCursor& cursor,s32 count){
	if (cursor.cursor.column==0&&cursor.cursor.line.index==0) return;
	
	MoveCursorLeft(cursor.cursor,count);
	DeleteCharCountAt(cursor.cursor,count);
}

void LineModeBase::InsertCharAt(Cursor cursor,char c,bool undoable){
	if (c=='\n'){
		textBuffer->InsertLineAfter(cursor.line.it,cursor.line.it->substr(cursor.column));
		cursor.line.it->erase(cursor.column);
	} else 
		cursor.line.it->insert(cursor.column,1,c);

	if (undoable)
		PushInsertionAction(cursor,c);
		
	SetModified();
}

void LineModeBase::InsertStringAt(Cursor cursor,const std::string& s,bool undoable){
	for (auto c : s){
		InsertCharAt(cursor,c,undoable);
		if (c=='\n'){
			++cursor.line;
			cursor.column = 0;
		} else
			++cursor.column;
	}
}

void LineModeBase::InsertLinesAt(Cursor cursor,const std::string& s){
	cursor.column = cursor.line.it->size();
	InsertCharAt(cursor,'\n');
	MoveCursorRight(cursor,1);
	for (auto c : s){
		InsertCharAt(cursor,c);
		MoveCursorRight(cursor,1);
	}
}

void LineModeBase::InsertTab(VisualCursor& cursor){
	if (textBuffer->IsTabIndented(cursor.cursor.line.it)){
		InsertCharAt(cursor.cursor,'\t');
		MoveVisualCursorRight(cursor,1);
	} else {
		for (size_t i=0;i<Config::tabSize;++i){
			InsertCharAt(cursor.cursor,' ');
			MoveVisualCursorRight(cursor,1);
		}
	}
}
