#pragma once

#include "core.h"

#include <stack>

template <typename T>
class UndoStack {
	std::stack<T> undoStack,redoStack;
	size_t saveHeight;
public:
	void PushAction(T action){
		if (undoStack.size()<saveHeight)
			saveHeight = -1ULL;
			
		undoStack.push(action);
		redoStack = {};
	}

	bool CanUndo() const noexcept {
		return !undoStack.empty();
	}

	bool CanRedo() const noexcept {
		return !redoStack.empty();
	}
	
	T PopUndo(){
		T a = undoStack.top();
		undoStack.pop();
		redoStack.push(a);
		return a;
	}

	T PopRedo(){
		T a = redoStack.top();
		redoStack.pop();
		undoStack.push(a);
		return a;
	}

	size_t UndoHeight() const noexcept {
		return undoStack.size();
	}

	size_t RedoHeight() const noexcept {
		return redoStack.size();
	}
	
	void SetSaveHeight(){
		saveHeight = undoStack.size();
	}
	
	bool IsAtSaveHeight() const noexcept {
		return saveHeight==undoStack.size();
	}
};
