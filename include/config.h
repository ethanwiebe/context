#pragma once

#include "core.h"

enum class TabMode : u8 {
	Spaces,
	Tabs
};

enum class MultiMode : u8 {
	Multi,
	Word,
	PascalWord
};

struct Config {
	s64 tabSize;
	s64 cursorMoveHeight;
	s64 multiAmount;

	bool displayLineNumbers;
	bool autoIndent;
	bool cursorLock;
	bool cursorWrap;
	bool smartHome;
	TabMode tabMode;
	MultiMode moveMode;
	MultiMode deleteMode;
};

enum class ConfigPropType : u8 {
	Number,
	String,
	Bool,
	List,
	Dict
};

struct ConfigProp {
	const char* name;
	ConfigPropType type;
};

extern Config gConfig;

const s64 gConfigPropCount = 11;

const ConfigProp gConfigProps[gConfigPropCount] = {
	{"tabSize",                ConfigPropType::Number},
	{"cursorMoveHeight",       ConfigPropType::Number},
	{"multiAmount",            ConfigPropType::Number},
	{"displayLineNumbers",     ConfigPropType::Bool},
	{"autoIndent",             ConfigPropType::Bool},
	{"cursorLock",             ConfigPropType::Bool},
	{"cursorWrap",             ConfigPropType::Bool},
	{"smartHome",              ConfigPropType::Bool},
	{"tabMode",                ConfigPropType::Number},
	{"moveMode",               ConfigPropType::Number},
	{"deleteMode",             ConfigPropType::Number}
};

bool NameIsConfigVar(std::string_view);
