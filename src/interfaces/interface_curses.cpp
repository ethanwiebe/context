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
	initscr();
	start_color();

	keypad(stdscr,true);
	noecho(); // don't echo typed keys into the terminal
	raw(); //pass input through instantly
	
	curs_set(0); //make default invisible

	refresh();

	InitColors();
	InitColorPairs();
	//ListColorPairs();

	colorDefinitions = {};//{{0,0,0},{174,174,174}};
//	colorDefinitions = {{1,2,3}};
	pairDefinitions = {};
//	pairDefinitions.emplace_back(colorDefinitions[0],colorDefinitions[0]);

	DefineAltKeys();

	logger << "Max color pairs: " << COLOR_PAIRS << "\n";
	logger << "Max colors: " << COLORS << "\n";
	logger << "Colors pairs: " << definedPairs << "\n";
	logger << "Colors: " << definedColors << "\n";
}

CursesInterface::~CursesInterface(){
	endwin();
	logger << "closing\n";
}

void CursesInterface::InitColors(){
	/*definedColors = 8;

	init_color(definedColors++,300,300,300); //bright black (gray)
	init_color(definedColors++,1000,    0,    0); //bright red
	init_color(definedColors++,0,    1000,    0); //bright green
	init_color(definedColors++,1000, 1000,    0); //bright yellow
	init_color(definedColors++,   0,    0, 1000); //bright blue
	init_color(definedColors++,1000,    0, 1000); //bright magenta
	init_color(definedColors++,   0, 1000, 1000); //bright cyan
	init_color(definedColors++,  1000, 1000, 1000); //bright white
*/
}

void CursesInterface::InitColorPairs(){
	reset_color_pairs();
/*	definedPairs = 0;
	
	s32 colorN,colorFore,colorBack;

	for (s32 i=0;i<definedColors*definedColors;i++){
		colorFore = i%definedColors;
		colorBack = i/definedColors;
		
		init_pair(i,colorFore,colorBack);
		++definedPairs;
	}
*/
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
		delete charArray;

	charArray = new chtype[w*h];
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

s32 CursesInterface::DefineColor(Color col){
	s32 colorIndex = 0;
	for (const auto& defColor : colorDefinitions){
		if (defColor == col){
			return colorIndex;
		}

		++colorIndex;
	}

	init_color(colorIndex+16,col.r*1000/255,col.g*1000/255,col.b*1000/255);
	colorDefinitions.push_back(col);

	return colorIndex;
}

s32 CursesInterface::DefinePair(Color fg,Color bg){
	s32 pairIndex = 0;
	for (const auto& pair : pairDefinitions){
		if (pair.first==fg && pair.second==bg){
			return pairIndex;
		}

		++pairIndex;
	}

	s32 fgIndex = DefineColor(fg);
	s32 bgIndex = DefineColor(bg);

	init_pair(pairIndex+16,fgIndex+16,bgIndex+16);

	pairDefinitions.emplace_back(fg,bg);

	return pairIndex;
}

void CursesInterface::RenderScreen(const TextScreen& textScreen){
	// convert text into chtype arrays
	s32 boldFlag;
	s32 pairNum;
	for (size_t i=0;i<textScreen.size();i++){
		boldFlag = textScreen[i].style.bold ? A_BOLD : 0;

		//boldFlag = textScreen[i].fg>=8&&textScreen[i].fg<16 ? A_BOLD : 0;
		pairNum = DefinePair(textScreen[i].style.fg,textScreen[i].style.bg);

		charArray[i] = (chtype)textScreen[i].c | COLOR_PAIR(pairNum+16) | boldFlag;
	}

	// print lines onto the terminal
	for (s32 y=0;y<LINES;y++){
		mvaddchnstr(y,0,&charArray[y*COLS],COLS);
	}

}


inline void NormalizeAlphabetKeys(s32& key,s32& mod){
	// alt letters
	if (key>=417&&key<=442){
		key -= 417-97;
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
	
}

inline void NormalizeHomeEndKeys(s32& key,s32& mod){
	if (key==391){ //home
		key = 262;
		mod |= KeyModifier::Shift;
	}

	if (key==386){
		key = 360;
		mod |= KeyModifier::Shift;
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
}


