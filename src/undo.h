#pragma once

#include "core.h"

#include <stack>

template <typename T>
class UndoStack {
	std::stack<T> undoStack,redoStack;
public:
	void PushAction(T action){
		undoStack.push(action);
		redoStack = {};
	}


	bool CanUndo() const {
		return !undoStack.empty();
	}

	bool CanRedo() const {
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

	size_t UndoHeight() const {
		return undoStack.size();
	}

	size_t RedoHeight() const {
		return redoStack.size();
	}
};
