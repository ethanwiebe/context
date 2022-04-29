#include "linemode.h"

s32 tenPow(s32 i){
	s32 p = 1;
	while (i--){
		p *= 10;
	}

	return p;
}

s32 numWidth(s32 i){
	s32 c = 1;
	while (i>9){
		i /= 10;
		++c;
	}
	return c;
}

TextScreen LineModeBase::GetTextScreen(s32 w,s32 h){
	TextScreen textScreen;

	s32 lineNumberWidth = std::max(numWidth(viewLine.index+h)+1,4);

	screenWidth = w;
	screenHeight = h;
	lineWidth = screenWidth-lineNumberWidth;
	innerHeight = screenHeight - 1;

	textScreen.SetSize(w,h);
	
	std::fill(textScreen.begin(),textScreen.end(),TextCell(' ',defaultStyle));

	auto it = viewLine;
	s32 lineLen,lineStart = 0;
	s32 lineNumber;
	char c;

	s32 i = 0;

	if (it.index>=0)
		i = GetIndexOfXPos(*it.it,screenSubline*lineWidth,lineWidth);

	for (s32 y=0;y<innerHeight;y++){
		if (it.index<0){
			textScreen[y*w] = TextCell('~',blankLineStyle);
			++it;
			continue;
		}
		if (it.it==file->end()){
			textScreen[y*w] = TextCell('~',blankLineStyle);
			continue;
		}

		lineStart = 0;
		lineLen = it.it->size();

		if (Config::displayLineNumbers){ //line numbers
			if (i==0){ //only print line numbers if this is the start of a line
				lineNumber = it.index+1;
				s32 x;
				s32 powerOfTen;
				for (s32 i=0;i<lineNumberWidth-1;i++){
					x = lineNumberWidth-2-i;
					powerOfTen = tenPow(i);
					if (powerOfTen>lineNumber) break;
	
					textScreen[y*w+x] = TextCell('0'+(lineNumber/tenPow(i))%10, lineNumberStyle);
				}
			}
			lineStart += lineNumberWidth;
		}

		for (s32 x=0;x<w;){
			if (i>=lineLen) break;


			c = (*it.it)[i];
			if (c&128) c = '?'; //utf8
			if (c=='\t') c = ' ';

			textScreen[y*w+x+lineStart] = TextCell(c,defaultStyle);

			UpdateXI(*it.it,x,i,lineWidth);
			if (x >= w-lineStart){
				y++;
				x = 0;
				if (y>=innerHeight) break;
			}
		}
		i = 0;
		++it;
	}

	s32 loc;
	for (auto cursor : cursors){
		loc = cursor.visualLine*w + GetXPosOfIndex(*cursor.line.it,cursor.column,lineWidth)%lineWidth + lineStart;
		if (loc>=0&&loc<screenWidth*innerHeight)
			textScreen[loc].style = cursorStyle;
	}
	
	std::string locString = "";

	locString += "CL: " + std::to_string(cursors[0].line.index);
	locString += ", CVL: " + std::to_string(cursors[0].visualLine);
	locString += ", CSL: " + std::to_string(cursors[0].subline);
	locString += ", CC: " + std::to_string(cursors[0].column);
	textScreen.RenderString(w-locString.size()-1,h-2,locString);

	locString = "";
	locString += "SL: " + std::to_string(viewLine.index);
	locString += ", SSL: " + std::to_string(screenSubline);
	textScreen.RenderString(w-locString.size()-1,h-1,locString);

	TUITextBox testBox{"Line Test\nNew Line\n\nLine",3,-5,20,3};
	testBox.SetTitle("open");
	testBox.Render(textScreen);

	return textScreen;
}

void LineModeBase::ProcessKeyboardEvent(KeyboardEvent* event){
	TextAction textAction;
	
	if (IsPrintable(event->key,event->mod)){
		textAction.action = Action::InsertChar;
		textAction.character = event->key;
	} else {
		textAction.action = FindActionFromKey((KeyEnum)event->key,(KeyModifier)event->mod);
		switch (textAction.action){
			case Action::MoveLeftChar:
			case Action::MoveRightChar:
			case Action::MoveUpLine:
			case Action::MoveDownLine:
			case Action::MoveScreenUpLine:
			case Action::MoveScreenDownLine:
				textAction.num = 1;
				break;

			case Action::MoveLeftMulti:
			case Action::MoveRightMulti:
			case Action::MoveUpMulti:
			case Action::MoveDownMulti:
				textAction.num = Config::multiAmount;
				break;

			default:
				break;

		}
	}


	ProcessTextAction(textAction);
}

