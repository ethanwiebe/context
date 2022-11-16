#include "editconfig.h"
#include "linemode.h"

EditConfig gEditConfig = {
	.tabSize = 4,
	.cursorMoveHeight = 3,
	.multiAmount = 4,
	.displayLineNumbers = true,
	.autoIndent = true,
	.cursorLock = false,
	.cursorWrap = false,
	.smartHome = true,
	.tabMode = TabMode::Tabs,
	.moveMode = MultiMode::Multi,
	.deleteMode = MultiMode::Word
};

const char* tabEnumValues[] = {
	"spaces",
	"tabs",
	NULL
};

const char* multiEnumValues[] = {
	"multi",
	"word",
	"pascal"
};

ModeOption gEditModeOps[11] = {
	{
		.type = OptionType::Bool,
		.name = "autoIndent",
		.boolLoc = &gEditConfig.autoIndent	
	},
	{
		.type = OptionType::Bool,
		.name = "cursorLock",
		.boolLoc = &gEditConfig.cursorLock
	},
	{
		.type = OptionType::Bool,
		.name = "displayLineNumbers",
		.boolLoc = &gEditConfig.displayLineNumbers
	},
	{
		.type = OptionType::Bool,
		.name = "smartHome",
		.boolLoc = &gEditConfig.smartHome
	},
	{
		.type = OptionType::Bool,
		.name = "cursorWrap",
		.boolLoc = &gEditConfig.cursorWrap
	},
	{
		.type = OptionType::PositiveInt,
		.name = "tabSize",
		.intLoc = &gEditConfig.tabSize
	},
	{
		.type = OptionType::NonNegativeInt,
		.name = "cursorMoveHeight",
		.intLoc = &gEditConfig.cursorMoveHeight
	},
	{
		.type = OptionType::PositiveInt,
		.name = "multiAmount",
		.intLoc = &gEditConfig.multiAmount
	},
	{
		.type = OptionType::Enum,
		.name = "tabMode",
		.enumLoc = (u64*)&gEditConfig.tabMode,
		.enumValues = tabEnumValues
	},
	{
		.type = OptionType::Enum,
		.name = "moveMode",
		.enumLoc = (u64*)&gEditConfig.moveMode,
		.enumValues = multiEnumValues
	},
	{
		.type = OptionType::Enum,
		.name = "deleteMode",
		.enumLoc = (u64*)&gEditConfig.deleteMode,
		.enumValues = multiEnumValues
	}
};

