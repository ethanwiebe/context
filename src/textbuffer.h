#pragma once

#include "core.h"
#include "config.h"

#include <string>
#include <string_view>
#include <list>

typedef std::list<std::string> LineList;
typedef LineList::iterator LineIterator;
typedef std::string_view LineView;

template<typename T>
struct IndexedIterator {
	T::iterator it;
	s32 index;

	IndexedIterator() = default;
	IndexedIterator(T::iterator lineIt) : it(lineIt), index(0){}
	IndexedIterator(T::iterator lineIt,s32 index) : it(lineIt), index(index){}

	IndexedIterator& operator+=(s32 n){
		while (n--){
			++index;
			if (index>0)
				++it;
		}

		return *this;
	}

	IndexedIterator& operator-=(s32 n){
		while (n--){
			--index;
			if (index>=0)
				--it;
		}

		return *this;
	}

	IndexedIterator& operator++(){
		++index;
		if (index>0)
			++it;

		return *this;
	}

	IndexedIterator& operator--(){
		--index;
		if (index>=0)
			--it;

		return *this;
	}

	IndexedIterator operator++(int){
		IndexedIterator old = *this;
		++index;
		if (index>0)
			++it;

		return old;
	}

	IndexedIterator operator--(int){
		IndexedIterator old = *this;
		--index;
		if (index>=0)
			--it;

		return old;
	}

	const auto& operator*() const {
		return *it;
	}
};

typedef IndexedIterator<LineList> LineIndexedIterator;

class TextBuffer {
	LineList lines;
public:
	
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

	s32 GetIndentationAt(LineIterator,s32=4);
	bool IsTabIndented(LineIterator);
};

void UpdateXI(const std::string&, s32&, s32&, s32);
s32 GetXPosOfIndex(const std::string&, s32, s32);
s32 GetIndexOfXPos(const std::string&, s32, s32);
