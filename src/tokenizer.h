#pragma once

#include "core.h"

#include <vector>
#include <string_view>

enum class TokenType : u8 {
	Name,
	Number,
	String,
	Comment,
	SpecialChar
};

struct Token {
	TokenType type;
	std::string_view token;
};

typedef std::vector<Token> TokenVector;

void ClipString(std::string_view&,std::string_view::iterator&);
void SkipWhitespace(std::string_view&,std::string_view::iterator&);

class TokenizerBase {
protected:
	std::string_view str;
	std::string_view::iterator pos;

public:
	TokenizerBase(std::string_view sv) : str(sv){
		pos = str.begin();
	}

	virtual void Reset(std::string_view newStr);

	virtual bool TokensLeft() const;
	virtual TokenVector GetTokens();
	virtual Token EmitToken() = 0;

	virtual ~TokenizerBase() = default;
};

class CommandTokenizer : public TokenizerBase {
protected:
	inline void TokenizeName();
	inline void TokenizeNumber();
	inline void TokenizeSingleQuoteString();
	inline void TokenizeDoubleQuoteString();
public:
	CommandTokenizer(std::string_view sv) : TokenizerBase(sv){
		SkipWhitespace(str,pos);
	}

	Token EmitToken() override;
};

class SyntaxTokenizer : public TokenizerBase {
	std::string strDelim1,strDelim2,strEscape;
	std::string comment,multiLineCommentStart,multiLineCommentEnd;

	inline void TokenizeName();
	inline void TokenizeNumber();
	inline void TokenizeString1();
	inline void TokenizeString2();
	inline void TokenizeSingleLineComment();
	inline void TokenizeMultiLineComment();
public:
	SyntaxTokenizer(std::string_view sv) : TokenizerBase(sv){
		SkipWhitespace(str,pos);
		strDelim1 = "'";
		strDelim2 = "\"";
		strEscape = "\\";
		comment = "//";
		multiLineCommentStart = "/*";
		multiLineCommentEnd = "*/";
	}

	Token EmitToken() override;
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
