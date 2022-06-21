#include "tokenizer.h"

inline bool IsAlphabet(char c){
	return (c>='a'&&c<='z')||(c>='A'&&c<='Z');
}

inline void TokenizeSingleLineComment(TokenizerBase& t){
	while (t.pos!=t.str.end()&&*t.pos!='\n') ++t.pos;
}

inline bool TokenizerBase::TokensLeft() const {
	return pos!=str.end();
}

void TokenizerBase::Reset(std::string_view newStr){
	str = newStr;
	begin = str.begin();
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
	while (pos!=str.end()&&(*pos==' '||*pos=='\n'||*pos=='\t')){
		++pos;
	}
	
	ClipString(str,pos);
}

bool SVPosStartsWith(std::string_view str,std::string_view::iterator pos,std::string_view check){
	if (check.empty()) return false;
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

inline void TokenizeNumber(TokenizerBase& t){
	while ((*t.pos>='0'&&*t.pos<='9')||*t.pos=='.') ++t.pos;
}

inline void TokenizeName(TokenizerBase& t){
	while (IsAlphabet(*t.pos)||(*t.pos>='0'&&*t.pos<='9')||*t.pos=='_'||*t.pos=='.'||*t.pos=='/'||*t.pos=='\\'||*t.pos=='-'||*t.pos==':') ++t.pos;
}

inline void TokenizeSingleQuoteString(TokenizerBase& t){
	while (t.pos!=t.str.end()&&*t.pos!='\'') ++t.pos;
}

inline void TokenizeDoubleQuoteString(TokenizerBase& t){
	while (t.pos!=t.str.end()&&*t.pos!='"') ++t.pos;
}

Token CommandTokenizer::EmitToken(){
	Token t;
	char initialC = *pos;

	if (initialC>='0'&&initialC<='9'){
		t.type = TokenType::Number;
		TokenizeNumber(*this);
	} else if (IsAlphabet(initialC)||initialC=='_'||initialC=='.'||initialC=='/'){
		t.type = TokenType::Name;
		TokenizeName(*this);
	} else if (initialC=='\''){
		t.type = TokenType::String;
		++pos;
		if (pos!=str.end()){
			ClipString(str,pos);
			TokenizeSingleQuoteString(*this);
		}
	} else if (initialC=='"'){
		t.type = TokenType::String;
		++pos;
		if (pos!=str.end()){
			ClipString(str,pos);
			TokenizeDoubleQuoteString(*this);
		}
	} else {
		t.type = TokenType::SpecialChar;
		++pos; //tokenize one char
	}

	t.token = {str.begin(),pos};
	t.col = pos-begin-t.token.size();
	
	if (t.type==TokenType::String&&pos!=str.end()) ++pos; //skip second quote

	SkipWhitespace(str,pos);
	if (pos==str.end()||*pos=='#'){ //comment in command
		pos = str.end();
	}

	return t;
}

inline void TokenizeSyntaxName(TokenizerBase& t){
	while (IsAlphabet(*t.pos)||(*t.pos>='0'&&*t.pos<='9')||*t.pos=='_') ++t.pos;
}

inline bool TokenizeUntilDelimiter(TokenizerBase& t,std::string_view delim,char escape='\0'){
	while (!SVPosStartsWith(t.str,t.pos,delim)){
		if (escape&&*t.pos==escape){
			++t.pos;
		}
		if (t.pos==t.str.end()) return false;
		++t.pos;
	}

	size_t count = delim.size();
	while (t.pos!=t.str.end()&&count--) ++t.pos;

	return true;
}

void SyntaxTokenizer::SetUnfinishedToken(TokenType type,u8 str){
	unfinishedToken = true;
	unfinishedTokenType = type;
	unfinishedStringType = str;
}

inline void SyntaxTokenizer::ChooseToken(Token& t,char initialC,char nextC){
	if ((initialC>='0'&&initialC<='9')||(initialC=='.'&&nextC>='0'&&nextC<='9')){
		t.type = TokenType::Number;
		TokenizeNumber(*this);
	} else if (IsAlphabet(initialC)||initialC=='_'){
		t.type = TokenType::Name;
		TokenizeSyntaxName(*this);
	} else if (SVPosStartsWith(str,pos,strDelim1)){
		t.type = TokenType::String;
		++pos;
		if (!TokenizeUntilDelimiter(*this,strDelim1,strEscape))
			SetUnfinishedToken(TokenType::String,1);
	} else if (SVPosStartsWith(str,pos,strDelim2)){
		t.type = TokenType::String;
		++pos;
		if (!TokenizeUntilDelimiter(*this,strDelim2,strEscape))
			SetUnfinishedToken(TokenType::String,2);
	} else if (SVPosStartsWith(str,pos,comment)){
		t.type = TokenType::Comment;
		TokenizeSingleLineComment(*this);
	} else if (SVPosStartsWith(str,pos,multiLineCommentStart)){
		t.type = TokenType::Comment;
		++pos;
		if (!TokenizeUntilDelimiter(*this,multiLineCommentEnd))
			SetUnfinishedToken(TokenType::Comment);
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
			if (unfinishedStringType==1){
				if (!TokenizeUntilDelimiter(*this,strDelim1,strEscape))
					SetUnfinishedToken(TokenType::String,1);
			} else if (unfinishedStringType==2){
				if (!TokenizeUntilDelimiter(*this,strDelim2,strEscape))
					SetUnfinishedToken(TokenType::String,2);
			} else {
				assert(false);
			}
		} else if (unfinishedTokenType==TokenType::Comment){
			t.type = TokenType::Comment;
			if (!TokenizeUntilDelimiter(*this,multiLineCommentEnd))
				SetUnfinishedToken(TokenType::Comment);
		} else if (unfinishedTokenType==TokenType::Directive){
			t.type = TokenType::Directive;
			if (TokenizeUntilDelimiter(*this,"\\"))
				SetUnfinishedToken(TokenType::Directive);
		}

	}
	
	t.token = {str.begin(),pos};
	SkipWhitespace(str,pos);
	return t;
}

