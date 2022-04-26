#include "linemode.h"

s32 tenPow(s32 i){
	s32 p = 1;
	while (i--){
		p *= 10;
	}

	return p;
}

TextScreen LineModeBase::GetTextScreen(s32 w,s32 h){
	TextScreen textScreen;

	s32 lineNumberWidth = 4;

	screenWidth = w;
	screenHeight = h;
	lineWidth = screenWidth-lineNumberWidth;

	textScreen.SetSize(w,h);
	
	std::fill(textScreen.begin(),textScreen.end(),TextCell(' ',defaultStyle));

	auto it = viewLine;
	s32 lineLen,lineStart;
	s32 lineNumber;
	char c;


	for (s32 y=0;y<h;y++){
		if (it.it==file->end()){
			textScreen[y*w] = TextCell('~',blankLineStyle);
			continue;
		}

		lineStart = 0;
		lineLen = it.it->size();
		s32 i = 0;

		if (Config::displayLineNumbers){ //line numbers
			lineNumber = it.index+1;
			s32 x;
			s32 powerOfTen;
			for (s32 i=0;i<lineNumberWidth-1;i++){
				x = lineNumberWidth-2-i;
				powerOfTen = tenPow(i);
				if (powerOfTen>lineNumber) break;

				textScreen[y*w+x] = TextCell('0'+(lineNumber/tenPow(i))%10, lineNumberStyle);
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
			}
		}

		++it;
	}

	s32 loc;
	for (auto cursor : cursors){
		loc = cursor.visualLine*w + GetXPosOfIndex(*cursor.line.it,cursor.column,lineWidth)%lineWidth + lineStart;
		//textScreen[loc].fg = 0;
		//textScreen[loc].bg = 7;
		textScreen[loc].style = cursorStyle;
	}

	std::string locString = std::to_string(cursors[0].line.index)+" "+std::to_string(cursors[0].visualLine)+", "+std::to_string(cursors[0].column) + " " + std::to_string(cursors[0].subline);
	textScreen.RenderString(1,h-1,locString);

	return textScreen;
}

void LineModeBase::ProcessKeyboardEvent(KeyboardEvent* event){
	TextAction textAction;
	
	if (IsPrintable(event->key,event->mod)){
		textAction.action = Action::InsertChar;
		textAction.character = event->key;
	} else {
		textAction.action = FindActionFromKey((KeyEnum)event->key,(KeyModifier)event->mod);
		textAction.num = 1;
	}


	ProcessTextAction(textAction);
}


void LineModeBase::MoveCursorDown(TextCursor& cursor,s32 num){
	if (cursor.line.index==(s32)(file->size()-1) &&
			cursor.subline==cursor.CurrentLineLen()/lineWidth) return;

	cursor.visualLine += num;
	cursor.SetLineFromVisualLine(viewLine,screenSubline,lineWidth);
}

void LineModeBase::MoveCursorUp(TextCursor& cursor,s32 num){
	if (cursor.line.index==0 &&
			cursor.subline==0) return;

	cursor.visualLine -= num;
	cursor.SetLineFromVisualLine(viewLine,screenSubline,lineWidth);
}

void LineModeBase::SetCursorColumn(TextCursor& cursor,s32 col){
	s32 oldSubline = cursor.subline;

	cursor.column = col;
	cursor.subline = GetXPosOfIndex(*cursor.line.it,col,lineWidth)/lineWidth;

	cursor.visualLine += cursor.subline-oldSubline;
}

// sets the cursor's line from a screen position (visualLine)
void TextCursor::SetLineFromVisualLine(IndexedIterator viewLine,s32 screenSubline,s32 w){
	s32 count = visualLine;
	s32 lineSize;

	// reset line to top of screen and move down
	line = viewLine;
	subline = screenSubline;
	column %= w;

	while (--count>=0){
		lineSize = GetXPosOfIndex(*line.it,line.it->size(),w);
		if (lineSize>=(subline+1)*w){
			column += w;
			++subline;
		} else {
			column %= w;
			subline = 0;
			++line;
		}

	}

	
}
