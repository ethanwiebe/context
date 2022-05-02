#pragma once

#include "core.h"

#include <vector>
#include <string_view>

enum class TokenType : u8 {
	Name,
	Number,
	String,
	SpecialChar
};

struct Token {
	TokenType type;
	std::string_view token;
};

typedef std::vector<Token> TokenVector;

class TokenizerBase {
protected:
	std::string_view str;

public:
	TokenizerBase(std::string_view sv) : str(sv){}

	virtual bool TokensLeft() const = 0;
	virtual Token EmitToken() = 0;
	virtual TokenVector GetTokens() = 0;

	virtual ~TokenizerBase() = default;
};

class CommandTokenizer : public TokenizerBase {
protected:
	std::string_view::iterator pos;

	inline void TokenizeName();
	inline void TokenizeNumber();
	inline void TokenizeSingleQuoteString();
	inline void TokenizeDoubleQuoteString();

	inline void ClipString(std::string_view::iterator);
	void SkipWhitespace();
public:
	CommandTokenizer(std::string_view sv) : TokenizerBase(sv), pos(str.begin()){
		SkipWhitespace();
	}

	inline bool TokensLeft() const override;
	Token EmitToken() override;
	TokenVector GetTokens() override;
};
