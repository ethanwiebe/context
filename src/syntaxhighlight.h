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
typedef std::vector<ColorLine> ColorLineList;

typedef ColorLineList::iterator ColorIterator;

typedef std::map<std::string_view,TextStyle> KeywordStyleMap;

class ColorBuffer {
	ColorLineList colors;
public:
	inline ColorLineList::iterator begin() noexcept {
		return colors.begin();
	}

	inline ColorLineList::iterator end() noexcept {
		return colors.end();
	}

	inline size_t size() const noexcept {
		return colors.size();
	}

	inline void clear() noexcept {
		colors.clear();
	}

	inline void resize(size_t s) noexcept {
		colors.resize(s);
	}

	inline ColorLine& operator[](size_t i) noexcept {
		return colors[i];
	}

};

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
	TextStyle highlightStyle,stringStyle,numberStyle,commentStyle;
public:
	ConfigurableSyntaxHighlighter(TextBuffer& b) : SyntaxHighlighter(b){
		highlightStyle = TextStyle(ColorYellow,ColorBlack,StyleFlag::NoFlag);		
		stringStyle = TextStyle(ColorGreen,ColorBlack,StyleFlag::NoFlag);
		numberStyle = TextStyle(ColorRed,ColorBlack,StyleFlag::NoFlag);
		commentStyle = TextStyle(ColorBlue,ColorBlack,StyleFlag::NoFlag);
		keywords = {};
		styleMap = {};
	}

	void AddKeywords(const std::vector<std::string>&,TextStyle);

	void FillColorBuffer(ColorBuffer& c) override;


	virtual TokenizerBase* GetTokenizer() const;
	virtual TextStyle GetStyleFromTokenType(TokenType) const;

	~ConfigurableSyntaxHighlighter() override;
private:
	bool TokenInKeywords(std::string_view,TextStyle&) const;
	void AddColorData(ColorLineList::iterator,std::string_view,s32,TextStyle) const;
};

class CPPSyntaxHighlighter : public ConfigurableSyntaxHighlighter {
	void BuildKeywords();
	TextStyle directiveStyle;
public:
	CPPSyntaxHighlighter(TextBuffer& b) : ConfigurableSyntaxHighlighter(b){
		BuildKeywords();
		directiveStyle = {ColorMagenta,ColorBlack,StyleFlag::NoFlag};
	}

	TokenizerBase* GetTokenizer() const override;
	TextStyle GetStyleFromTokenType(TokenType) const override;

};

SyntaxHighlighter* GetSyntaxHighlighterFromExtension(TextBuffer&,std::string_view extension);
