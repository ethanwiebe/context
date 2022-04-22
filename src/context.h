#pragma once

#include "core.h"

#include "modes/editmode.h"
#include "interfaces/interface_curses.h"

#include <vector>
#include <iostream>


class ContextEditor {
	std::vector<Handle<ModeBase>> modes;
	Handle<TextInterfaceBase> interface;
	size_t currentMode;

public:


	ContextEditor();

	void Loop();

};
