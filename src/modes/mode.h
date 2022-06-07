#pragma once

#include "../core.h"

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
	std::string modeErrorMessage;

	bool readonly,modified;
public:
	ModeBase(ContextEditor* c) : ctx(c){}
	ModeBase(const ModeBase&) = delete;
	ModeBase& operator=(const ModeBase&) = delete;

	std::string& GetErrorMessage(){
		return modeErrorMessage;
	}

	virtual void ProcessTextAction(TextAction) = 0;
	virtual TextScreen& GetTextScreen(s32,s32) = 0;

	virtual void ProcessCommand(const TokenVector&){}

	virtual bool SaveAction(const OSInterface&){return true;}
	virtual bool OpenAction(const OSInterface&,std::string_view){return true;}
	virtual void SetPath(const OSInterface&,std::string_view){}
	
	virtual bool Readonly(){
		return readonly;
	}

	virtual bool Modified(){
		return modified;
	}

	virtual bool HasPath(){return false;}

	virtual std::string_view GetBufferName(){return {};}
	virtual std::string_view GetModeName() = 0;
	virtual std::string_view GetStatusBarText(){return {};}

	virtual ~ModeBase() = default;
};

