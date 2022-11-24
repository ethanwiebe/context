#pragma once

#include <core.h>

#include <textbuffer.h>
#include "cursor.h"

#include <vector>
#include <string_view>

typedef std::vector<Cursor> FoundList;

struct LineDiff {
	LineIndexedIterator location;
	std::string before,after;
};

typedef std::vector<LineDiff> LineDiffInfo;

void FindAllMatches(TextBuffer&,FoundList&,std::string_view);
void FindAllMatchesUncased(TextBuffer&,FoundList&,std::string_view);

//returns number of successful replaces
size_t ReplaceAll(TextBuffer&,std::string_view,std::string_view,LineDiffInfo&);
size_t ReplaceAllUncased(TextBuffer&,std::string_view,std::string_view,LineDiffInfo&);
