#pragma once

#ifdef __linux__
#include "interfaces/os_linux.h"
#include "interfaces/interface_curses.h"

#define CONTEXT_USER_INTERFACE CursesInterface
#define CONTEXT_OS_INTERFACE LinuxOSImpl
#endif

#ifdef _WIN32
#include "interfaces/os_windows.h"
#include "interfaces/interface_wincon.h"

#define CONTEXT_USER_INTERFACE WinConInterface
#define CONTEXT_OS_INTERFACE WindowsOSImpl
#endif

