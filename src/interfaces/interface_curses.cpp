#include "interface_curses.h"

void DefineAltKeys(){
	std::string s = "\033";
	for (char i='a';i<='z';i++){
		define_key((s+i).c_str(),320+i);
	}

	s = "\033\133\065\073\065\176"; //ctrl page up
	define_key(s.c_str(),443);
	s = "\033\133\066\073\065\176"; //ctrl page down
	define_key(s.c_str(),444);

	s = "\033\073"; //alt semicolon
	define_key(s.c_str(),445);

	s = "\033\047"; //alt apostrophe
	define_key(s.c_str(),446);
	
	//ctrl-alt alpha
	for (s32 i=1;i<=26;i++){
		s = {};
		s += "\033";
		s += (char)i;
		define_key(s.c_str(),446+i);
	}

	//alt numbers
	for (s32 i=0;i<10;i++){
		s = {};
		s += "\033";
		s += '0'+i;
		define_key(s.c_str(),473+i);
	}
}

std::string GetModName(s32 mod){
	const auto shiftName = "Shift ";
	const auto ctrlName = "Ctrl ";
	const auto altName = "Alt ";

	std::string name{};

	if (mod&KeyModifier::Shift){
		name += shiftName;
	}

	if (mod&KeyModifier::Ctrl){
		name += ctrlName;
	}

	if (mod&KeyModifier::Alt){
		name += altName;
	}

	return name;
}

CursesInterface::CursesInterface(){
	charArray = nullptr;
	
	setlocale(LC_ALL,"");

	initscr();
	start_color();

	keypad(stdscr,true);
	noecho(); // don't echo typed keys into the terminal
	raw(); //pass input through instantly
	notimeout(stdscr,false);
	ESCDELAY = 50;
	
	curs_set(0); //make default cursor invisible

	InitColorPairs();

	colorDefinitions = {};
	pairDefinitions = {};

	DefineAltKeys();

	WindowResized(COLS,LINES);

	logger << "Has colors: " << 
			(has_colors() ? "TRUE" : "FALSE") << '\n';
			
	logger << "Can change colors: " << 
			(can_change_color() ? "TRUE" : "FALSE") << '\n';
			
	logger << "Max color pairs: " << COLOR_PAIRS << "\n";
	logger << "Max colors: " << COLORS << "\n";
	logger << "Escape delay: " << ESCDELAY << "\n";
}

CursesInterface::~CursesInterface(){
	endwin();
	delete[] charArray;
}

void CursesInterface::InitColorPairs(){
	reset_color_pairs();
};

void CursesInterface::ListColorPairs(){
	s16 fg,bg;
	for (s16 i=0;i<definedPairs;i++){
		pair_content(i,&fg,&bg);
		logger << "FG: " << fg << " BG: " << bg << "\n";
	}
}

void CursesInterface::WindowResized(s32 w,s32 h){
	if (charArray)
		delete[] charArray;

	charArray = new cchar_t[w*h];
}

KeyboardEvent* CursesInterface::GetKeyboardEvent(){
	s32 key = getch();

	if (key==KEY_RESIZE){
		resize_term(0,0);
		WindowResized(COLS,LINES);
		return nullptr;
	}

	s32 mod = (s32)KeyModifier::None;

	logger << "Pre:" << (char)key << " " << key << " " << GetModName(mod) << "\n";

	NormalizeKeys(key,mod);

	logger << "Post:" << (char)key << " " << key << " " << GetModName(mod) << "\n";

	lastEvent = KeyboardEvent(key,mod);

	return &lastEvent;
}

s32 CursesInterface::GetWidth(){
	return COLS;
}

s32 CursesInterface::GetHeight(){
	return LINES;
}

inline s32 CursesInterface::ColorsToPair(s32 fg, s32 bg) const {
	return bg*definedColors + fg;
}

