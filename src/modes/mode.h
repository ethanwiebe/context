#pragma once

#include "../core.h"

#include "../key.h"
#include "../textscreen.h"

#include <vector>

struct TextAction {
	Action action;
	
	union {
		s32 num;
		char character;
		const char* str;
	};
};


// modes sit on top of text files and modify them according
// to the stream of keyboard events they receive

class ModeBase {
protected:
	s32 screenWidth,screenHeight;
public:
	virtual void ProcessKeyboardEvent(KeyboardEvent*) = 0;
	virtual TextScreen GetTextScreen(s32,s32) = 0;

	virtual ~ModeBase() = default;
};

