#include "tokenizer.h"

inline bool IsAlphabet(char c){
	return (c>='a'&&c<='z')||(c>='A'&&c<='Z');
}

inline bool TokenizerBase::TokensLeft() const {
	return pos!=str.end();
}

void TokenizerBase::Reset(std::string_view newStr){
	str = newStr;
	pos = str.begin();
}

TokenVector TokenizerBase::GetTokens(){
	TokenVector tv;
	while (TokensLeft()){
		tv.push_back(EmitToken());
	}

	return tv;
}

void ClipString(std::string_view& str,std::string_view::iterator& pos){
	str = {pos,str.end()};
}

void SkipWhitespace(std::string_view& str,std::string_view::iterator& pos){
	while (*pos==' '||*pos=='\n'||*pos=='\t'){
		++pos;
	}
	ClipString(str,pos);
}

bool SVPosStartsWith(std::string_view str,std::string_view::iterator pos,std::string_view check){
	std::string_view::iterator checkIt = check.begin();
	while (pos!=str.end()){
		if (*pos!=*checkIt)
			return false;

		if (++checkIt==check.end())
			return true;

		++pos;
	}

	return false;
}

inline void CommandTokenizer::TokenizeNumber(){
	while (*pos>='0'&&*pos<='9') ++pos;
}

inline void CommandTokenizer::TokenizeName(){
	while (IsAlphabet(*pos)||(*pos>='0'&&*pos<='9')||*pos=='_'||*pos=='.'||*pos=='/'||*pos=='-') ++pos;
}

inline void CommandTokenizer::TokenizeSingleQuoteString(){
	while (pos!=str.end()&&*pos!='\'') ++pos;
}

inline void CommandTokenizer::TokenizeDoubleQuoteString(){
	while (pos!=str.end()&&*pos!='"') ++pos;
}

Token CommandTokenizer::EmitToken(){
	Token t;
	char initialC = *pos;

	if (initialC>='0'&&initialC<='9'){
		t.type = TokenType::Number;
		TokenizeNumber();
	} else if (IsAlphabet(initialC)||initialC=='_'||initialC=='.'||initialC=='/'){
		t.type = TokenType::Name;
		TokenizeName();
	} else if (initialC=='\''){
		t.type = TokenType::String;
		ClipString(str,++pos);
		TokenizeSingleQuoteString();
	} else if (initialC=='"'){
		t.type = TokenType::String;
		ClipString(str,++pos);
		TokenizeDoubleQuoteString();
	} else {
		t.type = TokenType::SpecialChar;
		++pos; //tokenize one char
	}

	t.token = {str.begin(),pos};
	
	if (t.type==TokenType::String&&pos!=str.end()) ++pos; //skip second quote

	SkipWhitespace(str,pos);

	return t;
}


inline void SyntaxTokenizer::TokenizeName(){
	while (IsAlphabet(*pos)||(*pos>='0'&&*pos<='9')||*pos=='_') ++pos;
}

inline void SyntaxTokenizer::TokenizeNumber(){
	while ((*pos>='0'&&*pos<='9')||*pos=='.') ++pos;
}

inline void SyntaxTokenizer::TokenizeString1(){
	while (pos!=str.end()&&!SVPosStartsWith(str,pos,strDelim1)){
		if (*pos==strEscape){
			++pos;
			if (pos==str.end()) break;
		}
		++pos;
	}
	if (pos==str.end()){
		unfinishedToken = true;
		unfinishedTokenType = TokenType::String;
		unfinishedStringType = 1;
		return;
	}
	size_t count = strDelim1.size();
	while (pos!=str.end()&&count--) ++pos;
}

inline void SyntaxTokenizer::TokenizeString2(){
	while (pos!=str.end()&&!SVPosStartsWith(str,pos,strDelim2)){
		if (*pos==strEscape){
			++pos;
			if (pos==str.end()) break;
		}
		++pos;
	}
	if (pos==str.end()){
		unfinishedToken = true;
		unfinishedTokenType = TokenType::String;
		unfinishedStringType = 2;
		return;
	}
	size_t count = strDelim2.size();
	while (pos!=str.end()&&count--) ++pos;

}

