#include "syntaxhighlight.h"

typedef IndexedIterator<std::string> CharIndexedIterator;

void ConfigurableSyntaxHighlighter::AddKeywords(const std::vector<std::string>& kws,TextStyle style){
	for (auto keyword : kws){
		keywords.push_back(keyword);
		styleMap[keywords.back()] = style;
	}
}

ConfigurableSyntaxHighlighter::~ConfigurableSyntaxHighlighter(){
	styleMap.clear();
	keywords.clear();
}

TextStyle ConfigurableSyntaxHighlighter::GetStyleFromTokenType(TokenType type) const {
	switch (type){
		case TokenType::String:
			return stringStyle;
		case TokenType::Comment:
			return commentStyle;
		case TokenType::Number:
			return numberStyle;
		default:
			break;
	}
	return defaultStyle;
}

TokenizerBase* ConfigurableSyntaxHighlighter::GetTokenizer() const {
	return new SyntaxTokenizer("");
}

void ConfigurableSyntaxHighlighter::FillColorBuffer(ColorBuffer& c){
	c.resize(buffer.size());
	
	Handle<TokenizerBase> tokenizer = Handle<TokenizerBase>(GetTokenizer());
	TokenInterface tokenInterface{*tokenizer};

	auto colorIt = c.begin();
	std::string_view token;
	for (auto& line : buffer){
		tokenInterface.Reset(line);
		colorIt->clear();
		colorIt->reserve(4);

		for (Token token : tokenInterface){
			logger << (s32)token.type << ":" << token.token << ",";
			auto index = (token.token.data()-line.data());
			if (token.type==TokenType::Name){
				TextStyle style;
				if (TokenInKeywords(token.token,style))
					AddColorData(colorIt,token.token,index,style);
			} else {
				AddColorData(colorIt,token.token,index,GetStyleFromTokenType(token.type));
			}
		}
		logger << "\n";
		++colorIt;
	}
}

bool ConfigurableSyntaxHighlighter::TokenInKeywords(std::string_view token,TextStyle& style) const {
	if (styleMap.contains(token)){
		style = styleMap.at(token);
		return true;
	}

	return false;
}

void ConfigurableSyntaxHighlighter::AddColorData(ColorIterator it,std::string_view token,s32 index,TextStyle style) const {
	it->emplace_back(index,token.size(),style);
}

TokenizerBase* CPPSyntaxHighlighter::GetTokenizer() const {
	return new CPPTokenizer("");
}

TextStyle CPPSyntaxHighlighter::GetStyleFromTokenType(TokenType type) const {
	switch (type){
		case TokenType::String:
			return stringStyle;
		case TokenType::Comment:
			return commentStyle;
		case TokenType::Number:
			return numberStyle;
		case TokenType::Directive:
			return directiveStyle;
		default:
			break;
	}
	return defaultStyle;
}

void CPPSyntaxHighlighter::BuildKeywords(){
	AddKeywords({"if","else","while","for","do",
			"switch","case","default","break","return",
			"using","template","typedef","typename","new",
			"delete"},statementStyle);

	AddKeywords({"void","bool","int","float","double",
			"long","char","auto","size_t","const","inline",
			"noexcept","constexpr","extern","static"},typeStyle);

}

static std::vector<std::string> pythonKeywords = {"for","if","elif","return","yield"};
static std::vector<std::string> pythonTypes = {"int","float","bool","object"};

SyntaxHighlighter* GetSyntaxHighlighterFromExtension(TextBuffer& buffer,std::string_view ext){
	if (ext.empty())
		return nullptr;

	if (ext=="cpp"||ext=="hpp"||ext=="c"||ext=="h"||ext=="c++"||ext=="h++"){
		return new CPPSyntaxHighlighter(buffer);
	} else if (ext=="pyc"||ext=="pyw"||ext=="py"){
		ConfigurableSyntaxHighlighter* sh = new ConfigurableSyntaxHighlighter(buffer);
		sh->AddKeywords(pythonKeywords,statementStyle);
		sh->AddKeywords(pythonTypes,typeStyle);
		return sh;
	}
	

	return nullptr;
}
