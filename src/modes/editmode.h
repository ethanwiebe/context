#pragma once

#include "../core.h"
#include "../interfaces/os.h"
#include "linemode.h"

class EditMode : public LineModeBase {
public:
	EditMode(ContextEditor*);

	void ProcessTextAction(TextAction) override;

	std::string_view GetModeName() override {return "edit";}

};
