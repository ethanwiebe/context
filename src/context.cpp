#include "context.h"

#include "platform.h"

#include "modes/editmode.h"
#include "command.h"

ContextEditor::ContextEditor(const std::string& file){
	SetKeybinds();

	yesAction = [](){};
	noAction = [](){};

	quit = false;
	entryMode = EntryMode::None;
	entryPos = 0;
	
	errorMessage = {};

	interface = Handle<TextInterfaceBase>(new CONTEXT_USER_INTERFACE());
	osInterface = Handle<OSInterface>(new CONTEXT_OS_INTERFACE());
	
	if (!file.empty()){
		if (osInterface->PathExists(file)){
			OpenMode(file);
			if (!errorMessage.empty())
				NewMode();
		} else {
			NewMode();
			modes[0]->SetPath(*osInterface,file);
		}
	} else {
		NewMode();
	}

	currentMode = 0;

	std::string configPath = osInterface->GetConfigFilePath();
	if (osInterface->FileIsReadable(configPath)){
		auto settings = MakeRef<TextBuffer>();
		if (osInterface->ReadFileIntoTextBuffer(configPath,settings)){
			for (const auto& line : *settings){
				if (line.empty())
					continue;
					
				entryString = line;
				SubmitCommand();
			}
		}
	}
	
	Loop();
}

void ContextEditor::Loop(){
	KeyboardEvent* event;
	TextAction textAction;
	while (!quit){
		TextScreen& textScreen = modes[currentMode]->GetTextScreen(
			interface->GetWidth(),interface->GetHeight()
		);
		DrawStatusBar(textScreen);

		interface->RenderScreen(textScreen);

		if ((event = interface->GetKeyboardEvent())){
			textAction = GetTextActionFromKey((KeyEnum)event->key,(KeyModifier)event->mod);
			
			if (entryMode==EntryMode::Command){
				ProcessCommandEntry(textAction);
			} else if (entryMode==EntryMode::YesNo){
				ProcessYesNoEntry(textAction);
			} else {
				if (!ProcessKeyboardEvent(textAction))
					modes[currentMode]->ProcessTextAction(textAction);
			}
		}
	}
}

void ContextEditor::MoveEntryPosLeft(size_t count){
	entryPos -= count;
	entryPos = std::max(entryPos,(ssize_t)0);
}

void ContextEditor::MoveEntryPosRight(size_t count){
	entryPos += count;
	entryPos = std::min(entryPos,(ssize_t)entryString.size());
}

bool ContextEditor::ProcessKeyboardEvent(TextAction action){
	switch (action.action){
		case Action::CloseMode:
			CloseMode(currentMode);
			return true;
		case Action::SaveMode:
			SaveMode(currentMode);
			return true;
		case Action::RenameMode: {
			std::string copy = "setpath " + std::string(modes[currentMode]->GetPath(*osInterface));
			BeginCommand(copy);
			return true;
		}
		case Action::NextMode:
			SwitchMode(currentMode+1);
			return true;
		case Action::PreviousMode:
			SwitchMode(currentMode-1);
			return true;
		case Action::NewMode:
			NewMode();
			SwitchMode(modes.size()-1);
			return true;
		case Action::Entry:
			BeginCommand("");
			return true;
		case Action::OpenMode:
			BeginCommand("open ");
			
			return true;

		default:
			break;
	}
	return false;
}

void ContextEditor::ProcessCommandEntry(TextAction textAction){
	switch (textAction.action){
		case Action::InsertChar:
			entryString.insert(entryPos,1,textAction.character);
			MoveEntryPosRight(1);
			break;
		case Action::DeletePreviousChar:
			if (!entryString.empty()&&entryPos!=0){
				MoveEntryPosLeft(1);
				entryString.erase(entryPos,1);
			}
			break;
		case Action::DeleteCurrentChar:
			if (!entryString.empty()&&entryPos!=(ssize_t)entryString.size()){
				entryString.erase(entryPos,1);
			}
			break;
		case Action::InsertLine:
			SubmitCommand();
			break;
		case Action::Escape:
		case Action::CloseMode:
			CancelCommand();
			break;
		case Action::Tab:
			AutocompleteCommand();
			break;
		case Action::MoveLeftMulti:
		case Action::MoveLeftChar:
			MoveEntryPosLeft(textAction.num);
			break;
		case Action::MoveRightMulti:
		case Action::MoveRightChar:
			MoveEntryPosRight(textAction.num);
			break;
		case Action::MoveToLineStart:
			entryPos = 0;
			break;
		case Action::MoveToLineEnd:
			entryPos = entryString.size();
			break;
			
		default:
			break;
	}
}

