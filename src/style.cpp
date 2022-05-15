#include "style.h"

#define SOLARIZED

Color ColorBlack = {0,0,0};
Color ColorWhite = {255,255,255};

#ifndef SOLARIZED
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
#else
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


TextStyle defaultStyle = {ColorForegroundDark,ColorBackgroundDark,StyleFlag::NoFlag};
TextStyle cursorStyle = {ColorBackgroundDark,ColorForegroundDark,StyleFlag::NoFlag};
TextStyle lineDrawingStyle = {ColorForegroundDark,ColorBackgroundDark,StyleFlag::AlternateCharacterSet};
TextStyle lineNumberStyle = {ColorContent1,ColorBackground2,StyleFlag::NoFlag};
TextStyle errorStyle = {ColorRed,ColorBackgroundDark,StyleFlag::NoFlag};
TextStyle blankLineStyle = {ColorContent1,ColorBackgroundDark,StyleFlag::NoFlag};
TextStyle statementStyle = {ColorYellow,ColorBackgroundDark,StyleFlag::NoFlag};
TextStyle typeStyle = {ColorGreen,ColorBackgroundDark,StyleFlag::NoFlag};
TextStyle funcStyle = {ColorCyan,ColorBackgroundDark,StyleFlag::NoFlag};
