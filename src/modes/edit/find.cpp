#include "find.h"

inline size_t FindInString(std::string_view searchStr,std::string_view str,size_t start){
	for (size_t i=start;i<str.size();++i){
		if (str[i]==searchStr[0]){
			bool found = true;
			for (size_t j=1;j<searchStr.size();++j){
				if (str[i+j]!=searchStr[j]){
					found = false;
					break;
				}
			}
			if (found){
				return i;
			}
		}
	}
	return std::string::npos;
}

inline bool UncasedCompare(char a,char b){
	if (a>='A'&&a<='Z') a += ('a'-'A');
	if (b>='A'&&b<='Z') b += ('a'-'A');
	return a==b;
}

inline size_t FindInStringUncased(std::string_view searchStr,std::string_view str,size_t start){
	for (size_t i=start;i<str.size();++i){
		if (UncasedCompare(str[i],searchStr[0])){
			bool found = true;
			for (size_t j=1;j<searchStr.size();++j){
				if (!UncasedCompare(str[i+j],searchStr[j])){
					found = false;
					break;
				}
			}
			if (found){
				return i;
			}
		}
	}
	return std::string::npos;
}

void FindAllMatches(TextBuffer& tb,FoundList& matches,std::string_view s,Cursor start,Cursor end){
	matches.clear();
	if (start==end) return;
	size_t loc = start.column;
	LineIndexedIterator currLine = start.line;
	auto endIt = end.line.it;
	while (currLine.it!=endIt){
		if (currLine.it!=start.line.it) loc = 0;
		
		loc = FindInString(s,*currLine.it,loc);
		while (loc!=std::string::npos){
			matches.emplace_back(currLine,loc);
			loc = FindInString(s,*currLine.it,loc+s.size());
		}
		++currLine;
	}
	
	// search on last line (if it exists)
	if (endIt!=tb.end()){
		loc = FindInString(s,*endIt,0);
		while (loc<=(size_t)end.column){
			// currLine.it is endIt
			matches.emplace_back(currLine,loc);
			loc = FindInString(s,*endIt,loc+s.size());
		}
	}
}

void FindAllMatchesUncased(TextBuffer& tb,FoundList& matches,std::string_view s,Cursor start,Cursor end){
	matches.clear();
	if (start==end) return;
	size_t loc = start.column;
	LineIndexedIterator currLine = start.line;
	auto endIt = end.line.it;
	while (currLine.it!=endIt){
		if (currLine.it!=start.line.it) loc = 0;
		
		loc = FindInStringUncased(s,*currLine.it,loc);
		while (loc!=std::string::npos){
			matches.emplace_back(currLine,loc);
			loc = FindInStringUncased(s,*currLine.it,loc+s.size());
		}
		++currLine;
	}
	
	// search on last line (if it exists)
	if (endIt!=tb.end()){
		loc = FindInString(s,*endIt,0);
		while (loc<=(size_t)end.column){
			// currLine.it is endIt
			matches.emplace_back(currLine,loc);
			loc = FindInStringUncased(s,*endIt,loc+s.size());
		}
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
		loc = FindInString(find,*currLine.it,0);
		if (loc!=std::string::npos){
			currDiff.location = currLine;
			currDiff.before = *currLine.it;
			hit = true;
		}
		
		while (loc!=std::string::npos){
			currLine.it->replace(loc,find.size(),replace);
			++replaceCount;
			loc = FindInString(find,*currLine.it,loc+replace.size());
		}
		
		if (hit){
			currDiff.after = *currLine.it;
			diffs.push_back(currDiff);
		}
		
		++currLine;
	}
	return replaceCount;
}

size_t ReplaceAllUncased(TextBuffer& buf,std::string_view find,std::string_view replace,LineDiffInfo& diffs){
	size_t loc;
	size_t replaceCount = 0;
	LineIndexedIterator currLine = {buf.begin()};
	LineDiff currDiff;
	bool hit;
	while (currLine.it!=buf.end()){
		hit = false;
		loc = FindInStringUncased(find,*currLine.it,0);
		if (loc!=std::string::npos){
			currDiff.location = currLine;
			currDiff.before = *currLine.it;
			hit = true;
		}
		
		while (loc!=std::string::npos){
			currLine.it->replace(loc,find.size(),replace);
			++replaceCount;
			loc = FindInStringUncased(find,*currLine.it,loc+replace.size());
		}
		
		if (hit){
			currDiff.after = *currLine.it;
			diffs.push_back(currDiff);
		}
		
		++currLine;
	}
	return replaceCount;
}
