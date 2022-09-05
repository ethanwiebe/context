#include "config.h"

Config gConfig = {
	.tabSize = 4,
	.cursorMoveHeight = 3,
	.multiAmount = 4,
	.style = "default",
	
	.displayLineNumbers = true,
	.autoIndent = true,
	.cursorLock = false,
	.cursorWrap = false,
	.smartHome = true,
	
	.tabMode = TabMode::Tabs,
	.moveMode = MultiMode::Multi,
	.deleteMode = MultiMode::Word
};

bool NameIsConfigVar(std::string_view name){
	for (size_t i=0;i<gConfigPropCount;++i){
		if (name==gConfigProps[i].name) return true;
	}
	
	return false;
}