s32 GetFallbackColor8(Color c){
	if (c==ColorBackgroundDark) return COLOR_BLACK;
	if (c==ColorForegroundDark) return COLOR_WHITE;
	if (c==ColorRed) return COLOR_RED;
	if (c==ColorYellow) return COLOR_YELLOW;
	if (c==ColorGreen) return COLOR_GREEN;
	if (c==ColorOrange) return COLOR_RED;
	if (c==ColorMagenta) return COLOR_MAGENTA;
	if (c==ColorBlue) return COLOR_BLUE;
	if (c==ColorCyan) return COLOR_CYAN;
	
	
	if (c.r>=110&&c.g>=110&&c.b>=110) return COLOR_WHITE;
	if (c.r<=80&&c.g<=80&&c.b<=80) return COLOR_BLACK;
	if (c.r>=160&&c.b>=160) return COLOR_MAGENTA;
	if (c.r>=160&&c.g>=160) return COLOR_YELLOW;
	if (c.g>=140) return COLOR_GREEN;
	if (c.r>=140) return COLOR_RED;
	if (c.b>=180) return COLOR_CYAN;
	if (c.b>=140) return COLOR_BLUE;
	return COLOR_WHITE;
}

s32 CursesInterface::DefineColor(Color col){
	if (COLORS<=8) return GetFallbackColor8(col);
	s32 colorIndex = 0;
	
	for (const auto& defColor : colorDefinitions){
		if (defColor == col)
			return colorIndex+16;

		++colorIndex;
	}

	init_color(colorIndex+16,col.r*1000/255,col.g*1000/255,col.b*1000/255);
	colorDefinitions.push_back(col);

	return colorIndex+16;
}

s32 CursesInterface::DefinePair(Color fg,Color bg){
	s32 pairIndex = 0;
	for (const auto& pair : pairDefinitions){
		if (pair.first==fg && pair.second==bg)
			return pairIndex;

		++pairIndex;
	}

	s32 fgIndex = DefineColor(fg);
	s32 bgIndex = DefineColor(bg);

	init_pair(pairIndex+16,fgIndex,bgIndex);

	pairDefinitions.emplace_back(fg,bg);

	return pairIndex;
}

void CursesInterface::RenderScreen(const TextScreen& textScreen){
	// convert text into chtype arrays
	s32 flags;
	s32 pairNum;
	wchar_t wchars[2];
	
	for (size_t i=0;i<textScreen.size();i++){
		flags = 0;
		flags |= textScreen[i].style.flags & StyleFlag::Bold ? A_BOLD : 0;
		flags |= textScreen[i].style.flags & StyleFlag::Underline ? A_UNDERLINE : 0; 
		flags |= textScreen[i].style.flags & StyleFlag::AlternateCharacterSet ? A_ALTCHARSET : 0;

		pairNum = DefinePair(textScreen[i].style.fg,textScreen[i].style.bg);
		wchars[0] = textScreen[i].c;
		wchars[1] = 0;
		
		setcchar(&charArray[i],wchars,flags,pairNum+16,(const void*)0);
	}

	// print lines onto the terminal
	for (s32 y=0;y<LINES;y++){
		mvadd_wchnstr(y,0,&charArray[y*COLS],COLS);
	}

}

inline void NormalizeAlphabetKeys(s32& key,s32& mod){
	// alt letters
	if (key>=417&&key<=442){
		key -= 417-97;
		mod |= KeyModifier::Alt;
	}

	if (key>=447&&key<=446+26){
		key -= 447-97;
		mod |= KeyModifier::Alt | KeyModifier::Ctrl;
	}

	if (key>=473&&key<484){
		key = '0'+key-473;
		mod |= KeyModifier::Alt;
	}

	if (key==445){
		key = 59;
		mod |= KeyModifier::Alt;
	}

	if (key==446){
		key = 39;
		mod |= KeyModifier::Alt;
	}

	// control letters
	if (key>=1&&key<=26){
		key += 97-1;
		mod |= KeyModifier::Ctrl;
	}	
}

inline void NormalizeFKeys(s32& key,s32& mod){
	if (key>=277&&key<=324){
		if (key>276&&key<=288)
			mod |= KeyModifier::Shift;
		if (key>288&&key<=312)
			mod |= KeyModifier::Ctrl;
		if (key>312)
			mod |= KeyModifier::Alt;

		key = (key-265)%12+265;
	}
}

