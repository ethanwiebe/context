#include "editconfig.h"
#include "linemode.h"

EditConfig gEditConfig = {
	.tabSize = 4,
	.cursorMoveHeight = 3,
	.multiAmount = 4,
	.displayLineNumbers = true,
	.autoIndent = true,
	.cursorLock = false,
	.cursorWrap = false,
	.smartHome = true,
	.tabMode = TabMode::Tabs,
	.moveMode = MultiMode::Multi,
	.deleteMode = MultiMode::Word
};

inline bool SetMultiMode(MultiMode& var,std::string_view val){
	if (val=="multi")
		var = MultiMode::Multi;
	else if (val=="word")
		var = MultiMode::Word;
	else if (val=="pascal")
		var = MultiMode::PascalWord;
	else
		return false;
	
	return true;
}

bool LineModeBase::SetConfigVar(const TokenVector& tokens){
	if (tokens.size()<3) return false;
	
	const auto& name = tokens[1];
	auto val = tokens[2].Stringify();
	
	if (name.Matches("tabSize")){
		ssize_t n = strtol(val.data(),NULL,10);
		if (n>0){
			gEditConfig.tabSize = n;
		} else {
			modeErrorMessage.Set("tabSize must be a positive integer");
		}
		return true;
	}
	if (name.Matches("cursorMoveHeight")){
		ssize_t n = strtol(val.data(),NULL,10);
		if (n>=0){
			gEditConfig.cursorMoveHeight = n;
		} else {
			modeErrorMessage.Set("cursorMoveHeight must be a non-negative integer");
		}
		return true;
	}
	if (name.Matches("multiAmount")){
		ssize_t n = strtol(val.data(),NULL,10);
		if (n>0){
			gEditConfig.multiAmount = n;
		} else {
			modeErrorMessage.Set("multiAmount must be a positive integer");
		}
		return true;
	}
	if (name.Matches("displayLineNumbers")){
		if (!ParseBool(gEditConfig.displayLineNumbers,val)){
			modeErrorMessage.Set("displayLineNumbers must be a boolean value");
		}
		return true;
	}
	if (name.Matches("autoIndent")){
		if (!ParseBool(gEditConfig.autoIndent,val)){
			modeErrorMessage.Set("autoIndent must be a boolean value");
		}
		return true;
	}
	if (name.Matches("cursorLock")){
		if (!ParseBool(gEditConfig.cursorLock,val)){
			modeErrorMessage.Set("cursorLock must be a boolean value");
		}
		return true;
	}
	if (name.Matches("cursorWrap")){
		if (!ParseBool(gEditConfig.cursorWrap,val)){
			modeErrorMessage.Set("cursorWrap must be a boolean value");
		}
		return true;
	}
	if (name.Matches("smartHome")){
		if (!ParseBool(gEditConfig.smartHome,val)){
			modeErrorMessage.Set("smartHome must be a boolean value");
		}
		return true;
	}
	if (name.Matches("tabMode")){
		if (val=="tabs")
			gEditConfig.tabMode = TabMode::Tabs;
		else if (val=="spaces")
			gEditConfig.tabMode = TabMode::Spaces;
		else
			modeErrorMessage.Set("tabMode must be either 'tabs' or 'spaces'");
		return true;
	}
	if (name.Matches("moveMode")){
		if (!SetMultiMode(gEditConfig.moveMode,val))
			modeErrorMessage.Set("moveMode must be one of 'multi', 'word', or 'pascal'");
	}
	if (name.Matches("deleteMode")){
		if (!SetMultiMode(gEditConfig.deleteMode,val))
			modeErrorMessage.Set("deleteMode must be one of 'multi', 'word', or 'pascal'");
		return true;
	}
	return false;
}
