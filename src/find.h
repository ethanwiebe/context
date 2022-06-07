#pragma once

#include "core.h"

#include "textbuffer.h"
#include "cursor.h"

#include <vector>
#include <string_view>

typedef std::vector<Cursor> FoundList;

void FindAllMatches(TextBuffer&,FoundList&,std::string_view);
