#pragma once

#include "core.h"
#include "linemode.h"
#include "editbinds.h"

class EditMode : public LineModeBase {
public:
	EditMode(ContextEditor*);

	void ProcessKeyboardEvent(KeyEnum,KeyModifier) override;
	bool ProcessMoveAction(VisualCursor&,TextAction);

	std::string_view GetModeName() override {return "edit";}
	
	void SetHelp(Ref<TextBuffer>);
};

ModeBase* CreateEditMode(ContextEditor*);
