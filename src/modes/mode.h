#pragma once

#include "core.h"

#include "../key.h"
#include "../textscreen.h"
#include "../tokenizer.h"
#include "../interfaces/os.h"

class ContextEditor;

// modes sit on top of text files and modify them according
// to the stream of keyboard events they receive
class ModeBase {
protected:
	ContextEditor* ctx;

	s32 screenWidth,screenHeight;
	std::string modeErrorMessage,modeInfoMessage;

public:
	ModeBase(ContextEditor* c) : ctx(c){}
	ModeBase(const ModeBase&) = delete;
	ModeBase& operator=(const ModeBase&) = delete;

	std::string& GetErrorMessage(){
		return modeErrorMessage;
	}
	
	std::string& GetInfoMessage(){
		return modeInfoMessage;
	}

	virtual void ProcessTextAction(TextAction) = 0;
	virtual TextScreen& GetTextScreen(s32,s32) = 0;

	virtual bool ProcessCommand(const TokenVector&){return false;}

	virtual bool SaveAction(const OSInterface&){return true;}
	virtual bool OpenAction(const OSInterface&,std::string_view){return true;}
	virtual std::string_view GetPath(const OSInterface&){return {};}
	virtual void SetPath(const OSInterface&,std::string_view){}
	
	virtual void UpdateStyle(){}
	
	virtual bool Readonly(){
		return false;
	}

	virtual bool Modified(){
		return false;
	}

	virtual bool HasPath(){return false;}

	virtual std::string_view GetBufferName(){return {};}
	virtual std::string_view GetModeName() = 0;
	virtual std::string_view GetStatusBarText(){return {};}

	virtual ~ModeBase() = default;
};

