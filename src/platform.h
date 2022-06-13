#pragma once

#ifdef __linux__
#include "interfaces/os_linux.h"
#include "interfaces/interface_curses.h"
#endif

#ifdef _WIN32
#include "interfaces/os_windows.h"
#include "interfaces/interface_wincon.h"
#endif

