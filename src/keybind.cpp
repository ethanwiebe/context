#include "keybind.h"

#define ADD_BIND(action,key,mod) { \
	if (!gKeyBindings.contains((action))){ \
		gKeyBindings[(action)] = {}; \
	} \
	gKeyBindings[(action)].emplace_back((KeyEnum)(key), (KeyModifier)(mod)); \
}

KeyMap gKeyBindings;
InvKeyMap gBoundKeys;

void SetKeybinds(){
	ADD_BIND(Action::MoveLeftChar,        KeyEnum::Left,       KeyModifier::None);
	ADD_BIND(Action::MoveLeftChar,        'h',                 KeyModifier::Alt);
	ADD_BIND(Action::MoveLeftMulti,       'h',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(Action::MoveRightChar,       KeyEnum::Right,      KeyModifier::None);
	ADD_BIND(Action::MoveRightChar,       'l',                 KeyModifier::Alt);
	ADD_BIND(Action::MoveRightMulti,      'l',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(Action::MoveUpLine,          KeyEnum::Up,         KeyModifier::None);
	ADD_BIND(Action::MoveUpLine,          'k',                 KeyModifier::Alt);
	ADD_BIND(Action::MoveUpMulti,         'k',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(Action::MoveDownLine,        KeyEnum::Down,       KeyModifier::None);
	ADD_BIND(Action::MoveDownLine,        'j',                 KeyModifier::Alt);
	ADD_BIND(Action::MoveDownMulti,       'j',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(Action::MoveScreenUpLine,    KeyEnum::PageUp,     KeyModifier::None);
	ADD_BIND(Action::MoveScreenDownLine,  KeyEnum::PageDown,   KeyModifier::None);

	ADD_BIND(Action::InsertLine,          KeyEnum::Enter,      KeyModifier::None);
	ADD_BIND(Action::InsertLine,          KeyEnum::Enter,      KeyModifier::Shift);
	ADD_BIND(Action::InsertLine,          'j',                 KeyModifier::Ctrl);

	ADD_BIND(Action::MoveToLineStart,     KeyEnum::Home,       KeyModifier::None);
	ADD_BIND(Action::MoveToLineStart,     'y',                 KeyModifier::Alt);
	ADD_BIND(Action::MoveToBufferStart,   'y',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(Action::MoveToLineEnd,       KeyEnum::End,        KeyModifier::None);
	ADD_BIND(Action::MoveToLineEnd,       'o',                 KeyModifier::Alt);
	ADD_BIND(Action::MoveToBufferEnd,     'o',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(Action::DeletePreviousChar,  KeyEnum::Backspace,  KeyModifier::None);
	ADD_BIND(Action::DeletePreviousChar,  'g',                 KeyModifier::Alt);
	
	ADD_BIND(Action::DeleteCurrentChar,   KeyEnum::Delete,     KeyModifier::None);
	ADD_BIND(Action::DeleteCurrentChar,   ';',                 KeyModifier::Alt);

	ADD_BIND(Action::InsertTab,           KeyEnum::Tab,        KeyModifier::None);
	ADD_BIND(Action::InsertTab,           'i',                 KeyModifier::Ctrl);

	for (const auto& [action, keybindlist] : gKeyBindings)
		for (const auto& keybind : keybindlist)
			gBoundKeys[keybind] = action;
}


Action FindActionFromKey(KeyEnum key,KeyModifier mod){
	KeyBind kb = {(KeyEnum)CharLower(key),mod};
	if (!gBoundKeys.contains(kb))
		return Action::None;

	return gBoundKeys.at(kb);
}
