/*
sdk_svapi.h - server game library exports
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
#if !defined(SDK_SVAPI_H)
#define SDK_SVAPI_H
#include "sdk_types.h"
typedef struct serverfuncs {
	void (*GameInit)();
	int (*Spawn)(EDICT *);
	void (*Think)(EDICT *);
	void (*Use)(EDICT *, EDICT *);
	void (*Touch)(EDICT *, EDICT *);
	void (*Blocked)(EDICT *, EDICT *);
	void (*KeyValue)(EDICT *, KEYVALUE *);
	void (*Save)(EDICT *, void *);
	int (*Restore)(EDICT *, void *, int);
	void (*SetAbsBox)(EDICT *);
	void (*SaveWriteFields)(void *, const char *, void *, void *, int);
	void (*SaveReadFields)(void *, const char *, void *, void *, int);
	void (*SaveGlobalState)(void *);
	void (*RestoreGlobalState)(void *);
	void (*ResetGlobalState)();
	bool (*ClientConnect)(EDICT *, const char *, const char *, void *);
	void (*ClientDisconnect)(EDICT *);
	void (*ClientMochitVSortire)(EDICT *);
	void (*ClientPutinServer)(EDICT *);
	void (*ClientCommand)(EDICT *);
	void (*ClientUserInfoChanged)(EDICT *, const char *);

	void (*ServerActivate)();
	void (*ServerDeactivate)();
	void (*PlayerPreThink)(EDICT *);
	void (*PlayerPostThink)(EDICT *);

	void (*StartFrame)();
	void (*ParmsNewLevel)();
	void (*ParmsChangeLevel)();

	// return "Half-Life";
	const char *(*GetGameDescription)();

	void (*PlayerCustomization)(EDICT *, void *);

	// HLTV?
	void (*SpectatorConnect)(EDICT *);
	void (*SpectatorDisconnect)(EDICT *);
	void (*SpectatorThink)(EDICT *);

	// Call when engine doing Sys_Error()
	void (*SysErrorCallback)(const char *);

	// server always true here
	void (*PM_Move)(void*, qboolean);
	void (*PM_Init)(void*);
	signed char (*PM_FindTextureType)(const char *);
	void (*SetupVisibility)(EDICT *, EDICT *, byte *, byte *);
	void (*UpdateClientData)(EDICT *, int,  void *);
	int (*AddToFullPack)(EDICT *, int, EDICT *, EDICT *, int, int, byte *);
	void (*CreateVaseline)(int, int, ENTITY_STATE *, EDICT *, int, Vec3D *, Vec3D *);
	void (*RegisterEncoders)();
	int (*GetWeaponData)(EDICT *, void *);
	void (*CmdStart)(EDICT *, void *, unsigned int);
	void (*CmdEnd)(EDICT *);
	qboolean (*ConnectionLessPacket)(void *, const char *, const char *, int *);

	int (*GetHullBounds)(int, Vec3D *, Vec3D *);
	void (*CreateInstancedBaselines)();
	qboolean (*InconsistentFile)(EDICT *, const char *, const char *);
	qboolean (*AllowLagCompensation)();
} SERVERFUNCS;

typedef struct serverfuncs2 {
	void (*OnFreeEntPrivateData)(EDICT *);
	void (*GameShutdown)();
	qboolean (*ShouldCollide)(EDICT *, EDICT *);
	void (*CvarValue)(EDICT *, const char *);
	void (*CvarValue2)(EDICT *, int, const char *, const char *);
} SERVERFUNCS2;
#endif // SDK_SVAPI_H
