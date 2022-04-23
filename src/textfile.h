#pragma once

#include "core.h"
#include "config.h"

#include <string>
#include <string_view>
#include <list>

typedef std::list<std::string> LineList;
typedef LineList::iterator LineIterator;
typedef std::string_view LineView;

struct IndexedIterator {
	LineIterator it;
	s32 index;

	IndexedIterator() = default;
	IndexedIterator(LineIterator lineIt) : it(lineIt), index(0){}

	IndexedIterator& operator+=(s32);

	IndexedIterator& operator-=(s32);

	IndexedIterator& operator++();

	IndexedIterator& operator--();
};

class TextFile {
	LineList lines;
public:
	
	LineIterator GetLineIterator(size_t) noexcept;

	void SetLine(LineIterator,const std::string&);
	void InsertLine(LineIterator,const std::string&); //increases length of line list by one
	void InsertLineAfter(LineIterator,const std::string&);
	void BackDeleteLine(LineIterator);
	void ForwardDeleteLine(LineIterator);

	LineIterator begin() noexcept;
	LineIterator end() noexcept;

	size_t size() const noexcept;
};

void UpdateXI(const std::string&, s32&, s32&, s32);
s32 GetXPosOfIndex(const std::string&, s32, s32);
s32 GetIndexOfXPos(const std::string&, s32, s32);
