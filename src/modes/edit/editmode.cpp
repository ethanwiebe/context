#include "editmode.h"

#include "../../context.h"

EditMode::EditMode(ContextEditor* ctx) : LineModeBase(ctx) {}

void EditMode::ProcessMoveAction(VisualCursor& cursor,TextAction a){
	switch (a.action){
		case EditAction::MoveScreenUpLine:
			MoveScreenUp(1);
			break;
		case EditAction::MoveScreenDownLine:
			MoveScreenDown(1);
			break;
		case EditAction::MoveUpPage:
			MoveVisualCursorUp(cursor,screenHeight-1);
			break;
		case EditAction::MoveDownPage:
			MoveVisualCursorDown(cursor,screenHeight-1);
			break;
		case EditAction::MoveUpLine:
			MoveVisualCursorUp(cursor,a.num);
			break;
		case EditAction::MoveDownLine:
			MoveVisualCursorDown(cursor,a.num);
			break;
		case EditAction::MoveLeftChar:
			MoveVisualCursorLeft(cursor,1);
			break;
		case EditAction::MoveRightChar:
			MoveVisualCursorRight(cursor,1);
			break;
		case EditAction::MoveLeftMulti:
			if (config.moveMode==MultiMode::Multi){
				MoveVisualCursorLeft(cursor,a.num);
			} else if (config.moveMode==MultiMode::Word){
				MoveVisualCursorLeftWord(cursor);
			} else if (config.moveMode==MultiMode::PascalWord){
				MoveVisualCursorLeftPascalWord(cursor);
			}
			break;
		case EditAction::MoveRightMulti:
			if (config.moveMode==MultiMode::Multi){
				MoveVisualCursorRight(cursor,a.num);
			} else if (config.moveMode==MultiMode::Word){
				MoveVisualCursorRightWord(cursor);
			} else if (config.moveMode==MultiMode::PascalWord){
				MoveVisualCursorRightPascalWord(cursor);
			}
			break;
		case EditAction::MoveUpMulti:
			MoveVisualCursorUp(cursor,a.num);
			break;
		case EditAction::MoveDownMulti:
			MoveVisualCursorDown(cursor,a.num);
			break;
		case EditAction::MoveToLineStart:
			MoveVisualCursorToLineStart(cursor);
			break;
		case EditAction::MoveToLineEnd:
			MoveVisualCursorToLineEnd(cursor);
			break;
		case EditAction::MoveToBufferStart:
			MoveVisualCursorToBufferStart(cursor);
			break;
		case EditAction::MoveToBufferEnd:
			MoveVisualCursorToBufferEnd(cursor);
			break;
		default:
			break;
	}
}

