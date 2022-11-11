#include "context.h"

#include "platform.h"

#include "modes/edit/editmode.h"
#include "command.h"
#include "util.h"
#include "profiler.h"

// for entry movement
#include "modes/edit/editbinds.h"

#include <thread>
#include <mutex>
#include <assert.h>

ContextEditor::ContextEditor(const std::string& file){
	screen = {};
	screen.SetSize(1,1);

	SetGlobalBinds();
	UpdateBinds(gBinds.at("ctx"));
	LineModeBase::RegisterBinds();
	UpdateBinds(gBinds.at("edit"));
	LoadStyle();
	
	commandBuffer.lines.resize(1);
	commandBuffer.SetLine(commandBuffer.begin(),"");
	
	yesAction = [](){};
	noAction = [](){};

	asyncIndex = 0;
	asyncState = {};
	
	willUpdate = true;

	quit = false;
	silentUpdate = false;
	entryMode = EntryMode::None;
	entryPos = 0;
	
	errorMessage = {"",false};
	infoMessage = {"",false};
	
	helpBuffer = MakeRef<TextBuffer>();

	interface = Handle<TextInterfaceBase>(new CONTEXT_USER_INTERFACE());
	osInterface = Handle<OSInterface>(new CONTEXT_OS_INTERFACE());
	
	currentMode = 0;
	if (!file.empty()){
		if (osInterface->PathExists(file)){
			OpenMode(file);
			if (!errorMessage.Empty())
				NewMode();
		} else {
			NewMode();
			modes[0]->SetPath(*osInterface,file);
		}
	} else {
		NewMode();
	}
	
	std::string configPath = osInterface->GetHomePath() + "/.ctxcfg";
	LOG(configPath);
	RunFile(configPath,true);
	
	Loop();
}

inline void ContextEditor::Render(){
	ProfileThis p{};
	
	if (interface->GetWidth()!=screen.GetWidth()||
		interface->GetHeight()!=screen.GetHeight()){
		screen.SetSize(interface->GetWidth(),interface->GetHeight());
	}
	
	TextScreen& textScreen = modes[currentMode]->GetTextScreen(
		interface->GetWidth(),interface->GetHeight()-2
	);
	DrawStatusBar();
	DrawTabsBar();
	
	screen.Blit(textScreen,0,1);

	interface->RenderScreen(screen);
}

inline void ContextEditor::Update(){
	if ((currentEvent = interface->GetKeyboardEvent())){
		willUpdate = true;
		
		if (entryMode==EntryMode::Command){
			ProcessCommandEntry((KeyEnum)currentEvent->key,(KeyModifier)currentEvent->mod);
		} else if (entryMode==EntryMode::YesNo){
			ProcessYesNoEntry((KeyEnum)currentEvent->key,(KeyModifier)currentEvent->mod);
		} else {
			if (!ProcessKeyboardEvent((KeyEnum)currentEvent->key,(KeyModifier)currentEvent->mod))
				modes[currentMode]->ProcessKeyboardEvent((KeyEnum)currentEvent->key,(KeyModifier)currentEvent->mod);
		}
	}
	
	if (quit) return;
	
	{
		std::scoped_lock lock{updateMutex};
		if (!willUpdate)
			return;
		
		silentUpdate = (currentEvent==nullptr)||(currentEvent->key==0);
		
		Render();
		
		willUpdate = false;
	}
}

void ContextEditor::Loop(){
	while (!quit){
		Update();
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
				if (!errorMessage.Empty()){
					errorMessage.Set("Error at line "+std::to_string(l)+
						": "+errorMessage.msg);
					break;
				}
				++l;
			}
		}
	} else {
		if (silent){
			errorMessage.Clear();
			errorMessage.msg.clear();
		}
	}
	infoMessage.Clear();
	infoMessage.msg.clear();
}

void ContextEditor::MoveEntryPosLeft(size_t count){
	entryPos -= count;
	entryPos = std::max(entryPos,(ssize_t)0);
}

void ContextEditor::MoveEntryPosRight(size_t count){
	entryPos += count;
	entryPos = std::min(entryPos,(ssize_t)entryString.size());
}

