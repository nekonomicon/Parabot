/*
sdk_types.h - main types defination
Copyright (C) 2016 Mittorn

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#pragma once
#if !defined(SDK_TYPES_H)
#define SDK_TYPES_H

#include <stddef.h>

typedef int string;
typedef struct edict EDICT;
typedef struct entity_state ENTITY_STATE;
typedef struct entvars ENTVARS;
typedef struct keyvalue KEYVALUE;
typedef unsigned char byte;
typedef byte rgb[3];
typedef struct globalvars GLOBALENTVARS;
typedef struct traceresult TRACERESULT;
#undef false
#undef true
#ifndef __cplusplus
typedef enum { false, true } qboolean;
#else
typedef int qboolean;
#endif
typedef struct cvar CVAR;
typedef enum { AT_NOTICE, AT_CONSOLE, AT_AICONSOLE, AT_WARNING, AT_ERROR, AT_LOGGED } alert_type_t;
#endif // SDK_TYPES_H

