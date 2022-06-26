#include "style.h"

#include "config.h"

std::map<u64,StyleSet> gStyleMap = {};

void LoadStyle(){
	if (!gStyleMap.contains(gConfig.style))
		gStyleMap[gConfig.style] = defaultStyleSet;
	
	StyleSet set = gStyleMap.at(gConfig.style);
	
#define STYLE(x) x ## Style = set.x ## Style;
	#include "stylenames.h"
#undef STYLE

}

void SaveStyle(){
	if (!gStyleMap.contains(gConfig.style))
		gStyleMap[gConfig.style] = defaultStyleSet;
		
	StyleSet& set = gStyleMap[gConfig.style];	
	
#define STYLE(x) set.x ## Style = x ## Style;
	#include "stylenames.h"
#undef STYLE

}

//#define SOLARIZED
#define GRUVBOX

Color ColorBlack = {0,0,0};
Color ColorWhite = {255,255,255};

#ifdef DEFAULTCOLORS
Color ColorRed = {255,0,0};
Color ColorOrange = {255,128,0};
Color ColorGreen = {0,255,0};
Color ColorYellow = {255,255,0};
Color ColorBlue = {0,0,255};
Color ColorCyan = {0,255,255};
Color ColorMagenta = {255,0,255};

Color ColorBackgroundDark = ColorBlack;
Color ColorBackgroundLight = ColorWhite;
Color ColorForegroundDark = ColorWhite;
Color ColorForegroundLight = ColorBlack;
#endif
#ifdef SOLARIZED
Color ColorBackground1 = {0,43,54};
Color ColorBackground2 = {7,54,66};
Color ColorContent1 = {88,110,117};
Color ColorContent2 = {101,123,131};
Color ColorContent3 = {131,148,150};
Color ColorContent4 = {147,161,161};
Color ColorBackgroundLight1 = {238,232,213};
Color ColorBackgroundLight2 = {253,246,227};

Color ColorYellow = {181,137,0};
Color ColorOrange = {203,75,22};
Color ColorRed = {220,50,47};
Color ColorMagenta = {211,54,130};
Color ColorViolet = {108,113,196};
Color ColorBlue = {38,139,210};
Color ColorCyan = {42,161,152};
Color ColorGreen = {133,153,0};


Color ColorBackgroundDark = ColorBackground1;
Color ColorBackgroundLight = ColorBackgroundLight1;
Color ColorForegroundDark = ColorContent4;
Color ColorForegroundLight = ColorContent1;
#endif
#ifdef GRUVBOX
Color ColorBackground1 = {0x28,0x28,0x28};
Color ColorBackground2 = {0x3c,0x38,0x36};
Color ColorBackground3 = {0x50,0x49,0x45};
Color ColorBackground4 = {0x66,0x5c,0x54};
Color ColorForeground1 = {0xeb,0xdb,0xb2};
Color ColorForeground2 = {0xd5,0xc4,0xa1};
Color ColorForeground3 = {0xbd,0xae,0x93};
Color ColorForeground4 = {0xa8,0x99,0x84};

Color ColorForeground0 = {0xfb,0xf1,0xc7};

Color ColorRed = {0xcc,0x24,0x1d};
Color ColorGreen = {0xb8,0xbb,0x26};
Color ColorYellow = {0xfa,0xbd,0x2f};
Color ColorBlue = {0x45,0x85,0x88};
Color ColorMagenta = {0xd3,0x86,0x9b};
Color ColorCyan = {0x8e,0xc0,0x7c};
Color ColorOrange = {0xfe,0x80,0x19};

Color ColorComment = {0x83,0xa5,0x98};

Color ColorGray = {0x92,0x83,0x74};


Color ColorBackground = ColorBackground1;
Color ColorForeground = ColorForeground1;

#endif

const StyleSet defaultStyleSet = {
	.textStyle = {ColorForeground,ColorBackground,StyleFlag::NoFlag},
	.cursorStyle = {ColorBackground,ColorForeground,StyleFlag::NoFlag},
	.lineNumberStyle = {ColorGray,ColorBackground2,StyleFlag::NoFlag},
	.emptyLineStyle = {ColorForeground3,ColorBackground,StyleFlag::NoFlag},
	.errorStyle = {ColorRed,ColorBlack,StyleFlag::NoFlag},
	.statementStyle = {ColorYellow,ColorBackground,StyleFlag::NoFlag},
	.typeStyle = {ColorGreen,ColorBackground,StyleFlag::NoFlag},
	.funcStyle = {ColorMagenta,ColorBackground,StyleFlag::NoFlag},
	.numberStyle = {ColorRed,ColorBackground,StyleFlag::NoFlag},
	.stringStyle = {ColorGreen,ColorBackground,StyleFlag::NoFlag},
	.directiveStyle = {ColorMagenta,ColorBackground,StyleFlag::NoFlag},
	.commentStyle = {ColorComment,ColorBackground,StyleFlag::NoFlag},
	.barStyle = {ColorWhite,ColorBackground3,StyleFlag::NoFlag},
	.tabBarStyle = {ColorForeground,ColorBackground3,StyleFlag::NoFlag},
	.tabBarSelectStyle = {ColorWhite,ColorBackground,StyleFlag::NoFlag},
	.highlightStyle = {ColorBlack,ColorYellow,StyleFlag::NoFlag},
	.highlightSelectStyle = {ColorBlack,ColorOrange,StyleFlag::NoFlag}
};

#define STYLE(x) TextStyle x ## Style;
#include "stylenames.h"
#undef STYLE

