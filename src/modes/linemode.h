#pragma once

#include "mode.h"
#include "../keybind.h"
#include "../config.h"
#include "../textbuffer.h"
#include "../tui.h"
#include "../logger.h"
#include "../undo.h"
#include "../syntaxhighlight.h"

#include <vector>

struct Cursor {
	LineIndexedIterator line;
	s32 column;
};

struct VisualCursor {
	Cursor cursor;
	s32 visualLine;
	s32 subline;
	s32 cachedX;

	VisualCursor(LineIterator it) : cursor(it,0), visualLine(0), subline(0), cachedX(0) {}

	inline s32 CurrentLineLen() const noexcept {
		return (*cursor.line).size();
	}

	void SetVisualLineFromLine(LineIndexedIterator viewLine,s32 screenSubline,s32 w,s32 h);
};

enum class BufferActionType {
	TextInsertion,
	TextDeletion
};

struct BufferAction {
	BufferActionType type;
	s32 line,column;
	s32 extendLine,extendColumn;

	s32 insertedLen;
	std::string text;

	BufferAction(BufferActionType type,s32 l,s32 c) : type(type),line(l),column(c),extendLine(l),extendColumn(c){
		insertedLen = 0;
		text = {};
	}

	bool Empty() const {
		return insertedLen==0 && text.empty();
	}

	void AddCharacter(char c){
		++insertedLen;
		text += c;
	}

	void PrependCharacter(char c){
		++insertedLen;
		text.insert(0,1,c);
	}
};


class LineModeBase : public ModeBase {
protected:
	Ref<TextBuffer> textBuffer;
	ColorBuffer colorBuffer;
	std::string bufferPath;

	LineIndexedIterator viewLine;
	ColorIterator colorLine;
	s32 screenSubline;
	std::vector<VisualCursor> cursors;
	s32 lineWidth,innerHeight;

	UndoStack<BufferAction> undoStack;
	BufferAction currentAction;

	bool selecting;
	Cursor selectAnchor,selectCursor;

	Handle<SyntaxHighlighter> syntaxHighlighter;

public:
	LineModeBase(ContextEditor* ctx);
	void InitIterators();
	void CalculateColorLine();

	TextScreen GetTextScreen(s32,s32) override;
	TextStyle GetTextStyleAt(ColorIterator,s32);
	std::string_view GetBufferName() override;

	bool OpenAction(const OSInterface& os,std::string_view path) override;
	bool SaveAction(const OSInterface& os) override;
	void SetPath(const OSInterface&,std::string_view) override;

	void GetSyntaxHighlighter(std::string_view);
	
	bool HasSavePath() override;

	void MoveScreenDown(s32,bool = true);
	void MoveScreenUp(s32,bool = false);
	void MoveScreenToVisualCursor(VisualCursor&);
	void LockScreenToVisualCursor(VisualCursor&);

	Cursor MakeCursorFromLineIndexedIterator(s32,s32,LineIndexedIterator);
	Cursor MakeCursor(s32,s32);

	void MoveVisualCursorDown(VisualCursor&,s32);
	void MoveVisualCursorUp(VisualCursor&,s32);
	void MoveVisualCursorLeft(VisualCursor&,s32);
	void MoveVisualCursorRight(VisualCursor&,s32);
	void MoveCursorLeft(Cursor&,s32) const;
	void MoveCursorRight(Cursor&,s32) const;
	void SetVisualCursorColumn(VisualCursor&,s32);

	void DeleteLine(VisualCursor&);

	void InsertCharAt(Cursor,char,bool=true);
	void InsertStringAt(Cursor,const std::string&,bool=true);
	void DeleteCharAt(Cursor,bool=true);
	void DeleteCharCountAt(Cursor,s32);

	void SetModified();

	void PerformBufferAction(VisualCursor&,const BufferAction&);

	void Undo(VisualCursor&);
	void Redo(VisualCursor&);

	void MakeNewAction(BufferActionType,s32,s32);
	bool InsertionExtendsAction(Cursor) const;
	bool DeletionExtendsAction(Cursor) const;

	void FinishOldAction(Cursor,BufferActionType);
	void PushInsertionAction(Cursor,char);
	void PushDeletionAction(Cursor,char);

	void StartSelecting(const VisualCursor&);
	void StopSelecting();
	void UpdateSelection(const VisualCursor&);
	Cursor GetSelectStartPos() const;
	Cursor GetSelectEndPos() const;
	void DeleteSelection(VisualCursor&);

	void UpdateHighlighter();
};