void EditMode::ProcessKeyboardEvent(KeyEnum key,KeyModifier mod){
	TextAction a = GetTextActionFromKey(key,mod,config.multiAmount);
	VisualCursor& cursor = cursors[0];
	ProcessMoveAction(cursor,a);
	
	if (finding){
		switch (a.action){
			case EditAction::InsertLine:
			case EditAction::Tab:
				CursorToNextMatch();
				return;
			case EditAction::Untab:
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
			case EditAction::InsertChar:
			case EditAction::InsertLine:
			case EditAction::Paste:
			case EditAction::PasteLines:
				VisualCursorDeleteSelection(cursor);
				break;
			case EditAction::DeleteCurrentChar:
			case EditAction::DeletePreviousChar:
			case EditAction::DeleteCurrentMulti:
			case EditAction::DeletePreviousMulti:
				VisualCursorDeleteSelection(cursor);
				return;
			case EditAction::Cut:
				VisualCursorDeleteSelection(cursor,true);
				ctx->GetClipboard() = copiedText;
				return;
			case EditAction::CutLines:
				CopyLinesInSelection();
				DeleteLinesInSelection(cursor);
				ctx->GetClipboard() = copiedText;
				return;
			case EditAction::Copy:
				CopySelection();
				StopSelecting();
				ctx->GetClipboard() = copiedText;
				return;
			case EditAction::CopyLines:
				CopyLinesInSelection();
				StopSelecting();
				ctx->GetClipboard() = copiedText;
				return;
			case EditAction::Tab:
				IndentSelection(cursor);
				return;
			case EditAction::Untab:
				DedentSelection(cursor);
				return;
			case EditAction::DeleteLine:
				DeleteLinesInSelection(cursor);
				return;
			case EditAction::UndoAction:
			case EditAction::RedoAction:
			case EditAction::Escape:
				StopSelecting();
				break;
			case EditAction::ToggleSelect:
				StopSelecting();
				return;
			default:
				break;
		}
	}
	if (!selecting){
		switch (a.action){
			case EditAction::InsertLine:
				VisualCursorInsertLine(cursor);
				break;
			case EditAction::InsertLineBelow:
				MoveVisualCursorToLineEnd(cursor);
				VisualCursorInsertLine(cursor);
				break;
			case EditAction::InsertLineAbove:
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
			case EditAction::DeletePreviousChar:
				VisualCursorDeletePreviousChar(cursor,1);
				break;
			case EditAction::DeleteCurrentChar:
				if (cursor.cursor.column==cursor.CurrentLineLen()
						&&cursor.cursor.line.index==(s32)(textBuffer->size()-1))
					break;
	
				DeleteCharAt(cursor.cursor);
				break;
			case EditAction::DeletePreviousMulti:
				if (config.deleteMode==MultiMode::Multi){
					VisualCursorDeletePreviousChar(cursor,a.num);
				} else if (config.deleteMode==MultiMode::Word){
					VisualCursorDeletePreviousWord(cursor);
				} else if (config.deleteMode==MultiMode::PascalWord){
					VisualCursorDeletePreviousPascalWord(cursor);
				}
				break;
			case EditAction::DeleteCurrentMulti:
				if (config.deleteMode==MultiMode::Multi){
					while (--a.num>=0){
						if (cursor.cursor.column==cursor.CurrentLineLen()&&
								cursor.cursor.line.index==(s32)(textBuffer->size()-1))
							break;
						DeleteCharAt(cursor.cursor);
					}
				} else if (config.deleteMode==MultiMode::Word){
					VisualCursorDeleteCurrentWord(cursor);
				} else if (config.deleteMode==MultiMode::PascalWord){
					VisualCursorDeleteCurrentPascalWord(cursor);
				}
				break;
			case EditAction::InsertChar:
				InsertCharAt(cursor.cursor,a.character);
				MoveVisualCursorRight(cursor,1);
				break;
			case EditAction::Tab:
				InsertTab(cursor);
				break;
			case EditAction::Untab:
				RemoveTab(cursor);
				break;
			case EditAction::DeleteLine:
				VisualCursorDeleteLine(cursor);
				break;
			case EditAction::UndoAction:
				Undo(cursor);
				break;
			case EditAction::RedoAction:
				Redo(cursor);
				break;
			case EditAction::ToggleSelect:
				StartSelecting(cursor);
				break;
			case EditAction::SelectAll:
				selecting = true;
				selectAnchor = MakeCursorAtBufferStart(*textBuffer);
				MoveVisualCursorToBufferEnd(cursor);
				UpdateSelection(cursor);
				break;
			case EditAction::Copy:
			case EditAction::CopyLines:
				copiedText = *cursor.cursor.line.it;
				ctx->GetClipboard() = copiedText;
				break;
			case EditAction::Cut:
			case EditAction::CutLines:
				copiedText = *cursor.cursor.line.it;
				VisualCursorDeleteLine(cursor);
				ctx->GetClipboard() = copiedText;
				break;
			case EditAction::Paste:
				copiedText = ctx->GetClipboard();
				InsertStringAt(cursor.cursor,copiedText);
				MoveCursorRight(cursor.cursor,copiedText.size());
				SetCachedX(cursor);
				break;
			case EditAction::PasteLines:
				copiedText = ctx->GetClipboard();
				InsertLinesAt(cursor.cursor,copiedText);
				break;
			case EditAction::Goto:
				ctx->BeginCommand("goto ");
				break;
			case EditAction::Find:
				ctx->BeginCommand("find ");
				break;
			case EditAction::Replace:
				ctx->BeginCommand("replace ");
				break;
			case EditAction::Escape:
				matches.clear();
				break;
			case EditAction::DebugAction:
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

ModeBase* CreateEditMode(ContextEditor* ctx){
	return new EditMode(ctx);
}