inline void SyntaxTokenizer::TokenizeSingleLineComment(){
	while (pos!=str.end()&&*pos!='\n') ++pos;
}

inline void SyntaxTokenizer::TokenizeMultiLineComment(){
	while (pos!=str.end()&&!SVPosStartsWith(str,pos,multiLineCommentEnd)) ++pos;
	if (pos==str.end()){
		unfinishedToken = true;
		unfinishedTokenType = TokenType::Comment;
		return;
	}
	size_t count = multiLineCommentEnd.size();
	while (pos!=str.end()&&count--) ++pos;
}

inline void SyntaxTokenizer::ChooseToken(Token& t,char initialC,char nextC){
	if ((initialC>='0'&&initialC<='9')||(initialC=='.'&&nextC>='0'&&nextC<='9')){
		t.type = TokenType::Number;
		TokenizeNumber();
	} else if (IsAlphabet(initialC)||initialC=='_'){
		t.type = TokenType::Name;
		TokenizeName();
	} else if (SVPosStartsWith(str,pos,strDelim1)){
		t.type = TokenType::String;
		++pos;
		TokenizeString1();
	} else if (SVPosStartsWith(str,pos,strDelim2)){
		t.type = TokenType::String;
		++pos;
		TokenizeString2();
	} else if (SVPosStartsWith(str,pos,comment)){
		t.type = TokenType::Comment;
		TokenizeSingleLineComment();
	} else if (SVPosStartsWith(str,pos,multiLineCommentStart)){
		t.type = TokenType::Comment;
		++pos;
		TokenizeMultiLineComment();
	} else {
		t.type = TokenType::SpecialChar;
		++pos; //tokenize one char
	}
}

Token SyntaxTokenizer::EmitToken(){
	Token t;
	char initialC = *pos;
	char nextC = ' ';
	if (pos+1!=str.end())
		nextC = *(pos+1);
	
	if (!unfinishedToken){
		ChooseToken(t,initialC,nextC);
	} else {
		unfinishedToken = false;
		if (unfinishedTokenType==TokenType::String){
			t.type = TokenType::String;
			if (unfinishedStringType==1)
				TokenizeString1();
			else if (unfinishedStringType==2)
				TokenizeString2();
			else
				assert(false);
		} else if (unfinishedTokenType==TokenType::Comment){
			t.type = TokenType::Comment;
			TokenizeMultiLineComment();
		}

	}
	
	t.token = {str.begin(),pos};
	SkipWhitespace(str,pos);
	return t;
}

void CPPTokenizer::TokenizeDirective(){
	while (pos!=str.end()&&*pos!='\n') ++pos;
}

void CPPTokenizer::ChooseToken(Token& t,char initialC,char nextC){
	if ((initialC>='0'&&initialC<='9')||(initialC=='.'&&nextC>='0'&&nextC<='9')){
		t.type = TokenType::Number;
		TokenizeNumber();
	} else if (IsAlphabet(initialC)||initialC=='_'){
		t.type = TokenType::Name;
		TokenizeName();
	} else if (SVPosStartsWith(str,pos,strDelim1)){
		t.type = TokenType::String;
		++pos;
		TokenizeString1();
	} else if (SVPosStartsWith(str,pos,strDelim2)){
		t.type = TokenType::String;
		++pos;
		TokenizeString2();
	} else if (SVPosStartsWith(str,pos,comment)){
		t.type = TokenType::Comment;
		TokenizeSingleLineComment();
	} else if (SVPosStartsWith(str,pos,multiLineCommentStart)){
		t.type = TokenType::Comment;
		++pos;
		TokenizeMultiLineComment();
	} else if (initialC=='#'&&IsAlphabet(nextC)){
		t.type = TokenType::Directive;
		TokenizeDirective();
	} else {
		t.type = TokenType::SpecialChar;
		++pos; //tokenize one char
	}
}

