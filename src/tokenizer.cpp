#include "tokenizer.h"

inline bool IsWhitespace(u8 c){
	return c==' '||c=='\t'||c=='\n';
}

inline bool IsAlphabet(u8 c){
	return (c>='a'&&c<='z')||(c>='A'&&c<='Z');
}

inline bool IsNumber(u8 c){
	return c>='0'&&c<='9';
}

void Tokenizer::SkipWhitespace(){
	while (!Done()&&IsWhitespace(currentChar)){
		NextChar();
	}
}

void Tokenizer::TokenizeSingleQuoteString(){
	while (!Done()&&currentChar!='\''){
		if (currentChar=='\\'){
			NextChar();
			SafeNextChar();
		} else {
			NextChar();
		}
	}
}

void Tokenizer::TokenizeDoubleQuoteString(){
	while (!Done()&&currentChar!='"'){
		if (currentChar=='\\'){
			NextChar();
			SafeNextChar();
		} else {
			NextChar();
		}
	}
}

void Tokenizer::TokenizeNumber(){
	bool hitDecimal = false;
	while (!Done()&&((currentChar>='0'&&currentChar<='9')||(currentChar=='.'&&!hitDecimal))){
		if (currentChar=='.') hitDecimal = true;
		NextChar();
	}
}

void Tokenizer::TokenizeHexNumber(){
	while (!Done()&&((currentChar>='0'&&currentChar<='9')||
					 (currentChar>='a'&&currentChar<='f')||
					 (currentChar>='A'&&currentChar<='F'))){
		NextChar();
	}
}

void Tokenizer::TokenizeSyntaxName(){
	while (!Done()&&
			(IsAlphabet(currentChar)||
			 IsNumber(currentChar)||
			 currentChar=='_'))
		NextChar();
}

void CommandTokenizer::SkipWhitespaceAndComment(){
	SkipWhitespace();
	
	while (!Done()&&currentChar=='#'){
		while (!Done()&&currentChar!='\n')
			NextChar();
		SkipWhitespace();
	}
}

void CommandTokenizer::PostSetBuffer(){
	SkipWhitespaceAndComment();
}

Token CommandTokenizer::EmitToken(){
	Token t;
	t.start = GetPos();
	
	if (currentChar=='\''){
		t.type = TokenType::String;
		NextChar();
		t.start = GetPos();
		TokenizeSingleQuoteString();
		t.end = GetPos();
		SafeNextChar();
	} else if (currentChar=='"'){
		t.type = TokenType::String;
		NextChar();
		t.start = GetPos();
		TokenizeDoubleQuoteString();
		t.end = GetPos();
		SafeNextChar();
	} else {
		t.type = TokenType::Name;
		while (!Done()&&!IsWhitespace(currentChar)){
			NextChar();
		}
		t.end = GetPos();
	}
	
	SkipWhitespaceAndComment();
	
	return t;
}

Token SyntaxTokenizer::EmitToken(){
	Token t;
	
	t.start = GetPos();
	
	if (IsNumber(currentChar)){
		t.type = TokenType::Number;
		NextChar();
		switch (currentChar){
			case 'x':
				NextChar();
				TokenizeHexNumber();
				break;
			case 'o':
			case 'b':
				NextChar();
				TokenizeNumber();
				break;
			default:
			// single number
				break;
		}
	} else if (currentChar=='.'&&IsNumber(PeekNextChar())){
		t.type = TokenType::Number;
		TokenizeNumber();
	} else if (IsAlphabet(currentChar)||currentChar=='_'){
		t.type = TokenType::Name;
		TokenizeSyntaxName();
	} else if (currentChar=='\''){
		t.type = TokenType::String;
		NextChar();
		TokenizeSingleQuoteString();
		SafeNextChar();
	} else if (currentChar=='"'){
		t.type = TokenType::String;
		NextChar();
		TokenizeDoubleQuoteString();
		SafeNextChar();
	} else if (PeekMatch(comment)||PeekMatch(altComment)){
		t.type = TokenType::Comment;
		while (!Done()&&currentChar!='\n'){
			NextChar();
		}
	} else if (PeekMatch(multiCommentStart)){
		t.type = TokenType::Comment;
		
		while (!Done()&&!PeekMatch(multiCommentEnd)){
			NextChar();
		}
		// include comment end in token
		s32 count = (s32)multiCommentEnd.size();
		while (--count>=0){
			SafeNextChar();
		}
	} else {
		t.type = TokenType::SpecialChar;
		NextChar();
	}
	
	t.end = GetPos();
	SkipWhitespace();
	
	return t;
}

void CPPTokenizer::TokenizeCPPNumber(){
	TokenizeNumber();
	while (!Done()&&IsAlphabet(currentChar)){
		NextChar();
	}
}

Token CPPTokenizer::EmitToken(){
	Token t;
	
	t.start = GetPos();
	
	if (IsNumber(currentChar)){
		t.type = TokenType::Number;
		NextChar();
		switch (currentChar){
			case 'x':
				NextChar();
				TokenizeHexNumber();
				break;
			case 'o':
			case 'b':
				NextChar();
				TokenizeCPPNumber();
				break;
			default:
				TokenizeCPPNumber();
				break;
		}
	} else if (currentChar=='.'&&IsNumber(PeekNextChar())){
		t.type = TokenType::Number;
		TokenizeCPPNumber();
	} else if (IsAlphabet(currentChar)||currentChar=='_'){
		t.type = TokenType::Name;
		TokenizeSyntaxName();
	} else if (currentChar=='\''){
		t.type = TokenType::String;
		NextChar();
		TokenizeSingleQuoteString();
		SafeNextChar();
	} else if (currentChar=='"'){
		t.type = TokenType::String;
		NextChar();
		TokenizeDoubleQuoteString();
		SafeNextChar();
	} else if (PeekMatch("//")){
		t.type = TokenType::Comment;
		while (!Done()&&currentChar!='\n')
			NextChar();
	} else if (PeekMatch("/*")){
		t.type = TokenType::Comment;
		while (!Done()&&!PeekMatch("*/")){
			NextChar();
		}
		SafeNextChar();
		SafeNextChar();
	} else if (currentChar=='#'&&IsAlphabet(PeekNextChar())){
		t.type = TokenType::Directive;
		
		while (!Done()&&currentChar!='\n'){
			NextChar();
		}
	} else {
		t.type = TokenType::SpecialChar;
		NextChar();
	}
	
	t.end = GetPos();
	SkipWhitespace();
	
	return t;
}
