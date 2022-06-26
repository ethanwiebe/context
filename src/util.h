#pragma once

#include "core.h"

s32 CharLower(s32);
s32 IsPrintable(s32,s32);

inline bool IsAlphabet(char c){
	return (c>='a'&&c<='z')||(c>='A'&&c<='Z');
}

std::string StringPostEllipsis(const std::string& str,size_t max);

