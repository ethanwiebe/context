#include "linemode.h"

void UpdateSublineUpwards(LineIndexedIterator& line,s32& subline,s32& column,s32 width,s32 num,bool constrain=false){
	s32 lineSize;
	while (--num>=0){
		if (constrain&&line.index==0&&subline==0) return;

		if (subline!=0){
			--subline;
			column -= width;
		} else {
			--line;
			lineSize = GetXPosOfIndex(*line,(*line).size(),width);
			subline = lineSize/width;
			column += subline*width;
		}
	}
}

void UpdateSublineDownwards(LineIndexedIterator& line,s32& subline,s32& column,s32 width,s32 num,bool constrain=false,s32 lineCount=0){
	s32 lineSize;
	while (--num>=0){
		lineSize = GetXPosOfIndex(*line,(*line).size(),width);

		if (constrain&&line.index==lineCount-1&&subline==lineSize/width) return;
		if (lineSize>=(subline+1)*width){
			++subline;
			column += width;
		} else {
			++line;
			column %= width;
			subline = 0;
		}
	}
}

void LineModeBase::MoveScreenDown(s32 num,bool constrain){
	s32 dummy;
	UpdateSublineDownwards(viewLine,screenSubline,dummy,lineWidth,num,constrain,textBuffer->size());
	cursors[0].SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
	CalculateScreenData();
}

void LineModeBase::MoveScreenUp(s32 num,bool constrain){
	s32 dummy;
	UpdateSublineUpwards(viewLine,screenSubline,dummy,lineWidth,num,constrain);
	cursors[0].SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
	CalculateScreenData();
}

void LineModeBase::LockScreenToVisualCursor(VisualCursor& cursor,bool constrain){
	viewLine = cursor.cursor.line;
	screenSubline = cursor.subline;

	MoveScreenUp(innerHeight/2,constrain);
}

s32 TrueLineDistance(LineIndexedIterator start,s32 sublineStart,
		LineIndexedIterator end,s32 sublineEnd,s32 lineWidth){
	s32 dist = 0;
	s32 dummy;
	
	if ((start.index==end.index&&sublineStart>sublineEnd) ||
		(start.index>end.index)){
		std::swap(start,end);
		std::swap(sublineStart,sublineEnd);
	}
	
	while (start!=end||sublineStart!=sublineEnd){
		UpdateSublineUpwards(end,sublineEnd,dummy,lineWidth,1);
		++dist;
	}
	return dist;
}

void LineModeBase::MoveScreenToVisualCursor(VisualCursor& cursor){
	if (gConfig.cursorLock){
		LineModeBase::LockScreenToVisualCursor(cursor);
		return;
	}

	cursor.SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
	if (cursor.visualLine > gConfig.cursorMoveHeight && 
			cursor.visualLine < innerHeight-1-gConfig.cursorMoveHeight) return;

	s32 lineDiff = TrueLineDistance(cursor.cursor.line,cursor.subline,viewLine,screenSubline,lineWidth);
		
	viewLine = cursor.cursor.line;
	screenSubline = cursor.subline;

	if (lineDiff>innerHeight/2)
		MoveScreenUp(innerHeight-1-gConfig.cursorMoveHeight,true);
	else
		MoveScreenUp(gConfig.cursorMoveHeight,true);
	
	cursor.SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
	CalculateScreenData();
}

void LineModeBase::MoveVisualCursorDown(VisualCursor& cursor,s32 num){
	UpdateSublineDownwards(cursor.cursor.line,cursor.subline,cursor.cursor.column,
			lineWidth,num,true,textBuffer->size());
			
	s32 newCursorX = GetIndexOfXPos(*cursor.cursor.line,cursor.cachedX+cursor.subline*lineWidth,lineWidth);
	s32 maxLine = cursor.CurrentLineLen();
	cursor.cursor.column = std::min(newCursorX,maxLine);
}

void LineModeBase::MoveVisualCursorUp(VisualCursor& cursor,s32 num){
	UpdateSublineUpwards(cursor.cursor.line,cursor.subline,cursor.cursor.column,
			lineWidth,num,true);
			
	s32 newCursorX = GetIndexOfXPos(*cursor.cursor.line,cursor.cachedX+cursor.subline*lineWidth,lineWidth);
	s32 maxLine = cursor.CurrentLineLen();
	cursor.cursor.column = std::min(newCursorX,maxLine);
}

