/*
physint.h - Server Physics Interface
Copyright (C) 2011 Uncle Mike

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
#ifndef PHYSINT_H
#define PHYSINT_H

// physic callbacks
typedef struct physics_interface
{
	int version;
	// passed through pfnCreate (0 is attempt to create, -1 is reject)
	int (*SV_CreateEntity)(EDICT *, const char *);
} PHYSICS_INTERFACE;
#endif//PHYSINT_H
