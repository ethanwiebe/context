#include "lineconfig.h"
#include "linemode.h"

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
			config.tabSize = n;
		} else {
			modeErrorMessage.Push("tabSize must be a positive integer");
		}
		return true;
	}
	if (name.Matches("cursorMoveHeight")){
		ssize_t n = strtol(val.data(),NULL,10);
		if (n>=0){
			config.cursorMoveHeight = n;
		} else {
			modeErrorMessage.Push("cursorMoveHeight must be a non-negative integer");
		}
		return true;
	}
	if (name.Matches("multiAmount")){
		ssize_t n = strtol(val.data(),NULL,10);
		if (n>0){
			config.multiAmount = n;
		} else {
			modeErrorMessage.Push("multiAmount must be a positive integer");
		}
		return true;
	}
	if (name.Matches("displayLineNumbers")){
		if (!ParseBool(config.displayLineNumbers,val)){
			modeErrorMessage.Push("displayLineNumbers must be a boolean value");
		}
		return true;
	}
	if (name.Matches("autoIndent")){
		if (!ParseBool(config.autoIndent,val)){
			modeErrorMessage.Push("autoIndent must be a boolean value");
		}
		return true;
	}
	if (name.Matches("cursorLock")){
		if (!ParseBool(config.cursorLock,val)){
			modeErrorMessage.Push("cursorLock must be a boolean value");
		}
		return true;
	}
	if (name.Matches("cursorWrap")){
		if (!ParseBool(config.cursorWrap,val)){
			modeErrorMessage.Push("cursorWrap must be a boolean value");
		}
		return true;
	}
	if (name.Matches("smartHome")){
		if (!ParseBool(config.smartHome,val)){
			modeErrorMessage.Push("smartHome must be a boolean value");
		}
		return true;
	}
	if (name.Matches("tabMode")){
		if (val=="tabs")
			config.tabMode = TabMode::Tabs;
		else if (val=="spaces")
			config.tabMode = TabMode::Spaces;
		else
			modeErrorMessage.Push("tabMode must be either 'tabs' or 'spaces'");
		return true;
	}
	if (name.Matches("moveMode")){
		if (!SetMultiMode(config.moveMode,val))
			modeErrorMessage.Push("moveMode must be one of 'multi', 'word', or 'pascal'");
	}
	if (name.Matches("deleteMode")){
		if (!SetMultiMode(config.deleteMode,val))
			modeErrorMessage.Push("deleteMode must be one of 'multi', 'word', or 'pascal'");
		return true;
	} if (name.Matches("syntaxHighlighter")){
		if (val!="none"&&!gSyntaxHighlighters.contains(val)){
			modeErrorMessage.Push("Unrecognized syntax highlighter '"+val+"'!");
			return true;
		}
		
		SetSyntaxHighlighter(val);
		highlighterNeedsUpdate = true;
		
		return true;
	}
	return false;
}

