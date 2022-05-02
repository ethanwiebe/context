#pragma once

#include "core.h"

#include "keybind.h"
#include "tokenizer.h"

#include "modes/editmode.h"
#include "interfaces/oslinux.h"
#include "interfaces/interface_curses.h"

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
	const std::string entryPrefix = "ctx> ";

	std::vector<Handle<ModeBase>> modes;
	Handle<TextInterfaceBase> interface;
	Handle<OSInterface> osInterface;

	size_t currentMode;

	bool quit;
	EntryMode entryMode;
//	bool commandEntry;
	std::string entryString;
	std::string yesNoMessage;
	std::string errorMessage;

	std::function<void()> yesAction,noAction;

	bool WriteFileChecks(std::string_view);
	bool ReadFileChecks(std::string_view);

	void BeginCommand();
	void CancelCommand();
	void SubmitCommand();
	void ProcessCommand(std::string_view);

	std::string ConstructModeString(size_t);
	void DrawStatusBar(TextScreen&);

	void ProcessKeyboardEvent(Action);
	void ProcessCommandEntry(TextAction);
	void ProcessYesNoEntry(TextAction);
	void Loop();
public:
	ContextEditor();

	void CloseMode(size_t);
	void ForceCloseMode(size_t);
	void SaveMode(size_t);
	void SaveAsMode(std::string_view,size_t);
	void SetPathMode(std::string_view,size_t);
	void SetPathAndSaveMode(std::string_view,size_t);
	void SwitchMode(size_t);

	void NewMode();
	void OpenMode(std::string_view);

	OSInterface* GetOSInterface() const;
};
