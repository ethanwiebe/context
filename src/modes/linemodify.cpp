#include "linemode.h"

void LineModeBase::VisualCursorInsertLine(VisualCursor& cursor){
	InsertCharAt(cursor.cursor,'\n');
	s32 indentLevel = textBuffer->GetIndentationAt(cursor.cursor.line.it,gConfig.tabSize);
	bool tabs = textBuffer->IsTabIndented(cursor.cursor.line.it);
	MoveCursorRight(cursor.cursor,1);
	if (gConfig.autoIndent){
		if (tabs){
			while (--indentLevel>=0){
				InsertCharAt(cursor.cursor,'\t');
				MoveVisualCursorRight(cursor,1);
			}
		} else {
			s32 spaceCount = gConfig.tabSize*indentLevel;
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
	SetCachedX(cursor);
}

void LineModeBase::VisualCursorDeletePreviousWord(VisualCursor& cursor){
	if (cursor.cursor.column==0&&cursor.cursor.line.index==0) return;
	if (cursor.cursor.column==0) return VisualCursorDeletePreviousChar(cursor,1);
	
	MoveCursorLeft(cursor.cursor,1);
	char c = GetCharAt(cursor.cursor);
	
	if (c!=' '&&c!='\t'&&(c<'a'||c>'z')&&(c<'A'||c>'Z')&&(c<'0'||c>'9')){
		DeleteCharAt(cursor.cursor);
		return SetCachedX(cursor);
	}
	
	while (c==' '||c=='\t'){ //delete leading whitespace
		DeleteCharAt(cursor.cursor);
		if (cursor.cursor.column==0) return SetCachedX(cursor);
		MoveCursorLeft(cursor.cursor,1);
		c = GetCharAt(cursor.cursor);
	}
	
	while ((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')){
		DeleteCharAt(cursor.cursor);
		if (cursor.cursor.column==0) return SetCachedX(cursor);
		MoveCursorLeft(cursor.cursor,1);
		c = GetCharAt(cursor.cursor);
	}
	
	MoveCursorRight(cursor.cursor,1);
	
	SetCachedX(cursor);
}

void LineModeBase::VisualCursorDeleteCurrentWord(VisualCursor& cursor){
	if (cursor.cursor.line.index==(s64)textBuffer->size()-1&&
			cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return;
	if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return DeleteCharAt(cursor.cursor);
	
	char c = GetCharAt(cursor.cursor);
	
	if (c!=' '&&c!='\t'&&(c<'a'||c>'z')&&(c<'A'||c>'Z')&&(c<'0'||c>'9')){
		DeleteCharAt(cursor.cursor);
		return;
	}
	
	while (c==' '||c=='\t'){ //delete leading whitespace
		DeleteCharAt(cursor.cursor);
		if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return;
		c = GetCharAt(cursor.cursor);
	}
	
	while ((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')){
		DeleteCharAt(cursor.cursor);
		if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return;
		c = GetCharAt(cursor.cursor);
	}
}

void LineModeBase::VisualCursorDeletePreviousPascalWord(VisualCursor& cursor){
	if (cursor.cursor.column==0&&cursor.cursor.line.index==0) return;
	if (cursor.cursor.column==0) return VisualCursorDeletePreviousChar(cursor,1);
	
	MoveCursorLeft(cursor.cursor,1);
	char c = GetCharAt(cursor.cursor);
	
	if (c!=' '&&c!='\t'&&(c<'a'||c>'z')&&(c<'A'||c>'Z')&&(c<'0'||c>'9')){
		DeleteCharAt(cursor.cursor);
		return SetCachedX(cursor);
	}
	
	while (c==' '||c=='\t'){ //delete leading whitespace
		DeleteCharAt(cursor.cursor);
		if (cursor.cursor.column==0) return SetCachedX(cursor);
		MoveCursorLeft(cursor.cursor,1);
		c = GetCharAt(cursor.cursor);
	}
	
	bool isNumber = false;
	while (c>='0'&&c<='9'){
		isNumber = true;
		DeleteCharAt(cursor.cursor);
		if (cursor.cursor.column==0) return SetCachedX(cursor);
		MoveCursorLeft(cursor.cursor,1);
		c = GetCharAt(cursor.cursor);
	}
	
	if (isNumber){
		MoveCursorRight(cursor.cursor,1);
		SetCachedX(cursor);
		return;
	}
	
	bool capFirst = false;
	while (c>='A'&&c<='Z'){
		capFirst = true;
		DeleteCharAt(cursor.cursor);
		if (cursor.cursor.column==0) return SetCachedX(cursor);
		MoveCursorLeft(cursor.cursor,1);
		c = GetCharAt(cursor.cursor);
	}
	
	if (!capFirst){
		while ((c>='a'&&c<='z')){
			DeleteCharAt(cursor.cursor);
			if (cursor.cursor.column==0) return SetCachedX(cursor);
			MoveCursorLeft(cursor.cursor,1);
			c = GetCharAt(cursor.cursor);
		}
		
		if (c>='A'&&c<='Z'){
			DeleteCharAt(cursor.cursor);
			return SetCachedX(cursor);
		}
	}
	
	MoveCursorRight(cursor.cursor,1);
	
	SetCachedX(cursor);
}

void LineModeBase::VisualCursorDeleteCurrentPascalWord(VisualCursor& cursor){
	if (cursor.cursor.line.index==(s64)textBuffer->size()-1&&
			cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return;
	if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return DeleteCharAt(cursor.cursor);
	
	char c = GetCharAt(cursor.cursor);
	
	if (c!=' '&&c!='\t'&&(c<'a'||c>'z')&&(c<'A'||c>'Z')&&(c<'0'||c>'9')){
		DeleteCharAt(cursor.cursor);
		return;
	}
	
	while (c==' '||c=='\t'){ //delete leading whitespace
		DeleteCharAt(cursor.cursor);
		if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return;
		c = GetCharAt(cursor.cursor);
	}
	
	bool isNumber = false;
	while (c>='0'&&c<='9'){
		isNumber = true;
		DeleteCharAt(cursor.cursor);
		if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return;
		c = GetCharAt(cursor.cursor);
	}
	
	if (isNumber){
		return;
	}
	
	size_t capCount = 0;
	while (c>='A'&&c<='Z'){
		DeleteCharAt(cursor.cursor);
		if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return;
		c = GetCharAt(cursor.cursor);
		if (c>='A'&&c<='Z'){
			MoveCursorRight(cursor.cursor,1);
			char testC = GetCharAt(cursor.cursor);
			if (testC>='a'&&testC<='z'){
				MoveCursorLeft(cursor.cursor,1);
				return;
			}
			MoveCursorLeft(cursor.cursor,1);
		}
		++capCount;
	}
	
	if (capCount>1){
		return;
	}
	
	while (c>='a'&&c<='z'){
		DeleteCharAt(cursor.cursor);
		if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return;
		c = GetCharAt(cursor.cursor);
	}
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
		for (ssize_t i=0;i<gConfig.tabSize;++i){
			InsertCharAt(cursor.cursor,' ');
			MoveVisualCursorRight(cursor,1);
		}
	}
}

void LineModeBase::RemoveTab(VisualCursor& cursor){
	if (cursor.cursor.column==0)
		return;
		
	Cursor temp = cursor.cursor;
	MoveCursorLeft(temp,1);
	char c = GetCharAt(temp);
	if (c=='\t'){
		VisualCursorDeletePreviousChar(cursor,1);
	} else if (c==' '){
		s64 count = gConfig.tabSize;
		while (--count>=0&&cursor.cursor.column!=0){
			VisualCursorDeletePreviousChar(cursor,1);
			temp = cursor.cursor;
			MoveCursorLeft(temp,1);
			if (GetCharAt(temp)!=' ') break;
		}
	}
}