inline void TokenizeDirective(TokenizerBase& t){
	while (t.pos!=t.str.end()&&*t.pos!='\n') ++t.pos;
}

inline void TokenizeHexNumber(TokenizerBase& t){
	while ((*t.pos>='0'&&*t.pos<='9')||(*t.pos>='a'&&*t.pos<='f')||(*t.pos>='A'&&*t.pos<='F')) ++t.pos;
}

void CPPTokenizer::ChooseToken(Token& t,char initialC,char nextC){
	if ((initialC>='0'&&initialC<='9')||(initialC=='.'&&nextC>='0'&&nextC<='9')){
		t.type = TokenType::Number;
		if (initialC=='0'&&nextC=='x'){
			++pos;++pos;
			TokenizeHexNumber(*this);
		} else {
			if (initialC=='0'&&(nextC=='b'||nextC=='o')) {++pos;++pos;}
			TokenizeNumber(*this);
		}
	} else if (IsAlphabet(initialC)||initialC=='_'){
		t.type = TokenType::Name;
		TokenizeSyntaxName(*this);
	} else if (SVPosStartsWith(str,pos,strDelim1)){
		t.type = TokenType::String;
		++pos;
		if (!TokenizeUntilDelimiter(*this,strDelim1,strEscape))
			SetUnfinishedToken(TokenType::String,1);
	} else if (SVPosStartsWith(str,pos,strDelim2)){
		t.type = TokenType::String;
		++pos;
		if (!TokenizeUntilDelimiter(*this,strDelim2,strEscape))
			SetUnfinishedToken(TokenType::String,2);

	} else if (SVPosStartsWith(str,pos,comment)){
		t.type = TokenType::Comment;
		TokenizeSingleLineComment(*this);
	} else if (SVPosStartsWith(str,pos,multiLineCommentStart)){
		t.type = TokenType::Comment;
		++pos;

		if (!TokenizeUntilDelimiter(*this,multiLineCommentEnd))
			SetUnfinishedToken(TokenType::Comment);
	} else if (initialC=='#'&&IsAlphabet(nextC)){
		t.type = TokenType::Directive;
		if (TokenizeUntilDelimiter(*this,"\\")){
			SetUnfinishedToken(TokenType::Directive);
		}
//		TokenizeDirective(*this);
	} else {
		t.type = TokenType::SpecialChar;
		++pos; //tokenize one char
	}
}

