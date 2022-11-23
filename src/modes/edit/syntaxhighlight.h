#pragma once

#include "core.h"

#include "textbuffer.h"
#include "style.h"
#include "tokenizer.h"
#include "logger.h"

#include <vector>
#include <list>
#include <string>
#include <string_view>
#include <map>

struct ColorData {
	s32 start,size;
	TextStyle style;
};

typedef std::vector<ColorData> ColorLine;
typedef std::vector<ColorLine> ColorBuffer;

typedef ColorBuffer::iterator ColorIterator;

typedef std::map<std::string_view,TextStyle*> KeywordStyleMap;

class SyntaxHighlighter {
protected:
	TextBuffer& buffer;
public:
	SyntaxHighlighter(TextBuffer& b) : buffer(b){}

	virtual void FillColorBuffer(ColorBuffer& c) = 0;

	virtual ~SyntaxHighlighter() = default;
};

class ConfigurableSyntaxHighlighter : public SyntaxHighlighter {
protected:
	std::list<std::string> keywords;
	KeywordStyleMap styleMap;
	std::string comment,altComment,multiLineCommentStart,multiLineCommentEnd;
public:
	ConfigurableSyntaxHighlighter(TextBuffer& b) : SyntaxHighlighter(b){
		// keywords is a container of strings
		// for the SVs in styleMap
		keywords = {};
		styleMap = {};
		
		comment = "//";
		altComment = "";
		multiLineCommentStart = "/*";
		multiLineCommentEnd = "*/";
	}

	void AddKeywords(const std::vector<std::string>&,TextStyle*);

	void FillColorBuffer(ColorBuffer& c) override;

	void SetComment(const std::string& s){
		comment = s;
	}
	
	void SetAltComment(const std::string& s){
		altComment = s;
	}

	void SetMultiLineComment(const std::string& start,const std::string& end){
		multiLineCommentStart = start;
		multiLineCommentEnd = end;
	}

	virtual Tokenizer* GetTokenizer() const;
	virtual TextStyle GetStyleFromTokenType(TokenType) const;

	~ConfigurableSyntaxHighlighter() override;
private:
	bool TokenInKeywords(const Token&,TextStyle&) const;
};

class CPPSyntaxHighlighter : public ConfigurableSyntaxHighlighter {
	void BuildKeywords();
public:
	CPPSyntaxHighlighter(TextBuffer& b) : ConfigurableSyntaxHighlighter(b){
		BuildKeywords();
	}

	Tokenizer* GetTokenizer() const override;
};

typedef SyntaxHighlighter* (*SHFunc)(TextBuffer&);
typedef std::map<std::string,SHFunc> SHMap;

extern SHMap gSyntaxHighlighters;
