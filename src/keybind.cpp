#include "keybind.h"

#include "util.h"

#define ADD_BIND(action,key,mod) AddBind(global,(s16)(action),{(KeyEnum)(key),(KeyModifier)(mod)})

BindDict gBinds;

void SetGlobalBinds(){
	BindSet global = {};
	
	#define ACTION(name) global.nameMap[#name] = (s16)Action::name;
	#include "actions.h"
	#undef ACTION
	
	ADD_BIND(Action::Entry,               'e',                 KeyModifier::Ctrl);
	
	ADD_BIND(Action::CloseMode,           'q',                 KeyModifier::Ctrl);
	ADD_BIND(Action::NewMode,             'n',                 KeyModifier::Ctrl|KeyModifier::Shift);
	ADD_BIND(Action::NextMode,            KeyEnum::PageDown,   KeyModifier::Ctrl);
	ADD_BIND(Action::PreviousMode,        KeyEnum::PageUp,     KeyModifier::Ctrl);
	ADD_BIND(Action::NextMode,            'm',                 KeyModifier::Ctrl);
	ADD_BIND(Action::PreviousMode,        'n',                 KeyModifier::Ctrl);
	ADD_BIND(Action::OpenMode,            'o',                 KeyModifier::Ctrl);
	ADD_BIND(Action::SaveMode,            's',                 KeyModifier::Ctrl);
	ADD_BIND(Action::RenameMode,          KeyEnum::F2,         KeyModifier::None);
	
	ADD_BIND(Action::MoveModeToNext,      KeyEnum::PageDown,   KeyModifier::Ctrl|KeyModifier::Alt);
	ADD_BIND(Action::MoveModeToPrevious,  KeyEnum::PageUp,     KeyModifier::Ctrl|KeyModifier::Alt);
	ADD_BIND(Action::MoveModeToNext,      'm',                 KeyModifier::Ctrl|KeyModifier::Alt);
	ADD_BIND(Action::MoveModeToPrevious,  'n',                 KeyModifier::Ctrl|KeyModifier::Alt);
	
	ADD_BIND(Action::Mode1,               '1',                 KeyModifier::Alt);
	ADD_BIND(Action::Mode2,               '2',                 KeyModifier::Alt);
	ADD_BIND(Action::Mode3,               '3',                 KeyModifier::Alt);
	ADD_BIND(Action::Mode4,               '4',                 KeyModifier::Alt);
	ADD_BIND(Action::Mode5,               '5',                 KeyModifier::Alt);
	ADD_BIND(Action::Mode6,               '6',                 KeyModifier::Alt);
	ADD_BIND(Action::Mode7,               '7',                 KeyModifier::Alt);
	ADD_BIND(Action::Mode8,               '8',                 KeyModifier::Alt);
	ADD_BIND(Action::Mode9,               '9',                 KeyModifier::Alt);
	ADD_BIND(Action::Mode10,              '0',                 KeyModifier::Alt);
	
	ADD_BIND(Action::Help,                KeyEnum::F1,         KeyModifier::None);
	
	gBinds["ctx"] = global;
}

void UpdateBinds(BindSet& page){
	page.boundMap.clear();
	for (const auto& [action, keybindlist] : page.keyMap)
		for (const auto& keybind : keybindlist)
			page.boundMap[keybind] = action;
}

s16 FindActionFromKey(const BindSet& binds,KeyEnum key,KeyModifier mod){
	KeyBind kb = {(KeyEnum)CharLower((s32)key),mod};
	if (!binds.boundMap.contains(kb))
		return ACTION_NOT_FOUND;

	return binds.boundMap.at(kb);
}

void AddBind(BindSet& page,s16 action,KeyBind bind){
	if (!page.keyMap.contains(action)){
		page.keyMap[action] = {};
	}
	page.keyMap[action].push_back(bind);
}