void ContextEditor::ProcessYesNoEntry(TextAction textAction){
	switch (textAction.action){
		case Action::InsertChar:
			if (textAction.character=='y'||textAction.character=='Y'){
				yesAction();
				entryMode = EntryMode::None;
			} else if (textAction.character=='n'||textAction.character=='N'){
				noAction();
				entryMode = EntryMode::None;
			}
			break;

		case Action::Escape:
			entryMode = EntryMode::None;
			break;

		default:
			break;
	}
}

void ContextEditor::AutocompleteCommand(){
	CommandTokenizer ct(entryString);
	TokenVector tokens = ct.GetTokens();
	
	if (tokens[0].token=="open"&&tokens.size()>1){
		std::string sub = entryString.substr(tokens[1].col,tokens[1].token.size());
		osInterface->AutocompletePath(sub);
		entryString = entryString.substr(0,tokens[1].col) + sub +
				entryString.substr(tokens[1].col+tokens[1].token.size());
		entryPos = tokens[1].col+sub.size();
	}
}

void ContextEditor::BeginCommand(const std::string& s){
	entryString.clear();
	entryString = s;
	entryPos = entryString.size();
	entryMode = EntryMode::Command;
}

void ContextEditor::CancelCommand(){
	entryMode = EntryMode::None;
}

void ContextEditor::SubmitCommand(){
	entryMode = EntryMode::None;
	
	CommandTokenizer ct(entryString);
	TokenVector tokens = ct.GetTokens();
	
	if (!ProcessCommand(tokens)){
		if (!modes[currentMode]->ProcessCommand(tokens)){
			errorMessage = "Unrecognized command '";
			errorMessage += tokens[0].token;
			errorMessage += "'";
		}
	}
}

inline void LogTokens(TokenVector tokens){
	for (const auto& token : tokens){
		LOG((s32)token.type << ":'" << token.token << "' @" << token.col << ",");
	}
}

bool ContextEditor::ProcessCommand(const TokenVector& tokens){
	if (!tokens.size()) return true;
	LogTokens(tokens);
	
	const Command* cmd;
	if (!GetCommandFromName(tokens[0].token,&cmd)){
		return false;
	}
	
	auto count = GetReqArgCount(*cmd);
	if (count>tokens.size()-1){
		errorMessage = "Expected ";
		errorMessage += std::to_string(count);
		errorMessage += " args, got ";
		errorMessage += std::to_string(tokens.size()-1);
		return true;
	}
	
	if (tokens[0].token=="open"){
		std::string_view path = tokens[1].token;
		OpenMode(path);
		return true;
	} else if (tokens[0].token=="saveas"){
		std::string_view path = tokens[1].token;
		SaveAsMode(path,currentMode);
		return true;
	} else if (tokens[0].token=="setpath"){
		std::string_view path = tokens[1].token;
		SetPathMode(path,currentMode);
		return true;
	} else if (tokens[0].token=="set"){
		std::string_view varName = tokens[1].token;
		std::string_view val = tokens[2].token;
		SetConfigVar(varName,val);
		return true;
	}
	
	return false;
}

std::string ContextEditor::ConstructModeString(size_t index){
	std::string modeStr = {};
	modeStr += modes[index]->GetBufferName();

	if (modes[index]->Modified())
		modeStr += '*';

	if (modes[index]->Readonly())
		modeStr += " (readonly)";

	modeStr += " (";
	modeStr += std::to_string(index+1) + '/' + std::to_string(modes.size()) + ")";
	
	return modeStr;
}

