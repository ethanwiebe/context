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

size_t ReplaceAll(TextBuffer& buf,std::string_view find,std::string_view replace,LineDiffInfo& diffs){
	size_t loc;
	size_t replaceCount = 0;
	LineIndexedIterator currLine = {buf.begin()};
	LineDiff currDiff;
	bool hit;
	while (currLine.it!=buf.end()){
		hit = false;
		loc = currLine.it->find(find);
		if (loc!=std::string::npos){
			currDiff.location = currLine;
			currDiff.before = *currLine.it;
			hit = true;
		}
		
		while (loc!=std::string::npos){
			currLine.it->replace(loc,find.size(),replace);
			++replaceCount;
			loc = currLine.it->find(find,loc+replace.size());
		}
		
		if (hit){
			currDiff.after = *currLine.it;
			diffs.push_back(currDiff);
		}
		
		++currLine;
	}
	return replaceCount;
}
