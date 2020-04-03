#pragma once
#undef UNICODE
#ifndef UTF8
#   define UNICODE
#endif

#define NOMINMAX
#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>        // The main Windows API header, “mother of all header files”.
#include <windowsx.h>       // Message cracker macros etc., fairly undocumented nowadays.
