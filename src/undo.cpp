#include "undo.h"

template <typename T>
void UndoStack<T>::PushAction(T action){
	undoStack.push(action);
	redoStack = {};
}

template <typename T>
bool UndoStack<T>::CanUndo() const {
	return !undoStack.empty();
}

template <typename T>
bool UndoStack<T>::CanRedo() const {
	return !redoStack.empty();
}

template <typename T>
T UndoStack<T>::GetUndo(){
	T action = undoStack.pop();
	redoStack.push(action);
	return action;
}

template <typename T>
T UndoStack<T>::GetRedo(){
	T action = redoStack.pop();
	undoStack.push(action);
	return action;
}
