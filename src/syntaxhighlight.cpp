#include "syntaxhighlight.h"

typedef IndexedIterator<std::string> CharIndexedIterator;

void ConfigurableSyntaxHighlighter::AddKeywords(const std::vector<std::string>& kws,TextStyle* style){
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
		case TokenType::Directive:
			return directiveStyle;
		default:
			break;
	}
	return textStyle;
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
	for (auto& line : buffer){
		tokenInterface.Reset(line);
		colorIt->clear();
		colorIt->reserve(4);
		std::string_view prevToken = {};

		for (Token token : tokenInterface){
			auto index = (token.token.data()-line.data());
			if (token.type==TokenType::Name){
				TextStyle style;
				if (prevToken!="."&&TokenInKeywords(token.token,style))
					AddColorData(colorIt,token.token,index,style);
			} else {
				AddColorData(colorIt,token.token,index,GetStyleFromTokenType(token.type));
			}
			prevToken = token.token;
		}
		++colorIt;
	}
}

bool ConfigurableSyntaxHighlighter::TokenInKeywords(std::string_view token,TextStyle& style) const {
	if (styleMap.contains(token)){
		style = *styleMap.at(token);
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

void CPPSyntaxHighlighter::BuildKeywords(){
	AddKeywords({"if","else","while","for","do","namespace",
			"switch","case","default","break","return","volatile",
			"using","template","typedef","typename","new","catch","concept",
			"delete","struct","class","enum","union","sizeof","try","catch",
			"alignof","alignas","public","private","protected","continue",
			"static_cast","static_assert","const_cast","dynamic_cast",
			"goto","requires","export","import","module","reinterpret_cast"},&statementStyle);

	AddKeywords({"void","bool","int","float","double","ptrdiff_t",
			"long","char","auto","size_t","ssize_t","const","inline",
			"noexcept","constexpr","extern","static","int8_t","virtual","operator",
			"int16_t","int32_t","int64_t","uint8_t","uint16_t",
			"uint32_t","uint64_t","u8","u16","u32","u64","s8",
			"s16","s32","s64","f32","f64","unsigned","signed","friend","final",
			"nullptr_t","short","override","register","char8_t","char16_t",
			"char32_t","wchar_t","consteval","constinit","mutable"},&typeStyle);
	AddKeywords({"nullptr","NULL","true","false","this"},&numberStyle);
}

static std::vector<std::string> pythonKeywords = {"for","while","if","elif","else","return","yield",
	"True","False","import","from","in","del","def","class","with","as","is","and","or","not","None",
	"try","except","finally","raise","global","continue","break"};
static std::vector<std::string> pythonFuncs = {"range","len","print","repr","ord","chr","isinstance","staticmethod","classmethod"
	"hex","round","pow","dir","open","quit","help","hash","next","__init__","__new__","__del__","__add__","__radd__",
	"__iadd__","__mul__","__rmul__","__imul__","__div__","__rdiv__","__idiv__","__sub__","__rsub__","__isub__",
	"__truediv__","__rtruediv__","__itruediv__","__floordiv__","__rfloordiv__","__ifloordiv__","__ge__","__le__",
	"__gt__","__lt__","__eq__","__ne__","__str__","__repr__","__neg__","__round__","__floor__","__ceil__","__float__",
	"__int__","__invert__","__sizeof__","__pow__","__divmod__","__rdivmod__","__idivmod__","__mod__","__rmod__","__imod__",
	"__or__","__ror__","__ior__","__xor__","__rxor__","__ixor__","__and__","__rand__","__iand__","__trunc__","__lshift__",
	"__rlshift__","__rshift__","__rrshift__","__ilshift__","irshift__","__setattr__","__delattr__","__hash__",
	"__iter__","__getitem__","__setitem__","__reversed__","__format__","__doc__","__delitem__","__contains__",
	"__class__","__len__"};
static std::vector<std::string> pythonTypes = {"int","float","bool","object","str","tuple",
	"list","map","set","dict","zip","enumerate","type","Exception","TypeError","ValueError","NotImplementedError"};

SyntaxHighlighter* GetSyntaxHighlighterFromExtension(TextBuffer& buffer,std::string_view ext){
	if (ext.empty())
		return nullptr;

	if (ext=="cpp"||ext=="hpp"||ext=="c"||ext=="h"||ext=="c++"||ext=="h++"||ext=="cc"||ext=="hh"){
		return new CPPSyntaxHighlighter(buffer);
	} else if (ext=="pyc"||ext=="pyw"||ext=="py"){
		ConfigurableSyntaxHighlighter* sh = new ConfigurableSyntaxHighlighter(buffer);
		sh->AddKeywords(pythonKeywords,&statementStyle);
		sh->AddKeywords(pythonFuncs,&funcStyle);
		sh->AddKeywords(pythonTypes,&typeStyle);
		sh->SetComment("#");
		sh->SetMultiLineComment("","");
		return sh;
	}
	

	return nullptr;
}
