#pragma once

#include "key.h"
#include "config.h"

#include <unordered_map>
#include <list>

#define ADD_BIND(action,key,mod) { \
	if (!gKeyBindings.contains((action))){ \
		gKeyBindings[(action)] = {}; \
	} \
	gKeyBindings[(action)].emplace_back((KeyEnum)(key), (KeyModifier)(mod)); \
}

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
		return (size_t)k.key ^ ((size_t)k.mod<<3);
	}
};			  


typedef std::list<KeyBind> BindList;
typedef std::unordered_map<Action,BindList> KeyMap;
typedef std::unordered_map<KeyBind,Action> InvKeyMap;

extern KeyMap gKeyBindings;

void SetKeybinds();
void UpdateBinds();
Action FindActionFromKey(KeyEnum,KeyModifier);
TextAction GetTextActionFromKey(KeyEnum,KeyModifier);
