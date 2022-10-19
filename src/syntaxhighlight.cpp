#include "syntaxhighlight.h"

typedef IndexedIterator<std::string> CharIndexedIterator;

inline s32 GetTokenStart(
		LineList::iterator textIt,
		const Token& token
	){
	if (token.start.line==textIt)
		return token.start.col-token.start.line->begin();
	return 0;
}

inline s32 GetTokenSize(
		LineList::iterator textIt,
		const Token& token
	){
	if (token.end.line==textIt){
		if (token.start.line==token.end.line)
			return token.end.col-token.start.col;
		return token.end.col-token.end.line->begin();
	} else if (token.start.line==textIt){
		return token.start.line->end()-token.start.col;
	} else {
		return textIt->size();
	}
}		

inline void AddColorData(
		ColorIterator it,
		LineList::iterator textIt,
		const Token& token,
		const TextStyle& style
	){
	
	it->emplace_back(GetTokenStart(textIt,token),GetTokenSize(textIt,token),style);
}

void ConfigurableSyntaxHighlighter::AddKeywords(
		const std::vector<std::string>& kws,
		TextStyle* style
	){
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

Tokenizer* ConfigurableSyntaxHighlighter::GetTokenizer() const {
	auto t = new SyntaxTokenizer();
	t->SetComment(comment);
	t->SetAltComment(altComment);
	t->SetMultiLineComment(multiLineCommentStart,multiLineCommentEnd);
	return t;
}

void ConfigurableSyntaxHighlighter::FillColorBuffer(ColorBuffer& c){
	c.resize(buffer.size());
	
	Handle<Tokenizer> tokenizer = Handle<Tokenizer>(GetTokenizer());
	tokenizer->SetBuffer(&buffer);

	auto colorIt = c.begin();
	colorIt->clear();
	auto textIt = buffer.begin();
	
	Token currToken,prevToken;
	currToken = {TokenType::SpecialChar,{textIt,textIt->begin()},{textIt,textIt->begin()+1}};
	
	while (!tokenizer->Done()){
		prevToken = currToken;
		currToken = tokenizer->EmitToken();
		
		while (currToken.start.line!=textIt){
			++textIt;
			++colorIt;
			colorIt->clear();
		}
		
		if (currToken.type==TokenType::Name||currToken.type==TokenType::SpecialChar){
			// name highlights only span 1 line
			TextStyle style;
			if (!prevToken.Matches(".")&&TokenInKeywords(currToken,style))
				AddColorData(colorIt,textIt,currToken,style);
		} else {
			TextStyle tStyle = GetStyleFromTokenType(currToken.type);
			while (textIt!=currToken.end.line){
				AddColorData(colorIt,textIt,currToken,tStyle);
				++textIt;
				++colorIt;
				colorIt->clear();
			}
			AddColorData(colorIt,textIt,currToken,tStyle);
		}
	}
}

bool ConfigurableSyntaxHighlighter::TokenInKeywords(const Token& token,TextStyle& style) const {
	for (const auto& [k,v] : styleMap){
		if (token.Matches(k)){
			style = *v;
			return true;
		}
	}

	return false;
}

Tokenizer* CPPSyntaxHighlighter::GetTokenizer() const {
	return new CPPTokenizer();
}

void CPPSyntaxHighlighter::BuildKeywords(){
	AddKeywords({"if","else","while","for","do","namespace",
			"switch","case","default","break","return","volatile",
			"using","template","typedef","typename","new","catch","concept",
			"delete","struct","class","enum","union","sizeof","try","catch",
			"alignof","alignas","public","private","protected","continue",
			"static_cast","static_assert","const_cast","dynamic_cast",
			"goto","requires","export","import","module","reinterpret_cast",
			"explicit"},&statementStyle);

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
	"try","except","finally","raise","global","continue","break","pass","assert"};
static std::vector<std::string> pythonFuncs = {"range","len","print","repr","ord","chr","isinstance","staticmethod",
	"classmethod","reversed","issubclass","setattr","iter","aiter","input","max","min","sorted","super",
	"round","pow","dir","open","quit","help","hash","next","vars","hex","bin","oct","all","compile","eval","exec",
	"delattr","id","globals","callable","hasattr","locals","any","ascii","filter","format","sum",
	"__init__","__new__","__del__","__add__","__radd__",
	"__iadd__","__mul__","__rmul__","__imul__","__div__","__rdiv__","__idiv__","__sub__","__rsub__","__isub__",
	"__truediv__","__rtruediv__","__itruediv__","__floordiv__","__rfloordiv__","__ifloordiv__","__ge__","__le__",
	"__gt__","__lt__","__eq__","__ne__","__str__","__repr__","__neg__","__round__","__floor__","__ceil__","__float__",
	"__int__","__invert__","__sizeof__","__pow__","__divmod__","__rdivmod__","__idivmod__","__mod__","__rmod__","__imod__",
	"__or__","__ror__","__ior__","__xor__","__rxor__","__ixor__","__and__","__rand__","__iand__","__trunc__","__lshift__",
	"__rlshift__","__rshift__","__rrshift__","__ilshift__","irshift__","__setattr__","__delattr__","__hash__",
	"__iter__","__getitem__","__setitem__","__reversed__","__format__","__doc__","__delitem__","__contains__",
	"__class__","__len__"};
static std::vector<std::string> pythonTypes = {"int","float","bool","object","str","tuple",
	"list","map","set","dict","zip","enumerate","type","Exception","TypeError","ValueError",
	"NotImplementedError","bytes","bytearray","memoryview","complex","slice","frozenset"};
	
static std::vector<std::string> glslKeywords = {"layout","in","out","inout","flat","uniform","if","else","for","while",
	"do","switch","case","discard","break","continue","return","struct"};
static std::vector<std::string> glslFuncs = {"smoothstep","texture","textureSize","textureQueryLevels","textureOffset","textureProj",
	"textureLod","textureGrad","textureGather","textureGatherOffset","textureGatherOffsets","texelFetch","texelFetchOffset","abs",
	"sin","sinh","asin","asinh","cos","cosh","acos","acosh","tan","atan","tanh","atanh","any","all","bitCount","ceil","clamp","cross",
	"dot","degrees","determinant","dFdx","dFdy","dFdxCoarse","dFdxFine","dFdyCoarse","dFdyFine","distance","equal","exp","exp2",
	"floor","fma","fract","frexp","fwidth","fwidthCoarse","fwidthFine","faceforward","inverse","inversesqrt","isnan","isinf","length",
	"ldexp","log","log2","max","min","mix","mod","modf","matrixCompMult","noise","noise1","noise2","noise3","noise4","normalize",
	"outerProduct","pow","radians","reflect","refract","round","sign","sqrt","step","transpose","trunc"};
static std::vector<std::string> glslTypes = {"vec3","vec2","vec4","uvec3","uvec2","uvec4","bvec2","bvec3","bvec4","ivec2","ivec3",
	"ivec4","dvec2","dvec3","dvec4","float","int","void","double","bool","mat2","mat3","mat4","mat2x2","mat2x3","mat2x4","mat3x2",
	"mat3x3","mat3x4","mat4x2","mat4x3","mat4x4","dmat2","dmat3","dmat4","dmat2x2","dmat2x3","dmat2x4","dmat3x2","dmat3x3","dmat3x4",
	"dmat4x2","dmat4x3","dmat4x4","uint","const","sampler1D","isampler1D","usampler1D","sampler2D","isampler2D","usampler2D","sampler3D",
	"isampler3D","usampler3D","samplerCube","isamplerCube","usamplerCube","sampler2DRect","isampler2DRect","usampler2DRect","sampler1DArray",
	"isampler1DArray","usampler1DArray","sampler2DArray","isampler2DArray","usampler2DArray","samplerCubeArray","isamplerCubeArray",
	"usamplerCubeArray","samplerBuffer","isamplerBuffer","usamplerBuffer","sampler2DMS","isampler2DMS","usampler2DMS","sampler2DMSArray",
	"isampler2DMSArray","usampler2DMSArray","sampler1DShadow","sampler2DShadow","samplerCubeShadow","sampler2DRectShadow","sampler1DArrayShadow",
	"sampler2DArrayShadow","samplerCubeArrayShadow"};
static std::vector<std::string> terseKeywords = {"def","axiom","theorem","lemma","template","import",
	"alias","let","module"};
static std::vector<std::string> terseDefs = {"true","false"};
static std::vector<std::string> terseFuncs = {"&"};

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
	} else if (ext=="vert"||ext=="frag"||ext=="glsl"||ext=="vs"||ext=="fs"||ext=="geom"||ext=="gs"){
		ConfigurableSyntaxHighlighter* sh = new ConfigurableSyntaxHighlighter(buffer);
		sh->AddKeywords(glslKeywords,&statementStyle);
		sh->AddKeywords(glslFuncs,&funcStyle);
		sh->AddKeywords(glslTypes,&typeStyle);
		sh->SetComment("//");
		sh->SetAltComment("#");
		sh->SetMultiLineComment("/*","*/");
		return sh;
	} else if (ext=="trs"||ext=="thm"||ext=="axm"){
		ConfigurableSyntaxHighlighter* sh = new ConfigurableSyntaxHighlighter(buffer);
		sh->AddKeywords(terseKeywords,&statementStyle);
		sh->AddKeywords(terseDefs,&typeStyle);
		sh->AddKeywords(terseFuncs,&funcStyle);
		sh->SetComment("//");
		sh->SetAltComment("#");
		sh->SetMultiLineComment("/*","*/");
		return sh;
	}

	return nullptr;
}
