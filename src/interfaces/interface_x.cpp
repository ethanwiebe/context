#ifdef __linux__

#include "interface_curses.h"
#include "../config.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <thread>
#include <chrono>

#define SHORT_SLEEP_START_THRESHOLD 40000
#define LONG_SLEEP_START_THRESHOLD 25000

using namespace std::chrono_literals;

std::map<KeySym,KeyEnum> xKeyMap;

Display* xDisplay;
Window inputGrabber;

void CursesInterface::InitX(){
	xDisplay = XOpenDisplay(NULL);
	LOG("Has X: " << (xDisplay!=nullptr));
	if (!xDisplay) return;
	
	int d;
	Window parent;
	
	XGetInputFocus(xDisplay,&parent,&d);
	
	XSelectInput(xDisplay,parent,KeyPressMask | KeyReleaseMask);
	
	xKeyMap[XK_Return] = KeyEnum::Enter;
	xKeyMap[XK_KP_Enter] = KeyEnum::Enter;
	xKeyMap[XK_Tab] = KeyEnum::Tab;
	xKeyMap[XK_ISO_Left_Tab] = KeyEnum::Tab;
	xKeyMap[XK_BackSpace] = KeyEnum::Backspace;
	xKeyMap[XK_Delete] = KeyEnum::Delete;
	xKeyMap[XK_Insert] = KeyEnum::Insert;
	xKeyMap[XK_Home] = KeyEnum::Home;
	xKeyMap[XK_KP_Home] = KeyEnum::Home;
	xKeyMap[XK_End] = KeyEnum::End;
	xKeyMap[XK_KP_End] = KeyEnum::End;
	xKeyMap[XK_Page_Down] = KeyEnum::PageDown;
	xKeyMap[XK_Page_Up] = KeyEnum::PageUp;
	xKeyMap[XK_Escape] = KeyEnum::Escape;
	xKeyMap[XK_Up] = KeyEnum::Up;
	xKeyMap[XK_Down] = KeyEnum::Down;
	xKeyMap[XK_Left] = KeyEnum::Left;
	xKeyMap[XK_Right] = KeyEnum::Right;
	xKeyMap[XK_KP_Up] = KeyEnum::Up;
	xKeyMap[XK_KP_Down] = KeyEnum::Down;
	xKeyMap[XK_KP_Left] = KeyEnum::Left;
	xKeyMap[XK_KP_Right] = KeyEnum::Right;
}

KeyboardEvent* CursesInterface::XKeyboardEvent(){
	if (!xDisplay){
		return CursesKeyboardEvent();
	}
	
	// pump curses system
	s32 key = getch();
	if (key==KEY_RESIZE){
		resize_term(0,0);
		WindowResized(COLS,LINES);
		lastEvent = {0,0};
		return &lastEvent;
	}
	
	XEvent xe;
	if (XPending(xDisplay)){
		XNextEvent(xDisplay,&xe);
		
		if (xe.type==KeyPress){
			s32 mod = 0;
			auto state = xe.xkey.state;
			
			if (((state & ShiftMask) != 0) ^ ((state & LockMask) != 0))
				mod |= 1; // KeyModifier::Shift
			if (state & ControlMask)
				mod |= 2; // KeyModifier::Ctrl
			if (state & Mod1Mask)
				mod |= 4; // KeyModifier::Alt
			
			KeySym sym = XLookupKeysym(&xe.xkey,state & 1);
			
			// numlock handling
			if (sym>=XK_KP_Space&&sym<=XK_KP_9){
				if (state & Mod2Mask){
					sym = XLookupKeysym(&xe.xkey,1);
				} else {
					sym = XLookupKeysym(&xe.xkey,0);
				}
			}
			
			LOG("Before: " << (s32)sym << ',' <<  mod);
			
			KeySym upper,lower;
			
			XConvertCase(sym,&lower,&upper);
			
			u8 val = (u8)sym;
			if (mod & 1){
				val = (u8)upper;
			}
			
			lastEvent = {(s32)val,mod};
			if (sym>=XK_F1&&sym<=XK_F12){
				lastEvent.key = sym-XK_F1 + (s32)KeyEnum::F1;
			} else if (sym>=XK_KP_Space&&sym<=XK_KP_9){
				lastEvent.key = sym-XK_KP_0 + (s32)'0';
			} else if (xKeyMap.contains(sym)){
				lastEvent.key = (s32)xKeyMap.at(sym);
			} else if (sym>0xff){
				return nullptr;
			}
			
			LOG("Key Press: " << (s32)lastEvent.key << ',' << (s32)lastEvent.mod);
			missCount = 0;
			return &lastEvent;
		}
	}
	
	if (missCount>LONG_SLEEP_START_THRESHOLD&&gConfig.sleepy)
		std::this_thread::sleep_for(50ms);
	else if (missCount>SHORT_SLEEP_START_THRESHOLD||gConfig.sleepy)
		std::this_thread::sleep_for(5ms);
		
	++missCount;
	
	return nullptr;
}

#endif
