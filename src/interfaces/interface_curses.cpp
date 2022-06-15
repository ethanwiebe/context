#ifdef __linux__

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
	#define KEYPREFIX "\033[1;"
	
	s = KEYPREFIX "5D"; //ctrl left
	define_key(s.c_str(),483);
	s = KEYPREFIX "5C"; //ctrl right
	define_key(s.c_str(),484);
	s = KEYPREFIX "5A"; //ctrl up
	define_key(s.c_str(),485);
	s = KEYPREFIX "5B"; //ctrl down
	define_key(s.c_str(),486);
	
	s = KEYPREFIX "3D"; //alt left
	define_key(s.c_str(),487);
	s = KEYPREFIX "3C"; //alt right
	define_key(s.c_str(),488);
	s = KEYPREFIX "3A"; //alt up
	define_key(s.c_str(),489);
	s = KEYPREFIX "3B"; //alt down
	define_key(s.c_str(),490);
	
	s = KEYPREFIX "7D"; //ctrl-alt left
	define_key(s.c_str(),491);
	s = KEYPREFIX "7C"; //ctrl-alt right
	define_key(s.c_str(),492);
	s = KEYPREFIX "7A"; //ctrl-alt up
	define_key(s.c_str(),493);
	s = KEYPREFIX "7B"; //ctrl-alt down
	define_key(s.c_str(),494);
	
	#undef KEYPREFIX
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
	SetMappings();

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

void CursesInterface::SetMappings(){
	constexpr auto ctrlalt = (KeyModifier)((s32)KeyModifier::Ctrl | 
								(s32)KeyModifier::Alt);
	for (s32 i=0;i<=25;++i){
		keyMapping['A'+i] = {(KeyEnum)(i+'A'),KeyModifier::Shift};
		keyMapping[417+i] = {(KeyEnum)(i+'A'),KeyModifier::Alt};
		keyMapping[447+i] = {(KeyEnum)(i+'A'),ctrlalt};
		
		keyMapping[i+1] = {(KeyEnum)(i+'A'),KeyModifier::Ctrl};
	}
	
	for (s32 i=0;i<10;++i){
		keyMapping[473+i] = {(KeyEnum)('0'+i),KeyModifier::Alt};
	}
	
	for (s32 i=0;i<12;++i){
		keyMapping[277+i] = {(KeyEnum)((s32)KeyEnum::F1+i),KeyModifier::Shift};
		keyMapping[289+i] = {(KeyEnum)((s32)KeyEnum::F1+i),KeyModifier::Ctrl};
		keyMapping[313+i] = {(KeyEnum)((s32)KeyEnum::F1+i),KeyModifier::Alt};
	}
	
	keyMapping[329] = {KeyEnum::Insert,KeyModifier::Ctrl};
	keyMapping[333] = {KeyEnum::Delete,KeyModifier::Ctrl};
	
	keyMapping[398] = {KeyEnum::PageUp,KeyModifier::Shift};
	keyMapping[443] = {KeyEnum::PageUp,KeyModifier::Ctrl};
	
	keyMapping[396] = {KeyEnum::PageDown,KeyModifier::Shift};
	keyMapping[444] = {KeyEnum::PageDown,KeyModifier::Ctrl};
	
	keyMapping[8] = {KeyEnum::Backspace,KeyModifier::Ctrl};
	
	keyMapping[391] = {KeyEnum::Home,KeyModifier::Shift};
	keyMapping[386] = {KeyEnum::End,KeyModifier::Shift};
	
	keyMapping[353] = {KeyEnum::Tab,KeyModifier::Shift};
	
	keyMapping[445] = {(KeyEnum)';',KeyModifier::Alt};
	keyMapping[446] = {(KeyEnum)'\'',KeyModifier::Alt};
	
	keyMapping[483] = {KeyEnum::Left,KeyModifier::Ctrl};
	keyMapping[484] = {KeyEnum::Right,KeyModifier::Ctrl};
	keyMapping[485] = {KeyEnum::Up,KeyModifier::Ctrl};
	keyMapping[486] = {KeyEnum::Down,KeyModifier::Ctrl};
	
	keyMapping[487] = {KeyEnum::Left,KeyModifier::Alt};
	keyMapping[488] = {KeyEnum::Right,KeyModifier::Alt};
	keyMapping[489] = {KeyEnum::Up,KeyModifier::Alt};
	keyMapping[490] = {KeyEnum::Down,KeyModifier::Alt};
	
	keyMapping[491] = {KeyEnum::Left,ctrlalt};
	keyMapping[492] = {KeyEnum::Right,ctrlalt};
	keyMapping[493] = {KeyEnum::Up,ctrlalt};
	keyMapping[494] = {KeyEnum::Down,ctrlalt};
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

	CursesKeyBind k = {(KeyEnum)key,(KeyModifier)0};
	if (keyMapping.contains(key))
		k = keyMapping[key];
	
	logger << "Post:" << (char)k.key << " " << (s32)k.key << " " << GetModName(k.mod) << "\n";

	lastEvent = KeyboardEvent((s32)k.key,k.mod);

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

#endif
