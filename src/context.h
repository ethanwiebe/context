#pragma once

#include "core.h"

#include "keybind.h"
#include "tokenizer.h"
#include "async.h"
#include "message.h"

#include "modes/mode.h"
#include "interfaces/os.h"
#include "interfaces/interface.h"
#include "logger.h"

#include <vector>
#include <string>
#include <string_view>
#include <functional>
#include <mutex>
#include <map>


enum class EntryMode {
	None,
	Command,
	YesNo,
	Proc
};

typedef std::vector<std::string> Procedure;

struct ExtData {
	ModeIndex mode;
	std::string proc;
};

class ContextEditor {
	const std::string entryPrefix = "> ";

	TextScreen screen;

	std::vector<Handle<ModeBase>> modes;
	Handle<TextInterfaceBase> interface;
	Handle<OSInterface> osInterface;
	
	Ref<TextBuffer> helpBuffer;

	size_t currentMode;
	
	std::string clipboardText;
	
	bool willUpdate;
	bool silentUpdate;
	
	size_t runFileDepth = 0;
	size_t procDepth = 0;

	bool quit;
	EntryMode entryMode;
	TextBuffer commandBuffer;
	std::string entryString;
	std::string yesNoMessage;
	MessageQueue errorMessage;
	MessageQueue infoMessage;
	bool error = false;
	
	ssize_t entryPos;
	
	std::map<std::string,Procedure> procs;
	std::map<std::string,ExtData> extData;
	std::map<ModeIndex,std::string> modeHooks;
	
	std::string currentProcName;
	Procedure currentProc;

	std::function<void()> yesAction,noAction;
	
	KeyboardEvent* currentEvent;
	
	std::mutex asyncMutex,updateMutex;
	size_t asyncIndex;
	std::vector<AsyncData> asyncState;
	
	inline void PushError(std::string&& msg){
		LOG("ERROR: (" << (s64)errorMessage.msgs.size()<<") "<<msg<<"\n");
		errorMessage.Push(std::move(msg));
		error = true;
	}
	
	inline void ModeNotFoundError(const std::string& name){
		PushError("Unrecognized mode '"+name+"'!");
	}
	
	inline void ProcNotFoundError(const std::string& name){
		PushError("No proc named '"+name+"' found!");
	}

	bool WriteFileChecks(std::string_view);
	bool ReadFileChecks(std::string_view);
	
	void RunFile(std::string_view,bool = false);
	void RunProc(const std::string&);
	
	void MoveEntryPosLeft(size_t);
	void MoveEntryPosRight(size_t);
	
	TokenVector GetCommandTokens();
	void CancelCommand();
	void SubmitCommand();
	void AutocompleteCommand();
	bool ProcessCommand(const TokenVector&);

	void SetStyleOpts(std::string_view,std::string_view,std::string_view,std::string_view);
	void SetConfigVar(const TokenVector&);
	void SetModeConfigVar(const TokenVector&);
	void SetConfigBind(const TokenVector&);
	
	void SetExtensionData(const std::string& ext,ModeIndex mode,const std::string& proc);
	
	void ProcessExtension(const std::string& path,ModeIndex&,std::string&);
	
	std::string ConstructModeString(size_t);
	void DrawStatusBar();
	
	std::string GetTabString(size_t,size_t);
	void DrawTabsBar();

	bool ProcessKeyboardEvent(KeyEnum,KeyModifier);
	void ProcessCommandEntry(KeyEnum,KeyModifier);
	void ProcessYesNoEntry(KeyEnum);
	void Loop();
	inline void Update();
	inline void Render();
public:
	ContextEditor(const std::string& file);
	
	KeyboardEvent* GetCurrentEvent() const {
		return currentEvent;
	}
	
	void BeginCommand(const std::string&);

	void CloseMode(size_t);
	void ForceCloseMode(size_t);
	void SaveMode(size_t);
	void SaveAsMode(std::string_view,size_t);
	void SetPathMode(std::string_view,size_t);
	void SetPathAndSaveMode(std::string_view,size_t);
	void SwitchMode(size_t);
	void MoveMode(size_t,size_t);

	void NewMode();
	void OpenMode(std::string_view);
	
	void OpenHelpMode();
	
	// internal async helpers
	void AsyncFinished(size_t);
	bool GetAsyncCancel(size_t) const;
	
	// use for async tasks
	size_t StartAsyncTask(const AsyncContext&);
	bool IsAsyncTaskDone(size_t);
	void CancelAsyncTask(size_t);
	
	std::string& GetClipboard();

	OSInterface* GetOSInterface() const;
};