bool ContextEditor::ProcessKeyboardEvent(KeyEnum key,KeyModifier mod){
	s16 found = FindActionFromKey(gBinds.at("ctx"),key,mod);
	if (found==ACTION_NOT_FOUND) return false;
	
	Action action = (Action)found;
	
	switch (action){
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
		case Action::MoveModeToNext:
			MoveMode(currentMode,currentMode+1);
			SwitchMode(currentMode+1);
			return true;
		case Action::MoveModeToPrevious:
			MoveMode(currentMode,currentMode-1);
			SwitchMode(currentMode-1);
			return true;
		case Action::Mode1:
		case Action::Mode2:
		case Action::Mode3:
		case Action::Mode4:
		case Action::Mode5:
		case Action::Mode6:
		case Action::Mode7:
		case Action::Mode8:
		case Action::Mode9:
		case Action::Mode10:
			if (modes.size()>(size_t)action-(size_t)Action::Mode1){
				SwitchMode((size_t)action-(size_t)Action::Mode1);
			}
			return true;
		case Action::NewMode:
			NewMode();
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

void ContextEditor::ProcessCommandEntry(KeyEnum key,KeyModifier mod){
	TextAction textAction = GetTextActionFromKey(key,mod);
	switch (textAction.action){
		case EditAction::InsertChar:
			entryString.insert(entryPos,1,textAction.character);
			MoveEntryPosRight(1);
			break;
		case EditAction::DeletePreviousChar:
			if (!entryString.empty()&&entryPos!=0){
				MoveEntryPosLeft(1);
				entryString.erase(entryPos,1);
			}
			break;
		case EditAction::DeleteCurrentChar:
			if (!entryString.empty()&&entryPos!=(ssize_t)entryString.size()){
				entryString.erase(entryPos,1);
			}
			break;
		case EditAction::InsertLine:
			SubmitCommand();
			break;
		case EditAction::Escape:
			CancelCommand();
			break;
		case EditAction::Tab:
			AutocompleteCommand();
			break;
		case EditAction::MoveLeftMulti:
		case EditAction::MoveLeftChar:
			MoveEntryPosLeft(textAction.num);
			break;
		case EditAction::MoveRightMulti:
		case EditAction::MoveRightChar:
			MoveEntryPosRight(textAction.num);
			break;
		case EditAction::MoveToLineStart:
			entryPos = 0;
			break;
		case EditAction::MoveToLineEnd:
			entryPos = entryString.size();
			break;
		case EditAction::DeleteLine:
			entryString.clear();
			entryPos = 0;
			break;
			
		default:
			break;
	}
}

void ContextEditor::ProcessYesNoEntry(KeyEnum key,KeyModifier mod){
	if (key==(KeyEnum)'y'||key==(KeyEnum)'Y'){
		yesAction();
		entryMode = EntryMode::None;
	} else if (key==(KeyEnum)'n'||key==(KeyEnum)'N'){
		noAction();
		entryMode = EntryMode::None;
	} else if (key==KeyEnum::Escape){
		entryMode = EntryMode::None;
	}
}

size_t GetTokenCol(const Token& t){
	assert(t.start.line==t.end.line);
	return t.start.col-t.start.line->begin();
}

size_t GetTokenSize(const Token& t){
	return t.end.col-t.start.col;
}

TokenVector ContextEditor::GetCommandTokens(){
	commandBuffer.SetLine(commandBuffer.begin(),entryString);
	CommandTokenizer ct;
	
	ct.SetBuffer(&commandBuffer);
	
	TokenVector tokens = {};
	while (!ct.Done()){
		tokens.push_back(ct.EmitToken());
	}
	
	return tokens;
}

void ContextEditor::AutocompleteCommand(){
	TokenVector tokens = GetCommandTokens();
	
	if ((tokens[0].Matches("open")||tokens[0].Matches("saveas"))&&tokens.size()>1){
		std::string sub = entryString.substr(GetTokenCol(tokens[1]),
											GetTokenSize(tokens[1]));
		osInterface->AutocompletePath(sub);
		entryString = entryString.substr(0,GetTokenCol(tokens[1])) + sub +
				entryString.substr(GetTokenCol(tokens[1])+GetTokenSize(tokens[1]));
		entryPos = GetTokenCol(tokens[1])+sub.size();
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
	
	TokenVector tokens = GetCommandTokens();
	
	if (!modes[currentMode]->ProcessCommand(tokens)){
		if (!ProcessCommand(tokens)){
			errorMessage.Set("Unrecognized command '"+tokens[0].Stringify()+"'");
		}
	}
}

#ifndef NDEBUG
inline void LogTokens(TokenVector tokens){
	for (const auto& token : tokens){
		LOG((s32)token.type << ":'" << token.Stringify() << ",");
	}
}
#endif

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
#ifndef NDEBUG
	LogTokens(tokens);
#endif
	
	const Command* cmd;
	if (!GetCommandFromName(tokens[0].Stringify(),&cmd)){
		return false;
	}
	
	auto count = GetReqArgCount(*cmd);
	if (count>tokens.size()-1){
		errorMessage.Set( "Expected "+std::to_string(count)+" args, got "+
			std::to_string(tokens.size()-1) );
		return true;
	}
	
	if (tokens[0].Matches("open")){
		std::string path = tokens[1].Stringify();
		std::string parsedPath = ParsePath(path,*osInterface);
		OpenMode(parsedPath);
		return true;
	} else if (tokens[0].Matches("saveas")){
		std::string path = tokens[1].Stringify();
		std::string parsedPath = ParsePath(path,*osInterface);
		SaveAsMode(parsedPath,currentMode);
		return true;
	} else if (tokens[0].Matches("set")){
		std::string varName = tokens[1].Stringify();
		std::string val = tokens[2].Stringify();
		SetConfigVar(varName,val);
		return true;
	} else if (tokens[0].Matches("bind")){
		std::string actionName = tokens[1].Stringify();
		SetConfigBind(actionName,tokens);
		return true;
	} else if (tokens[0].Matches("style")){
		std::string styleName = tokens[1].Stringify();
		std::string fg = tokens[2].Stringify();
		std::string bg = tokens[3].Stringify();
		std::string opts = {};
		if (tokens.size()>4)
			opts = tokens[4].Stringify();
		SetStyleOpts(styleName,fg,bg,opts);
		for (size_t i=0;i<modes.size();++i){
			modes[i]->UpdateStyle();
		}
		return true;
	} else if (tokens[0].Matches("source")){
		std::string path = tokens[1].Stringify();
		std::string parsedPath = ParsePath(path,*osInterface);
		RunFile(parsedPath);
		return true;
	}
	
	return false;
}

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

inline bool ResolveActionMode(const std::string& name,std::string& modeOut){
	auto index = name.find('.');
	
	if (index==std::string::npos||index==0) return false;
	
	modeOut = name.substr(0,index);
	return true;
}

inline bool ResolveActionName(const std::string& name,std::string& actionOut){
	auto index = name.find('.');
	if (index==name.size()-1) return false;
	
	actionOut = name.substr(index+1);
	return true;
}

inline KeyModifier ParseMod(std::string_view& bindstr){
	if (bindstr.starts_with("ctrl-")||bindstr.starts_with("CTRL-")||
		bindstr.starts_with("Ctrl-")){
		bindstr.remove_prefix(5);
		return KeyModifier::Ctrl;
	}
	
	if (bindstr.starts_with("C-")||bindstr.starts_with("c-")){
		bindstr = {bindstr.begin()+2,bindstr.end()};
		bindstr.remove_prefix(2);
		return KeyModifier::Ctrl;
	}
	
	if (bindstr.starts_with("shift-")||bindstr.starts_with("SHIFT-")||
		bindstr.starts_with("Shift-")){
		bindstr.remove_prefix(6);
		return KeyModifier::Shift;
	}
	
	if (bindstr.starts_with("S-")||bindstr.starts_with("s-")){
		bindstr.remove_prefix(2);
		return KeyModifier::Shift;
	}
	
	if (bindstr.starts_with("alt-")||bindstr.starts_with("ALT-")||
		bindstr.starts_with("Alt-")){
		bindstr.remove_prefix(4);
		return KeyModifier::Alt;
	}
	
	if (bindstr.starts_with("A-")||bindstr.starts_with("a-")||
		bindstr.starts_with("M-")||bindstr.starts_with("m-")){
		bindstr.remove_prefix(2);
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

inline void SkipWhitespace(std::string_view& str,std::string_view::iterator pos){
	while (pos!=str.end()&&(*pos==' '||*pos=='\n'||*pos=='\t')){
		++pos;
	}
	
	str = {pos,str.end()};
}

inline bool ParseKeybind(std::vector<KeyBind>& binds,std::string_view bindstr){
	std::string_view::iterator pos;
	KeyEnum key = KeyEnum::None;
	KeyModifier mod = KeyModifier::None;
	
	KeyModifier t = KeyModifier::None;
	while (true){
		if (bindstr.size()==0) return false;
		pos = bindstr.begin();
		SkipWhitespace(bindstr,pos);
		t = ParseMod(bindstr);
		if (t==KeyModifier::None) break;
		if (bindstr[0]=='-') bindstr.remove_prefix(1);
		
		mod = (KeyModifier)((s32)mod | (s32)t);
	}
	
	key = ParseKey(bindstr);
	
	if (key==KeyEnum::None) return false;
	
	binds.emplace_back(key,mod);
	
	LOG((char)key << ", " << (s32)key << ", " << (s32)mod);
	
	return true;
}

void ContextEditor::SetConfigBind(const std::string& actionStr,const TokenVector& tokens){
	std::string modeName,actionName;
	
	if (!ResolveActionMode(actionStr,modeName)){
		errorMessage.Set("Unrecognized mode in action: '"+std::string(actionStr)+"'");
		return;
	}
	
	if (!ResolveActionName(actionStr,actionName)){
		errorMessage.Set("Badly formatted action: '"+std::string(actionStr)+"'");
		return;
	}
	
	auto bindSet = gBinds.at(modeName);
	if (!bindSet.nameMap.contains(actionName)){
		errorMessage.Set("Unrecognized action: '"+std::string(actionStr)+"'");
		return;
	}
	
	s16 action = bindSet.nameMap.at(actionName);
	
	auto bindIt = tokens.begin();
	++bindIt; ++bindIt; //start after first arg
	std::vector<KeyBind> parsedBinds = {};
	
	while (bindIt!=tokens.end()){
		if (!ParseKeybind(parsedBinds,bindIt->Stringify())){
			errorMessage.Set("Could not parse keybind '"+bindIt->Stringify()+"'");
			break;
		}
		
		++bindIt;
	}
	
	BindSet& page = gBinds.at(modeName);
	
	page.keyMap[action].clear();
	for (const auto& bind : parsedBinds){
		AddBind(page,action,bind);
	}
	
	UpdateBinds(page);
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
		errorMessage.Set("Unrecognized style name '"+copied+"'!");
		return;
	}
	
	TextStyle& styleVar = styleNameMap.at(copied);
	
	Color fg,bg;
	if (!ParseColor(fgStr,fg)){
		errorMessage.Set("Cannot convert '"+std::string(fgStr)+"' into a color!");
		return;
	}
	
	if (!ParseColor(bgStr,bg)){
		errorMessage.Set("Cannot convert '"+std::string(bgStr)+"' into a color!");
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
				std::string cs = {};
				cs += c;
				errorMessage.Set("Unrecognized style option '"+cs+"'!");
				return;
			}
		}
	}
	
	styleVar = {fg,bg,sf};
}

std::string ContextEditor::ConstructModeString(size_t index){
	std::string modeStr = {};
	
	std::string_view path = modes[index]->GetPath(*osInterface);
	if (!path.empty())
		modeStr += path;
	else
		modeStr += "(unnamed)";

	if (modes[index]->Modified())
		modeStr += '*';

	if (modes[index]->Readonly())
		modeStr += " (readonly)";

	return modeStr;
}

void ContextEditor::DrawStatusBar(){
	s32 w,h;
	w = screen.GetWidth();
	h = screen.GetHeight();

	std::string modeStr = ConstructModeString(currentMode);
	std::string modeStatusBar = {};
	modeStatusBar += modes[currentMode]->GetStatusBarText();

	for (s32 x=0;x<w;++x)
		screen.SetAt(x,h-1,TextCell(' ',barStyle));
	
	if (entryMode==EntryMode::Command){
		screen.RenderString(0,h-1,entryPrefix + entryString,barStyle);
		auto x = entryPrefix.size()+entryPos;
		auto cell = screen.GetAt(x,h-1);
		cell.style = cursorStyle;
		screen.SetAt(x,h-1,cell);
	} else if (entryMode==EntryMode::YesNo){
		screen.RenderString(0,h-1,yesNoMessage + " Y/N ",barStyle);
	} else {
		Message& modeError = modes[currentMode]->GetErrorMessage();
		Message& modeInfo = modes[currentMode]->GetInfoMessage();
		if (!silentUpdate){
			if (!errorMessage.Empty())
				errorMessage.Clear();
			else if (!modeError.Empty())
				modeError.Clear();
			else if (!infoMessage.Empty())
				infoMessage.Clear();
			else if (!modeInfo.Empty())
				modeInfo.Clear();
		}
	
		if (!errorMessage.Empty()){
			screen.RenderString(0,h-1,errorMessage.msg,errorStyle);
			errorMessage.Mark();
		} else if (!modeError.Empty()){
			std::string formattedError = {};
			formattedError += modes[currentMode]->GetModeName();
			formattedError += ": ";
			formattedError += modeError.msg;
			screen.RenderString(0,h-1,formattedError,errorStyle);
			modeError.Mark();
		} else if (!infoMessage.Empty()){
			screen.RenderString(0,h-1,infoMessage.msg,barStyle);
			infoMessage.Mark();
		} else if (!modeInfo.Empty()){
			std::string formattedInfo = {};
			formattedInfo += modes[currentMode]->GetModeName();
			formattedInfo += ": ";
			formattedInfo += modeInfo.msg;
			screen.RenderString(0,h-1,formattedInfo,barStyle);
			modeInfo.Mark();
		} else if (!modeStatusBar.empty()){
			screen.RenderString(0,h-1,modeStatusBar,barStyle);
		}

		screen.RenderString(w-1-modeStr.size(),h-1,modeStr,barStyle);
	}
}

const size_t tabBarWidth = 16;

inline std::string_view BaseName(std::string_view s){
	auto index = s.find_last_of('/');
	if (index!=std::string::npos)
		s.remove_prefix(index+1);
	return s;
}

std::string ContextEditor::GetTabString(size_t index,size_t tabW){
	std::string s = " ";
	s += std::to_string(index+1);
	s += ' ';
	std::string path = std::string(BaseName(modes[index]->GetPath(*osInterface)));
	if (path.empty())
		path = "(unnamed)";
		
	bool mod = modes[index]->Modified();
	size_t max;
	if (mod)
		max = tabW-s.size()-2; //extra room for *
	else
		max = tabW-s.size()-1;
		
	if (path.size() > max){
		s += StringPostEllipsis(path,max);
		if (mod) s += '*';
	} else {
		s += path;
		if (mod) s += '*';
	}
	
	while (s.size()<tabW)
		s += ' ';
	
	return s;
}

void ContextEditor::DrawTabsBar(){
	// how many tabs per page (minus 3 for ' ...')
	size_t tabCount = (screen.GetWidth()-4)/tabBarWidth;
	size_t sansSymbolTabCount = (screen.GetWidth())/tabBarWidth;
	// if there is just enough space to display one more
	// tab without the symbols then let it happen
	if (modes.size()==sansSymbolTabCount)
		tabCount = sansSymbolTabCount;
	
	// current page of tabs
	size_t pageNum = currentMode/tabCount;
	size_t pageCount = modes.size()/tabCount;
	if (modes.size()%tabCount!=0)
		pageCount++;
	
	size_t startX = 0;
	if (pageNum!=0)
		startX = 2;
	
	for (size_t i=0;i<(size_t)screen.GetWidth();++i){
		screen.SetAt(i,0,{' ',tabBarStyle});
	}
	
	for (size_t i=0;i<tabCount;++i){
		size_t currTab = i+pageNum*tabCount;
		if (currTab>=modes.size()) break;
		
		if (currTab==currentMode)
			screen.RenderString(startX+i*tabBarWidth,0,
				GetTabString(currTab,tabBarWidth),tabBarSelectStyle);
		else
			screen.RenderString(startX+i*tabBarWidth,0,
				GetTabString(currTab,tabBarWidth),tabBarStyle);
	}
	if (modes.size()>tabCount){
		if (pageNum!=pageCount-1)
			screen.RenderString(screen.GetWidth()-2,0,"+",tabBarStyle);
		if (pageNum!=0)
			screen.RenderString(0,0,"+",tabBarStyle);
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
				errorMessage.Set("File is readonly!");
			} else {
				errorMessage.Set("Could not save!");
			}
		}
	}
}

