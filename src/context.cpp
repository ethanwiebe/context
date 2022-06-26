#include "context.h"

#include "platform.h"

#include "modes/editmode.h"
#include "command.h"
#include "util.h"

ContextEditor::ContextEditor(const std::string& file){
	SetKeybinds();
	LoadStyle();

	yesAction = [](){};
	noAction = [](){};

	quit = false;
	entryMode = EntryMode::None;
	entryPos = 0;
	
	errorMessage = {};
	
	helpBuffer = MakeRef<TextBuffer>();

	interface = Handle<TextInterfaceBase>(new CONTEXT_USER_INTERFACE());
	osInterface = Handle<OSInterface>(new CONTEXT_OS_INTERFACE());
	
	currentMode = 0;
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
	
	std::string configPath = osInterface->GetHomePath() + "/.ctxcfg";
	RunFile(configPath,true);
	
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
		DrawTabsBar(textScreen);

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

void ContextEditor::RunFile(std::string_view path,bool silent){
	if (ReadFileChecks(path)){
		size_t l = 0;
		auto settings = MakeRef<TextBuffer>();
		if (osInterface->ReadFileIntoTextBuffer(path,settings)){
			for (const auto& line : *settings){
				if (line.empty()){
					++l;
					continue;
				}
					
				entryString = line;
				SubmitCommand();
				if (!errorMessage.empty()){
					errorMessage = "Error at line "+std::to_string(l)
							+": "+errorMessage;
					break;
				}
				++l;
			}
		}
	} else {
		if (silent) errorMessage.clear();
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
			std::string copy = "saveas " + std::string(modes[currentMode]->GetPath(*osInterface));
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
			//SwitchMode(modes.size()-1);
			return true;
		case Action::Entry:
			BeginCommand("");
			return true;
		case Action::OpenMode:
			BeginCommand("open ");			
			return true;
		case Action::Help:
			OpenHelpMode();
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
		case Action::DeleteLine:
			entryString.clear();
			entryPos = 0;
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

std::string ParsePath(std::string_view path,const OSInterface& os){
	std::string parsed = {};
	
	if (path[0]=='~'){
		parsed += os.GetHomePath();
		path = {path.begin()+1,path.end()};
	}
	parsed += path;
	return parsed;
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
		std::string parsedPath = ParsePath(path,*osInterface);
		OpenMode(parsedPath);
		return true;
	} else if (tokens[0].token=="saveas"){
		std::string_view path = tokens[1].token;
		std::string parsedPath = ParsePath(path,*osInterface);
		SaveAsMode(parsedPath,currentMode);
		return true;
	} else if (tokens[0].token=="set"){
		std::string_view varName = tokens[1].token;
		std::string_view val = tokens[2].token;
		SetConfigVar(varName,val);
		return true;
	} else if (tokens[0].token=="bind"){
		std::string_view actionName = tokens[1].token;
		SetConfigBind(actionName,tokens);
		return true;
	} else if (tokens[0].token=="style"){
		std::string_view styleName = tokens[1].token;
		std::string_view fg = tokens[2].token;
		std::string_view bg = tokens[3].token;
		std::string_view opts = {};
		if (tokens.size()>4)
			opts = tokens[4].token;
		SetStyleOpts(styleName,fg,bg,opts);
		modes[currentMode]->UpdateStyle();
		return true;
	} else if (tokens[0].token=="source"){
		std::string_view path = tokens[1].token;
		std::string parsedPath = ParsePath(path,*osInterface);
		RunFile(parsedPath);
		return true;
	}
	
	return false;
}

#define ACTION(x) {#x, Action::x},
const std::map<std::string,Action> actionNameMap = {
	#include "actions.h"
};
#undef ACTION

#define STYLE(x) {#x, x##Style},
const std::map<std::string,TextStyle&> styleNameMap = {
	#include "stylenames.h"
};
#undef STYLE

const std::map<std::string,KeyEnum> keyNameMap = {
	{"Esc",KeyEnum::Escape},
	{"Tab",KeyEnum::Tab},
	{"Up",KeyEnum::Up},
	{"Left",KeyEnum::Left},
	{"Right",KeyEnum::Right},
	{"Down",KeyEnum::Down},
	{"Backspace",KeyEnum::Backspace},
	{"Del",KeyEnum::Delete},
	{"Enter",KeyEnum::Enter},
	{"Return",KeyEnum::Enter},
	{"Space",KeyEnum::Space},
	{"PageUp",KeyEnum::PageUp},
	{"PageDown",KeyEnum::PageDown},
	{"PgUp",KeyEnum::PageUp},
	{"PgDn",KeyEnum::PageDown},
	{"Home",KeyEnum::Home},
	{"End",KeyEnum::End},
	{"Ins",KeyEnum::Insert},
};

inline Action GetActionFromName(std::string_view name){
	std::string copied = std::string(name);
	if (actionNameMap.contains(copied))
		return actionNameMap.at(copied);
		
	return Action::None;
}

inline KeyModifier ParseMod(std::string_view& bindstr){
	if (bindstr.starts_with("ctrl")||bindstr.starts_with("CTRL")||bindstr.starts_with("Ctrl")){
		bindstr = {bindstr.begin()+4,bindstr.end()};
		return KeyModifier::Ctrl;
	}
	
	if (bindstr.starts_with("shift")||bindstr.starts_with("SHIFT")||bindstr.starts_with("Shift")){
		bindstr = {bindstr.begin()+5,bindstr.end()};
		return KeyModifier::Shift;
	}
	
	if (bindstr.starts_with("alt")||bindstr.starts_with("ALT")||bindstr.starts_with("Alt")){
		bindstr = {bindstr.begin()+3,bindstr.end()};
		return KeyModifier::Alt;
	}
	
	return KeyModifier::None;
}

inline KeyEnum ParseKey(std::string_view& bindstr){
	if (bindstr.size()==1){
		if (bindstr[0]>='A'&&bindstr[0]<='Z')
			return (KeyEnum)CharLower(bindstr[0]);
		if (IsPrintable(bindstr[0],KeyModifier::None))
			return (KeyEnum)bindstr[0];
	} else if (bindstr[0]=='F'||bindstr[0]=='f'){
		bindstr = {bindstr.begin()+1,bindstr.end()};
		if (bindstr.size()==2){
			if (bindstr == "10") return KeyEnum::F10;
			if (bindstr == "11") return KeyEnum::F11;
			if (bindstr == "12") return KeyEnum::F12;
		}
		if (bindstr == "1") return KeyEnum::F1;
		if (bindstr == "2") return KeyEnum::F2;
		if (bindstr == "3") return KeyEnum::F3;
		if (bindstr == "4") return KeyEnum::F4;
		if (bindstr == "5") return KeyEnum::F5;
		if (bindstr == "6") return KeyEnum::F6;
		if (bindstr == "7") return KeyEnum::F7;
		if (bindstr == "8") return KeyEnum::F8;
		if (bindstr == "9") return KeyEnum::F9;
	}
	
	for (const auto& k : keyNameMap){
		if (bindstr.starts_with(k.first))
			return k.second;
	}
	
	return KeyEnum::None;
}

inline bool ParseKeybind(std::string_view bindstr,KeyEnum& key,KeyModifier& mod){
	std::string_view::iterator pos;
	mod = KeyModifier::None;
	
	KeyModifier t;
	while (true){
		if (bindstr.size()==0) return false;
		pos = bindstr.begin();
		SkipWhitespace(bindstr,pos);
		t = ParseMod(bindstr);
		if (t==KeyModifier::None) break;
		if (bindstr[0]=='-') bindstr = {bindstr.begin()+1,bindstr.end()};
		mod = (KeyModifier)((s32)mod | (s32)t);
	}
	
	key = ParseKey(bindstr);
	
	if (key==KeyEnum::None) return false;
	
	LOG((char)key << ", " << (s32)key << ", " << (s32)mod);
	
	return true;
}

void ContextEditor::SetConfigBind(std::string_view actionName,const TokenVector& tokens){
	Action action = GetActionFromName(actionName);
	if (action==Action::None){
		errorMessage = "Unrecognized action: '";
		errorMessage += actionName;
		errorMessage += "'";
		return;
	}
	
	bool cleared = false;
	
	auto bindIt = tokens.begin();
	++bindIt; ++bindIt; //start after first arg
	KeyEnum key;
	KeyModifier mod;
	while (bindIt!=tokens.end()){
		if (ParseKeybind(bindIt->token,key,mod)){
			if (!cleared){
				cleared = true;
				gKeyBindings[action].clear();
			}
			ADD_BIND(action,key,mod);
		} else {
			errorMessage = "Could not parse keybind '";
			errorMessage += bindIt->token;
			errorMessage += "'";
			break;
		}
		
		++bindIt;
	}
	UpdateBinds();
}

bool ParseColor(std::string_view c,Color& color){
	if (c.size()!=6)
		return false;
	
	size_t n = strtol(c.data(),NULL,16);
	
	if (n==0&&c!="000000")
		return false;
	
	color.r = n>>16;
	color.g = (n>>8)&255;
	color.b = n&255;
		
	return true;
}

void ContextEditor::SetStyleOpts(std::string_view styleName,
		std::string_view fgStr,
		std::string_view bgStr,
		std::string_view opts){
	std::string copied = std::string(styleName);
	if (!styleNameMap.contains(copied)){
		errorMessage = "Unrecognized style name '";
		errorMessage += copied;
		errorMessage += "'!";
		return;
	}
	
	TextStyle& styleVar = styleNameMap.at(copied);
	
	Color fg,bg;
	if (!ParseColor(fgStr,fg)){
		errorMessage = "Cannot convert '";
		errorMessage += fgStr;
		errorMessage += "' into a color!";
		return;
	}
	
	if (!ParseColor(bgStr,bg)){
		errorMessage = "Cannot convert '";
		errorMessage += bgStr;
		errorMessage += "' into a color!";
		return;
	}
	
	u8 sf = StyleFlag::NoFlag;
	
	if (!opts.empty()){
		for (auto c : opts){
			if (c=='b'||c=='B')
				sf |= StyleFlag::Bold;
			else if (c=='u'||c=='U')
				sf |= StyleFlag::Underline;
			else {
				errorMessage = "Unrecognized style option '";
				errorMessage += c;
				errorMessage += "'!";
				return;
			}
		}
	}
	
	styleVar = {fg,bg,sf};
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
	std::string modeStatusBar = {};
	modeStatusBar += modes[currentMode]->GetStatusBarText();

	for (s32 x=0;x<w;++x){
		ts.SetAt(x,h-1,TextCell(' ',barStyle));
	}

	std::string& modeError = modes[currentMode]->GetErrorMessage();
	std::string& modeInfo = modes[currentMode]->GetInfoMessage();
	if (entryMode==EntryMode::Command){
		ts.RenderString(0,h-1,entryPrefix + entryString,barStyle);
		auto x = entryPrefix.size()+entryPos;
		auto cell = ts.GetAt(x,h-1);
		cell.style = cursorStyle;
		ts.SetAt(x,h-1,cell);
	} else if (entryMode==EntryMode::YesNo){
		ts.RenderString(0,h-1,yesNoMessage + " Y/N ",barStyle);
	} else {
		if (!modeStatusBar.empty())
			ts.RenderString(0,h-1,modeStatusBar,barStyle);

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
		} else if (!modeInfo.empty()){
			std::string formattedInfo = {};
			formattedInfo += modes[currentMode]->GetModeName();
			formattedInfo += ": ";
			formattedInfo += modeInfo;
			ts.RenderString(0,h-1,formattedInfo,barStyle);
			modeInfo.clear();
		}

		ts.RenderString(w-1-modeStr.size(),h-1,modeStr,barStyle);
	}
}

const size_t tabBarWidth = 16;

std::string ContextEditor::GetTabString(size_t index,size_t tabW){
	std::string s = " ";
	s += std::to_string(index+1);
	s += ' ';
	std::string name = std::string(modes[index]->GetBufferName());
	bool mod = modes[index]->Modified();
	size_t max;
	if (mod)
		max = tabW-s.size()-2; //extra room for *
	else
		max = tabW-s.size()-1;
		
	if (name.size() > max){
		s += StringPostEllipsis(name,max);
		if (mod) s += '*';
	} else {
		s += name;
		if (mod) s += '*';
	}
	
	while (s.size()<tabW)
		s += ' ';
	
	return s;
}

void ContextEditor::DrawTabsBar(TextScreen& ts){
	// how many tabs per page
	size_t tabCount = ts.GetWidth()/tabBarWidth;
	// current page of tabs
	size_t pageNum = currentMode/tabCount;
	
	for (size_t i=0;i<ts.GetWidth();++i){
		ts.SetAt(i,0,{' ',tabBarStyle});
	}
	
	for (size_t i=0;i<tabCount;++i){
		size_t currTab = i+pageNum*tabCount;
		if (currTab>=modes.size()) break;
		
		if (currTab==currentMode)
			ts.RenderString(i*tabBarWidth,0,GetTabString(currTab,tabBarWidth),tabBarSelectStyle);
		else
			ts.RenderString(i*tabBarWidth,0,GetTabString(currTab,tabBarWidth),tabBarStyle);
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

	if (modes[index]->Modified()){
		if (!modes[index]->SaveAction(*osInterface)){
			if (modes[index]->Readonly()){
				errorMessage = "File is readonly!";
			} else {
				errorMessage = "Could not save!";
			}
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
		errorMessage = "File '";
		errorMessage += path;
		errorMessage += "' does not exist!";
		return false;
	}
	if (!osInterface->PathIsFile(path)){
		errorMessage = "Path '";
		errorMessage += path;
		errorMessage += "' is not a file!";
		return false;
	}
	if (!osInterface->FileIsReadable(path)){
		errorMessage = "File '";
		errorMessage += path;
		errorMessage += "' is not readable!";
		return false;
	}
	
	return true;
}

void ContextEditor::NewMode(){
	if (modes.size()!=0){
		modes.insert(modes.begin()+currentMode+1,Handle<ModeBase>(new EditMode(this)));
		++currentMode;
	} else {
		modes.push_back(Handle<ModeBase>(new EditMode(this)));
		currentMode = 0;
	}
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
		modes.insert(modes.begin()+currentMode+1,std::move(openedMode));
		//modes.push_back(std::move(openedMode));
		//currentMode = modes.size()-1;
		++currentMode;
	} else {
		errorMessage = "Could not open '";
		errorMessage += copiedPath;
		errorMessage += "'!";
	}
}

#include "help.h"

void ContextEditor::OpenHelpMode(){
	EditMode* helpMode = new EditMode(this);
	
	helpMode->SetHelp(MakeRef<TextBuffer>(gHelpBuffer));
	
	Handle<ModeBase> m = Handle<ModeBase>(helpMode);
	
	modes.push_back(std::move(m));
	currentMode = modes.size()-1;
}

void ContextEditor::SaveAsMode(std::string_view path,size_t index){
	bool exists = osInterface->PathExists(path);
	if (!WriteFileChecks(path))
		return;

	if (exists){
		entryMode = EntryMode::YesNo;
		yesNoMessage = "Overwrite?";
		yesAction = std::bind(&ContextEditor::SetPathAndSaveMode,
				this,std::string(path),index);
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
	if (val=="true"||val=="1")
		var = true;
	else if (val=="false"||val=="0")
		var = false;
	else
		return false;
	
	return true;
}

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
	} else if (name=="style"){
		size_t n = strtol(val.data(),NULL,10);
		SaveStyle();
		gConfig.style = n;
		LoadStyle();
		modes[currentMode]->UpdateStyle();
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
		if (!SetMultiMode(gConfig.moveMode,val))
			errorMessage = "moveMode must be one of 'multi', 'word', or 'pascal'";
	} else if (name=="deleteMode"){
		if (!SetMultiMode(gConfig.deleteMode,val))
			errorMessage = "deleteMode must be one of 'multi', 'word', or 'pascal'";
	}
}

OSInterface* ContextEditor::GetOSInterface() const {
	return osInterface.get();
}

std::string& ContextEditor::GetClipboard(){
	return clipboardText;
}
