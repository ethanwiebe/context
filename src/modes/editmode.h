#pragma once

#include "../core.h"
#include "../fileio.h"
#include "linemode.h"

class EditMode : public LineModeBase {
	IndexedIterator screenTop;
public:
	EditMode();

	void ProcessTextAction(TextAction) override;

};
