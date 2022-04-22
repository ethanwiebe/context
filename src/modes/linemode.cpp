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

	screenWidth = w;
	screenHeight = h;

	textScreen.SetSize(w,h);
	
	std::fill(textScreen.begin(),textScreen.end(),TextCell(' ',0,0));

	auto it = viewLine;
	s32 lineLen,lineStart;
	s32 lineNumber;
	char c;

	s32 lineNumberWidth = 4;

	for (s32 y=0;y<h;y++){
		if (it.it==file->end()){
			textScreen[y*w] = TextCell('~',0,0);
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

				textScreen[y*w+x] = TextCell('0'+(lineNumber/tenPow(i))%10, 0,0);
			}
			lineStart += lineNumberWidth;
		}

		for (s32 x=lineStart;x<w;){
			if (i>=lineLen) break;


			c = (*it.it)[i];
			if (c&128) c = '?'; //utf8
			if (c=='\t') c = ' ';

			textScreen[y*w+x] = TextCell(c,0,0);

			UpdateXI(*it.it,x,i);
			if (x >= w){
				y++;
				x = 0;
			}
		}

		++it;
	}

	s32 loc;
	for (auto cursor : cursors){
		loc = cursor.visualLine*w + GetXPosOfIndex(*cursor.line.it,cursor.column)%screenWidth + lineStart;
		textScreen[loc].fg = 1;
	}

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
	//cursor.line += num;
	cursor.visualLine += num;
	cursor.SetLineFromVisualLine(viewLine,screenWidth);
}

void LineModeBase::MoveCursorUp(TextCursor& cursor,s32 num){
	//cursor.line -= num;
	cursor.visualLine -= num;
	cursor.SetLineFromVisualLine(viewLine,screenWidth);
}

void LineModeBase::SetCursorColumn(TextCursor& cursor,s32 col){
	s32 oldSubline = cursor.subline;

	cursor.column = col;
	cursor.subline = col/screenWidth;

	cursor.visualLine += cursor.subline-oldSubline;
}

void TextCursor::SetLineFromVisualLine(IndexedIterator viewLine,s32 w){
	s32 count = visualLine;
	s32 lineSize;
	line = viewLine;
	while (count--){
		lineSize = line.it->size();
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
