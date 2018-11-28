/*
sdk_common.h - engine structures
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
#if !defined(SDK_COMMON_H)
#define SDK_COMMON_H

#include "platform.h"
#include "sdk_types.h"
#include "sdk_svapi.h"
#include "sdk_engapi.h"
#include "sdk_defines.h"
#include "meta_api.h"
#include "physint.h"

typedef struct entvars {
	string classname, globalname;
	Vec3D origin, oldorigin, velocity, basevelocity;
	Vec3D clbasevelocity, movedir;
	Vec3D angles, avelocity,  punchangle, v_angle;
	Vec3D endpos, startpos;
	float impacttime, starttime;
	int fixangle;
	float idealpitch, pitch_speed, ideal_yaw, yaw_speed;
	int modelindex;
	string model;
	int viewmodel, weaponmodel;
	Vec3D absmin, absmax, mins, maxs, size;
	float ltime, nextthink;
	int movetype, solid, skin, body, effects;
	float gravity, friction;
	int light_level, sequence, gaitsequence;
	float frame, animtime, framerate;
	byte controller[4], blending[2];
	float scale;
	int rendermode;
	float renderamt;
	Vec3D rendercolor;
	int renderfx;
	float health, frags;
	int weapons;
	float takedamage;
	int deadflag;
	Vec3D view_ofs;
	int button;
	int impulse;
	EDICT *chain, *dmg_inflictor, *enemy, *aiment, *owner, *groundentity;
	int spawnflags, flags, colormap, team;
	float max_health, teleport_time, armortype, armorvalue;
	int waterlevel, watertype;
	string target, targetname, netname, message;
	float dmg_take, dmg_save, dmg, dmgtime;
	string noice, noise1, noice2, noice3;
	float speed, air_finished, pain_finished, radsuit_finished;
	EDICT *pContainingEntity;
	int playerclass;
	float maxspeed, fov;
	int weaponanim, pushmsec, isInDuck, TimeStepSound, SwimTime, DuckTime, StepLeft;
	float FallVelocity;
	int gamestate, oldbuttons, groupinfo;
	int userInteger[4];
	float userFloat[4];
	Vec3D userVector[4];
	EDICT *userEntity[4];
} ENTVARS;

typedef struct link {
	struct link *p, *n;
} LINK;

typedef struct edict {
	qboolean free;
	int serialnumber;
	LINK area;
	int headnode, num_leafs;
	short leafnums[48];
	float freetime;
	void *pvPrivateData;
	ENTVARS v;
} EDICT;

typedef struct globalvars {
	float time, frametime, forceretouch;
	int mapname, startspot;
	float deathmatch, coop, teamplay, serverflags, foundsecrets;
	Vec3D fwd, up, right;
	float trallsolid, trstartsolid, trfraction;
	Vec3D trendpos, trplanenormal;
	float trplanedist;
	EDICT *traceent;
	float traceinopen, traceinwater;
	int tracegitgroup, traceflags;
	int msgentity, cdtrack, maxclients, maxentities;
	char *pStringBase;
	void *pSaveData;
	Vec3D *landmarkoffset;
} GLOBALENTVARS;

typedef struct keyvalue {
	const char *classname;
	const char *keyname;
	const char *value;
	bool handled;
} KEYVALUE;

typedef struct entity_state {
	int entityType, number;
	float msgtime;
	int msgnum;
	Vec3D origin, angles;
	int modelindex, sequence;
	float frame;
	int colormap;
	short skin;
	unsigned short solid;
	int effects;
	float scale;
	byte eflags;
	int rendermode, renderamt;
	rgb rendercolor;
	int renderfx;
	int movetype;
	float animtime, framerate;
	int body;
	byte contoller[4];
	byte blending[4];
	Vec3D velocity, mins, maxs;
	int aiment, owner;
	float firction, graviry;
	int team, playerclass, health, spectator, weaponmodel, gaitsequence;
	Vec3D basevelocity;
	int usehull, oldbuttons, onground, stepleft;
	float FallVelocity, fov;
	int weaponanim;
	Vec3D startpos, endpos;
	float impacttime, starttime;
	int userInt[4];
	float userFloat[4];
	Vec3D userVector[4];
	EDICT *userEntity[4];
} ENTITY_STATE;

enum {
	MAX_PHYSINFO_STRING = 256
};

typedef struct clientdata {
	Vec3D		origin;
	Vec3D		velocity;

	int		viewmodel;
	Vec3D		punchangle;
	int		flags;
	int		waterlevel;
	int		watertype;
	Vec3D		view_ofs;
	float		health;

	int		isInDuck;
	int		weapons; // remove?

	int		flTimeStepSound;
	int		flDuckTime;
	int		flSwimTime;
	int		waterjumptime;

	float		maxspeed;

	float		fov;
	int		weaponanim;

	int		iId;
	int		ammo[4];
	float		m_flNextAttack;

	int		tfstate;
	int		pushmsec;
	int		deadflag;
	char		physinfo[MAX_PHYSINFO_STRING];

	// For mods
	int userInteger[4];
	float userFloat[4];
	Vec3D userVector[4];
} CLIENTDATA;

typedef struct traceresult {
	qboolean allsolid, startsolid, inopen, inwater;
	float	 fraction;
	Vec3D	 endpos;
	float	 planedist;
	Vec3D	 planenormal;
	EDICT	*hit;
	int	 hitgroup;
} TRACERESULT;

typedef struct cvar {
	const char	*name;
	const char	*string;
	int		 flags;
	float		 value;
	struct cvar	*next;
} CVAR;

typedef struct common {
	GLOBALENTVARS		*globals;
	ENGINEAPI		*engfuncs;
	GAMEDLLFUNCS		*gamedll;
	PHYSICS_INTERFACE	*physfuncs;
	META_API_FUNCS		 metamod;
	char			 modname[32];
	HINSTANCE		 gamedll_handle;
	HINSTANCE		 self_handle;
	int 			 gamedll_flags;
} COMMON;

extern COMMON com;
extern const Vec3D zerovector;
extern const Vec3D nullvector;

// Use this instead of ALLOC_STRING on constant strings
#define STRING(offset)		(const char *)(com.globals->pStringBase + (int)offset)
#if !defined PB_64BIT
#define MAKE_STRING(str)	((int)(long int)str - (int)(long int)STRING(0))
#else
static inline int MAKE_STRING(const char *szValue)
{
	long long ptrdiff = szValue - STRING(0);
	if (ptrdiff > INT_MAX || ptrdiff < INT_MIN)
		return com.engfuncs->AllocEngineString(szValue);
	else
		return (int)ptrdiff;
}
#endif

#endif // SDK_COMMON_H

