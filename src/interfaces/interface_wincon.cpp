#ifdef _WIN32

#include "interface_wincon.h"

#include <windows.h>
#include <cstring>
#include <winuser.h>

HANDLE outHandle;
HANDLE inHandle;

#define CSI L"\x1b["

wchar_t* charArray;
wchar_t* currChar;

bool shiftDown,ctrlDown,altDown;

s64 totalSize;

INPUT_RECORD consoleEvent;

bool EnableVTMode(HANDLE h){
    // Set output mode to handle virtual terminal sequences
    if (h == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(h, &dwMode))
    {
        return false;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
    if (!SetConsoleMode(h, dwMode))
    {
        return false;
    }
    return true;
}


WinConInterface::WinConInterface(){
	outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	
	inHandle = GetStdHandle(STD_INPUT_HANDLE);
	
	totalSize = 0;
	
	DWORD mode = 0;
	GetConsoleMode(inHandle,&mode);
	mode &= ~ENABLE_PROCESSED_INPUT; //disable ctrl-c
	SetConsoleMode(inHandle,mode);
	
	SetConsoleTitle("context");
	
	std::ios::sync_with_stdio(false);
	
	trueColor = EnableVTMode(outHandle);
	
	LOG("True color: " << trueColor);
	
	ResizeScreen(GetWidth(),GetHeight());
	
	wprintf(CSI "?1049h"); //switch to alt screen buffer
	wprintf(CSI "?25l"); //hide cursor
	
	SetMappings();
	shiftDown = false;
	ctrlDown = false;
	altDown = false;
}

WinConInterface::~WinConInterface(){
	wprintf(CSI "0m"); //reset color
	wprintf(CSI "?1049l"); //switch back to normal
	wprintf(CSI "?25h"); //show cursor
	
	delete[] charArray;
}

void WinConInterface::SetMappings(){
	for (u8 c='A';c<='Z';++c){
		keyMapping[c] = (KeyEnum)c;
	}
	
	for (u8 c='0';c<='9';++c){
		keyMapping[c] = (KeyEnum)c;
	}
	
	for (u8 c=0;c<12;++c){
		keyMapping[c+VK_F1] = (KeyEnum)((s32)KeyEnum::F1+c);
	}
	
	keyMapping[VK_ESCAPE] = KeyEnum::Escape;
	keyMapping[VK_RETURN] = KeyEnum::Enter;
	keyMapping[VK_TAB] = KeyEnum::Tab;
	keyMapping[VK_SPACE] = KeyEnum::Space;
	keyMapping[VK_PRIOR] = KeyEnum::PageUp;
	keyMapping[VK_NEXT] = KeyEnum::PageDown;
	keyMapping[VK_END] = KeyEnum::End;
	keyMapping[VK_HOME] = KeyEnum::Home;
	keyMapping[VK_LEFT] = KeyEnum::Left;
	keyMapping[VK_RIGHT] = KeyEnum::Right;
	keyMapping[VK_UP] = KeyEnum::Up;
	keyMapping[VK_DOWN] = KeyEnum::Down;
	keyMapping[VK_BACK] = KeyEnum::Backspace;
	keyMapping[VK_DELETE] = KeyEnum::Delete;
}

s32 WinConInterface::GetWidth(){
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(outHandle,&info);
	
	return info.dwSize.X;
}

s32 WinConInterface::GetHeight(){
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(outHandle,&info);
	
	return info.srWindow.Bottom+1;
}

inline void SetColor(Color fg,Color bg){
	currChar += swprintf(currChar,CSI L"38;2;%d;%d;%dm",fg.r,fg.g,fg.b);
	currChar += swprintf(currChar,CSI L"48;2;%d;%d;%dm",bg.r,bg.g,bg.b);
}

inline void SetBold(){
	currChar += swprintf(currChar,CSI L"1m");
}

inline void SetUnbold(){
	currChar += swprintf(currChar,CSI L"22m");
}

inline void SetStyle(const TextStyle& style){
	SetColor(style.fg,style.bg);
	if (style.flags & StyleFlag::Bold){
		SetBold();
	} else {
		SetUnbold();
	}
}

void WinConInterface::RenderScreen(const TextScreen& textScreen){
	s32 w = textScreen.GetWidth(), h = textScreen.GetHeight();
	currChar = charArray;
	
	TextStyle currStyle = textScreen[0].style;
	SetStyle(currStyle);
	
	for (s32 y=0;y<h;++y){
		for (s32 x=0;x<w;++x){
			auto cell = textScreen[y*w+x];
			
			if (cell.style!=currStyle){
				currStyle = cell.style;
				SetStyle(currStyle);
			}

			*currChar++ = (wchar_t)cell.c;
		}
	}
	
	*currChar = 0;
	SetConsoleCursorPosition(outHandle,{0,0});
	wprintf(CSI L"?25l"); //hide cursor
	WriteConsoleW(outHandle,charArray,wcslen(charArray),NULL,NULL);
}

void WinConInterface::GetEvent(){
	DWORD eventCount = 0;
	ReadConsoleInput(inHandle,&consoleEvent,1,&eventCount);
}

KeyboardEvent* WinConInterface::GetKeyboardEvent(){
	lastEvent = {0,0};
	
	GetEvent();
	
	KEY_EVENT_RECORD keyEvent;
	
	if (consoleEvent.EventType==WINDOW_BUFFER_SIZE_EVENT){
		auto s = consoleEvent.Event.WindowBufferSizeEvent.dwSize;
		ResizeScreen(s.X,s.Y);
		return &lastEvent;
	}

	if (consoleEvent.EventType==KEY_EVENT){
		keyEvent = consoleEvent.Event.KeyEvent;
		
		if (keyEvent.wVirtualKeyCode==VK_SHIFT){
			shiftDown = keyEvent.bKeyDown;
		} else if (keyEvent.wVirtualKeyCode==VK_CONTROL){
			ctrlDown = keyEvent.bKeyDown;
		} else if (keyEvent.wVirtualKeyCode==VK_MENU){
			altDown = keyEvent.bKeyDown;
		} else if (keyEvent.bKeyDown){
			if (keyEvent.wVirtualKeyCode>='0'&&keyEvent.wVirtualKeyCode<='9'&&shiftDown)
				lastEvent.key = keyEvent.uChar.AsciiChar;
			else if (keyMapping.contains(keyEvent.wVirtualKeyCode))
				lastEvent.key = (s32)keyMapping[keyEvent.wVirtualKeyCode];
			else
				lastEvent.key = keyEvent.uChar.AsciiChar;
		}
	}
	
	if (!lastEvent.key){
		return nullptr;
	}
	
	lastEvent.mod = 0;
	
	if (shiftDown)
		lastEvent.mod |= KeyModifier::Shift;
	
	if (ctrlDown)
		lastEvent.mod |= KeyModifier::Ctrl;
	
	if (altDown)
		lastEvent.mod |= KeyModifier::Alt;
	
	
	if (lastEvent.key){
		LOG("After :" << lastEvent.key << ", " << lastEvent.mod);
	}
	
	return &lastEvent;
}

void WinConInterface::ResizeScreen(s32 w,s32 h){
	if (charArray)
		delete[] charArray;
	charArray = new wchar_t[w*h*39+1]; //probably will be big enough
	totalSize = w*h*40+1;
}

#endif
