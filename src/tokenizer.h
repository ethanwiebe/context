#pragma once

#include "core.h"

#include "textbuffer.h"

#include <string>
#include <string_view>
#include <vector>

enum class TokenType : u8 {
	Name,
	Number,
	String,
	Comment,
	SpecialChar,
	Directive,
	EndOfFile
};

struct TokenPos {
	LineList::iterator line;
	std::string::iterator col;
};

struct Token {
	TokenType type;
	TokenPos start,end;
	
	inline bool Matches(const std::string_view& s) const {
		auto lineCopy = start.line;
		auto colCopy = start.col;
		auto sIt = s.begin();
		while (lineCopy!=end.line||colCopy!=end.col){
			if (colCopy==lineCopy->end()){
				if (*sIt!='\n') return false;
			} else {
				if (*colCopy!=*sIt) return false;
			}
			
			if (colCopy==lineCopy->end()){
				++lineCopy;
				colCopy = lineCopy->begin();
			} else {
				++colCopy;
			}
			++sIt;
		}
		return sIt==s.end();
	}
	
	inline std::string Stringify() const {
		std::string out = {};
		
		auto lineCopy = start.line;
		auto colCopy = start.col;
		while (lineCopy!=end.line||colCopy!=end.col){
			if (colCopy==lineCopy->end()){
				out += '\n';
			} else {
				out += *colCopy;
			}
			
			if (colCopy==lineCopy->end()){
				++lineCopy;
				colCopy = lineCopy->begin();
			} else {
				++colCopy;
			}
		}
		
		return out;
	}
};

typedef std::vector<Token> TokenVector;

struct Tokenizer {
	TextBuffer* buffer;
	LineList::iterator bufferPos;
	std::string::iterator linePos;
	u8 lastChar;
	u8 currentChar;
	
	void SkipWhitespace();
	void TokenizeSingleQuoteString();
	void TokenizeDoubleQuoteString();
	void TokenizeNumber();
	void TokenizeHexNumber();
	void TokenizeSyntaxName();
	
	inline void SetBuffer(TextBuffer* buf){
		buffer = buf;
		bufferPos = buffer->begin();
		linePos = bufferPos->begin();
		lastChar = '\n';
		if (linePos!=bufferPos->end())
			currentChar = *linePos;
		else
			currentChar = '\n';
			
		PostSetBuffer();
	}
	
	virtual void PostSetBuffer(){
		SkipWhitespace();
	}
	
	inline TokenPos GetPos() const {
		return {bufferPos,linePos};
	}
	
	inline bool Done() const {
		return bufferPos==buffer->end();
	}
	
	inline u8 GetCharAtPos() const {
		if (linePos!=bufferPos->end())
			return *linePos;
		else
			return '\n';
	}
	
	inline void NextChar(){
		lastChar = currentChar;
		if (linePos==bufferPos->end()){
			++bufferPos;
			if (bufferPos==buffer->end()) return;
			linePos = bufferPos->begin();
		} else {
			++linePos;
		}
		
		currentChar = GetCharAtPos();
	}
	
	inline void SafeNextChar(){
		if (!Done())
			NextChar();
	}
	
	inline u8 PeekNextChar(){
		auto cachedLine = bufferPos;
		auto cachedCol = linePos;
		auto cachedPrevChar = lastChar;
		auto cachedChar = currentChar;
		
		NextChar();
		u8 newChar = currentChar;
		
		bufferPos = cachedLine;
		linePos = cachedCol;
		lastChar = cachedPrevChar;
		currentChar = cachedChar;
		
		return newChar;
	}
	
	inline bool PeekMatch(const std::string& s){
		if (s.empty()) return false;
		
		if (currentChar!=s[0])
			return false;
			
		if (s.size()==1) return true;
	
		auto cachedLine = bufferPos;
		auto cachedCol = linePos;
		auto cachedPrevChar = lastChar;
		auto cachedChar = currentChar;
		
		bool match = true;
		for (const auto c : s){
			if (c!=currentChar){
				match = false;
				break;
			}
			NextChar();
		}
		
		bufferPos = cachedLine;
		linePos = cachedCol;
		lastChar = cachedPrevChar;
		currentChar = cachedChar;
		
		return match;
	}
	
	virtual Token EmitToken() = 0;
};

struct CommandTokenizer : public Tokenizer {
	void SkipWhitespaceAndComment();

	void PostSetBuffer() override;
	Token EmitToken() override;
};

struct SyntaxTokenizer : public Tokenizer {
	std::string comment = {},altComment = {},
		multiCommentStart = {},multiCommentEnd = {};

	constexpr void SetComment(const std::string& str){
		comment = str;
	}
	
	constexpr void SetAltComment(const std::string& str){
		altComment = str;
	}
	
	constexpr void SetMultiLineComment(const std::string& start,const std::string& end){
		multiCommentStart = start;
		multiCommentEnd = end;
	}

	Token EmitToken() override;
};

struct CPPTokenizer : public Tokenizer {
	void TokenizeCPPNumber();

	Token EmitToken() override;
};
