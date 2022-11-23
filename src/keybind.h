#pragma once

#include "key.h"
#include "config.h"

#include <unordered_map>
#include <vector>

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

typedef std::vector<KeyBind> BindVec;
typedef std::unordered_map<s16,BindVec> KeyMap;
typedef std::unordered_map<KeyBind,s16> InvKeyMap;
typedef std::unordered_map<std::string,s16> ActionNameMap;

struct BindSet {
	KeyMap keyMap;
	InvKeyMap boundMap;
	ActionNameMap nameMap;
};

typedef std::unordered_map<std::string,BindVec> ProcBindMap;
typedef std::unordered_map<KeyBind,std::string> ProcInvBindMap;

struct ProcBindSet {
	ProcBindMap map;
	ProcInvBindMap invMap;
	
	void SetBinds(const std::string& procName,const BindVec& binds){
		map[procName] = binds;
		UpdateBinds();
	}
	
	void UpdateBinds(){
		invMap.clear();
		for (const auto& [procName, bindVec] : map)
			for (const auto& keybind : bindVec)
				invMap[keybind] = procName;
	}
};

typedef std::unordered_map<std::string,BindSet> BindDict;

extern BindDict gBinds;

void AddBind(BindSet&,s16 action,KeyBind bind);
void SetGlobalBinds();
void UpdateBinds(BindSet&);
s16 FindActionFromKey(const BindSet&,KeyEnum,KeyModifier);

