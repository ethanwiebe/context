#pragma once

#include "core.h"

#include <stack>

template <typename T>
class UndoStack {
	std::stack<T> undoStack,redoStack;
public:
	void PushAction(T action);

	bool CanUndo() const; 

	bool CanRedo() const;

	T GetUndo();

	T GetRedo();
};
