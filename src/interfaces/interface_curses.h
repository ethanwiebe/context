#pragma once

#include "interface.h"
#include "../logger.h"

#include <ncurses.h>

#include <string>
#include <string_view>
#include <utility>

typedef std::pair<Color,Color> ColorPair;

class CursesInterface : public TextInterfaceBase {
	KeyboardEvent lastEvent;
	chtype* charArray;

	s32 definedColors, definedPairs;

	std::vector<Color> colorDefinitions;
	std::vector<ColorPair> pairDefinitions;

public:
	CursesInterface();
	~CursesInterface();

	void RenderScreen(const TextScreen&) override;
	KeyboardEvent* GetKeyboardEvent() override;

	void WindowResized(s32,s32) override;

	s32 GetHeight() override;
	s32 GetWidth() override;

private:
	void InitColorPairs();
	void ListColorPairs();
	inline s32 ColorsToPair(s32,s32) const;

	s32 DefinePair(Color,Color);
	s32 DefineColor(Color);
};

inline void NormalizeKeys(s32&,s32&);
