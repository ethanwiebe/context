#include "tokenizer.h"

inline bool CommandTokenizer::TokensLeft() const {
	return pos!=str.end();
}

void CommandTokenizer::SkipWhitespace(){
	while (*pos==' '||*pos=='\n'||*pos=='\t'){
		++pos;
	}
	ClipString(pos);
}

inline void CommandTokenizer::ClipString(std::string_view::iterator it){
	str = {it,str.end()};
}

inline void CommandTokenizer::TokenizeNumber(){
	while (*pos>='0'&&*pos<='9') ++pos;
}

inline void CommandTokenizer::TokenizeName(){
	while ((*pos>='a'&&*pos<='z')||(*pos>='A'&&*pos<='Z')||(*pos>='0'&&*pos<='9')||*pos=='_'||*pos=='.'||*pos=='/'||*pos=='-') ++pos;
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
	} else if ((initialC>='a'&&initialC<='z')||(initialC>='A'&&initialC<='Z')||initialC=='_'||initialC=='.'||initialC=='/'){
		t.type = TokenType::Name;
		TokenizeName();
	} else if (initialC=='\''){
		t.type = TokenType::String;
		ClipString(++pos);
		TokenizeSingleQuoteString();
	} else if (initialC=='"'){
		t.type = TokenType::String;
		ClipString(++pos);
		TokenizeDoubleQuoteString();
	} else {
		t.type = TokenType::SpecialChar;
		++pos; //tokenize one char
	}

	t.token = {str.begin(),pos};
	
	if (t.type==TokenType::String&&pos!=str.end()) ++pos; //skip second quote

	SkipWhitespace();

	return t;
}

TokenVector CommandTokenizer::GetTokens(){
	TokenVector tv;
	while (TokensLeft()){
		tv.push_back(EmitToken());
	}

	return tv;
}
