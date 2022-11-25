#pragma once

#include "core.h"

#include "../key.h"
#include "../message.h"
#include "../textscreen.h"
#include "../tokenizer.h"
#include "../interfaces/os.h"

class ContextEditor;

typedef s32 ModeIndex;

#define MODE_NOT_FOUND (-1)

class ModeBase {
protected:
	ContextEditor* ctx;

	s32 screenWidth,screenHeight;
	MessageQueue modeErrorMessage,modeInfoMessage;

public:
	bool autoReload = false;
	s64 accessTime = 0;

	ModeBase(ContextEditor* c) : ctx(c){}
	ModeBase(const ModeBase&) = delete;
	ModeBase& operator=(const ModeBase&) = delete;

	MessageQueue& GetErrorMessage(){
		return modeErrorMessage;
	}
	
	MessageQueue& GetInfoMessage(){
		return modeInfoMessage;
	}

	virtual void ProcessKeyboardEvent(KeyEnum,KeyModifier) = 0;
	virtual TextScreen& GetTextScreen(s32,s32) = 0;

	virtual bool ProcessCommand(const TokenVector&){return false;}
	virtual bool SetConfigVar(const TokenVector&){return false;}

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

	virtual std::string_view GetModeName() = 0;
	virtual std::string_view GetStatusBarText(){return {};}
	
	virtual ~ModeBase() = default;
};

ModeIndex ModeNameToIndex(const char*);
const char*  ModeIndexToName(ModeIndex);
ModeBase* CreateMode(ModeIndex,ContextEditor*);

