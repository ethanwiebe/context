#pragma once

#include "key.h"

#include <unordered_map>
#include <list>

struct KeyBind {
	KeyEnum key;
	KeyModifier mod;

	bool operator==(const KeyBind& other) const {
		return key==other.key && mod==other.mod;
	}
};


template <>
struct std::hash<KeyBind>
{
	size_t operator()(const KeyBind& k) const {
		return k.key ^ (k.mod<<3);
	}
};			  


typedef std::list<KeyBind> BindList;
typedef std::unordered_map<Action,BindList> KeyMap;
typedef std::unordered_map<KeyBind,Action> InvKeyMap;

void SetKeybinds();
Action FindActionFromKey(KeyEnum,KeyModifier);

