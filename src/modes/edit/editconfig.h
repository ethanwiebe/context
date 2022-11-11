#pragma once

#include "core.h"

enum class TabMode : u8 {
	Spaces,
	Tabs
};

enum class MultiMode : u8 {
	Multi,
	Word,
	PascalWord
};

struct EditConfig {
	s64 tabSize;
	s64 cursorMoveHeight;
	s64 multiAmount;

	bool displayLineNumbers;
	bool autoIndent;
	bool cursorLock;
	bool cursorWrap;
	bool smartHome;
	TabMode tabMode;
	MultiMode moveMode;
	MultiMode deleteMode;
};

extern EditConfig gEditConfig;
