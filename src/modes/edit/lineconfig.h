#pragma once

#include "core.h"

enum class TabMode : u64 {
	Spaces = 0,
	Tabs = 1
};

enum class MultiMode : u64 {
	Multi = 0,
	Word = 1,
	PascalWord = 2
};

struct LineConfig {
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
