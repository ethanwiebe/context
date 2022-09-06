#pragma once

#include "interface.h"
#include "../logger.h"

#include <locale.h>
#include <ncurses.h>

#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <map>

struct CursesKeyBind {
	KeyEnum key;
	KeyModifier mod;
};

typedef std::map<s32,CursesKeyBind> CursesKeyMapping;
typedef std::pair<Color,Color> ColorPair;

class CursesInterface : public TextInterfaceBase {
	KeyboardEvent lastEvent;
	cchar_t* charArray;

	s32 definedColors, definedPairs;
	
	size_t missCount;

	std::vector<Color> colorDefinitions;
	std::vector<ColorPair> pairDefinitions;

	CursesKeyMapping keyMapping;
public:
	CursesInterface();
	~CursesInterface();

	void RenderScreen(const TextScreen&) override;
	KeyboardEvent* GetKeyboardEvent() override;

	s32 GetHeight() override;
	s32 GetWidth() override;

private:
	void InitColorPairs();
	void ListColorPairs();
	inline s32 ColorsToPair(s32,s32) const;
	
	void WindowResized(s32,s32);

	s32 DefinePair(Color,Color);
	s32 DefineColor(Color);
	void SetMappings();
};

