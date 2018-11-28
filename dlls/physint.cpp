/*
physint.cpp - Server Physics Interface
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

#include <string.h>
#include "exportdef.h"
#include "parabot.h"
#include "bot.h"
#include "physint.h"

//
// Xash3D physics interface
//

//
// attempt to create custom entity when default method is failed
// 0 - attempt to create, -1 - reject to create
//
static int dispatch_createentity(EDICT *e, const char *name)
{
	LINK_ENTITY_FUNC spawnedict = (LINK_ENTITY_FUNC)GetProcAddress(com.gamedll_handle, name);

	if(spawnedict) {	// found the valid spawn
		spawnedict(&e->v);
		return 0;	// handled
	}

	if(com.physfuncs->SV_CreateEntity)
		return com.physfuncs->SV_CreateEntity(e, name);

	return -1;
}

extern "C" bool EXPORT Server_GetPhysicsInterface( int version, void *engfuncs, PHYSICS_INTERFACE *functiontable )
{
	SERVER_GETPHYSICSINTERFACE other_Server_GetPhysicsInterface = (SERVER_GETPHYSICSINTERFACE)GetProcAddress(com.gamedll_handle, "Server_GetPhysicsInterface");

	if(other_Server_GetPhysicsInterface)
		other_Server_GetPhysicsInterface(version, engfuncs, functiontable);

	com.physfuncs = (PHYSICS_INTERFACE *)malloc(sizeof(PHYSICS_INTERFACE));
	com.physfuncs->SV_CreateEntity = functiontable->SV_CreateEntity;
	functiontable->version = version;
	functiontable->SV_CreateEntity = dispatch_createentity;

	return true;
}
