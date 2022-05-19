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

SyntaxTokenizer* ConfigurableSyntaxHighlighter::GetTokenizer() const {
	return new SyntaxTokenizer("");
}

void ConfigurableSyntaxHighlighter::FillColorBuffer(ColorBuffer& c){
	c.resize(buffer.size());
	
	Handle<SyntaxTokenizer> tokenizer = Handle<SyntaxTokenizer>(GetTokenizer());
	TokenInterface tokenInterface{*tokenizer};

	tokenizer->SetComment(comment);
	tokenizer->SetMultiLineCommentStart(multiLineCommentStart);
	tokenizer->SetMultiLineCommentEnd(multiLineCommentEnd);

	auto colorIt = c.begin();
	std::string_view token;
	for (auto& line : buffer){
		tokenInterface.Reset(line);
		colorIt->clear();
		colorIt->reserve(4);

		for (Token token : tokenInterface){
			auto index = (token.token.data()-line.data());
			if (token.type==TokenType::Name){
				TextStyle style;
				if (TokenInKeywords(token.token,style))
					AddColorData(colorIt,token.token,index,style);
			} else {
				AddColorData(colorIt,token.token,index,GetStyleFromTokenType(token.type));
			}
		}
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

SyntaxTokenizer* CPPSyntaxHighlighter::GetTokenizer() const {
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
			"delete","struct","class","enum","union","sizeof",
			"alignof","public","private","protected","continue"},statementStyle);

	AddKeywords({"void","bool","int","float","double","ptrdiff_t",
			"long","char","auto","size_t","ssize_t","const","inline",
			"noexcept","constexpr","extern","static","int8_t","virtual",
			"int16_t","int32_t","int64_t","uint8_t","uint16_t",
			"uint32_t","uint64_t","u8","u16","u32","u64","s8",
			"s16","s32","s64","f32","f64","unsigned","friend",
			"nullptr_t","short"},typeStyle);
	AddKeywords({"nullptr","NULL","true","false"},numberStyle);
}

static std::vector<std::string> pythonKeywords = {"for","while","if","elif","else","return","yield",
	"True","False","import","from","in","del","def","class","with","as","and","or","not","None",
	"try","except","finally","global","continue","break"};
static std::vector<std::string> pythonFuncs = {"range","len","print","repr","ord","chr","isinstance",
	"hex","round","pow","dir","open","quit","help","hash","next"};
static std::vector<std::string> pythonTypes = {"int","float","bool","object","str","tuple",
	"list","map","set","dict","zip","enumerate","type"};

SyntaxHighlighter* GetSyntaxHighlighterFromExtension(TextBuffer& buffer,std::string_view ext){
	if (ext.empty())
		return nullptr;

	if (ext=="cpp"||ext=="hpp"||ext=="c"||ext=="h"||ext=="c++"||ext=="h++"||ext=="cc"||ext=="hh"){
		return new CPPSyntaxHighlighter(buffer);
	} else if (ext=="pyc"||ext=="pyw"||ext=="py"){
		ConfigurableSyntaxHighlighter* sh = new ConfigurableSyntaxHighlighter(buffer);
		sh->AddKeywords(pythonKeywords,statementStyle);
		sh->AddKeywords(pythonFuncs,funcStyle);
		sh->AddKeywords(pythonTypes,typeStyle);
		sh->SetComment("#");
		sh->SetMultiLineComment("","");
		return sh;
	}
	

	return nullptr;
}
