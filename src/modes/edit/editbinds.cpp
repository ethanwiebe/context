#include "editbinds.h"
#include "lineconfig.h"
#include "../../util.h"
#include "linemode.h"

#define ADD_BIND(action,key,mod) AddBind(edit,(s16)(action),{(KeyEnum)(key),(KeyModifier)(mod)})

void LineModeBase::RegisterBinds(){
	BindSet edit = {};
	
	#define ACTION(name) edit.nameMap[#name] = (s16)EditAction::name;
	#include "editactions.h"
	#undef ACTION
	
	ADD_BIND(EditAction::MoveLeftChar,        KeyEnum::Left,       KeyModifier::None);
	ADD_BIND(EditAction::MoveLeftChar,        'h',                 KeyModifier::Alt);
	ADD_BIND(EditAction::MoveLeftMulti,       KeyEnum::Left,       KeyModifier::Ctrl);
	ADD_BIND(EditAction::MoveLeftMulti,       'h',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(EditAction::MoveRightChar,       KeyEnum::Right,      KeyModifier::None);
	ADD_BIND(EditAction::MoveRightChar,       'l',                 KeyModifier::Alt);
	ADD_BIND(EditAction::MoveRightMulti,      KeyEnum::Right,      KeyModifier::Ctrl);
	ADD_BIND(EditAction::MoveRightMulti,      'l',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(EditAction::MoveUpLine,          KeyEnum::Up,         KeyModifier::None);
	ADD_BIND(EditAction::MoveUpLine,          'k',                 KeyModifier::Alt);
	ADD_BIND(EditAction::MoveUpMulti,         KeyEnum::Up,         KeyModifier::Ctrl);
	ADD_BIND(EditAction::MoveUpMulti,         'k',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(EditAction::MoveDownLine,        KeyEnum::Down,       KeyModifier::None);
	ADD_BIND(EditAction::MoveDownLine,        'j',                 KeyModifier::Alt);
	ADD_BIND(EditAction::MoveDownMulti,       KeyEnum::Down,       KeyModifier::Ctrl);
	ADD_BIND(EditAction::MoveDownMulti,       'j',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(EditAction::MoveScreenUpLine,    KeyEnum::PageUp,     KeyModifier::Shift);
	ADD_BIND(EditAction::MoveScreenDownLine,  KeyEnum::PageDown,   KeyModifier::Shift);

	ADD_BIND(EditAction::MoveUpPage,          KeyEnum::PageUp,     KeyModifier::None);
	ADD_BIND(EditAction::MoveDownPage,        KeyEnum::PageDown,   KeyModifier::None);
	
	ADD_BIND(EditAction::MoveUpPage,          '\'',                KeyModifier::Alt);
	ADD_BIND(EditAction::MoveDownPage,        ';',                 KeyModifier::Alt);

	ADD_BIND(EditAction::InsertLine,          KeyEnum::Enter,      KeyModifier::None);
	ADD_BIND(EditAction::InsertLine,          KeyEnum::Enter,      KeyModifier::Shift);
	ADD_BIND(EditAction::InsertLine,          'j',                 KeyModifier::Ctrl);
	
	ADD_BIND(EditAction::InsertLineBelow,     'p',                 KeyModifier::Alt);
	ADD_BIND(EditAction::InsertLineAbove,     'p',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(EditAction::MoveToLineStart,     KeyEnum::Home,       KeyModifier::None);
	ADD_BIND(EditAction::MoveToLineStart,     'y',                 KeyModifier::Alt);
	ADD_BIND(EditAction::MoveToBufferStart,   KeyEnum::Home,       KeyModifier::Ctrl);
	ADD_BIND(EditAction::MoveToBufferStart,   'y',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(EditAction::MoveToLineEnd,       KeyEnum::End,        KeyModifier::None);
	ADD_BIND(EditAction::MoveToLineEnd,       'o',                 KeyModifier::Alt);
	ADD_BIND(EditAction::MoveToBufferEnd,     KeyEnum::End,        KeyModifier::Ctrl);
	ADD_BIND(EditAction::MoveToBufferEnd,     'o',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(EditAction::DeletePreviousChar,  KeyEnum::Backspace,  KeyModifier::None);
	ADD_BIND(EditAction::DeletePreviousChar,  KeyEnum::Backspace,  KeyModifier::Shift);
	ADD_BIND(EditAction::DeletePreviousChar,  'u',                 KeyModifier::Alt);
	
	ADD_BIND(EditAction::DeleteCurrentChar,   KeyEnum::Delete,     KeyModifier::None);
	ADD_BIND(EditAction::DeleteCurrentChar,   'i',                 KeyModifier::Alt);

	ADD_BIND(EditAction::DeletePreviousMulti, KeyEnum::Backspace,  KeyModifier::Ctrl);
	ADD_BIND(EditAction::DeletePreviousMulti, 'u',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(EditAction::DeleteCurrentMulti,  KeyEnum::Delete,     KeyModifier::Ctrl);
	ADD_BIND(EditAction::DeleteCurrentMulti,  'i',                 KeyModifier::Ctrl|KeyModifier::Alt);

	ADD_BIND(EditAction::Tab,                 KeyEnum::Tab,        KeyModifier::None);
	ADD_BIND(EditAction::Tab,                 'i',                 KeyModifier::Ctrl);
	
	ADD_BIND(EditAction::Untab,               KeyEnum::Tab,        KeyModifier::Shift);
	ADD_BIND(EditAction::Untab,               'u',                 KeyModifier::Ctrl);

	ADD_BIND(EditAction::DeleteLine,          'd',                 KeyModifier::Ctrl);

	ADD_BIND(EditAction::UndoAction,          'z',                 KeyModifier::Ctrl);
	ADD_BIND(EditAction::RedoAction,          'y',                 KeyModifier::Ctrl);
	ADD_BIND(EditAction::UndoAction,          ',',                 KeyModifier::Alt);
	ADD_BIND(EditAction::RedoAction,          '.',                 KeyModifier::Alt);

	ADD_BIND(EditAction::ToggleSelect,        's',                 KeyModifier::Alt);
	
	ADD_BIND(EditAction::SelectAll,           'a',                 KeyModifier::Ctrl);
	
	ADD_BIND(EditAction::Cut,                 'x',                 KeyModifier::Ctrl);
	ADD_BIND(EditAction::CutLines,            'x',                 KeyModifier::Alt);
	ADD_BIND(EditAction::Copy,                'c',                 KeyModifier::Ctrl);
	ADD_BIND(EditAction::CopyLines,           'c',                 KeyModifier::Alt);
	ADD_BIND(EditAction::Paste,               'v',                 KeyModifier::Ctrl);
	ADD_BIND(EditAction::PasteLines,          'v',                 KeyModifier::Alt);

	ADD_BIND(EditAction::Escape,              KeyEnum::Escape,     KeyModifier::None);
	
	ADD_BIND(EditAction::Goto,                'g',                 KeyModifier::Ctrl);
	ADD_BIND(EditAction::Find,                'f',                 KeyModifier::Ctrl);
	ADD_BIND(EditAction::FindCase,            'f',                 KeyModifier::Alt);
	ADD_BIND(EditAction::Replace,             'r',                 KeyModifier::Ctrl);
	ADD_BIND(EditAction::ReplaceCase,         'r',                 KeyModifier::Alt);

	ADD_BIND(EditAction::DebugAction,         KeyEnum::F11,        KeyModifier::Ctrl);
	
	gBinds["edit"] = edit;
}

TextAction GetTextActionFromKey(KeyEnum key,KeyModifier mod,s64 multiAmount){
	TextAction textAction;

	if (IsPrintable((s32)key,mod)){
		textAction.action = EditAction::InsertChar;
		if (!(mod & KeyModifier::Shift) && (char)key>='A' && (char)key<='Z')
			textAction.character = (char)key+0x20;
		else
			textAction.character = (char)key;
	} else {
		textAction.action = (EditAction)FindActionFromKey(gBinds.at("edit"),key,mod);
		switch (textAction.action){
			case EditAction::MoveLeftChar:
			case EditAction::MoveRightChar:
			case EditAction::MoveUpLine:
			case EditAction::MoveDownLine:
			case EditAction::MoveScreenUpLine:
			case EditAction::MoveScreenDownLine:
				textAction.num = 1;
				break;
			case EditAction::MoveLeftMulti:
			case EditAction::MoveRightMulti:
			case EditAction::MoveUpMulti:
			case EditAction::MoveDownMulti:
			case EditAction::DeletePreviousMulti:
			case EditAction::DeleteCurrentMulti:
				textAction.num = multiAmount;
				break;
			
			default:
				break;
		}
	}

	return textAction;
}

