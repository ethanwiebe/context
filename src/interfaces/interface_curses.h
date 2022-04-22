#pragma once

#include "interface.h"
#include "../logger.h"

//#include <xcurses/curses.h>
#include <ncurses.h>
#include <string>

class CursesInterface : public TextInterfaceBase {
	KeyboardEvent lastEvent;
	chtype* charArray;

public:
	CursesInterface();
	~CursesInterface();

	void RenderScreen(const TextScreen&) override;
	KeyboardEvent* GetKeyboardEvent() override;
	void WindowResized(s32,s32) override;

	s32 GetHeight() override;
	s32 GetWidth() override;

private:
	void InitColorPairs();
	void ListColorPairs();
};

inline void NormalizeKeys(s32&,s32&);
