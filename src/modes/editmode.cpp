#include "editmode.h"

EditMode::EditMode(ContextEditor* ctx) : LineModeBase(ctx) {}

void EditMode::ProcessTextAction(TextAction a){
	VisualCursor& cursor = cursors[0];
	switch (a.action){
		case Action::MoveScreenUpLine:
			MoveScreenUp(1);
			break;
		case Action::MoveScreenDownLine:
			MoveScreenDown(1);
			break;
		case Action::MoveUpLine:
			MoveVisualCursorUp(cursor,a.num);
			break;
		case Action::MoveDownLine:
			MoveVisualCursorDown(cursor,a.num);
			break;
		case Action::MoveLeftChar:
			MoveVisualCursorLeft(cursor,1);
			break;
		case Action::MoveRightChar:
			MoveVisualCursorRight(cursor,1);
			break;
		case Action::MoveLeftMulti:
			MoveVisualCursorLeft(cursor,a.num);
			break;
		case Action::MoveRightMulti:
			MoveVisualCursorRight(cursor,a.num);
			break;
		case Action::MoveUpMulti:
			MoveVisualCursorUp(cursor,a.num);
			break;
		case Action::MoveDownMulti:
			MoveVisualCursorDown(cursor,a.num);
			break;
		case Action::InsertLine:
			InsertCharAt(cursor.cursor,'\n');
			MoveVisualCursorRight(cursor,1);
			break;
		case Action::DeletePreviousChar:
			if (cursor.cursor.column==0&&cursor.cursor.line.index==0) //at start of textBuffer
				break;

			MoveVisualCursorLeft(cursor,1);
			DeleteCharAt(cursor.cursor);
			break;
		case Action::DeleteCurrentChar:
			if (cursor.cursor.column==cursor.CurrentLineLen()&&cursor.cursor.line.index==(s32)textBuffer->size()-1)
				break;

			DeleteCharAt(cursor.cursor);
			break;
		case Action::MoveToLineStart:
			SetVisualCursorColumn(cursor,0);
			break;
		case Action::MoveToLineEnd:
			SetVisualCursorColumn(cursor,cursor.CurrentLineLen());
			break;
		case Action::MoveToBufferStart:
			cursor.cursor.line = {textBuffer->begin()};
			SetVisualCursorColumn(cursor,0);
			break;
		case Action::MoveToBufferEnd:
			cursor.cursor.line = {--textBuffer->end(),(s32)textBuffer->size()-1};
			SetVisualCursorColumn(cursor,cursor.CurrentLineLen());
			break;
		case Action::InsertChar:
			InsertCharAt(cursor.cursor,a.character);
			MoveVisualCursorRight(cursor,1);
			break;
		case Action::InsertTab:
			InsertCharAt(cursor.cursor,'\t');
			MoveVisualCursorRight(cursor,1);
			break;
		case Action::DeleteLine:
			DeleteLine(cursor);
			break;
		case Action::UndoAction:
			Undo(cursor);
			break;
		case Action::RedoAction:
			Redo(cursor);
			break;

		default:
			break;
	}
}

