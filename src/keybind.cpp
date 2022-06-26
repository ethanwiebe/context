#include "keybind.h"

#include "util.h"

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

	ADD_BIND(Action::MoveScreenUpLine,    KeyEnum::PageUp,     KeyModifier::Shift);
	ADD_BIND(Action::MoveScreenDownLine,  KeyEnum::PageDown,   KeyModifier::Shift);

	ADD_BIND(Action::MoveUpPage,          KeyEnum::PageUp,     KeyModifier::None);
	ADD_BIND(Action::MoveDownPage,        KeyEnum::PageDown,   KeyModifier::None);

	ADD_BIND(Action::InsertLine,          KeyEnum::Enter,      KeyModifier::None);
	ADD_BIND(Action::InsertLine,          KeyEnum::Enter,      KeyModifier::Shift);
	ADD_BIND(Action::InsertLine,          'j',                 KeyModifier::Ctrl);
	ADD_BIND(Action::InsertLine,          'm',                 KeyModifier::Ctrl|KeyModifier::Alt);
	
	ADD_BIND(Action::InsertLineBelow,     'p',                 KeyModifier::Alt);
	ADD_BIND(Action::InsertLineAbove,     'p',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(Action::MoveToLineStart,     KeyEnum::Home,       KeyModifier::None);
	ADD_BIND(Action::MoveToLineStart,     'y',                 KeyModifier::Alt);
	ADD_BIND(Action::MoveToBufferStart,   'y',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(Action::MoveToLineEnd,       KeyEnum::End,        KeyModifier::None);
	ADD_BIND(Action::MoveToLineEnd,       'o',                 KeyModifier::Alt);
	ADD_BIND(Action::MoveToBufferEnd,     'o',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(Action::DeletePreviousChar,  KeyEnum::Backspace,  KeyModifier::None);
	ADD_BIND(Action::DeletePreviousChar,  KeyEnum::Backspace,  KeyModifier::Ctrl);
	ADD_BIND(Action::DeletePreviousChar,  KeyEnum::Backspace,  KeyModifier::Shift);
	ADD_BIND(Action::DeletePreviousChar,  'u',                 KeyModifier::Alt);
	
	ADD_BIND(Action::DeleteCurrentChar,   KeyEnum::Delete,     KeyModifier::None);
	ADD_BIND(Action::DeleteCurrentChar,   'i',                 KeyModifier::Alt);

	ADD_BIND(Action::DeletePreviousMulti, 'u',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(Action::DeleteCurrentMulti,  KeyEnum::Delete,     KeyModifier::Ctrl);
	ADD_BIND(Action::DeleteCurrentMulti,  'i',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(Action::Tab,                 KeyEnum::Tab,        KeyModifier::None);
	ADD_BIND(Action::Tab,                 'i',                 KeyModifier::Ctrl);
	
	ADD_BIND(Action::Untab,               KeyEnum::Tab,        KeyModifier::Shift);

	ADD_BIND(Action::DeleteLine,          'd',                 KeyModifier::Ctrl);

	ADD_BIND(Action::UndoAction,          'z',                 KeyModifier::Ctrl);
	ADD_BIND(Action::RedoAction,          'y',                 KeyModifier::Ctrl);

	ADD_BIND(Action::ToggleSelect,        's',                 KeyModifier::Alt);
	
	ADD_BIND(Action::SelectAll,           'a',                 KeyModifier::Ctrl);
	
	ADD_BIND(Action::Cut,                 'x',                 KeyModifier::Ctrl);
	ADD_BIND(Action::CutLines,            'x',                 KeyModifier::Alt);
	ADD_BIND(Action::Copy,                'c',                 KeyModifier::Ctrl);
	ADD_BIND(Action::CopyLines,           'c',                 KeyModifier::Alt);
	ADD_BIND(Action::Paste,               'v',                 KeyModifier::Ctrl);
	ADD_BIND(Action::PasteLines,          'v',                 KeyModifier::Alt);

	ADD_BIND(Action::Escape,              KeyEnum::Escape,     KeyModifier::None);
	
	ADD_BIND(Action::Entry,               'e',                 KeyModifier::Ctrl);
	
	ADD_BIND(Action::CloseMode,           'q',                 KeyModifier::Ctrl);
	ADD_BIND(Action::NewMode,             'n',                 KeyModifier::Ctrl);
	ADD_BIND(Action::NextMode,            KeyEnum::PageDown,   KeyModifier::Ctrl);
	ADD_BIND(Action::PreviousMode,        KeyEnum::PageUp,     KeyModifier::Ctrl);
	ADD_BIND(Action::OpenMode,            'o',                 KeyModifier::Ctrl);
	ADD_BIND(Action::SaveMode,            's',                 KeyModifier::Ctrl);
	ADD_BIND(Action::RenameMode,          KeyEnum::F2,         KeyModifier::None);
	
	ADD_BIND(Action::Mode1,               '1',                 KeyModifier::Ctrl);
	ADD_BIND(Action::Mode2,               '2',                 KeyModifier::Ctrl);
	ADD_BIND(Action::Mode3,               '3',                 KeyModifier::Ctrl);
	ADD_BIND(Action::Mode4,               '4',                 KeyModifier::Ctrl);
	ADD_BIND(Action::Mode5,               '5',                 KeyModifier::Ctrl);
	ADD_BIND(Action::Mode6,               '6',                 KeyModifier::Ctrl);
	ADD_BIND(Action::Mode7,               '7',                 KeyModifier::Ctrl);
	ADD_BIND(Action::Mode8,               '8',                 KeyModifier::Ctrl);
	ADD_BIND(Action::Mode9,               '9',                 KeyModifier::Ctrl);
	ADD_BIND(Action::Mode10,              '0',                 KeyModifier::Ctrl);
	
	ADD_BIND(Action::Help,                KeyEnum::F1,         KeyModifier::None);
	
	ADD_BIND(Action::Goto,                'g',                 KeyModifier::Ctrl);
	ADD_BIND(Action::Find,                'f',                 KeyModifier::Ctrl);
	ADD_BIND(Action::Replace,             'r',                 KeyModifier::Ctrl);

	ADD_BIND(Action::DebugAction,         KeyEnum::F11,        KeyModifier::Ctrl);

	UpdateBinds();
}

void UpdateBinds(){
	gBoundKeys.clear();
	for (const auto& [action, keybindlist] : gKeyBindings)
		for (const auto& keybind : keybindlist)
			gBoundKeys[keybind] = action;
}

Action FindActionFromKey(KeyEnum key,KeyModifier mod){
	KeyBind kb = {(KeyEnum)CharLower((s32)key),mod};
	if (!gBoundKeys.contains(kb))
		return Action::None;

	return gBoundKeys.at(kb);
}

TextAction GetTextActionFromKey(KeyEnum key,KeyModifier mod){
	TextAction textAction;

	if (IsPrintable((s32)key,mod)){
		textAction.action = Action::InsertChar;
		if (!(mod & KeyModifier::Shift) && (char)key>='A' && (char)key<='Z')
			textAction.character = (char)key+0x20;
		else
			textAction.character = (char)key;
	} else {
		textAction.action = FindActionFromKey(key,mod);
		switch (textAction.action){
			case Action::MoveLeftChar:
			case Action::MoveRightChar:
			case Action::MoveUpLine:
			case Action::MoveDownLine:
			case Action::MoveScreenUpLine:
			case Action::MoveScreenDownLine:
				textAction.num = 1;
				break;
			case Action::MoveLeftMulti:
			case Action::MoveRightMulti:
			case Action::MoveUpMulti:
			case Action::MoveDownMulti:
			case Action::DeletePreviousMulti:
			case Action::DeleteCurrentMulti:
				textAction.num = gConfig.multiAmount;
				break;
			
			default:
				break;
		}
	}

	return textAction;
}