void ContextEditor::DrawStatusBar(TextScreen& ts){
	s32 w,h;
	w = ts.GetWidth();
	h = ts.GetHeight();

	std::string modeStr = ConstructModeString(currentMode);
	std::string modeInfo = {};
	modeInfo += modes[currentMode]->GetStatusBarText();

	for (s32 x=0;x<w;++x){
		ts.SetAt(x,h-1,TextCell(' ',barStyle));
	}

	std::string& modeError = modes[currentMode]->GetErrorMessage();
	if (entryMode==EntryMode::Command){
		ts.RenderString(0,h-1,entryPrefix + entryString,barStyle);
		auto x = entryPrefix.size()+entryPos;
		auto cell = ts.GetAt(x,h-1);
		cell.style = cursorStyle;
		ts.SetAt(x,h-1,cell);
	} else if (entryMode==EntryMode::YesNo){
		ts.RenderString(0,h-1,yesNoMessage + " Y/N ",barStyle);
	} else {
		if (!modeInfo.empty())
			ts.RenderString(0,h-1,modeInfo,barStyle);

		if (!errorMessage.empty()){
			ts.RenderString(0,h-1,errorMessage,errorStyle);
			errorMessage.clear();
		} else if (!modeError.empty()){
			std::string formattedError = {};
			formattedError += modes[currentMode]->GetModeName();
			formattedError += ": ";
			formattedError += modeError;
			ts.RenderString(0,h-1,formattedError,errorStyle);
			modeError.clear();
		}

		ts.RenderString(w-1-modeStr.size(),h-1,modeStr,barStyle);
	}
}

void ContextEditor::CloseMode(size_t index){
	if (modes[index]->Modified()){
		entryMode = EntryMode::YesNo;
		yesNoMessage = "Close without saving?";
		yesAction = std::bind(&ContextEditor::ForceCloseMode,this,index);
		noAction = [](){};
		return;
	}
	
	ForceCloseMode(index);
}

void ContextEditor::ForceCloseMode(size_t index){
	if (modes.size()==1){
		quit = true;
	}

	if (index==currentMode&&currentMode){
		--currentMode;
	}

	auto it = modes.begin();
	while (index--){
		++it;
	}

	modes.erase(it);
}

void ContextEditor::SaveMode(size_t index){
	if (!modes[index]->HasPath()||modes[index]->Readonly()){
		BeginCommand("saveas ");
		return;
	}

	if (!modes[index]->SaveAction(*osInterface)){
		if (modes[index]->Readonly()){
			errorMessage = "File is readonly!";
		} else {
			errorMessage = "Could not save!";
		}
	}
}

void ContextEditor::SwitchMode(size_t index){
	currentMode = (index+modes.size())%modes.size();
}

bool ContextEditor::WriteFileChecks(std::string_view path){
	if (!osInterface->FileIsWritable(path)){
		errorMessage = "File ";
		errorMessage += path;
		errorMessage += " is not writable!";
		return false;
	}

	return true;
}

bool ContextEditor::ReadFileChecks(std::string_view path){
	if (!osInterface->PathExists(path)){
		errorMessage = "File ";
		errorMessage += path;
		errorMessage += " does not exist!";
		return false;
	}
	if (!osInterface->PathIsFile(path)){
		errorMessage = "Path ";
		errorMessage += path;
		errorMessage += " is not a file!";
		return false;
	}
	if (!osInterface->FileIsReadable(path)){
		errorMessage = "File ";
		errorMessage += path;
		errorMessage += " is not readable!";
		return false;
	}
	
	return true;
}

void ContextEditor::NewMode(){
	modes.push_back(Handle<ModeBase>(new EditMode(this)));
}

void ContextEditor::OpenMode(std::string_view path){
	std::string copiedPath = std::string(path);
	
#ifdef _WIN32
	FixWindowsPath(copiedPath);
#endif

	for (size_t i=0;i<modes.size();++i){
		if (osInterface->PathsAreSame(modes[i]->GetPath(*osInterface),copiedPath)){
			SwitchMode(i);
			return;
		}
	}

	if (!ReadFileChecks(copiedPath))
		return;

	Handle<ModeBase> openedMode = Handle<ModeBase>(new EditMode(this));
	if (openedMode->OpenAction(*osInterface,copiedPath)){
		modes.push_back(std::move(openedMode));
		currentMode = modes.size()-1;
	} else {
		errorMessage = "Could not open ";
		errorMessage += copiedPath;
		errorMessage += "!";
	}
}

