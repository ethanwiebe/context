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
	IndexedIterator(LineIterator lineIt,s32 index) : it(lineIt), index(index){}

	IndexedIterator& operator+=(s32);

	IndexedIterator& operator-=(s32);

	IndexedIterator& operator++();

	IndexedIterator& operator--();

	const std::string& operator*() const;
};

class TextBuffer {
	LineList lines;
public:
//	TextBuffer();
	
	LineIterator GetLineIterator(size_t) noexcept;

	void SetLine(LineIterator,const std::string&);
	void InsertLine(LineIterator,const std::string&); //increases length of line list by one
	void InsertLineAfter(LineIterator,const std::string&);
	void BackDeleteLine(LineIterator);
	void ForwardDeleteLine(LineIterator);

	inline LineIterator begin() noexcept {
		return lines.begin();
	}

	inline LineIterator end() noexcept {
		return lines.end();
	}

	inline size_t size() const noexcept {
		return lines.size();
	}

	void clear() noexcept;

	inline bool empty() const noexcept {
		return lines.empty();
	}
};

void UpdateXI(const std::string&, s32&, s32&, s32);
s32 GetXPosOfIndex(const std::string&, s32, s32);
s32 GetIndexOfXPos(const std::string&, s32, s32);
