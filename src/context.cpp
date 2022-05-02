#include "context.h"

ContextEditor::ContextEditor(){
	SetKeybinds();

	yesAction = [](){};
	noAction = [](){};

	quit = false;
	entryMode = EntryMode::None;
	
	errorMessage = {};

	interface = Handle<TextInterfaceBase>(new CursesInterface());	
	osInterface = Handle<OSInterface>(new LinuxOSImpl());

	NewMode();
	currentMode = 0;

	Loop();
}

void ContextEditor::Loop(){
	KeyboardEvent* event;
	TextAction textAction;
	TextScreen textScreen;
	while (!quit){
		textScreen = modes[currentMode]->GetTextScreen(interface->GetWidth(),interface->GetHeight());
		DrawStatusBar(textScreen);

		interface->RenderScreen(textScreen);

		if ((event = interface->GetKeyboardEvent())){
			textAction = GetTextActionFromKey((KeyEnum)event->key,(KeyModifier)event->mod);
			
			if (entryMode==EntryMode::Command){
				ProcessCommandEntry(textAction);
			} else if (entryMode==EntryMode::YesNo){
				ProcessYesNoEntry(textAction);
			} else if (ActionIsCommand(textAction.action)){
				ProcessKeyboardEvent(textAction.action);
			} else {
				modes[currentMode]->ProcessTextAction(textAction);
			}

		}
	}
}

void ContextEditor::ProcessKeyboardEvent(Action action){
	switch (action){
		case Action::CloseMode:
			CloseMode(currentMode);
			break;
		case Action::SaveMode:
			SaveMode(currentMode);
			break;
		case Action::NextMode:
			SwitchMode(currentMode+1);
			break;
		case Action::PreviousMode:
			SwitchMode(currentMode-1);
			break;
		case Action::NewMode:
			NewMode();
			SwitchMode(modes.size()-1);
			break;
		case Action::OpenMode:
			BeginCommand();
			entryString = "open ";
			break;

		default:
			break;
	}
}

void ContextEditor::ProcessCommandEntry(TextAction textAction){
	switch (textAction.action){
		case Action::InsertChar:
			entryString.push_back(textAction.character);
			break;
		case Action::DeletePreviousChar:
			if (!entryString.empty()) entryString.pop_back();
			break;
		case Action::InsertLine:
			SubmitCommand();
			break;
		case Action::Escape:
			CancelCommand();
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
			} else if (textAction.character=='n'||textAction.character=='N'){
				noAction();
			}
			entryMode = EntryMode::None;
			break;

		case Action::Escape:
			entryMode = EntryMode::None;
			break;

		default:
			break;
	}
}

void ContextEditor::BeginCommand(){
	entryString.clear();
	entryMode = EntryMode::Command;
}

void ContextEditor::CancelCommand(){
	entryMode = EntryMode::None;
}

void ContextEditor::SubmitCommand(){
	entryMode = EntryMode::None;
	ProcessCommand(entryString);
}

inline void LogTokens(TokenVector tokens){
	for (const auto& token : tokens){
		logger << (s32)token.type << ":" << token.token << ",";
	}
	logger << "\n";
}

void ContextEditor::ProcessCommand(std::string_view sv){
	CommandTokenizer ct(sv);
	TokenVector tokens = ct.GetTokens();
	
	LogTokens(tokens);	
	
	if (tokens.size()>=2){
		if (tokens[0].type==TokenType::Name&&tokens[0].token=="open"){
			std::string_view path = tokens[1].token;
			OpenMode(path);
		} else if (tokens[0].type==TokenType::Name&&tokens[0].token=="saveas"){
			std::string_view path = tokens[1].token;
			SaveAsMode(path,currentMode);
		}
	}
}

std::string ContextEditor::ConstructModeString(size_t index){
	std::string modeStr = {};
	modeStr += modes[index]->GetBufferName();

	if (modes[index]->Modified())
		modeStr += '*';

	if (modes[index]->Readonly())
		modeStr += " (readonly)";

	modeStr += " (";
	modeStr += std::to_string(index+1) + "/" + std::to_string(modes.size()) + ")";
	
	return modeStr;
}

void ContextEditor::DrawStatusBar(TextScreen& ts){
	s32 w,h;
	w = ts.GetWidth();
	h = ts.GetHeight();

	std::string modeStr = ConstructModeString(currentMode);

	for (s32 x=0;x<w;++x){
		ts.SetAt(x,h-1,TextCell(' ',defaultStyle));
	}

	std::string modeError = modes[currentMode]->GetErrorMessage();
	if (entryMode==EntryMode::Command){
		ts.RenderString(0,h-1,entryPrefix + entryString);
	} else if (entryMode==EntryMode::YesNo){
		ts.RenderString(0,h-1,yesNoMessage + " Y/N ");
	} else {
		if (!errorMessage.empty()){
			ts.RenderString(0,h-1,errorMessage,errorStyle);
			errorMessage.clear();
		} else if (!modeError.empty()){
			ts.RenderString(0,h-1,modeError,errorStyle);
			modeError.clear();
		}

		ts.RenderString(w-1-modeStr.size(),h-1,modeStr);
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
	if (!modes[index]->HasSavePath()||modes[index]->Readonly()){
		entryMode = EntryMode::Command;
		entryString = "saveas ";
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
	if (!ReadFileChecks(path))
		return;

	Handle<ModeBase> openedMode = Handle<ModeBase>(new EditMode(this));
	if (openedMode->OpenAction(*osInterface,path)){
		modes.push_back(std::move(openedMode));
		currentMode = modes.size()-1;
	} else {
		errorMessage = "Could not open ";
		errorMessage += path;
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
	modes[index]->SetPath(*osInterface,path);
}

OSInterface* ContextEditor::GetOSInterface() const {
	return osInterface.get();
}
