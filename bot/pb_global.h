#pragma warning( disable : 4786 )	// disable warnings


#if !defined( PB_GLOBAL_H )
#define PB_GLOBAL_H

#if !defined( _DEBUG )
#define NDEBUG			// no assert!
#endif

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "hl_classes.h"
#include "pb_path.h"
#include "assert.h"
#include "utilityfuncs.h"



// define constants used to identify the MOD we are playing...
#define VALVE_DLL		1
#define TFC_DLL			2
#define CSTRIKE_DLL		3
#define HOLYWARS_DLL	4
#define DMC_DLL			5
#define GEARBOX_DLL		6


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



float worldTime();
// returns the game time

float serverMaxSpeed();
// returns the maximum speed clients can reach

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




void checkForBreakpoint( int reason );
// for debugging

void debugFile( char *msg );

void debugMsg( const char *str1, const char *str2=0, const char *str3=0, const char *str4=0 );
void debugMsg( const char *str1, int data1, int data2=NO_I_VALUE, int data3=NO_I_VALUE );
void debugMsg( const char *str1, float data1, float data2=NO_F_VALUE, float data3=NO_F_VALUE );

void infoMsg( const char *str1, const char *str2=0, const char *str3=0, const char *str4=0 );

void errorMsg( const char *str1, const char *str2=0, const char *str3=0, const char *str4=0 );

void debugSound( edict_t *recipient, const char *sample );

void debugBeam( Vector start, Vector end, int life, int color=1 );
void debugMarker( Vector pos, int life );

#endif