void ContextEditor::SwitchMode(size_t index){
	currentMode = (index+modes.size())%modes.size();
}

void ContextEditor::MoveMode(size_t index,size_t newIndex){
	index = (index+modes.size())%modes.size();
	newIndex = (newIndex+modes.size())%modes.size();
	
	std::swap(modes[index],modes[newIndex]);
}

bool ContextEditor::WriteFileChecks(std::string_view path){
	if (!osInterface->FileIsWritable(path)){
		errorMessage.Set("File "+std::string(path)+" is not writable!");
		return false;
	}

	return true;
}

bool ContextEditor::ReadFileChecks(std::string_view path){
	if (!osInterface->PathExists(path)){
		errorMessage.Set("File '"+std::string(path)+"' does not exist!");
		return false;
	}
	if (!osInterface->PathIsFile(path)){
		errorMessage.Set("Path '"+std::string(path)+"' is not a file!");
		return false;
	}
	if (!osInterface->FileIsReadable(path)){
		errorMessage.Set("File '"+std::string(path)+"' is not readable!");
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
		if (!modes.size()){
			modes.push_back(std::move(openedMode));
			currentMode = 0;
		} else {
			modes.insert(modes.begin()+currentMode+1,std::move(openedMode));
			++currentMode;
		}
	} else {
		errorMessage.Set("Could not open '"+copiedPath+"'!");
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

void ContextEditor::SetConfigVar(std::string_view name,std::string_view val){
	if (name=="style"){
		SaveStyle();
		gConfig.style = val;
		if (!StyleExists(gConfig.style)){
			infoMessage.Set("New style '"+gConfig.style+"' created.");
		}
		LoadStyle();
		for (size_t i=0;i<modes.size();++i)
			modes[i]->UpdateStyle();
	} else if (name=="sleepy"){
		if (!ParseBool(gConfig.sleepy,val)){
			errorMessage.Set("sleepy must be a boolean value!");
		}
	} else {
		errorMessage.Set("'"+std::string(name)+"' is not a config var!");
	}
}

void WaitForAsyncTimer(ContextEditor* ctx,size_t index,AsyncContext a){
	if (a.preDelay>0.0f){
		ctx->GetOSInterface()->Sleep(a.preDelay);
		if (ctx->GetAsyncCancel(index)){
			ctx->AsyncFinished(index);
			return;
		}
	}
	
	a.func();
	
	ctx->AsyncFinished(index);
}

bool ContextEditor::GetAsyncCancel(size_t i) const {
	for (const auto& a : asyncState){
		if (a.index==i)
			return a.canceled;
	}
	return true;
}

void ContextEditor::AsyncFinished(size_t i){
	AsyncData async = {};
	bool found = false;
	(void)found;
	
	{
		std::scoped_lock lock{asyncMutex};
		
		for (auto& a : asyncState){
			if (a.index==i){
				async = a;
				found = true;
				break;
			}
		}
	}
	
	assert(found);
	{
		std::scoped_lock lock{updateMutex};
		if (async.updateAfter){
			willUpdate = true;
			silentUpdate = true;
		}
	}
	
	{
		std::scoped_lock lock{asyncMutex};
		for (auto it=asyncState.begin();it!=asyncState.end();++it){
			if (it->index==i){
				asyncState.erase(it);
				break;
			}
		}
	}
}

size_t ContextEditor::StartAsyncTask(const AsyncContext& async){
	size_t i = asyncIndex++;
	{
		std::scoped_lock lock{asyncMutex};
		asyncState.emplace_back(i,false,async.updateAfter);
	}
	
	std::thread task{std::bind(WaitForAsyncTimer,this,i,async)};
	
	task.detach();
	
	return i;
}

bool ContextEditor::IsAsyncTaskDone(size_t index){
	for (const auto& async : asyncState){
		if (async.index == index){
			return false;
		}
	}
	
	return true;
}
void ContextEditor::CancelAsyncTask(size_t index){
	std::scoped_lock lock{asyncMutex};
	for (auto& async : asyncState){
		if (async.index == index){
			async.canceled = true;
			return;
		}
	}
}

OSInterface* ContextEditor::GetOSInterface() const {
	return osInterface.get();
}

std::string& ContextEditor::GetClipboard(){
	return clipboardText;
}
