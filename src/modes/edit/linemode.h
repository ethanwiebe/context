#pragma once

#include "../mode.h"
#include <keybind.h>
#include <logger.h>
#include <undo.h>
#include <textbuffer.h>
#include "syntaxhighlight.h"
#include "find.h"
#include "cursor.h"
#include "lineconfig.h"

#include <vector>
#include <wchar.h>
#include <mutex>

struct VisualCursor {
	Cursor cursor;
	s32 visualLine;
	s32 subline;
	s32 cachedX;

	VisualCursor(LineIterator it) : cursor(it,0), visualLine(0), subline(0), cachedX(0) {}

	inline s32 CurrentLineLen() const noexcept {
		return (*cursor.line).size();
	}

	void SetVisualLineFromLine(LineIndexedIterator viewLine,s32 screenSubline,s32 w,s32 h,s32 tabSize);
};

enum class BufferActionType {
	TextInsertion,
	TextDeletion,
	LineReplacement
};

struct BufferAction {
	BufferActionType type;
	s32 line,column;
	s32 extendLine,extendColumn;
	
	s32 insertedLen;
	std::string text;
	
	LineDiffInfo lineDiffs;
	bool redo;

	BufferAction(BufferActionType type,s32 l,s32 c) : type(type),line(l),column(c),extendLine(l),extendColumn(c){
		insertedLen = 0;
		text = {};
		lineDiffs = {};
		redo = false;
	}

	bool Empty() const {
		if (type!=BufferActionType::LineReplacement)
			return insertedLen==0 && text.empty();
		return lineDiffs.empty();
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
	TextScreen textScreen;
	Ref<TextBuffer> textBuffer;
	ColorBuffer colorBuffer,altColorBuffer;
	std::string copiedText;
	std::string bufferPath;
	std::string cursorPosText;
	bool highlighterNeedsUpdate;
	size_t highlightTask;
	
	std::mutex colorMutex;

	LineIndexedIterator viewLine;
	ColorIterator colorLine;
	s32 screenSubline;
	std::vector<VisualCursor> cursors;
	s32 lineWidth;
	s32 lineNumberWidth;

	bool showDebugInfo;

	UndoStack<BufferAction> undoStack;
	BufferAction currentAction;
	
	FoundList matches;
	std::string findText;
	s32 findNum;
	bool finding;

	bool selecting;
	Cursor selectAnchor,selectCursor;

	Handle<SyntaxHighlighter> syntaxHighlighter;

	bool readonly,modified;
	
	LineConfig config;
	
	void UpdateHighlighterTask();
public:
	LineModeBase(ContextEditor* ctx);
	void InitIterators();
	void CalculateScreenData();
	inline void SetColorLine();

	void SetCachedX(VisualCursor&);
	
	bool IsTabIndented(LineIterator) const;

	TextScreen& GetTextScreen(s32,s32) override;
	TextStyle GetTextStyleAt(ColorIterator,s32);
	std::string_view GetStatusBarText() override;
	
	bool ProcessCommand(const TokenVector&) override;
	bool SetConfigVar(const TokenVector&) override;
	
	static void RegisterBinds();

	bool OpenAction(const OSInterface& os,std::string_view path) override;
	bool SaveAction(const OSInterface& os) override;
	
	bool Modified() override;
	bool Readonly() override {return readonly;}
	
	void UpdateStyle() override;
	
	std::string_view GetPath(const OSInterface&) override;
	void SetPath(const OSInterface&,std::string_view) override;
	
	void SetSyntaxHighlighter(const std::string&);
	
	bool HasPath() override;

	void MoveScreenDown(s32,bool=true);
	void MoveScreenUp(s32,bool=false);
	void MoveScreenToVisualCursor(VisualCursor&);
	void LockScreenToVisualCursor(VisualCursor&,bool=false);

	Cursor MakeCursorFromLineIndexedIterator(s32,s32,LineIndexedIterator);
	Cursor MakeCursor(s32,s32);

	void MoveVisualCursorDown(VisualCursor&,s32);
	void MoveVisualCursorUp(VisualCursor&,s32);
	void MoveVisualCursorLeft(VisualCursor&,s32);
	void MoveVisualCursorRight(VisualCursor&,s32);
	void MoveCursorLeft(Cursor&,s32) const;
	void MoveCursorRight(Cursor&,s32) const;
	void SetVisualCursorColumn(VisualCursor&,s32);
	
	void MoveVisualCursorToLineStart(VisualCursor&);
	void MoveVisualCursorToLineEnd(VisualCursor&);
	
	void MoveVisualCursorToBufferStart(VisualCursor&);
	void MoveVisualCursorToBufferEnd(VisualCursor&);
	
	void MoveVisualCursorLeftWord(VisualCursor&);
	void MoveVisualCursorRightWord(VisualCursor&);
	
	void MoveVisualCursorLeftPascalWord(VisualCursor&);
	void MoveVisualCursorRightPascalWord(VisualCursor&);
	
	void DeleteLine(Cursor);
	
	void VisualCursorInsertLine(VisualCursor&);
	void VisualCursorDeleteLine(VisualCursor&);

	void InsertCharAt(Cursor,char,bool=true);
	void InsertStringAt(Cursor,const std::string&,bool=true);
	void InsertLinesAt(Cursor,const std::string&);
	void DeleteCharAt(Cursor,bool=true);
	void DeleteCharCountAt(Cursor,s32);

	void VisualCursorDeletePreviousChar(VisualCursor&,s32);
	
	void VisualCursorDeletePreviousWord(VisualCursor&);
	void VisualCursorDeleteCurrentWord(VisualCursor&);
	
	void VisualCursorDeletePreviousPascalWord(VisualCursor&);
	void VisualCursorDeleteCurrentPascalWord(VisualCursor&);

	void InsertTab(VisualCursor&);
	void RemoveTab(VisualCursor&);

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
	void PushLineReplacementAction(LineDiffInfo&&);
	
	void ForceFinishAction(BufferActionType = BufferActionType::TextInsertion);

	void StartSelecting(const VisualCursor&);
	void StopSelecting();
	void UpdateSelection(const VisualCursor&);
	Cursor GetSelectStartPos() const;
	Cursor GetSelectEndPos() const;
	void VisualCursorDeleteSelection(VisualCursor&,bool=false);
	void CopySelection();
	void CopyLinesInSelection();
	void IndentSelection(VisualCursor&);
	void DedentSelection(VisualCursor&);
	void DeleteLinesInSelection(VisualCursor&);

	void UpdateHighlighter();
	
	void FindTextInBuffer(std::string_view,bool);
	void CursorToNextMatch();
	void CursorToPreviousMatch();
};

inline char GetCharAt(Cursor cursor){
	if (cursor.column==(s32)cursor.line.it->size()) return '\n';
	return (*cursor.line.it)[cursor.column];
}

s32 GetIndentationAt(LineIterator,s32);
void UpdateXI(const std::string& str,s32& x,s32& i,s32 width,s32 tabSize);
s32 GetXPosOfIndex(const std::string& str,s32 index,s32 width,s32 tabSize);
s32 GetIndexOfXPos(const std::string& str,s32 x,s32 width,s32 tabSize);

