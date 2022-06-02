#pragma once

#include "core.h"

enum class TabMode {
	Spaces,
	Tabs
};

namespace Config {
	const s32 tabSize = 4;

	const bool displayLineNumbers = true;
	const bool autoIndent = true;
	const bool cursorLock = false;
	const bool cursorWraps = false;
	const bool smartHome = true;
	const s32 cursorMoveHeight = 3;
	const s32 multiAmount = 4;
	const s32 pageSize = 50;
	const TabMode tabMode = TabMode::Tabs;
};