void LineModeBase::MoveVisualCursorLeft(VisualCursor& cursor,s32 num){
	s32 newCol = cursor.cursor.column-num;
	
	if (gConfig.cursorWrap){	
		while (newCol<0){ //move cursor up lines until only horiz adjust is left
			if (cursor.cursor.line.index==0){
				SetVisualCursorColumn(cursor,0);
				return;
			}
	
			--cursor.cursor.line;
			newCol += cursor.CurrentLineLen()+1;
		}
	} else {
		if (newCol<0)
			newCol = 0;
	}

	cursor.cursor.column = newCol;
	cursor.cachedX = GetXPosOfIndex(*cursor.cursor.line,cursor.cursor.column,lineWidth)%lineWidth;
}

void LineModeBase::MoveVisualCursorRight(VisualCursor& cursor,s32 num){
	s32 newCol = cursor.cursor.column+num;

	if (gConfig.cursorWrap){
		while (newCol>cursor.CurrentLineLen()){
			if (cursor.cursor.line.index==(s32)textBuffer->size()-1){
				SetVisualCursorColumn(cursor,cursor.CurrentLineLen());
				return;
			}
	
			newCol -= cursor.CurrentLineLen()+1;
			++cursor.cursor.line;
		}
	} else {
		if (newCol>cursor.CurrentLineLen())
			newCol = cursor.CurrentLineLen();
	}

	cursor.cursor.column = newCol;
	cursor.cachedX = GetXPosOfIndex(*cursor.cursor.line,cursor.cursor.column,lineWidth)%lineWidth;
}

void LineModeBase::MoveVisualCursorLeftWord(VisualCursor& cursor){
	if (cursor.cursor.column==0&&cursor.cursor.line.index==0) return;
	if (cursor.cursor.column==0){
		if (gConfig.cursorWrap)
			return MoveCursorLeft(cursor.cursor,1);
		return;
	}
	
	MoveCursorLeft(cursor.cursor,1);
	char c = GetCharAt(cursor.cursor);
	
	if (c!=' '&&c!='\t'&&(c<'a'||c>'z')&&(c<'A'||c>'Z')){
		return SetCachedX(cursor);
	}
	
	while (c==' '||c=='\t'){ //delete leading whitespace
		if (cursor.cursor.column==0) return SetCachedX(cursor);
		MoveCursorLeft(cursor.cursor,1);
		c = GetCharAt(cursor.cursor);
	}
	
	while ((c>='a'&&c<='z')||(c>='A'&&c<='Z')){
		if (cursor.cursor.column==0) return SetCachedX(cursor);
		MoveCursorLeft(cursor.cursor,1);
		c = GetCharAt(cursor.cursor);
	}
	
	MoveCursorRight(cursor.cursor,1);
	
	SetCachedX(cursor);
}

void LineModeBase::MoveVisualCursorRightWord(VisualCursor& cursor){
	if (cursor.cursor.line.index==(s64)textBuffer->size()-1&&
			cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return;
	if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()){
		if (gConfig.cursorWrap)
			return MoveCursorRight(cursor.cursor,1);
		return;
	}
	
	char c = GetCharAt(cursor.cursor);
	
	if (c!=' '&&c!='\t'&&(c<'a'||c>'z')&&(c<'A'||c>'Z')){
		MoveCursorRight(cursor.cursor,1);
		return;
	}
	
	while (c==' '||c=='\t'){ //delete leading whitespace
		MoveCursorRight(cursor.cursor,1);
		if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return SetCachedX(cursor);
		c = GetCharAt(cursor.cursor);
	}
	
	while ((c>='a'&&c<='z')||(c>='A'&&c<='Z')){
		MoveCursorRight(cursor.cursor,1);
		if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return SetCachedX(cursor);
		c = GetCharAt(cursor.cursor);
	}
	
	SetCachedX(cursor);
}

void LineModeBase::MoveVisualCursorLeftPascalWord(VisualCursor& cursor){
	if (cursor.cursor.column==0&&cursor.cursor.line.index==0) return;
	if (cursor.cursor.column==0){
		if (gConfig.cursorWrap)
			return MoveCursorLeft(cursor.cursor,1);
		return;
	}
	
	MoveCursorLeft(cursor.cursor,1);
	char c = GetCharAt(cursor.cursor);
	
	if (c!=' '&&c!='\t'&&(c<'a'||c>'z')&&(c<'A'||c>'Z')){
		return SetCachedX(cursor);
	}
	
	while (c==' '||c=='\t'){ //delete leading whitespace
		if (cursor.cursor.column==0) return SetCachedX(cursor);
		MoveCursorLeft(cursor.cursor,1);
		c = GetCharAt(cursor.cursor);
	}
	
	while ((c>='a'&&c<='z')){
		if (cursor.cursor.column==0) return SetCachedX(cursor);
		MoveCursorLeft(cursor.cursor,1);
		c = GetCharAt(cursor.cursor);
	}
	
	if (c>='A'&&c<='Z'){
		return SetCachedX(cursor);
	}
	
	MoveCursorRight(cursor.cursor,1);
	
	SetCachedX(cursor);
}

