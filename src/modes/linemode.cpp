#include "linemode.h"

#include "../context.h"

#ifdef _WIN32
inline s32 wcwidth(u32 c){
	if (c>127)
		return 2;
	if (c<32)
		return 2;
	return 1;
}
#endif

s32 tenPow(s32 i){
	s32 p = 1;
	while (i--)
		p *= 10;

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

s32 TrueLineDistance(LineIndexedIterator,s32,LineIndexedIterator,s32,s32);

LineModeBase::LineModeBase(ContextEditor* ctx) : 
		ModeBase(ctx),
		screenSubline(0),
		currentAction(BufferActionType::TextInsertion,0,0)
{		
	textBuffer = MakeRef<TextBuffer>();
	textBuffer->InsertLine(textBuffer->begin(),"");
	colorBuffer = {};
	colorBuffer.assign(textBuffer->size(),{});
	altColorBuffer = {};
	highlighterNeedsUpdate = true;
	highlightTask = -1ULL;
	
	readonly = false;
	modified = false;
	InitIterators();
	SetColorLine();
	bufferPath = {};
	undoStack = {};
	selecting = false;
	finding = false;

	showDebugInfo = false;

	screenWidth = -1;
	screenHeight = -1;
}

void LineModeBase::UpdateHighlighterTask(){
	syntaxHighlighter->FillColorBuffer(altColorBuffer);
	
	std::scoped_lock lock{colorMutex};
	colorBuffer = altColorBuffer;
	SetColorLine();
}

void LineModeBase::UpdateHighlighter(){
	if (!syntaxHighlighter)
		return;

	std::scoped_lock lock{colorMutex};
	
	AsyncContext a = {};
	a.func = std::bind(&LineModeBase::UpdateHighlighterTask,this);
	a.preDelay = 0.0f;
	a.updateAfter = true;
	if (!ctx->IsAsyncTaskDone(highlightTask)){
		ctx->CancelAsyncTask(highlightTask);
		a.preDelay = 0.03f;
	}
	highlightTask = ctx->StartAsyncTask(a);
	highlighterNeedsUpdate = false;
}

TextStyle LineModeBase::GetTextStyleAt(ColorIterator it,s32 index){
	if (!syntaxHighlighter||!it->size())
		return textStyle;

	for (const ColorData& cd : *it){
		if (cd.start>index) break;

		if (cd.start<=index&&cd.start+cd.size>index)
			return cd.style;
	}

	return textStyle;
}

inline void HandleUTF8(LineIndexedIterator it,u32& c,s32 i){
	if (c>>5==6){
		if (it.it->size()-i < 2){
			c = '?';
			return;
		}
		
		c = (c&0x1F)<<6;
		c |= (*it.it)[i+1]&0x3F;
	} else if (c>>4==14){
		if (it.it->size()-i < 3){
			c = '?';
			return;
		}
		
		c = (c&0x0F)<<12;
		c |= ((*it.it)[i+1]&0x3F)<<6;
		c |= (*it.it)[i+2]&0x3F;
	} else if (c>>3==30){
		if (it.it->size()-i < 4){
			c = '?';
			return;
		}
		
		c = (c&0x07)<<18;
		c |= ((*it.it)[i+1]&0x3F)<<12;
		c |= ((*it.it)[i+2]&0x3F)<<6;
		c |= (*it.it)[i+3]&0x3F;
	}
	if (wcwidth(c)!=1) c = '?';
}

TextScreen& LineModeBase::GetTextScreen(s32 w,s32 h){
	if (w!=screenWidth||h!=screenHeight){
		screenWidth = w;
		screenHeight = h;
		innerHeight = screenHeight - 2;
	
		textScreen.SetSize(w,h);

		CalculateScreenData();
		SetCachedX(cursors.front());
	}
	
	if (highlighterNeedsUpdate)
		UpdateHighlighter();
		
	std::scoped_lock lock{colorMutex};
	
	// fix color buffer line count
	size_t cursorColorInsert=cursors.front().cursor.line.index;
	if (cursorColorInsert)
		--cursorColorInsert;
		
	while (colorBuffer.size() < textBuffer->size()){
		colorBuffer.emplace(colorBuffer.cbegin()+cursorColorInsert);
	}
	
	cursorColorInsert = std::min(cursorColorInsert+1,textBuffer->size()-1);
	
	while (colorBuffer.size() > textBuffer->size()){
		colorBuffer.erase(colorBuffer.cbegin()+cursorColorInsert);
	}
	
	SetColorLine();

	SetVisualCursorColumn(cursors.front(),cursors.front().cursor.column);
	MoveScreenToVisualCursor(cursors.front());

	std::fill(textScreen.begin(),textScreen.end(),TextCell(' ',textStyle));

	auto it = viewLine;
	auto colorLineIt = colorLine;

	s32 lineLen,lineStart = 0;
	s32 lineNumber;
	u32 c;
	Cursor startSelect = GetSelectStartPos();
	Cursor endSelect = GetSelectEndPos();
	bool inSelection = selecting && viewLine.index > startSelect.line.index; //TODO: handle sublines here
	
	s32 findCount = 0;
	s32 findIndex = 0;
	bool cursorFind = findNum==0;
	
	FoundList::iterator currentFind = matches.begin();
	if (matches.size()){
		while (currentFind!=matches.end()&&currentFind->line.index<viewLine.index){
			++findIndex;
			++currentFind;
		}
	}

	s32 oldX,xDiff;
	s32 i = 0;

	if (it.index>=0&&it.it!=textBuffer->end())
		i = GetIndexOfXPos(*it.it,screenSubline*lineWidth,lineWidth);

	for (s32 y=1;y<innerHeight+1;y++){
		if (it.index<0||it.it==textBuffer->end()){
			if (gConfig.displayLineNumbers){
				for (s32 n=0;n<lineNumberWidth;++n){
					textScreen[y*w+n] = TextCell(' ',lineNumberStyle);
				}
	
				textScreen[y*w+lineNumberWidth] = TextCell('~',emptyLineStyle);
			} else {
				textScreen[y*w] = TextCell('~',emptyLineStyle);
			}
			if (it.index<0){
				++it;
			}
			continue;
		}

		lineStart = 0;
		lineLen = it.it->size();

		if (gConfig.displayLineNumbers){ //line numbers
			for (s32 n=0;n<lineNumberWidth;++n){ //fill in style
				textScreen[y*w+n] = TextCell(' ',lineNumberStyle);
			}

			if (i==0){ //only print line numbers if this is the start of a line
				lineNumber = it.index+1;
				s32 x;
				s32 powerOfTen;
				for (s32 i=0;i<lineNumberWidth-2;i++){
					x = lineNumberWidth-2-i;
					powerOfTen = tenPow(i);
					if (powerOfTen>lineNumber) break;
	
					textScreen[y*w+x] = TextCell('0'+(lineNumber/powerOfTen)%10, lineNumberStyle);
				}
			}
			lineStart += lineNumberWidth;
		}

		for (s32 x=0;x<w;){
			if (selecting){
				if (it.index==startSelect.line.index
						&&i==startSelect.column) inSelection = true;
				if (it.index>endSelect.line.index||
						(it.index==endSelect.line.index&&
						 i>endSelect.column)) inSelection = false;
			}
			
			if (findCount){
				--findCount;
				if (!findCount)
					cursorFind = false;
			}
			
			if (matches.size()!=0&&currentFind!=matches.end()){
				while (currentFind!=matches.end()&&currentFind->line.index<it.index){
					++findIndex;
					++currentFind;
				}
				while (currentFind!=matches.end()&&currentFind->line.index==it.index&&
						currentFind->column<i){
					++findIndex;
					++currentFind;
				}
				
				if (findIndex==findNum&&!findCount) cursorFind = true;

				if (it.index==currentFind->line.index&&i==currentFind->column)
					findCount = findText.size();	
			}

			if (i>=lineLen) c = ' ';
			else 			c = (*it.it)[i]&255;
			
			if (c=='\t') c = ' ';
			else if (c<32) c = '?';
			else if (c&128) HandleUTF8(it,c,i);

			TextStyle usedStyle = GetTextStyleAt(colorLineIt,i);
			if (inSelection) std::swap(usedStyle.bg,usedStyle.fg);
			if (findCount){
				if (cursorFind) usedStyle = highlightSelectStyle;
				else usedStyle = highlightStyle;
			}
			textScreen[y*w+x+lineStart] = TextCell(c,usedStyle);

			if (i>=lineLen) break;

			oldX = x;
			UpdateXI(*it.it,x,i,lineWidth);
			xDiff = x-oldX;

			if (inSelection){
				while (--xDiff>0) //draw selection over tabs
					std::swap(textScreen[y*w+x-xDiff+lineStart].style.fg,
							textScreen[y*w+x-xDiff+lineStart].style.bg);
			}

			if (x >= w-lineStart){
				y++;
				x = 0;
				if (y>=innerHeight) break;

				if (gConfig.displayLineNumbers){
					for (s32 n=0;n<lineNumberWidth;++n){
						textScreen[y*w+n] = TextCell(' ',lineNumberStyle);
					}
				}
			}
		}
		i = 0;
		++it;
		++colorLineIt;
	}

	s32 loc;
	for (const auto& cursor : cursors){
		loc = (cursor.visualLine+1)*w + GetXPosOfIndex(*cursor.cursor.line.it,cursor.cursor.column,lineWidth)%lineWidth + lineStart;
		if (loc>=0&&loc<screenWidth*innerHeight)
			textScreen[loc].style = cursorStyle;
	}
	
	if (showDebugInfo){
		std::string locString = "";

		locString += "CL: " + std::to_string(cursors[0].cursor.line.index);
		locString += ", CVL: " + std::to_string(cursors[0].visualLine);
		locString += ", CSL: " + std::to_string(cursors[0].subline);
		locString += ", CC: " + std::to_string(cursors[0].cursor.column);
		locString += ", CX: " + std::to_string(GetXPosOfIndex(*cursors[0].cursor.line,cursors[0].cursor.column,lineWidth));
		textScreen.RenderString(w-locString.size()-1,h-3,locString);
	
		s32 lineDiff = TrueLineDistance(cursors[0].cursor.line,cursors[0].subline,viewLine,screenSubline,lineWidth);

		locString = "";
		locString += "DL: " + std::to_string(lineDiff);
		locString += ", SL: " + std::to_string(viewLine.index);
		locString += ", SSL: " + std::to_string(screenSubline);
		locString += ", UH: " + std::to_string(undoStack.UndoHeight());
		locString += ", RH: " + std::to_string(undoStack.RedoHeight());
		locString += ", FC: " + std::to_string(matches.size());
		textScreen.RenderString(w-locString.size()-1,h-2,locString);
	}

	return textScreen;
}

std::string_view LineModeBase::GetStatusBarText(){
	cursorPosText = {};
	cursorPosText += ' ';

	if (finding){
		cursorPosText += std::to_string(findNum+1);
		cursorPosText += '/';
		cursorPosText += std::to_string(matches.size());
	} else {
		cursorPosText += std::to_string(cursors[0].cursor.line.index+1);
		cursorPosText += ", ";
		cursorPosText += std::to_string(cursors[0].cursor.column+1);
	}
	
	return cursorPosText;
}

std::string RemoveEscapes(std::string_view token){
	std::string s = {};
	
	for (size_t i=0;i<token.size();++i){
		if (token[i]=='\\'&&i!=token.size()-1){
			++i;
			s += token[i];
			continue;
		}
		
		s += token[i];
	}
	
	return s;
}

bool LineModeBase::ProcessCommand(const TokenVector& tokens){
	if (tokens[0].token=="goto"){
		if (tokens.size()<2) return true;
		s32 l = strtol(tokens[1].token.data(),NULL,10);
		s32 c = 0;
		if (tokens.size()>=3) c = strtol(tokens[2].token.data(),NULL,10);
		l = std::min(std::max(l-1,0),(s32)textBuffer->size()-1);
		c = std::max(c-1,0);
		cursors.front().cursor = MakeCursor(l,c);
		return true;
	} else if (tokens[0].token=="find"){
		if (tokens.size()<2){
			modeErrorMessage = "Expected 2 arguments, got ";
			modeErrorMessage += std::to_string(tokens.size());
			return true;
		}
		if (tokens[1].token.empty()){
			modeErrorMessage = "'' is not a valid search string!";
			return true;
		}
		
		std::string fixedToken = RemoveEscapes(tokens[1].token);
		
		FindTextInBuffer(fixedToken);
		if (!matches.size()){
			modeErrorMessage = "No matches for '"+findText+"'";
		} else {
			finding = true;
			CursorToNextMatch();
		}
		return true;
	} else if (tokens[0].token=="replace"){
		if (tokens.size()<2){
			modeErrorMessage = "Expected at least 2 arguments, got ";
			modeErrorMessage += std::to_string(tokens.size());
			return true;
		}
		if (tokens[1].token.empty()){
			modeErrorMessage = "'' is not a valid search string!";
			return true;
		}
		std::string find = RemoveEscapes(tokens[1].token);
		
		std::string replace;
		if (tokens.size()<3)
			replace = {};
		else
			replace = RemoveEscapes(tokens[2].token);
		
		
		LineDiffInfo diffs = {};
		
		size_t replaceCount = ReplaceAll(*textBuffer,find,replace,diffs);
		if (replaceCount==0){
			modeErrorMessage = "No matches for '";
			modeErrorMessage += find;
			modeErrorMessage += "'";
		} else {
			PushLineReplacementAction(std::move(diffs));
			modeInfoMessage = "Replaced ";
			modeInfoMessage += std::to_string(replaceCount);
			modeInfoMessage += " occurences of '";
			modeInfoMessage += find;
			modeInfoMessage += "'";
			SetModified();
			highlighterNeedsUpdate = true;
		}
		
		
		return true;
	}
	return false;
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

	colorBuffer = {};
	colorBuffer.assign(textBuffer->size(),{});

	GetSyntaxHighlighter(bufferPath);
	highlighterNeedsUpdate = true;
	InitIterators();

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
	modeInfoMessage = "Saved successfully.";

	return true;
}

bool LineModeBase::HasPath(){
	return !bufferPath.empty();
}

std::string_view LineModeBase::GetPath(const OSInterface&){
	return bufferPath;
}

void LineModeBase::SetPath(const OSInterface& os,std::string_view path){
	bufferPath.clear();
	bufferPath += path;
	readonly = !os.FileIsWritable(bufferPath);
	GetSyntaxHighlighter(bufferPath);
	SetModified();
}

void LineModeBase::GetSyntaxHighlighter(std::string_view path){
	size_t start = path.find_last_of('.');
	if (start==std::string_view::npos||start==path.size()-1){
		syntaxHighlighter = nullptr;
		return;
	}

	std::string_view extension = path.substr(start+1);

	syntaxHighlighter = Handle<SyntaxHighlighter>(
		GetSyntaxHighlighterFromExtension(*textBuffer,extension)
	);
}

void LineModeBase::InitIterators(){
	viewLine = {textBuffer->begin()};
	colorLine = colorBuffer.begin();
	cursors.clear();
	cursors.emplace_back(textBuffer->begin());
	selectAnchor = cursors[0].cursor;
	selectCursor = cursors[0].cursor;
}

inline void LineModeBase::SetColorLine(){
	if (viewLine.index<0) colorLine = colorBuffer.begin();
	else colorLine = colorBuffer.begin()+viewLine.index;
}

void LineModeBase::CalculateScreenData(){
	lineNumberWidth = std::max(numWidth(std::max(viewLine.index+screenHeight,0))+2,5);
	lineWidth = screenWidth;
	if (gConfig.displayLineNumbers)
		lineWidth -= lineNumberWidth;

	SetColorLine();
}

Cursor LineModeBase::MakeCursor(s32 line,s32 column){
	LineIndexedIterator from{textBuffer->begin()};
	return MakeCursorFromLineIndexedIterator(line,column,from);
}

Cursor LineModeBase::MakeCursorFromLineIndexedIterator(s32 line,s32 column,LineIndexedIterator it){
	while (it.index>line)
		--it;

	while (it.index<line)
		++it;

	column = std::min(column,(s32)it.it->size());
	return {it,column};
}

void LineModeBase::SetModified(){
	modified = true;
	highlighterNeedsUpdate = true;
	matches.clear();
}

bool LineModeBase::Modified(){
	if (textBuffer->size()==1&&textBuffer->begin()->empty()&&bufferPath.empty())
		return false;
	
	if (undoStack.UndoHeight()==0&&currentAction.Empty())
		return false;
	
	return modified;
}

void LineModeBase::UpdateStyle(){
	highlighterNeedsUpdate = true;
}

void LineModeBase::FindTextInBuffer(std::string_view text){
	findText = text;
	FindAllMatches(*textBuffer,matches,text);
}

void LineModeBase::CursorToNextMatch(){
	auto line = cursors.front().cursor.line.index;
	auto col = cursors.front().cursor.column;
	Cursor found = matches.front();
	findNum = 0;
	s32 findCount = 0;
	for (auto pos : matches){
		if (pos.line.index>line){
			found = pos;
			findNum = findCount;
			break;
		}
		
		if (pos.line.index==line&&pos.column>col){
			found = pos;
			findNum = findCount;
			break;
		}
		++findCount;
	}
	
	cursors.front().cursor = found;
	SetVisualCursorColumn(cursors.front(),found.column);
	SetCachedX(cursors.front());
	LockScreenToVisualCursor(cursors.front(),true);
}

void LineModeBase::CursorToPreviousMatch(){
	auto line = cursors.front().cursor.line.index;
	auto col = cursors.front().cursor.column;
	Cursor found = matches.back();
	s32 findCount = matches.size()-1;
	for (auto pos : matches){
		if (pos.line.index>line)
			break;
		if (pos.line.index==line&&pos.column>=col)
			break;
		
		found = pos;
		++findCount;
	}
	
	findNum = findCount%matches.size();
	cursors.front().cursor = found;
	SetVisualCursorColumn(cursors.front(),found.column);
	SetCachedX(cursors.front());
	LockScreenToVisualCursor(cursors.front(),true);
}
