#include "key.h"

s32 CharLower(s32 c){
	if (c>=65&&c<=90) return c+32;
	return c;
}

s32 IsPrintable(s32 key,s32 mod){
	return (key>=' '&&key<='~'&&!(mod&(KeyModifier::Ctrl|KeyModifier::Alt)));
}

bool ActionIsCommand(Action a){
	switch (a){
		case Action::OpenMode:
		case Action::CloseMode:
		case Action::NewMode:
		case Action::NextMode:
		case Action::PreviousMode:
		case Action::SaveMode:
		case Action::SaveAsMode:
			return true;

		default:
			return false;
	}

	return false;
}
