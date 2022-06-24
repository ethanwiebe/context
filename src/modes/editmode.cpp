#include "editmode.h"

#include "../context.h"

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
			if (gConfig.moveMode==MultiMode::Multi){
				MoveVisualCursorLeft(cursor,a.num);
			} else if (gConfig.moveMode==MultiMode::Word){
				MoveVisualCursorLeftWord(cursor);
			} else if (gConfig.moveMode==MultiMode::PascalWord){
				MoveVisualCursorLeftPascalWord(cursor);
			}
			break;
		case Action::MoveRightMulti:
			if (gConfig.moveMode==MultiMode::Multi){
				MoveVisualCursorRight(cursor,a.num);
			} else if (gConfig.moveMode==MultiMode::Word){
				MoveVisualCursorRightWord(cursor);
			} else if (gConfig.moveMode==MultiMode::PascalWord){
				MoveVisualCursorRightPascalWord(cursor);
			}
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
	
	if (finding){
		switch (a.action){
			case Action::InsertLine:
			case Action::Tab:
				CursorToNextMatch();
				return;
			case Action::Untab:
				CursorToPreviousMatch();
				return;
		
			default:
				finding = false;
				matches.clear();
				break;
		}
	}
	
	if (selecting){
		switch(a.action){
			case Action::InsertChar:
			case Action::InsertLine:
			case Action::Paste:
			case Action::PasteLines:
				VisualCursorDeleteSelection(cursor);
				break;
			case Action::DeleteCurrentChar:
			case Action::DeletePreviousChar:
			case Action::DeleteCurrentMulti:
			case Action::DeletePreviousMulti:
				VisualCursorDeleteSelection(cursor);
				return;
			case Action::Cut:
				VisualCursorDeleteSelection(cursor,true);
				ctx->GetClipboard() = copiedText;
				return;
			case Action::CutLines:
				CopyLinesInSelection();
				DeleteLinesInSelection(cursor);
				ctx->GetClipboard() = copiedText;
				return;
			case Action::Copy:
				CopySelection();
				StopSelecting();
				ctx->GetClipboard() = copiedText;
				return;
			case Action::CopyLines:
				CopyLinesInSelection();
				StopSelecting();
				ctx->GetClipboard() = copiedText;
				return;
			case Action::Tab:
				IndentSelection();
				return;
			case Action::Untab:
				DedentSelection();
				return;
			case Action::DeleteLine:
				DeleteLinesInSelection(cursor);
				return;
			case Action::UndoAction:
			case Action::RedoAction:
			case Action::Escape:
				StopSelecting();
				break;
			case Action::ToggleSelect:
				StopSelecting();
				return;
			default:
				break;
		}
	}
	if (!selecting){
		switch (a.action){
			case Action::InsertLine:
				VisualCursorInsertLine(cursor);
				break;
			case Action::InsertLineBelow:
				MoveVisualCursorToLineEnd(cursor);
				VisualCursorInsertLine(cursor);
				break;
			case Action::InsertLineAbove:
				if (cursor.cursor.line.index==0){
					MoveVisualCursorToLineStart(cursor);
					VisualCursorInsertLine(cursor);
					MoveVisualCursorUp(cursor,1);
					break;
				} else {
					MoveVisualCursorUp(cursor,1);
					MoveVisualCursorToLineEnd(cursor);
				}
				VisualCursorInsertLine(cursor);
				break;
			case Action::DeletePreviousChar:
				VisualCursorDeletePreviousChar(cursor,1);
				break;
			case Action::DeleteCurrentChar:
				if (cursor.cursor.column==cursor.CurrentLineLen()
						&&cursor.cursor.line.index==(s32)(textBuffer->size()-1))
					break;
	
				DeleteCharAt(cursor.cursor);
				break;
			case Action::DeletePreviousMulti:
				if (gConfig.deleteMode==MultiMode::Multi){
					VisualCursorDeletePreviousChar(cursor,a.num);
				} else if (gConfig.deleteMode==MultiMode::Word){
					VisualCursorDeletePreviousWord(cursor);
				} else if (gConfig.deleteMode==MultiMode::PascalWord){
					VisualCursorDeletePreviousPascalWord(cursor);
				}
				break;
			case Action::DeleteCurrentMulti:
				if (gConfig.deleteMode==MultiMode::Multi){
					while (--a.num>=0){
						if (cursor.cursor.column==cursor.CurrentLineLen()&&
								cursor.cursor.line.index==(s32)(textBuffer->size()-1))
							break;
						DeleteCharAt(cursor.cursor);
					}
				} else if (gConfig.deleteMode==MultiMode::Word){
					VisualCursorDeleteCurrentWord(cursor);
				} else if (gConfig.deleteMode==MultiMode::PascalWord){
					VisualCursorDeleteCurrentPascalWord(cursor);
				}
				break;
			case Action::InsertChar:
				InsertCharAt(cursor.cursor,a.character);
				MoveVisualCursorRight(cursor,1);
				break;
			case Action::Tab:
				InsertTab(cursor);
				break;
			case Action::Untab:
				RemoveTab(cursor);
				break;
			case Action::DeleteLine:
				VisualCursorDeleteLine(cursor);
				break;
			case Action::UndoAction:
				Undo(cursor);
				break;
			case Action::RedoAction:
				Redo(cursor);
				break;
			case Action::ToggleSelect:
				StartSelecting(cursor);
				break;
			case Action::SelectAll:
				selecting = true;
				selectAnchor = MakeCursorAtBufferStart(*textBuffer);
				selectCursor = MakeCursorAtBufferEnd(*textBuffer);
				MoveVisualCursorToBufferEnd(cursor);
				break;
			case Action::Copy:
			case Action::CopyLines:
				copiedText = *cursor.cursor.line.it;
				ctx->GetClipboard() = copiedText;
				break;
			case Action::Cut:
			case Action::CutLines:
				copiedText = *cursor.cursor.line.it;
				VisualCursorDeleteLine(cursor);
				ctx->GetClipboard() = copiedText;
				break;
			case Action::Paste:
				copiedText = ctx->GetClipboard();
				InsertStringAt(cursor.cursor,copiedText);
				MoveCursorRight(cursor.cursor,copiedText.size());
				SetCachedX(cursor);
				break;
			case Action::PasteLines:
				copiedText = ctx->GetClipboard();
				InsertLinesAt(cursor.cursor,copiedText);
				break;
			case Action::Goto:
				ctx->BeginCommand("goto ");
				break;
			case Action::Find:
				ctx->BeginCommand("find ");
				break;
			case Action::Replace:
				ctx->BeginCommand("replace ");
				break;
			case Action::Escape:
				matches.clear();
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


void EditMode::SetHelp(Ref<TextBuffer> buf){
	readonly = true;
	bufferPath = "ctxhelp";
	modified = false;
	
	textBuffer->clear();
	*textBuffer = *buf;
}