void ContextEditor::SaveAsMode(std::string_view path,size_t index){
	bool exists = osInterface->PathExists(path);
	if (!WriteFileChecks(path))
		return;

	if (exists){
		entryMode = EntryMode::YesNo;
		yesNoMessage = "Overwrite?";
		yesAction = std::bind(&ContextEditor::SetPathAndSaveMode,this,path,index);
		noAction = [](){};
		return;
	}

	SetPathAndSaveMode(path,index);
}

void ContextEditor::SetPathAndSaveMode(std::string_view path,size_t index){
	SetPathMode(path,index);
	SaveMode(index);
}

void ContextEditor::SetPathMode(std::string_view path,size_t index){
	std::string copiedPath = std::string(path);
	
#ifdef _WIN32
	FixWindowsPath(copiedPath);
#endif
	
	modes[index]->SetPath(*osInterface,copiedPath);
}

inline bool SetBool(bool& var,std::string_view val){
	if (val=="true"||val=="1"){
		var = true;
		return true;
	} else if (val=="false"||val=="0"){
		var = false;
		return true;
	}
	
	return false;
}

void ContextEditor::SetConfigVar(std::string_view name,std::string_view val){
	if (!NameIsConfigVar(name)){
		errorMessage = {};
		errorMessage += "'";
		errorMessage += name;
		errorMessage += "' is not a config var!";
		return;
	}
	
	if (name=="tabSize"){
		ssize_t n = strtol(val.data(),NULL,10);
		if (n>0){
			gConfig.tabSize = n;
		} else {
			errorMessage = "tabSize must be a positive integer";
		}
	} else if (name=="cursorMoveHeight"){
		ssize_t n = strtol(val.data(),NULL,10);
		if (n>=0){
			gConfig.cursorMoveHeight = n;
		} else {
			errorMessage = "cursorMoveHeight must be a non-negative integer";
		}
	} else if (name=="multiAmount"){
		ssize_t n = strtol(val.data(),NULL,10);
		if (n>0){
			gConfig.multiAmount = n;
		} else {
			errorMessage = "multiAmount must be a positive integer";
		}
	} else if (name=="displayLineNumbers"){
		if (!SetBool(gConfig.displayLineNumbers,val)){
			errorMessage = "displayLineNumbers must be a boolean value";
		}
	} else if (name=="autoIndent"){
		if (!SetBool(gConfig.autoIndent,val)){
			errorMessage = "autoIndent must be a boolean value";
		}
	} else if (name=="cursorLock"){
		if (!SetBool(gConfig.cursorLock,val)){
			errorMessage = "cursorLock must be a boolean value";
		}
	} else if (name=="cursorWrap"){
		if (!SetBool(gConfig.cursorWrap,val)){
			errorMessage = "cursorWrap must be a boolean value";
		}
	} else if (name=="smartHome"){
		if (!SetBool(gConfig.smartHome,val)){
			errorMessage = "smartHome must be a boolean value";
		}
	} else if (name=="tabMode"){
		if (val=="tabs")
			gConfig.tabMode = TabMode::Tabs;
		else if (val=="spaces")
			gConfig.tabMode = TabMode::Spaces;
		else
			errorMessage = "tabMode must be either 'tabs' or 'spaces'";
	} else if (name=="moveMode"){
		if (val=="multi")
			gConfig.moveMode = MultiMode::Multi;
		else if (val=="word")
			gConfig.moveMode = MultiMode::Word;
		else if (val=="pascal")
			gConfig.moveMode = MultiMode::PascalWord;
		else
			errorMessage = "moveMode must be one of 'multi', 'word', or 'pascal'";
	} else if (name=="deleteMode"){
		if (val=="multi")
			gConfig.deleteMode = MultiMode::Multi;
		else if (val=="word")
			gConfig.deleteMode = MultiMode::Word;
		else if (val=="pascal")
			gConfig.deleteMode = MultiMode::PascalWord;
		else
			errorMessage = "deleteMode must be one of 'multi', 'word', or 'pascal'";
	}
}

OSInterface* ContextEditor::GetOSInterface() const {
	return osInterface.get();
}

std::string& ContextEditor::GetClipboard(){
	return clipboardText;
}
