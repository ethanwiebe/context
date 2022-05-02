#include "editmode.h"

EditMode::EditMode(ContextEditor* ctx) : LineModeBase(ctx) {}

void EditMode::ProcessTextAction(TextAction a){
	auto& cursor = cursors[0];
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
			MoveCursorLeft(cursor,1);
			break;
		case Action::MoveRightChar:
			MoveCursorRight(cursor,1);
			break;
		case Action::MoveLeftMulti:
			MoveCursorLeft(cursor,a.num);
			break;
		case Action::MoveRightMulti:
			MoveCursorRight(cursor,a.num);
			break;
		case Action::MoveUpMulti:
			MoveCursorUp(cursor,a.num);
			break;
		case Action::MoveDownMulti:
			MoveCursorDown(cursor,a.num);
			break;
		case Action::InsertLine:
			InsertCharAt(cursor.line,cursor.column,'\n');
			MoveCursorRight(cursor,1);
			break;
		case Action::DeletePreviousChar:
			if (cursor.column==0&&cursor.line.it==textBuffer->begin()) //at start of textBuffer
				break;

			MoveCursorLeft(cursor,1);
			DeleteCharAt(cursor.line,cursor.column);
			break;
		case Action::DeleteCurrentChar:
			if (cursor.column==cursor.CurrentLineLen()&&cursor.line.it==--textBuffer->end())
				break;

			DeleteCharAt(cursor.line,cursor.column);
			break;
		case Action::MoveToLineStart:
			SetCursorColumn(cursor,0);
			break;
		case Action::MoveToLineEnd:
			SetCursorColumn(cursor,cursor.CurrentLineLen());
			break;
		case Action::MoveToBufferStart:
			cursor.line = {textBuffer->begin()};
			SetCursorColumn(cursor,0);
			break;
		case Action::MoveToBufferEnd:
			cursor.line = {--textBuffer->end(),(s32)textBuffer->size()-1};
			SetCursorColumn(cursor,cursor.CurrentLineLen());
			break;
		case Action::InsertChar:
			InsertCharAt(cursor.line,cursor.column,a.character);
			MoveCursorRight(cursor,1);
			break;
		case Action::InsertTab:
			InsertCharAt(cursor.line,cursor.column,'\t');
			MoveCursorRight(cursor,1);
			break;

		default:
			break;
	}
}

