#pragma once
#if !defined(PB_GLOBAL_H)
#define PB_GLOBAL_H

#include "meta_api.h"
#include "pb_path.h"
#include "assert.h"

// define constants used to identify the MOD we are playing...
#define VALVE_DLL	1
#define TFC_DLL		2
#define CSTRIKE_DLL	3
#define GEARBOX_DLL	4
#define FRONTLINE_DLL	5
#define HOLYWARS_DLL	6
#define DMC_DLL		7
#define AG_DLL		8
#define HUNGER_DLL	9

// bot-actions
enum {
	BOT_JUMP = 1,	
	BOT_LONGJUMP,
	BOT_USE,
	BOT_DUCK,
	BOT_FIRE_PRIM,
	BOT_FIRE_SEC,
	BOT_STRAFE_LEFT,
	BOT_STRAFE_RIGHT,
	BOT_DELAYED_JUMP,
	BOT_RELOAD,
	BOT_RELEASE_SEC,
	BOT_DUCK_LONG,	
	BOT_STOP_RUNNING
};

// break-reasons
#define BREAK_WEAPON		1
#define BREAK_GOALS			2

// Default values for message functions
#define NO_I_VALUE -123456789
#define NO_F_VALUE -0.123456789

enum {
	GAMEDLL_METAMOD =	BIT(0), // Bot library loaded as metamod plugin
	GAMEDLL_DEBUG =	BIT(1), // Debug log
	GAMEDLL_TEAMPLAY =	BIT(2), // Teamplay gamemode
	GAMEDLL_DOM =	BIT(3), // Domination gamemode
	GAMEDLL_CTF =	BIT(4), // Capture The Flag gamemode
	GAMEDLL_BMOD =	BIT(5), // Bubblemod game library
	GAMEDLL_SEVS =	BIT(6) // Severians mod
};

extern const CVAR *flashlight;
extern const CVAR *footsteps;
extern const CVAR *freezetime;
extern const CVAR *gamemode;
extern const CVAR *teamplay;
extern const CVAR *maxspeed;

extern const CVAR *bm_cbar;
extern const CVAR *bm_gluon;
extern const CVAR *bm_trip;

bool LOSExists(const Vec3D *v1, const Vec3D *v2);
// traces line with ignore monster from v1 to v2

EDICT* getEntity( const char *classname, Vec3D *pos );
// returns a pointer to edict at pos if it exists, else 0

// methods based on mapgraph:
NAVPOINT *getNavpoint( int index );
int getNavpointIndex( EDICT *entity );
PB_Path* getPath( int pathId );
int getTotalAttempts();
void incTotalAttempts();

#if _DEBUG
void checkForBreakpoint( int reason );
// for debugging

void pb2dMsg( int x, int y, const char *szFmt, ... );
void pb3dMsg( int x, int y, const char *szFmt, ... );
void print3dDebugInfo();
void debugBeam( Vec3D *start, vec3_t *end, int life, int color=1 );
void debugMarker( Vec3D *pos, int life );
#else // _DEBUG
#define checkForBreakpoint( reason )
#define debugSound( recipient, sample )
#define debugMarker( pos, life )
#define print3dDebugInfo()
#if _MSC_VER == 1200
#define pb2dMsg( x )
#define pb3dMsg( x )
#else // _MSC_VER
#define pb2dMsg( x, y, szFmt, ... )
#define pb3dMsg( x, y, szFmt, ... )
#define debugBeam(...)
#endif // _MSC_VER
#endif // _DEBUG
#endif // PB_GLOBAL_H
