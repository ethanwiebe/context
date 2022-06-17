#pragma once

#include "../key.h"
#include "../textscreen.h"

//inherit from this to support a platform
class TextInterfaceBase {
public:
	virtual void RenderScreen(const TextScreen&) = 0;
	virtual KeyboardEvent* GetKeyboardEvent() = 0;

	// height in lines
	virtual s32 GetHeight() = 0;

	// width in characters
	virtual s32 GetWidth() = 0;

	virtual ~TextInterfaceBase() = default;
};
