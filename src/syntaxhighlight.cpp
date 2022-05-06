#include "syntaxhighlight.h"

typedef IndexedIterator<std::string> CharIndexedIterator;

void ConfigurableSyntaxHighlighter::SetKeywords(std::vector<std::string>&& kws){
	keywords = kws;
	for (const auto& keyword : keywords){
		styleMap[keyword] = TextStyle(ColorYellow,ColorBlack,StyleFlag::NoFlag);
	}
}

void ConfigurableSyntaxHighlighter::SetTypes(std::vector<std::string>&& t){
	types = t;
	for (const auto& type : types){
		styleMap[type] = TextStyle(ColorGreen,ColorBlack,StyleFlag::NoFlag);
	}
}

void SkipWhitespace(CharIndexedIterator& charIt,std::string::iterator end){
	while (charIt.it!=end&&(*charIt==' '||*charIt=='\t'))
		++charIt;
}

std::string_view PeekToken(CharIndexedIterator charIt,std::string::iterator end){
	auto startIt = charIt++;
	while (charIt.it!=end &&
			((*charIt >= 'A' && *charIt <= 'Z') ||
			(*charIt >= 'a' && *charIt <= 'z') ||
			(*charIt >= '0' && *charIt <= '9')) &&
			*charIt!=' '&&*charIt!='\t')
		++charIt;

	return std::string_view{startIt.it,charIt.it};
}

void ConfigurableSyntaxHighlighter::FillColorBuffer(ColorBuffer& c) const {
	c.resize(buffer.size());
	
	SyntaxTokenizer tokenizer{""};
	TokenInterface tokenInterface{tokenizer};

	auto colorIt = c.begin();
	std::string_view token;
	for (auto& line : buffer){
		tokenInterface.Reset(line);
		colorIt->clear();
		colorIt->reserve(4);

		for (Token token : tokenInterface){
			auto index = (token.token.data()-line.data());
			if (token.type==TokenType::Name){
				if (TokenInKeywords(token.token))
					AddColorData(colorIt,token.token,index,styleMap.at(token.token));
			} else if (token.type==TokenType::String){
				AddColorData(colorIt,token.token,index,stringStyle);
			} else if (token.type==TokenType::Number){
				AddColorData(colorIt,token.token,index,numberStyle);
			} else if (token.type==TokenType::Comment){
				AddColorData(colorIt,token.token,index,commentStyle);
			}
		}

		++colorIt;
	}
}

bool ConfigurableSyntaxHighlighter::TokenInKeywords(std::string_view token) const {
	return styleMap.contains(token);
}

void ConfigurableSyntaxHighlighter::AddColorData(ColorIterator it,std::string_view token,s32 index,TextStyle style) const {
	it->emplace_back(index,token.size(),style);
}
