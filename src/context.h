#pragma once

#include "core.h"

#include "keybind.h"
#include "tokenizer.h"

#include "modes/mode.h"
#include "interfaces/os.h"
#include "interfaces/interface.h"

#include <vector>
#include <string>
#include <string_view>
#include <functional>

enum class EntryMode {
	None,
	Command,
	YesNo
};

class ContextEditor {
	const std::string entryPrefix = "> ";

	std::vector<Handle<ModeBase>> modes;
	Handle<TextInterfaceBase> interface;
	Handle<OSInterface> osInterface;
	
	Ref<TextBuffer> helpBuffer;

	size_t currentMode;
	
	std::string clipboardText;

	bool quit;
	EntryMode entryMode;
	std::string entryString;
	std::string yesNoMessage;
	std::string errorMessage;
	std::string infoMessage;
	
	ssize_t entryPos;

	std::function<void()> yesAction,noAction;

	bool WriteFileChecks(std::string_view);
	bool ReadFileChecks(std::string_view);
	
	void RunFile(std::string_view,bool = false);
	
	void MoveEntryPosLeft(size_t);
	void MoveEntryPosRight(size_t);
	
	void CancelCommand();
	void SubmitCommand();
	void AutocompleteCommand();
	bool ProcessCommand(const TokenVector&);

	void SetStyleOpts(std::string_view,std::string_view,std::string_view,std::string_view);
	void SetConfigVar(std::string_view,std::string_view);
	void SetConfigBind(std::string_view,const TokenVector&);
	
	std::string ConstructModeString(size_t);
	void DrawStatusBar(TextScreen&);
	
	std::string GetTabString(size_t,size_t);
	void DrawTabsBar(TextScreen&);

	bool ProcessKeyboardEvent(TextAction);
	void ProcessCommandEntry(TextAction);
	void ProcessYesNoEntry(TextAction);
	void Loop();
public:
	ContextEditor(const std::string& file);
	
	void BeginCommand(const std::string&);

	void CloseMode(size_t);
	void ForceCloseMode(size_t);
	void SaveMode(size_t);
	void SaveAsMode(std::string_view,size_t);
	void SetPathMode(std::string_view,size_t);
	void SetPathAndSaveMode(std::string_view,size_t);
	void SwitchMode(size_t);

	void NewMode();
	void OpenMode(std::string_view);
	
	void OpenHelpMode();
	
	std::string& GetClipboard();

	OSInterface* GetOSInterface() const;
};
