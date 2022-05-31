#include "editmode.h"

EditMode::EditMode(ContextEditor* ctx) : LineModeBase(ctx) {}

void EditMode::ProcessMoveAction(VisualCursor& cursor,TextAction a){
	switch (a.action){
		case Action::MoveScreenUpLine:
			MoveScreenUp(1);
			break;
		case Action::MoveScreenDownLine:
			MoveScreenDown(1);
			break;
		case Action::MoveUpPage:
			MoveVisualCursorUp(cursor,innerHeight-1);
			break;
		case Action::MoveDownPage:
			MoveVisualCursorDown(cursor,innerHeight-1);
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
		case Action::MoveToLineStart:
			MoveVisualCursorToLineStart(cursor);
			break;
		case Action::MoveToLineEnd:
			MoveVisualCursorToLineEnd(cursor);
			break;
		case Action::MoveToBufferStart:
			MoveVisualCursorToBufferStart(cursor);
			break;
		case Action::MoveToBufferEnd:
			MoveVisualCursorToBufferEnd(cursor);
			break;
		default:
			break;
	}
}

void EditMode::ProcessTextAction(TextAction a){
	VisualCursor& cursor = cursors[0];
	ProcessMoveAction(cursor,a);
	if (selecting){
		switch(a.action){
			case Action::InsertChar:
			case Action::PasteClipboard:
				VisualCursorDeleteSelection(cursor);
				break;
			case Action::DeleteCurrentChar:
			case Action::DeletePreviousChar:
			case Action::DeleteCurrentMulti:
			case Action::DeletePreviousMulti:
				VisualCursorDeleteSelection(cursor);
				return;
			case Action::CutSelection:
				VisualCursorDeleteSelection(cursor,true);
				return;
			case Action::CopySelection:
				CopySelection();
				StopSelecting();
				return;
			case Action::Tab:
				IndentSelection();
				return;
			case Action::Untab:
				DedentSelection();
				return;
			case Action::UndoAction:
			case Action::RedoAction:
			case Action::Escape:
				StopSelecting();
				break;
			default:
				break;
		}
	}
	if (!selecting){
		switch (a.action){
			case Action::InsertLine:
				InsertLine(cursor);
				break;
			case Action::InsertLineBelow:
				MoveVisualCursorToLineEnd(cursor);
				InsertLine(cursor);
				break;
			case Action::InsertLineAbove:
				if (cursor.cursor.line.index==0){
					MoveVisualCursorToLineStart(cursor);
					InsertLine(cursor);
					MoveVisualCursorUp(cursor,1);
					break;
				} else {
					MoveVisualCursorUp(cursor,1);
					MoveVisualCursorToLineEnd(cursor);
				}
				InsertLine(cursor);
				break;
			case Action::DeletePreviousChar:
				if (cursor.cursor.column==0&&cursor.cursor.line.index==0) //at start of textBuffer
					break;
	
				MoveVisualCursorLeft(cursor,1);
				DeleteCharAt(cursor.cursor);
				break;
			case Action::DeleteCurrentChar:
				if (cursor.cursor.column==cursor.CurrentLineLen()
						&&cursor.cursor.line.index==(s32)(textBuffer->size()-1))
					break;
	
				DeleteCharAt(cursor.cursor);
				break;
			case Action::DeletePreviousMulti:
				while (--a.num>=0){
					if (cursor.cursor.column==0&&cursor.cursor.line.index==0)
						break;
	
					MoveVisualCursorLeft(cursor,1);
					DeleteCharAt(cursor.cursor);
				}
				break;
			case Action::DeleteCurrentMulti:
				while (--a.num>=0){
					if (cursor.cursor.column==cursor.CurrentLineLen()&&
							cursor.cursor.line.index==(s32)(textBuffer->size()-1))
						break;
					DeleteCharAt(cursor.cursor);
				}
				break;
			case Action::InsertChar:
				InsertCharAt(cursor.cursor,a.character);
				MoveVisualCursorRight(cursor,1);
				break;
			case Action::Tab:
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
			case Action::ToggleSelect:
				if (selecting) StopSelecting();
				else StartSelecting(cursor);
				break;
			case Action::SelectAll:
				selecting = true;
				selectAnchor = MakeCursorAtBufferStart(*textBuffer);
				selectCursor = MakeCursorAtBufferEnd(*textBuffer);
				MoveVisualCursorToBufferEnd(cursor);
				break;
			case Action::PasteClipboard:
				InsertStringAt(cursor.cursor,copiedText);
				MoveVisualCursorRight(cursor,copiedText.size());
				break;
			case Action::DebugAction:
				showDebugInfo = !showDebugInfo;
				break;
	
			default:
				break;
		}
	}
	if (selecting) UpdateSelection(cursor);
}

