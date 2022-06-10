#include "linemode.h"

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
	
	highlighterNeedsUpdate = true;
	matches.clear();
}

void LineModeBase::Redo(VisualCursor& cursor){
	if (!currentAction.Empty()){
		undoStack.PushAction(currentAction);
		MakeNewAction(BufferActionType::TextInsertion,0,0);	
	}

	if (!undoStack.CanRedo()) return;

	BufferAction redoAction = undoStack.PopRedo();
	
	PerformBufferAction(cursor,redoAction);
	
	highlighterNeedsUpdate = true;
	matches.clear();
}

void LineModeBase::MakeNewAction(BufferActionType type,s32 line,s32 col){
	currentAction = {type,line,col};
}

void LineModeBase::PerformBufferAction(VisualCursor& cursor,
		const BufferAction& action){
	Cursor c;
	if (action.type==BufferActionType::TextInsertion){
		c = MakeCursorFromLineIndexedIterator(action.line,
				action.column,cursor.cursor.line);
		InsertStringAt(c,action.text,false);
		cursor.cursor = c;
		MoveVisualCursorRight(cursor,action.text.size());
	} else if (action.type==BufferActionType::TextDeletion){
		c = MakeCursorFromLineIndexedIterator(action.extendLine,
				action.extendColumn,cursor.cursor.line);
		cursor.cursor = c;
		for (s32 i=0;i<action.insertedLen;++i){
			DeleteCharAt(c,false);
		}
		SetVisualCursorColumn(cursor,cursor.cursor.column);
	}
}

bool LineModeBase::InsertionExtendsAction(Cursor cursor) const {
	return currentAction.extendLine==cursor.line.index&&
			currentAction.extendColumn==cursor.column;
}

bool LineModeBase::DeletionExtendsAction(Cursor cursor) const {
	bool cond1 = currentAction.line==cursor.line.index&&
			currentAction.column==cursor.column;
	MoveCursorRight(cursor,1);
	bool cond2 = currentAction.extendLine==cursor.line.index&&
			currentAction.extendColumn==cursor.column;
	return cond1||cond2;
}

void LineModeBase::FinishOldAction(Cursor cursor,BufferActionType type){
	if (currentAction.type!=type ||
			(type==BufferActionType::TextDeletion&&!DeletionExtendsAction(cursor)) ||
			(type==BufferActionType::TextInsertion&&!InsertionExtendsAction(cursor))){
			
		if (!currentAction.Empty())
			undoStack.PushAction(currentAction);

		MakeNewAction(type,cursor.line.index,cursor.column);
	}
}

void LineModeBase::PushInsertionAction(Cursor cursor,char c){
	FinishOldAction(cursor,BufferActionType::TextInsertion);

	currentAction.AddCharacter(c);
	MoveCursorRight(cursor,1);
	currentAction.extendLine = cursor.line.index;
	currentAction.extendColumn = cursor.column;
}

void LineModeBase::PushDeletionAction(Cursor cursor,char c){
	FinishOldAction(cursor,BufferActionType::TextDeletion);

	Cursor cursorcopy = cursor;
	MoveCursorRight(cursorcopy,1);
	if (cursorcopy.line.index==currentAction.extendLine&&
			cursorcopy.column==currentAction.extendColumn){
		currentAction.PrependCharacter(c);
		currentAction.extendLine = cursor.line.index;
		currentAction.extendColumn = cursor.column;
		currentAction.line = cursor.line.index;
		currentAction.column = cursor.column;
	} else {
		currentAction.AddCharacter(c);
	}
}

void LineModeBase::ForceFinishAction(){
	if (!currentAction.Empty())
		undoStack.PushAction(currentAction);

	MakeNewAction(BufferActionType::TextInsertion,0,0);
}