inline void NormalizeInsertDeleteKeys(s32& key,s32& mod){
	if (key==329){ //insert
		key = 331;
		mod |= KeyModifier::Ctrl;
	}

	if (key==333){ //delete
		key = 330;
		mod |= KeyModifier::Ctrl;
	}
}

inline void NormalizePageKeys(s32& key,s32& mod){
	if (key==398){ //page up
		key = 339;
		mod |= KeyModifier::Shift;
	}

	if (key==396){ //page down
		key = 338;
		mod |= KeyModifier::Shift;
	}

	if (key==443){ //page up
		key = 339;
		mod |= KeyModifier::Ctrl;
	}

	if (key==444){ //page down
		key = 338;
		mod |= KeyModifier::Ctrl;
	}
}

inline void NormalizeBackspaceKey(s32& key,s32& mod){
	if (key==8){
		key = KeyEnum::Backspace;
		mod |= KeyModifier::Ctrl;
	}
}

inline void NormalizeHomeEndKeys(s32& key,s32& mod){
	if (key==391){ //shift home
		key = 262;
		mod |= KeyModifier::Shift;
	}

	if (key==386){ //shift end
		key = 360;
		mod |= KeyModifier::Shift;
	}

	if (key==334){ //ctrl end
		key = 360;
		mod |= KeyModifier::Ctrl;
	}

}

inline void NormalizeNavPad(s32& key,s32& mod){
	NormalizeHomeEndKeys(key,mod);
	NormalizeInsertDeleteKeys(key,mod);
	NormalizePageKeys(key,mod);
}

inline s32 NormalizeUpKey(s32 key,s32 mod){
	// shift up
	if (key==547&&(mod & KeyModifier::Shift)){
		return KeyEnum::Up;
	}

	// control up
	if (key==480&&(mod & KeyModifier::Ctrl)){
		key = KeyEnum::Up;
	}

	// alt up
	if (key==490&&(mod & KeyModifier::Alt)){
		key = KeyEnum::Up;
	}

	return key;
}

inline s32 NormalizeDownKey(s32 key,s32 mod){
	// shift down
	if (key==548&&(mod & KeyModifier::Shift)){
		key = KeyEnum::Down;
	}

	// control down
	if (key==481&&(mod & KeyModifier::Ctrl)){
		key = KeyEnum::Down;
	}

	// alt down
	if (key==491&&(mod & KeyModifier::Alt)){
		key = KeyEnum::Down;
	}

	return key;
}

inline s32 NormalizeLeftKey(s32 key,s32 mod){
	// shift left
	if (key==391&&(mod & KeyModifier::Shift)){
		key = KeyEnum::Left;
	}

	// control left
	if (key==443&&(mod & KeyModifier::Ctrl)){
		key = KeyEnum::Left;
	}

	// alt left
	if (key==493&&(mod & KeyModifier::Alt)){
		key = KeyEnum::Left;
	}

	return key;
}

inline s32 NormalizeRightKey(s32 key,s32 mod){
	//shift right
	if (key==400&&(mod & KeyModifier::Shift)){
		key = KeyEnum::Right;
	}

	// control right
	if (key==444&&(mod & KeyModifier::Ctrl)){
		key = KeyEnum::Right;
	}

	// alt right
	if (key==492&&(mod & KeyModifier::Alt)){
		key = KeyEnum::Right;
	}

	return key;
}

inline void NormalizeDirectionKeys(s32& key,s32& mod){
	key = NormalizeUpKey(key,mod);
	key = NormalizeDownKey(key,mod);
	key = NormalizeLeftKey(key,mod);
	key = NormalizeRightKey(key,mod);
}

inline void NormalizeKeys(s32& key,s32& mod){
	NormalizeAlphabetKeys(key,mod);
	NormalizeFKeys(key,mod);
	NormalizeNavPad(key,mod);
	NormalizeBackspaceKey(key,mod);
	NormalizeDirectionKeys(key,mod);
	if (key==353){
		key = KeyEnum::Tab;
		mod = KeyModifier::Shift;
	}
}


