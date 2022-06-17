#pragma once

#include "core.h"
#include "linemode.h"

class EditMode : public LineModeBase {
public:
	EditMode(ContextEditor*);

	void ProcessTextAction(TextAction) override;
	void ProcessMoveAction(VisualCursor&,TextAction);

	std::string_view GetModeName() override {return "edit";}

};
