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

LineModeBase::LineModeBase(ContextEditor* ctx) : ModeBase(ctx),screenSubline(0),currentAction(BufferActionType::TextInsertion,0,0) {
	textBuffer = MakeRef<TextBuffer>();
	textBuffer->InsertLine(textBuffer->begin(),"");
	readonly = false;
	modified = false;
	InitIterators();
	bufferPath = {};
	undoStack = {};
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

	if (it.index>=0&&it.it!=textBuffer->end())
		i = GetIndexOfXPos(*it.it,screenSubline*lineWidth,lineWidth);

	for (s32 y=0;y<innerHeight;y++){
		if (it.index<0){
			textScreen[y*w] = TextCell('~',blankLineStyle);
			++it;
			continue;
		}
		if (it.it==textBuffer->end()){
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

			TextStyle usedStyle = defaultStyle;
#ifndef NDEBUG
			if (it.index==currentAction.line&&i==currentAction.column) usedStyle = yellowHighlightStyle;
			if (it.index==currentAction.extendLine&&i==currentAction.extendColumn) usedStyle = greenHighlightStyle;
#endif
			textScreen[y*w+x+lineStart] = TextCell(c,usedStyle);

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
		loc = cursor.visualLine*w + GetXPosOfIndex(*cursor.cursor.line.it,cursor.cursor.column,lineWidth)%lineWidth + lineStart;
		if (loc>=0&&loc<screenWidth*innerHeight)
			textScreen[loc].style = cursorStyle;
	}
	
	std::string locString = "";

	locString += "CL: " + std::to_string(cursors[0].cursor.line.index);
	locString += ", CVL: " + std::to_string(cursors[0].visualLine);
	locString += ", CSL: " + std::to_string(cursors[0].subline);
	locString += ", CC: " + std::to_string(cursors[0].cursor.column);
	textScreen.RenderString(w-locString.size()-1,h-3,locString);

	locString = "";
	locString += "SL: " + std::to_string(viewLine.index);
	locString += ", SSL: " + std::to_string(screenSubline);
	locString += ", UH: " + std::to_string(undoStack.UndoHeight());
	locString += ", RH: " + std::to_string(undoStack.RedoHeight());
	textScreen.RenderString(w-locString.size()-1,h-2,locString);

	/*TUITextBox testBox{"Line Test\nNew Line\n\nLine",3,-5,20,3};
	testBox.SetTitle("open");
	testBox.Render(textScreen);*/

	return textScreen;
}

std::string_view LineModeBase::GetBufferName(){
	if (bufferPath.empty()) return "unnamed";
	size_t index = bufferPath.find_last_of('/')+1;
	std::string_view result = bufferPath;
	result.remove_prefix(index);

	return result;
}

bool LineModeBase::OpenAction(const OSInterface& os, std::string_view path){
	if (!os.ReadFileIntoTextBuffer(path,textBuffer))
		return false;

	readonly = !os.FileIsWritable(path);
	modified = false;
	bufferPath.clear();
	bufferPath += path;
	if (textBuffer->empty()){
		textBuffer->InsertLine(textBuffer->end(),{});
	}
	InitIterators();

	logger << "bufferPath: " << bufferPath << "\n";

	return true;
}

bool LineModeBase::SaveAction(const OSInterface& os){
	readonly = !os.FileIsWritable(bufferPath);
	if (readonly)
		return false;

	if (!os.WriteTextBufferIntoFile(bufferPath,textBuffer)){
		return false;
	}

	modified = false;

	return true;
}

bool LineModeBase::HasSavePath(){
	return !bufferPath.empty();
}

void LineModeBase::SetPath(const OSInterface& os,std::string_view path){
	bufferPath.clear();
	bufferPath += path;
	modified = true;
	readonly = !os.FileIsWritable(bufferPath);
}

void LineModeBase::InitIterators(){
	viewLine = {textBuffer->begin()};
	cursors.clear();
	cursors.emplace_back(textBuffer->begin());
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
	s32 dummy;
	UpdateSublineDownwards(viewLine,screenSubline,dummy,lineWidth,num,constrain,textBuffer->size());
	cursors[0].SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
}

void LineModeBase::MoveScreenUp(s32 num,bool constrain){
	s32 dummy;
	UpdateSublineUpwards(viewLine,screenSubline,dummy,lineWidth,num,constrain);
	cursors[0].SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
}

void LineModeBase::LockScreenToVisualCursor(VisualCursor& cursor){
	viewLine = cursor.cursor.line;
	screenSubline = cursor.subline;

	MoveScreenUp(innerHeight/2);
}

void LineModeBase::MoveScreenToVisualCursor(VisualCursor& cursor){
	if (Config::cursorLock){
		LineModeBase::LockScreenToVisualCursor(cursor);
		return;
	}

	cursor.SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
	if (cursor.visualLine > Config::cursorMoveHeight && 
			cursor.visualLine < innerHeight-1-Config::cursorMoveHeight) return;


	s32 lineDiff = cursor.cursor.line.index-viewLine.index;
	viewLine = cursor.cursor.line;
	screenSubline = cursor.subline;

	if (lineDiff>innerHeight/2)
		MoveScreenUp(innerHeight-1-Config::cursorMoveHeight,true);
	else
		MoveScreenUp(Config::cursorMoveHeight,true);
	

	cursor.SetVisualLineFromLine(viewLine,screenSubline,lineWidth,innerHeight);
}

void LineModeBase::MoveVisualCursorDown(VisualCursor& cursor,s32 num){
	s32 oldCursorX = GetXPosOfIndex(*cursor.cursor.line,cursor.cursor.column,lineWidth)%lineWidth;

	UpdateSublineDownwards(cursor.cursor.line,cursor.subline,cursor.cursor.column,lineWidth,num,true,textBuffer->size());

	s32 newCursorX = GetIndexOfXPos(*cursor.cursor.line,cursor.cachedX+cursor.subline*lineWidth,lineWidth);
	SetVisualCursorColumn(cursor,std::min(newCursorX,cursor.CurrentLineLen()));
}

void LineModeBase::MoveVisualCursorUp(VisualCursor& cursor,s32 num){
	s32 oldCursorX = GetXPosOfIndex(*cursor.cursor.line,cursor.cursor.column,lineWidth)%lineWidth;

	UpdateSublineUpwards(cursor.cursor.line,cursor.subline,cursor.cursor.column,lineWidth,num,true);

	s32 newCursorX = GetIndexOfXPos(*cursor.cursor.line,cursor.cachedX+cursor.subline*lineWidth,lineWidth);
	SetVisualCursorColumn(cursor,std::min(newCursorX,cursor.CurrentLineLen()));
}

void LineModeBase::MoveVisualCursorLeft(VisualCursor& cursor,s32 num){
	s32 newCol = cursor.cursor.column-num;
	
	while (newCol<0){ //move cursor up lines until only horiz adjust is left
		if (cursor.cursor.line.index==0){
			SetVisualCursorColumn(cursor,0);
			return;
		}

		MoveVisualCursorUp(cursor,1);
		newCol += cursor.CurrentLineLen()+1;
	}

	SetVisualCursorColumn(cursor,newCol);
	cursor.cachedX = GetXPosOfIndex(*cursor.cursor.line,cursor.cursor.column,lineWidth)%lineWidth;
}

void LineModeBase::MoveVisualCursorRight(VisualCursor& cursor,s32 num){
	s32 newCol = cursor.cursor.column+num;

	while (newCol>cursor.CurrentLineLen()){
		if (cursor.cursor.line.index==(s32)textBuffer->size()-1){
			SetVisualCursorColumn(cursor,cursor.CurrentLineLen());
			return;
		}

		newCol -= cursor.CurrentLineLen()+1;
		MoveVisualCursorDown(cursor,1);
	}

	SetVisualCursorColumn(cursor,newCol);
	cursor.cachedX = GetXPosOfIndex(*cursor.cursor.line,cursor.cursor.column,lineWidth)%lineWidth;
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

void LineModeBase::SetVisualCursorColumn(VisualCursor& cursor,s32 col){
	cursor.cursor.column = col;
	cursor.subline = GetXPosOfIndex(*cursor.cursor.line,col,lineWidth)/lineWidth;
	MoveScreenToVisualCursor(cursor);
}

void VisualCursor::SetVisualLineFromLine(IndexedIterator viewLine,s32 screenSubline,s32 w,s32 h){
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

Cursor LineModeBase::MakeCursor(s32 line,s32 column){
	IndexedIterator from{textBuffer->begin()};
	
	return MakeCursorFromIndexedIterator(line,column,from);
}

Cursor LineModeBase::MakeCursorFromIndexedIterator(s32 line,s32 column,IndexedIterator it){
	while (it.index>line){
		--it;
	}

	while (it.index<line){
		++it;
	}

	return {it,column};
}

void LineModeBase::DeleteLine(VisualCursor& cursor){
	SetVisualCursorColumn(cursor,0);
	DeleteCharCountAt(cursor.cursor,cursor.CurrentLineLen());
	if (cursor.cursor.line.index!=(s32)textBuffer->size()-1){
		DeleteCharAt(cursor.cursor);
	} else if (cursor.cursor.line.index!=0){
		MoveVisualCursorLeft(cursor,1);
		DeleteCharAt(cursor.cursor);
	}

}

void LineModeBase::DeleteCharAt(Cursor cursor,bool undoable){
	char deletedChar;
	modified = true;
	
	if (cursor.column==(s32)cursor.line.it->size()){
		deletedChar = '\n';
	} else {
		deletedChar = (*cursor.line.it)[cursor.column];
	}

	if (undoable)
		PushDeletionAction(cursor,deletedChar);

	if (deletedChar=='\n'){
		textBuffer->ForwardDeleteLine(cursor.line.it);
	} else {
		cursor.line.it->erase(cursor.column,1);
	}
}

void LineModeBase::DeleteCharCountAt(Cursor cursor,s32 count){
	while (--count>=0){
		DeleteCharAt(cursor);
	}
}

void LineModeBase::InsertCharAt(Cursor cursor,char c,bool undoable){
	modified = true;

	if (c=='\n'){
		textBuffer->InsertLineAfter(cursor.line.it,cursor.line.it->substr(cursor.column));
		cursor.line.it->erase(cursor.column);
	} else 
		cursor.line.it->insert(cursor.column,1,c);

	if (undoable)
		PushInsertionAction(cursor,c);
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

void LineModeBase::Undo(VisualCursor& cursor){
	if (!currentAction.Empty()){
		undoStack.PushAction(currentAction);
		MakeNewAction(BufferActionType::TextInsertion,0,0);	
	}
	
	if (!undoStack.CanUndo()) return;

	BufferAction undoAction = undoStack.PopUndo();
	if (undoAction.type==BufferActionType::TextInsertion){
		undoAction.type = BufferActionType::TextDeletion;
	} else if (undoAction.type==BufferActionType::TextDeletion){
		undoAction.type = BufferActionType::TextInsertion;
	}

	std::swap(undoAction.line,undoAction.extendLine);
	std::swap(undoAction.column,undoAction.extendColumn);

	PerformBufferAction(cursor,undoAction);
}

void LineModeBase::Redo(VisualCursor& cursor){
	if (!currentAction.Empty()){
		undoStack.PushAction(currentAction);
		MakeNewAction(BufferActionType::TextInsertion,0,0);	
	}

	if (!undoStack.CanRedo()) return;

	BufferAction redoAction = undoStack.PopRedo();
	
	PerformBufferAction(cursor,redoAction);
}

void LineModeBase::MakeNewAction(BufferActionType type,s32 line,s32 col){
	currentAction = {type,line,col};
}


void LineModeBase::PerformBufferAction(VisualCursor& cursor,const BufferAction& action){
	Cursor c;
	if (action.type==BufferActionType::TextInsertion){
		c = MakeCursorFromIndexedIterator(action.line,action.column,cursor.cursor.line);
		InsertStringAt(c,action.text,false);
		cursor.cursor = c;
		MoveVisualCursorRight(cursor,action.text.size());
	} else if (action.type==BufferActionType::TextDeletion){
		c = MakeCursorFromIndexedIterator(action.extendLine,action.extendColumn,cursor.cursor.line);
		cursor.cursor = c;
		for (s32 i=0;i<action.insertedLen;++i){
			DeleteCharAt(c,false);
		}
		SetVisualCursorColumn(cursor,cursor.cursor.column);
	}
}

void LineModeBase::PushInsertionAction(Cursor cursor,char c){
	if (currentAction.type!=BufferActionType::TextInsertion ||
			!InsertionExtendsAction(cursor)){
		if (!currentAction.Empty())
			undoStack.PushAction(currentAction);

		MakeNewAction(BufferActionType::TextInsertion,cursor.line.index,cursor.column);
	}

	currentAction.AddCharacter(c);
	MoveCursorRight(cursor,1);
	currentAction.extendLine = cursor.line.index;
	currentAction.extendColumn = cursor.column;
}

void LineModeBase::PushDeletionAction(Cursor cursor,char c){
	if (currentAction.type!=BufferActionType::TextDeletion ||
			!DeletionExtendsAction(cursor)){
		if (!currentAction.Empty())
			undoStack.PushAction(currentAction);

		MakeNewAction(BufferActionType::TextDeletion,cursor.line.index,cursor.column);
	}

	Cursor cursorcopy = cursor;
	MoveCursorRight(cursorcopy,1);
	if (cursorcopy.line.index==currentAction.extendLine&&cursorcopy.column==currentAction.extendColumn){
		currentAction.PrependCharacter(c);
		currentAction.extendLine = cursor.line.index;
		currentAction.extendColumn = cursor.column;
		currentAction.line = cursor.line.index;
		currentAction.column = cursor.column;
	} else {
		currentAction.AddCharacter(c);
	}
}

bool LineModeBase::InsertionExtendsAction(Cursor cursor) const {
	return currentAction.extendLine==cursor.line.index&&currentAction.extendColumn==cursor.column;
}

bool LineModeBase::DeletionExtendsAction(Cursor cursor) const {
	bool cond1 = currentAction.line==cursor.line.index&&currentAction.column==cursor.column;
	MoveCursorRight(cursor,1);
	bool cond2 = currentAction.extendLine==cursor.line.index&&currentAction.extendColumn==cursor.column;
	return cond1||cond2;
}
