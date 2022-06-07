#include "find.h"

void FindAllMatches(TextBuffer& tb,FoundList& matches,std::string_view s){
	matches.clear();
	size_t loc;
	LineIndexedIterator currLine = {tb.begin()};
	while (currLine.it!=tb.end()){
		loc = currLine.it->find(s);
		while (loc!=std::string::npos){
			matches.emplace_back(currLine,loc);
			loc = currLine.it->find(s,loc+s.size());
		}
		
		++currLine;
	}
}
