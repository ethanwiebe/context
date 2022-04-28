#include "editmode.h"

EditMode::EditMode(){
	file = FileManager::ReadFile("test.txt");
	viewLine = {file->begin()};
	screenSubline = 0;
	cursors.emplace_back(file->begin());

	SetKeybinds();
}

void EditMode::ProcessTextAction(TextAction a){
	auto& cursor = cursors[0];
	s32 multiAmount;
	switch (a.action){
		case Action::MoveScreenUpLine:
			MoveScreenUp(1);
			break;
		case Action::MoveScreenDownLine:
			MoveScreenDown(1);
			break;
		case Action::MoveUpLine:
			MoveCursorUp(cursor,a.num);
			break;
		case Action::MoveDownLine:
			MoveCursorDown(cursor,a.num);
			break;
		case Action::MoveLeftChar:
			if (cursor.column>0){
				SetCursorColumn(cursor,cursor.column-1);
			}
			break;
		case Action::MoveRightChar:
			if (cursor.column<cursor.CurrentLineLen()){
				SetCursorColumn(cursor,cursor.column+1);
			}
			break;
		case Action::MoveLeftMulti:
			multiAmount = std::min(a.num,cursor.column);
			SetCursorColumn(cursor,cursor.column-multiAmount);
			break;
		case Action::MoveRightMulti:
			multiAmount = std::min(a.num,cursor.CurrentLineLen()-cursor.column);
			SetCursorColumn(cursor,cursor.column+multiAmount);
			break;
		case Action::MoveUpMulti:
			MoveCursorUp(cursor,a.num);
			break;
		case Action::MoveDownMulti:
			MoveCursorDown(cursor,a.num);
			break;
		case Action::InsertLine: {
			std::string cut = cursor.line.it->substr(cursor.column);
			cursor.line.it->erase(cursor.column);
			file->InsertLineAfter(cursor.line.it,cut);
			MoveCursorDown(cursor,1);
			SetCursorColumn(cursor,0);
			break;
		}
		case Action::DeletePreviousChar:
			if (cursor.column==0&&cursor.line.it==file->begin()){ //at start of file
				break;
			} else if (cursor.column==0){
				MoveCursorUp(cursor,1);
				auto cachedLen = cursor.CurrentLineLen();
				auto itcopy = cursor.line.it;
				file->BackDeleteLine(++itcopy);
				SetCursorColumn(cursor,cachedLen);
			} else {
				SetCursorColumn(cursor,cursor.column-1);
				cursor.line.it->erase(cursor.column,1);
			}
			break;
		case Action::DeleteCurrentChar:
			if (cursor.column==cursor.CurrentLineLen()&&cursor.line.it==--file->end()){
				break;
			} else if (cursor.column==cursor.CurrentLineLen()){
				file->ForwardDeleteLine(cursor.line.it);
			} else {
				cursor.line.it->erase(cursor.column,1);
			}
			break;
		case Action::MoveToLineStart:
			SetCursorColumn(cursor,0);
			break;
		case Action::MoveToLineEnd:
			SetCursorColumn(cursor,cursor.CurrentLineLen());
			break;
		case Action::MoveToBufferStart:
			cursor.line = {file->begin()};
			SetCursorColumn(cursor,0);
			break;
		case Action::MoveToBufferEnd:
			cursor.line = {--file->end(),(s32)file->size()-1};
			SetCursorColumn(cursor,cursor.CurrentLineLen());
			break;
		case Action::InsertChar:
			cursor.line.it->insert(cursor.column,1,a.character);
			SetCursorColumn(cursor,cursor.column+1);
			break;
		case Action::InsertTab:
			cursor.line.it->insert(cursor.column,1,'\t');
			SetCursorColumn(cursor,cursor.column+1);
			break;
	}
}

