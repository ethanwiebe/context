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

class TokenizerBase {
public:
	std::string_view str;
	std::string_view::iterator pos;
	std::string_view::iterator begin;

	TokenizerBase(std::string_view sv) : str(sv){
		pos = str.begin();
		begin = str.begin();
	}

	virtual void Reset(std::string_view newStr);

	virtual bool TokensLeft() const;
	virtual TokenVector GetTokens();
	virtual Token EmitToken() = 0;

	virtual ~TokenizerBase() = default;
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

	Token EmitToken() override;
};

class SyntaxTokenizer : public TokenizerBase {
protected:
	bool unfinishedToken;
	u8 unfinishedStringType;
	TokenType unfinishedTokenType;

	std::string strDelim1,strDelim2;
	std::string comment,multiLineCommentStart,multiLineCommentEnd;
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
		multiLineCommentStart = "/*";
		multiLineCommentEnd = "*/";
		unfinishedToken = false;
	}

	inline void SetComment(const std::string& s){
		comment = s;
	}

	inline void SetMultiLineCommentStart(const std::string& s){
		multiLineCommentStart = s;
	}

	inline void SetMultiLineCommentEnd(const std::string& s){
		multiLineCommentEnd = s;
	}

	Token EmitToken() override;
};

class CPPTokenizer : public SyntaxTokenizer {

protected:
	void ChooseToken(Token&,char,char) override;
public:
	CPPTokenizer(std::string_view sv) : SyntaxTokenizer(sv){}
};

class TokenInterface {
	TokenizerBase& tokenizer;
	TokenVector tokens;
public:
	TokenInterface(TokenizerBase& t) : tokenizer(t){}

	void Reset(std::string_view sv){
		tokenizer.Reset(sv);
		tokens = tokenizer.GetTokens();
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
