#pragma once

#include "core.h"

#include <vector>
#include <string_view>
#include <assert.h>

enum class TokenType : u8 {
	Name,
	Number,
	String,
	Comment,
	SpecialChar,
	Directive
};

struct Token {
	TokenType type;
	s16 col;
	std::string_view token;
};

typedef std::vector<Token> TokenVector;

void ClipString(std::string_view&,std::string_view::iterator&);
void SkipWhitespace(std::string_view&,std::string_view::iterator&);

template <typename Tokenizer>
TokenVector GetTokens(Tokenizer& t){
	TokenVector tv;
	while (t.TokensLeft()){
		tv.push_back(t.EmitToken());
	}
	
	return tv;
}

class TokenizerBase {
public:
	std::string_view str;
	std::string_view::iterator pos;
	std::string_view::iterator begin;

	TokenizerBase(std::string_view sv) : str(sv){
		pos = str.begin();
		begin = str.begin();
	}

	inline void Reset(std::string_view newStr){
		str = newStr;
		begin = str.begin();
		pos = str.begin();
	}

	inline bool TokensLeft(){
		return pos != str.end();
	}

	~TokenizerBase() = default;
};

class CommandTokenizer : public TokenizerBase {
public:
	CommandTokenizer(std::string_view sv) : TokenizerBase(sv){
		SkipWhitespace(str,pos);
		if (pos!=str.end()&&*pos=='#'){
			pos = str.end();
			str = {pos,pos};
		}
	}

	Token EmitToken();
};

class SyntaxTokenizer : public TokenizerBase {
protected:
	bool unfinishedToken;
	u8 unfinishedStringType;
	TokenType unfinishedTokenType;

	std::string strDelim1,strDelim2;
	std::string comment,altComment,multiLineCommentStart,multiLineCommentEnd;
	char strEscape;

	virtual void ChooseToken(Token&,char,char);
	void SetUnfinishedToken(TokenType,u8=0);
public:
	SyntaxTokenizer(std::string_view sv) : TokenizerBase(sv){
		SkipWhitespace(str,pos);
		strDelim1 = "'";
		strDelim2 = "\"";
		strEscape = '\\';
		comment = "//";
		altComment = "";
		multiLineCommentStart = "/*";
		multiLineCommentEnd = "*/";
		unfinishedToken = false;
	}

	inline void SetComment(const std::string& s){
		comment = s;
	}
	
	inline void SetAltComment(const std::string& s){
		altComment = s;
	}

	inline void SetMultiLineCommentStart(const std::string& s){
		multiLineCommentStart = s;
	}

	inline void SetMultiLineCommentEnd(const std::string& s){
		multiLineCommentEnd = s;
	}

	Token EmitToken();
};

class CPPTokenizer : public SyntaxTokenizer {

protected:
	void ChooseToken(Token&,char,char);
public:
	CPPTokenizer(std::string_view sv) : SyntaxTokenizer(sv){}
};

template <typename Tokenizer>
class TokenInterface {
	Tokenizer& tokenizer;
	TokenVector tokens;
public:
	TokenInterface(Tokenizer& t) : tokenizer(t){}

	void Reset(std::string_view sv){
		tokenizer.Reset(sv);
		tokens = GetTokens<Tokenizer>(tokenizer);
	}
	
	inline TokenVector::const_iterator begin() const noexcept {
		return tokens.cbegin();
	}

	inline TokenVector::const_iterator end() const noexcept {
		return tokens.cend();
	}

	inline size_t size() const noexcept {
		return tokens.size();
	}
};
