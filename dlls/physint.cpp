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
#include "extdll.h"
#include "bot.h"
#include "physint.h"

extern HINSTANCE h_Library;

//
// Xash3D physics interface
//

//
// attempt to create custom entity when default method is failed
// 0 - attempt to create, -1 - reject to create
//
int DispatchCreateEntity( edict_t *pent, const char *szName )
{
	LINK_ENTITY_FUNC SpawnEdict = (LINK_ENTITY_FUNC)dlsym( h_Library, szName );

	if( SpawnEdict )	// found the valid spawn
	{
		SpawnEdict( &pent->v );
		return 0;	// handled
	}

	return -1; // failed
}
//
//
// run custom physics for each entity
// return 0 to use built-in engine physic
//

int DispatchPhysicsEntity( edict_t *pEdict )
{
	return 0;
}

static physics_interface_t gPhysicsInterface =
{
	SV_PHYSICS_INTERFACE_VERSION,
	DispatchCreateEntity,
	DispatchPhysicsEntity,
};

extern "C" int EXPORT Server_GetPhysicsInterface( int iVersion, server_physics_api_t *pfuncsFromEngine, physics_interface_t *pFunctionTable )
{
	if( !pFunctionTable || !pfuncsFromEngine || iVersion != SV_PHYSICS_INTERFACE_VERSION )
	{
		return FALSE;
	}

	memcpy( pFunctionTable, &gPhysicsInterface, sizeof(physics_interface_t) );

	return TRUE;
}
