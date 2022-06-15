#pragma once

#include "../interface.h"

#include "../logger.h"

#include <vector>
#include <map>

typedef std::map<u8,KeyEnum> WinKeyMapping;

class WinConInterface : public TextInterfaceBase {
	KeyboardEvent lastEvent;
	bool trueColor;
	
	u8 oldKeys[256];
	u8 keys[256];

	WinKeyMapping keyMapping;
public:
	WinConInterface();
	~WinConInterface();
	
	void RenderScreen(const TextScreen&) override;
	KeyboardEvent* GetKeyboardEvent() override;

	s32 GetWidth() override;
	s32 GetHeight() override;
	
private:
	void ResizeScreen(s32,s32);
	void WaitingLoop();
	void SetMappings();
};