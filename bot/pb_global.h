#pragma warning( disable : 4786 )	// disable warnings


#if !defined( PB_GLOBAL_H )
#define PB_GLOBAL_H

#include "extdll.h"
#include "dllapi.h"
#include "meta_api.h"
#include "entity_state.h"
#include "pb_path.h"
#include "assert.h"
#include "utilityfuncs.h"

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
#define BOT_JUMP			1	
#define BOT_LONGJUMP		2
#define BOT_USE				3
#define BOT_DUCK			4
#define BOT_FIRE_PRIM		5
#define BOT_FIRE_SEC		6
#define BOT_STRAFE_LEFT		7
#define BOT_STRAFE_RIGHT	8
#define BOT_DELAYED_JUMP	9
#define BOT_RELOAD		   10
#define BOT_RELEASE_SEC	   11
#define BOT_DUCK_LONG	   12	
#define BOT_STOP_RUNNING   13

// break-reasons
#define BREAK_WEAPON		1
#define BREAK_GOALS			2

// Default values for message functions
#define NO_I_VALUE -123456789
#define NO_F_VALUE -0.123456789

extern unsigned int g_uiGameFlags;

#define GAME_METAMOD		BIT(0) // Bot library loaded as metamod plugin
#define GAME_DEBUG		BIT(1) // Debug log
#define GAME_TEAMPLAY		BIT(2) // Teamplay gamemode
#define GAME_DOM		BIT(3) // Domination gamemode
#define GAME_CTF		BIT(4) // Capture The Flag gamemode
#define GAME_BMOD		BIT(5) // Bubblemod game library
#define GAME_SEVS		BIT(6) // Severians mod

extern const cvar_t *flashlight;
extern const cvar_t *footsteps;
extern const cvar_t *freezetime;
extern const cvar_t *gamemode;
extern const cvar_t *teamplay;
extern const cvar_t *maxspeed;

extern const cvar_t *bm_cbar;
extern const cvar_t *bm_gluon;
extern const cvar_t *bm_trip;

float worldTime();
// returns the game time

float serverMaxSpeed();
// returns the maximum speed clients can reach

int GetFrameRateInterval();
// returns frame rate interval for new clients

bool LOSExists( Vector v1, Vector v2 );
// traces line with ignore monster from v1 to v2

edict_t* getEntity( const char *classname, Vector pos );
// returns a pointer to edict at pos if it exists, else 0

// methods based on mapgraph:
PB_Navpoint& getNavpoint( int index );
int getNavpointIndex( edict_t *entity );
PB_Path* getPath( int pathId );
int getTotalAttempts();
void incTotalAttempts();

extern void infoMsg( const char *szFmt, ... );
extern void errorMsg( const char *szFmt, ... );

#ifdef _DEBUG
void checkForBreakpoint( int reason );
// for debugging

void pb2dMsg( int x, int y, const char *szFmt, ... );
void pb3dMsg( int x, int y, const char *szFmt, ... );
void print3dDebugInfo();

void debugFile( const char *szFmt, ... );
void debugMsg( const char *szFmt, ... );

void debugSound( edict_t *recipient, const char *sample );
void debugBeam( Vector start, Vector end, int life, int color=1 );
void debugMarker( Vector pos, int life );
#else // _DEBUG
#define checkForBreakpoint( reason )
#define debugSound( recipient, sample )
#define debugMarker( pos, life )
#define print3dDebugInfo()
#if _MSC_VER == 1200
#define pb2dMsg( x )
#define pb3dMsg( x )
#define debugFile( szFmt )
#define debugMsg( szFmt )
#define debugBeam( szFmt )
#else // _MSC_VER
#define pb2dMsg( x, y, szFmt, ... )
#define pb3dMsg( x, y, szFmt, ... )
#define debugFile( szFmt, ... )
#define debugMsg( szFmt, ... )
#define debugBeam(...)
#endif // _MSC_VER
#endif // _DEBUG
#endif // PB_GLOBAL_H
