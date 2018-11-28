/*
sdk_engapi.h - engine server api
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
#if !defined(SDK_ENGAPI_H)
#define SDK_ENGAPI_H

typedef struct engineapi {
	int		 (*PrecacheModel)(const char *);
	int		 (*PrecacheSound)(const char *);
	void		 (*SetModel)(EDICT *, const char *);
	int		 (*ModelIndex)(const char *);
	int		 (*ModelFrames)(int);
	void		 (*SetSize)(EDICT *, Vec3D *, Vec3D *);
	void		 (*Changelevel)(const char *, const char *);
	void		 (*GetSpawnParams)(EDICT *);
	void		 (*SaveSpawnParams)(EDICT *);
	float		 (*VecToYaw)(Vec3D *);
	void		 (*VecToAngles)(Vec3D *, Vec3D *);
	void		 (*MoveToOrigin)(EDICT *, Vec3D *, float, int);
	void		 (*ChangeYaw)(EDICT *);
	void		 (*ChangePitch)(EDICT *);
	EDICT		*(*FindEntityByString)(EDICT *, const char *, const char *);
	int		 (*GetEntityIllum)(EDICT *);
	EDICT		*(*FindEntityInSphere)(EDICT *, Vec3D *, float);
	EDICT		*(*FindClientInPVS)(EDICT *);
	EDICT		*(*EntitiesInPVS)(EDICT *);
	void		 (*MakeVectors)(Vec3D *);
	void		 (*AngleVectors)(Vec3D *, Vec3D *, Vec3D *, Vec3D *);
	EDICT		*(*CreateEntity)();
	void		 (*RemoveEntity)(EDICT *);
	EDICT		*(*CreateNamedEntity)(string);
	void		 (*MakeStatic)(EDICT *);
	qboolean	 (*EntIsOnFloor)(EDICT *);
	int		 (*DropToFloor)(EDICT *);
	int		 (*WalkMove)(EDICT *, float, float, int);
	void		 (*SetOrigin)(EDICT *, Vec3D *);
	void		 (*EmitSound)(EDICT *, int, const char *, float, float, int, int);
	void		 (*EmitAmbientSound)(EDICT *, Vec3D *, const char *, float, float, int, int);
	void		 (*TraceLine)(const Vec3D *, const Vec3D *, int, EDICT *, TRACERESULT *);
	void		 (*TraceToss)(EDICT *, EDICT *, TRACERESULT *);
	int		 (*TraceMonsterHull)(Vec3D *, Vec3D *, qboolean, EDICT *, TRACERESULT *);
	void		 (*TraceHull)(Vec3D *, Vec3D *, qboolean, int, EDICT *, TRACERESULT *);
	void		 (*TraceModel)(Vec3D *, Vec3D *, int, EDICT *, TRACERESULT *);
	const char	*(*TraceTexture)(EDICT *, Vec3D *, Vec3D *);
	void		 (*TraceSphere)(Vec3D *, Vec3D *, qboolean, float, EDICT *, TRACERESULT *);
	void		 (*GetAimVector)(EDICT *, float, Vec3D *);
	void		 (*ServerCommand)(const char *);// Cbuf_AddText()
	void		 (*ServerExecute)(); // Cbuf_Execute()
	void		 (*ClientCommand)(EDICT *, const char *, ...);
	void		 (*ParticleEffect)(Vec3D *, Vec3D *, float, float);
	void		 (*LightStyle)(int, const char *);
	int		 (*DecalIndex)(const char *);
	int		 (*PointContents)(Vec3D *);

	// message funcs
	void		 (*MessageBegin)(int, int, const Vec3D *, EDICT *);
	void		 (*MessageEnd)();
	void		 (*WriteByte)(int);
	void		 (*WriteChar)(int);
	void		 (*WriteShort)(int);
	void		 (*WriteLong)(int);
	void		 (*WriteAngle)(float);
	void		 (*WriteCoord)(float);
	void		 (*WriteString)(const char *);
	void		 (*WriteEntity)(int);

	// cvar funcs
	void		 (*CvarRegister)(CVAR *);
	float		 (*CvarGetFloat)(const char *);
	const char	*(*CvarGetString)(const char *);
	void		 (*CvarSetFloat)(const char *, float);
	void		 (*CvarSetString)(const char *, const char *);

	// printing funcs
	void		 (*AlertMessage)(alert_type_t, const char *, ...);

	// stdio FILE*
	void		 (*EngineFprintf)(void *, const char *, ...);

	// private data funcs
	void		*(*PvAllocEntPrivateData)(EDICT *, size_t);
	void		*(*PvEntPrivateData)(EDICT *);
	void		 (*FreeEntPrivateData)(EDICT *);

	// string offset funcs
	const char	*(*SzFromIndex)(string);
	string		 (*AllocEngineString)(const char *);

	// entity pointer conversion funcs
	ENTVARS		*(*GetVarsOfEnt)(EDICT *);
	EDICT		*(*PEntityOfEntOffset)(unsigned int);
	unsigned int	*(*EntOffsetOfPEntity)(EDICT *);
	int		 (*IndexOfEdict)(EDICT *);
	EDICT		*(*PEntityOfEntIndex)(int);
	EDICT		*(*FindEntityByVars)(ENTVARS *);

	void		*(*GetModelPtr)(EDICT *);
	int		 (*RegUserMsg)(const char *, int);
	void		 (*AnimationAutomove)(EDICT *, int, Vec3D *, Vec3D *);
	void		 (*GetBonePosition)(EDICT *, int, Vec3D *, Vec3D *);

	void		*(*FunctionFromName)(const char *);
	const char	*(*NameForFunction)(void *);

	void		*(*ClientPrintF)(EDICT *, int, const char *);
	void		*(*ServerPrint)(const char *);
	const char	*(*Cmd_Args)();
	const char	*(*Cmd_Argv)(int);
	int		 (*Cmd_Argc)();

	void		 (*GetAttachment)(EDICT *, int, Vec3D *, Vec3D *);

	void		 (*CRC32_Init)(unsigned int *);
	void		 (*CRC32_ProcessBuffer)(unsigned int *, byte *, unsigned int);
	void		 (*CRC32_ProcessByte)(unsigned int *, byte);
	void		 (*CRC32_Final)(unsigned int *);

	int		 (*RandomLong)(int, int);
	float		 (*RandomFloat)(float, float);

	void		 (*SetView)(EDICT *, EDICT *);
	float		 (*Time)();
	void		 (*CrosshairAngle)(EDICT *, float, float);
	byte		*(*LoadFileForMe)(const char *, int *);
	void		 (*FreeFile)(void *);

	void		 (*EndSection)(const char *);
	int		 (*CompareFileTime)(const char *, const char *, int *);
	void		 (*GetGameDir)(char *);
	void		 (*Cvar_RegisterVariable)(CVAR *);
	void		 (*FadeClientVolume)(EDICT *, int, int, int, int);
	void		 (*SetClientMaxspeed)(EDICT *, float);
	EDICT		*(*CreateFakeClient)(const char *);
	void		 (*RunPlayerMove)(EDICT *, const Vec3D *, float, float, float, unsigned short, byte, byte);
	int		 (*NumberOfEntities)();
	const char	*(*GetInfoKeyBuffer)(EDICT *);
	const char	*(*InfoKeyValue)(const char *, const char *);
	void		 (*SetInfoKeyValue)(const char *, const char *, const char *);
	void		 (*SetClientKeyValue)(int, const char *, const char *, const char *);
	int		 (*IsMapValid)(const char *);
	void		 (*StaticDecal)(Vec3D *, int, int, int);
	int		 (*PrecacheGeneric)(const char *);
	int		 (*GetPlayerUserid )(EDICT *);
	void		 (*BuildSoundMsg)(EDICT *, int, const char *, float, float, int, int, int, int, Vec3D *, EDICT *);
	qboolean	 (*IsDedicatedServer)();
	CVAR		*(*CvarGetPointer)(const char *);
	unsigned int	 (*GetPlayerWONId)(EDICT *);
} ENGINEAPI;
#endif // SDK_ENGAPI_H

