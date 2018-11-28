#pragma once
#if !defined(PLATFORM_H)
#define PLATFORM_H

// Allow "DEBUG" in addition to default "_DEBUG"
#if _DEBUG
#define DEBUG 1
#endif

#if _WIN32
#define ARCH_SUFFIX
#define OS_LIB_EXT "dll"
#undef CreateDirectory
#define CreateDirectory(p, n) CreateDirectoryA(p, n)
#else // _WIN32

#if __APPLE__
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
#include <stdio.h>
#define CreateDirectory(p, n) mkdir(p, 0777)
#define GetProcAddress dlsym
#define LoadLibrary(x) dlopen(x, RTLD_NOW)
#define Sleep(x) usleep(x * 1000)
typedef void* HINSTANCE;

#define stricmp strcasecmp
#define _stricmp strcasecmp
#define strnicmp strncasecmp
#define _strnicmp strncasecmp
#define WINAPI
#define MAX_PATH PATH_MAX
#include <stdarg.h>
#endif // _WIN32

// Misc C-runtime library headers
#include <stdlib.h>
#include <math.h>

// Exports
#include "exportdef.h"

#if !defined(BIT)
#define BIT(X) (1U<<X)
#endif // BIT

#define Q_STREQ(s1,s2)		(strcmp(s1, s2) == 0)
#define Q_STRIEQ(s1,s2)		(_stricmp(s1, s2) == 0)
#define Q_STRNEQ(s1,s2,size)	(strncmp(s1, s2, size) == 0)
#define Q_STRNIEQ(s1,s2,size)	(_strnicmp(s1, s2, size) == 0)

#if !defined(Q_min)
#define Q_min(a,b)  (((a) < (b)) ? (a) : (b))
#endif // Q_min
#if !defined(Q_max)
#define Q_max(a,b)  (((a) > (b)) ? (a) : (b))
#endif // Q_max

#if !defined(clamp)
#define clamp( val, min, max ) ( Q_max( max, Q_min( val, min )))
#endif // clamp

#endif // PLATFORM_H
