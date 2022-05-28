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
		case Action::MoveUpPage:
			MoveVisualCursorUp(cursor,a.num);
			break;
		case Action::MoveDownPage:
			MoveVisualCursorDown(cursor,a.num);
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
			InsertLine(cursor);
			UpdateHighlighter();
			break;
		case Action::InsertLineAtEnd:
			MoveVisualCursorToLineEnd(cursor);
			InsertLine(cursor);
			UpdateHighlighter();
			break;
		case Action::DeletePreviousChar:
			if (selecting){
				DeleteSelection(cursor);
				UpdateHighlighter();
				break;
			}

			if (cursor.cursor.column==0&&cursor.cursor.line.index==0) //at start of textBuffer
				break;

			MoveVisualCursorLeft(cursor,1);
			DeleteCharAt(cursor.cursor);
			UpdateHighlighter();
			break;
		case Action::DeleteCurrentChar:
			if (selecting){
				DeleteSelection(cursor);
				UpdateHighlighter();
				break;
			}

			if (cursor.cursor.column==cursor.CurrentLineLen()
					&&cursor.cursor.line.index==(s32)(textBuffer->size()-1))
				break;

			DeleteCharAt(cursor.cursor);
			UpdateHighlighter();
			break;
		case Action::DeletePreviousMulti:
			if (selecting){
				DeleteSelection(cursor);
				UpdateHighlighter();
				break;
			}

			while (--a.num>=0){
				if (cursor.cursor.column==0&&cursor.cursor.line.index==0)
					break;

				MoveVisualCursorLeft(cursor,1);
				DeleteCharAt(cursor.cursor);
			}
			UpdateHighlighter();
			break;
		case Action::DeleteCurrentMulti:
			if (selecting){
				DeleteSelection(cursor);
				UpdateHighlighter();
				break;
			}

			while (--a.num>=0){
				if (cursor.cursor.column==cursor.CurrentLineLen()&&
						cursor.cursor.line.index==(s32)(textBuffer->size()-1))
					break;
				DeleteCharAt(cursor.cursor);
			}
			UpdateHighlighter();
			break;
		case Action::MoveToLineStart:
			MoveVisualCursorToLineStart(cursor);
			break;
		case Action::MoveToLineEnd:
			MoveVisualCursorToLineEnd(cursor);
			break;
		case Action::MoveToBufferStart:
			cursor.cursor = MakeCursorAtBufferStart(*textBuffer);
			SetVisualCursorColumn(cursor,cursor.cursor.column);
			SetCachedX(cursor);
			break;
		case Action::MoveToBufferEnd:
			cursor.cursor = MakeCursorAtBufferEnd(*textBuffer);
			SetVisualCursorColumn(cursor,cursor.cursor.column);
			SetCachedX(cursor);
			break;
		case Action::InsertChar:
			if (selecting) DeleteSelection(cursor);
			InsertCharAt(cursor.cursor,a.character);
			MoveVisualCursorRight(cursor,1);
			UpdateHighlighter();
			break;
		case Action::InsertTab:
			InsertCharAt(cursor.cursor,'\t');
			MoveVisualCursorRight(cursor,1);
			UpdateHighlighter();
			break;
		case Action::DeleteLine:
			DeleteLine(cursor);
			UpdateHighlighter();
			break;
		case Action::UndoAction:
			Undo(cursor);
			UpdateHighlighter();
			break;
		case Action::RedoAction:
			Redo(cursor);
			UpdateHighlighter();
			break;
		case Action::ToggleSelect:
			if (selecting) StopSelecting();
			else StartSelecting(cursor);
			break;
		case Action::DebugAction:
			showDebugInfo = !showDebugInfo;
			break;

		default:
			break;
	}
	if (selecting) UpdateSelection(cursor);
}

