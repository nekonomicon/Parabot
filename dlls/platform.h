#pragma once
#ifndef PLATFORM_H
#define PLATFORM_H

// Allow "DEBUG" in addition to default "_DEBUG"
#ifdef _DEBUG
#define DEBUG 1
#endif

// Silence certain warnings
#pragma warning(disable : 4244)		// int or float down-conversion
#pragma warning(disable : 4305)		// int or float data truncation
#pragma warning(disable : 4201)		// nameless struct/union
#pragma warning(disable : 4514)		// unreferenced inline function removed
#pragma warning(disable : 4100)		// unreferenced formal parameter

// Prevent tons of unused windows definitions
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#define HSPRITE HSPRITE_win32
#include <windows.h>
#undef HSPRITE
#define ARCH_SUFFIX
#define OS_LIB_EXT "dll"
#undef CreateDirectory
#define CreateDirectory(p, n) CreateDirectoryA(p, n)
#else // _WIN32

#ifdef __APPLE__
#define ARCH_SUFFIX
#define OS_LIB_EXT "dylib"
#else // __APPLE__
	#if defined(__amd64__) || defined(_M_X64)
	#define ARCH_SUFFIX "_amd64"
	#elif defined(__i386__) || defined(_X86_) || defined(_M_IX86)
	#define ARCH_SUFFIX "_i386"
	#else
	#define ARCH_SUFFIX
	#endif
#define OS_LIB_EXT "so"
#endif // __APPLE__

#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#define CreateDirectory(p, n) mkdir(p, 0777)
#define GetProcAddress dlsym
#define LoadLibrary(x) dlopen(x, RTLD_NOW)
#define Sleep(x) usleep(x * 1000)
typedef void* HINSTANCE;
#define stricmp strcasecmp
#define _stricmp strcasecmp
#define strnicmp strncasecmp
#define _strnicmp strncasecmp
#define FAR
#define WINAPI
#define FALSE 0
#define TRUE (!FALSE)
typedef unsigned long ULONG;
typedef unsigned char BYTE;
typedef int BOOL;
#define MAX_PATH PATH_MAX
#include <stdarg.h>
#endif //_WIN32

#ifndef strcpy_s
#ifdef strlcpy
#define strcpy_s(src, size, dst) strlcpy(src, dst, size)
#else // strlcpy
#define strcpy_s(src, size, dst) strncpy(src, dst, size-1); \
					src[size-1] = '\0'
#endif // strlcpy
#endif // strcpy_s

// Misc C-runtime library headers
#include <stdlib.h>
#include <math.h>

// Exports
#include "exportdef.h"

#ifndef BIT
#define BIT(X) (1U<<X)
#endif // BIT

#ifndef Q_min
#define Q_min(a,b)  (((a) < (b)) ? (a) : (b))
#endif // Q_min
#ifndef Q_max
#define Q_max(a,b)  (((a) > (b)) ? (a) : (b))
#endif // Q_max

#ifndef clamp
#define clamp( val, min, max ) ( Q_max( max, Q_min( val, min )))
#endif // clamp

#endif // PLATFORM_H