void UpdateSublineUpwards(IndexedIterator& line,s32& subline,s32& column,s32 width,s32 num,bool constrain=false){
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

void UpdateSublineDownwards(IndexedIterator& line,s32& subline,s32& column,s32 width,s32 num,bool constrain=false,s32 lineCount=0){
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
	//s32 moveDist = (s32)(file->size()-1) - viewLine.index;
	//moveDist += GetXPosOfIndex(*viewLine,(*viewLine).size(),lineWidth)/lineWidth-screenSubline;
	//if (moveDist<=0) return;

	//num = std::min(moveDist,num);
	
	s32 dummy;
	UpdateSublineDownwards(viewLine,screenSubline,dummy,lineWidth,num,constrain,file->size());
	cursors[0].SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
}

void LineModeBase::MoveScreenUp(s32 num,bool constrain){
	s32 dummy;
	UpdateSublineUpwards(viewLine,screenSubline,dummy,lineWidth,num,constrain);
	cursors[0].SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
}

void LineModeBase::LockScreenToCursor(TextCursor& cursor){
	viewLine = cursor.line;
	screenSubline = cursor.subline;

	MoveScreenUp(innerHeight/2);
}

void LineModeBase::MoveScreenToCursor(TextCursor& cursor){
	if (Config::cursorLock){
		LineModeBase::LockScreenToCursor(cursor);
		return;
	}

	cursor.SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
	if (cursor.visualLine > Config::cursorMoveHeight && 
			cursor.visualLine < innerHeight-1-Config::cursorMoveHeight) return;


	s32 lineDiff = cursor.line.index-viewLine.index;
	viewLine = cursor.line;
	screenSubline = cursor.subline;

	if (lineDiff>innerHeight/2)
		MoveScreenUp(innerHeight-1-Config::cursorMoveHeight,true);
	else
		MoveScreenUp(Config::cursorMoveHeight,true);
	

	cursor.SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
}

void LineModeBase::MoveCursorDown(TextCursor& cursor,s32 num){
	s32 oldCursorX = GetXPosOfIndex(*cursor.line,cursor.column,lineWidth)%lineWidth;

	UpdateSublineDownwards(cursor.line,cursor.subline,cursor.column,lineWidth,num,true,file->size());

	s32 newCursorX = GetIndexOfXPos(*cursor.line,oldCursorX+cursor.subline*lineWidth,lineWidth);
	SetCursorColumn(cursor,std::min(newCursorX,cursor.CurrentLineLen()));
}

void LineModeBase::MoveCursorUp(TextCursor& cursor,s32 num){
	s32 oldCursorX = GetXPosOfIndex(*cursor.line,cursor.column,lineWidth)%lineWidth;

	UpdateSublineUpwards(cursor.line,cursor.subline,cursor.column,lineWidth,num,true);

	s32 newCursorX = GetIndexOfXPos(*cursor.line,oldCursorX+cursor.subline*lineWidth,lineWidth);
	SetCursorColumn(cursor,std::min(newCursorX,cursor.CurrentLineLen()));
}

void LineModeBase::SetCursorColumn(TextCursor& cursor,s32 col){
	cursor.column = col;
	cursor.subline = GetXPosOfIndex(*cursor.line,col,lineWidth)/lineWidth;

	MoveScreenToCursor(cursor);
}

void TextCursor::SetVisualLineFromLine(IndexedIterator viewLine,s32 screenSubline,s32 w,s32 h){
	s32 count = 0;

	if (line.index<viewLine.index){
		visualLine = -100;
		return;
	}

	if (line.index==viewLine.index&&subline<screenSubline){
		visualLine = -100;
		return;
	}

	if (line.index-viewLine.index > h-1){
		visualLine = -100;
		return;
	}

	s32 lineSize;

	while (viewLine.index!=line.index||screenSubline!=subline){
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

