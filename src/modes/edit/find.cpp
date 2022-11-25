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

typedef size_t(*FindInStrFunc)(std::string_view,std::string_view,size_t);

void FindAllMatchesSub(TextBuffer& tb,FoundList& matches,std::string_view s,Cursor start,Cursor end,FindInStrFunc func){
	matches.clear();
	if (start==end) return;
	size_t loc = start.column;
	LineIndexedIterator currLine = start.line;
	auto endIt = end.line.it;
	while (currLine.it!=endIt){
		if (currLine.it!=start.line.it) loc = 0;
		
		loc = func(s,*currLine.it,loc);
		while (loc!=std::string::npos){
			matches.emplace_back(currLine,loc);
			loc = func(s,*currLine.it,loc+s.size());
		}
		++currLine;
	}
	
	// search on last line (if it exists)
	if (endIt!=tb.end()){
		loc = func(s,*endIt,0);
		while (loc<=(size_t)end.column){
			// currLine.it is endIt
			matches.emplace_back(currLine,loc);
			loc = func(s,*endIt,loc+s.size());
		}
	}
}

void FindAllMatches(TextBuffer& tb,FoundList& matches,std::string_view s,Cursor start,Cursor end,bool cased){
	if (cased)
		FindAllMatchesSub(tb,matches,s,start,end,FindInString);
	else
		FindAllMatchesSub(tb,matches,s,start,end,FindInStringUncased);
}

size_t ReplaceAllSub(TextBuffer& buf,std::string_view find,
		std::string_view replace,LineDiffInfo& diffs,Cursor start,Cursor end,FindInStrFunc func){
	if (start==end) return 0;
	size_t replaceCount = 0;
	size_t loc = start.column;
	LineIndexedIterator currLine = start.line;
	LineDiff currDiff;
	auto endIt = end.line.it;
	bool hit;
	while (currLine.it!=endIt){
		hit = false;
		if (currLine.it!=start.line.it) loc = 0;
		loc = func(find,*currLine.it,loc);
		if (loc!=std::string::npos){
			currDiff.location = currLine;
			currDiff.before = *currLine.it;
			hit = true;
		}
		
		while (loc!=std::string::npos){
			currLine.it->replace(loc,find.size(),replace);
			++replaceCount;
			loc = func(find,*currLine.it,loc+replace.size());
		}
		
		if (hit){
			currDiff.after = *currLine.it;
			diffs.push_back(currDiff);
		}
		
		++currLine;
	}
	
	size_t endCol = end.column;
	if (endIt!=buf.end()){
		hit = false;
		loc = func(find,*endIt,0);
		if (loc<=endCol){
			currDiff.location = currLine;
			currDiff.before = *currLine.it;
			hit = true;
		}
		
		while (loc<=endCol){
			currLine.it->replace(loc,find.size(),replace);
			endCol -= find.size()-replace.size();
			++replaceCount;
			loc = func(find,*currLine.it,loc+replace.size());
		}
		
		if (hit){
			currDiff.after = *currLine.it;
			diffs.push_back(currDiff);
		}
	}
	
	return replaceCount;
}

size_t ReplaceAll(TextBuffer& buf,std::string_view find,
		std::string_view replace,LineDiffInfo& diffs,Cursor start,Cursor end,bool cased){
	if (cased)
		return ReplaceAllSub(buf,find,replace,diffs,start,end,FindInString);
	else
		return ReplaceAllSub(buf,find,replace,diffs,start,end,FindInStringUncased);
}

