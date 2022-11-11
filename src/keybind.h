#pragma once

#include "key.h"
#include "config.h"

#include <unordered_map>
#include <list>

#define ACTION_NOT_FOUND (-1)

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
typedef std::unordered_map<s16,BindList> KeyMap;
typedef std::unordered_map<KeyBind,s16> InvKeyMap;
typedef std::unordered_map<std::string,s16> ActionNameMap;

struct BindSet {
	KeyMap keyMap;
	InvKeyMap boundMap;
	ActionNameMap nameMap;
};

typedef std::unordered_map<std::string,BindSet> BindDict;

extern BindDict gBinds;

void AddBind(BindSet&,s16 action,KeyBind bind);
void SetGlobalBinds();
void UpdateBinds(BindSet&);
s16 FindActionFromKey(const BindSet&,KeyEnum,KeyModifier);