void LineModeBase::MoveVisualCursorRightPascalWord(VisualCursor& cursor){
	if (cursor.cursor.line.index==(s64)textBuffer->size()-1&&
			cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return;
	if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()){
		if (gConfig.cursorWrap)
			return MoveCursorRight(cursor.cursor,1);
		return;
	}
	
	char c = GetCharAt(cursor.cursor);
	
	if (c!=' '&&c!='\t'&&(c<'a'||c>'z')&&(c<'A'||c>'Z')){
		MoveCursorRight(cursor.cursor,1);
		return;
	}
	
	while (c==' '||c=='\t'){ //delete leading whitespace
		MoveCursorRight(cursor.cursor,1);
		if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return SetCachedX(cursor);
		c = GetCharAt(cursor.cursor);
	}
	
	if (c>='A'&&c<='Z'){
		MoveCursorRight(cursor.cursor,1);
		c = GetCharAt(cursor.cursor);
	}
	
	while (c>='a'&&c<='z'){
		MoveCursorRight(cursor.cursor,1);
		if (cursor.cursor.column==(s64)cursor.cursor.line.it->size()) return SetCachedX(cursor);
		c = GetCharAt(cursor.cursor);
	}
	
	SetCachedX(cursor);
}

void LineModeBase::MoveCursorLeft(Cursor& cursor,s32 num) const {
	s32 newCol = cursor.column-num;

	while (newCol<0){
		if (cursor.line.index==0){
			cursor.column = 0;
			return;
		}
		
		--cursor.line;
		newCol += cursor.line.it->size()+1;
	}

	cursor.column = newCol;
}

void LineModeBase::MoveCursorRight(Cursor& cursor,s32 num) const {
	s32 newCol = cursor.column+num;

	while (newCol>(s32)cursor.line.it->size()){
		if (cursor.line.index==(s32)textBuffer->size()-1){
			cursor.column = cursor.line.it->size();
			return;
		}

		newCol -= cursor.line.it->size()+1;
		++cursor.line;
	}

	cursor.column = newCol;
}

void LineModeBase::MoveVisualCursorToLineStart(VisualCursor& cursor){
	s32 col = cursor.cursor.column;
	SetVisualCursorColumn(cursor,0);
	if (gConfig.smartHome){
		while (GetCharAt(cursor.cursor)==' '||GetCharAt(cursor.cursor)=='\t')
			MoveVisualCursorRight(cursor,1);
		
		if (cursor.cursor.column==col) //already pressed home
			SetVisualCursorColumn(cursor,0);
	}
	SetCachedX(cursor);
}

void LineModeBase::MoveVisualCursorToLineEnd(VisualCursor& cursor){
	SetVisualCursorColumn(cursor,cursor.CurrentLineLen());
	SetCachedX(cursor);
}

void LineModeBase::MoveVisualCursorToBufferStart(VisualCursor& cursor){
	cursor.cursor = MakeCursorAtBufferStart(*textBuffer);
	SetVisualCursorColumn(cursor,cursor.cursor.column);
	SetCachedX(cursor);
}

void LineModeBase::MoveVisualCursorToBufferEnd(VisualCursor& cursor){
	cursor.cursor = MakeCursorAtBufferEnd(*textBuffer);
	SetVisualCursorColumn(cursor,cursor.cursor.column);
	SetCachedX(cursor);
}

void LineModeBase::SetVisualCursorColumn(VisualCursor& cursor,s32 col){
	cursor.cursor.column = col;
	cursor.subline = GetXPosOfIndex(*cursor.cursor.line,col,lineWidth)/lineWidth;
}

void LineModeBase::SetCachedX(VisualCursor& cursor){
	cursor.cachedX = GetXPosOfIndex(*cursor.cursor.line.it,cursor.cursor.column,lineWidth)%lineWidth;
}

void VisualCursor::SetVisualLineFromLine(LineIndexedIterator viewLine,s32 screenSubline,s32 w,s32 h){
	s32 count = 0;

	if (cursor.line.index<viewLine.index){
		visualLine = -100;
		return;
	}

	if (cursor.line.index==viewLine.index&&subline<screenSubline){
		visualLine = -100;
		return;
	}

	if (cursor.line.index-viewLine.index > h-1){
		visualLine = -100;
		return;
	}

	s32 lineSize;

	while (viewLine.index!=cursor.line.index||screenSubline!=subline){
		lineSize = GetXPosOfIndex(*viewLine,(*viewLine).size(),w);
		if (lineSize >= (screenSubline+1)*w){
			++screenSubline;
		} else {
			++viewLine;
			screenSubline = 0;
		}

		++count;
	}

	if (count > h-1)
		visualLine = -100;
	else
		visualLine = count;	
}

