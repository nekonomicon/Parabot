//
// HPB_bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// bot.h
//

#ifndef BOT_H
#define BOT_H

#include "exportdef.h"
#ifdef _WIN32
#define ARCH_SUFFIX
#define OS_LIB_EXT "dll"
#undef CreateDirectory
#define CreateDirectory(p, n) CreateDirectoryA(p, n)
#else
#ifdef __APPLE__
#define ARCH_SUFFIX
#define OS_LIB_EXT "dylib"
#else
	#if defined(__amd64__) || defined(_M_X64)
	#define ARCH_SUFFIX "_amd64"
	#elif defined(__i386__) || defined(_X86_) || defined(_M_IX86)
	#define ARCH_SUFFIX "_i386"
	#else
	#define ARCH_SUFFIX
	#endif
#define OS_LIB_EXT "so"
#endif
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>
#define CreateDirectory(p, n) mkdir(p, 0777)
#define GetProcAddress dlsym
#define LoadLibrary(x) dlopen(x, RTLD_NOW)
#define Sleep(x) usleep(x * 1000)
typedef void* HINSTANCE;
#define stricmp strcasecmp
#define _stricmp strcasecmp
#define strnicmp strncasecmp
#define _strnicmp strncasecmp
#define FAR
#define WINAPI
#endif

typedef int (FAR *GETENTITYAPI)(DLL_FUNCTIONS *, int);
typedef int (FAR *GETNEWDLLFUNCTIONS)(NEW_DLL_FUNCTIONS *, int *); 

typedef void (*GIVEFNPTRSTODLL)(enginefuncs_t *, globalvars_t *);
typedef void (FAR *LINK_ENTITY_FUNC)(entvars_t *);

// define some function prototypes...
BOOL ClientConnect( edict_t *pEntity, const char *pszName,
                    const char *pszAddress, char szRejectReason[ 128 ] );
void ClientPutInServer( edict_t *pEntity );
void ClientCommand( edict_t *pEntity );

void FakeClientCommand(edict_t *pBot, const char *arg1, const char *arg2, const char *arg3);

const char *Cmd_Args( void );
const char *Cmd_Argv( int argc );
int Cmd_Argc( void );


// more constants...
#define BOT_PITCH_SPEED  20  // degrees per frame for rotation
#define BOT_YAW_SPEED    20  // degrees per frame for rotation

#define RESPAWN_IDLE             1
#define RESPAWN_NEED_TO_RESPAWN  2
#define RESPAWN_IS_RESPAWNING    3

// game start messages for TFC...
#define MSG_TFC_IDLE          1
#define MSG_TFC_TEAM_SELECT   2
#define MSG_TFC_CLASS_SELECT  3

// game start messages for CS...
#define MSG_CS_IDLE         1
#define MSG_CS_TEAM_SELECT  2
#define MSG_CS_CT_SELECT    3
#define MSG_CS_T_SELECT     4

// game start messages for OpFor...
#define MSG_OPFOR_IDLE		1
#define MSG_OPFOR_TEAM_SELECT	2
#define MSG_OPFOR_CLASS_SELECT	3

// define player classes used in TFC
#define TFC_CLASS_CIVILIAN  0
#define TFC_CLASS_SCOUT     1
#define TFC_CLASS_SNIPER    2
#define TFC_CLASS_SOLDIER   3
#define TFC_CLASS_DEMOMAN   4
#define TFC_CLASS_MEDIC     5
#define TFC_CLASS_HWGUY     6
#define TFC_CLASS_PYRO      7
#define TFC_CLASS_SPY       8
#define TFC_CLASS_ENGINEER  9


#define BOT_SKIN_LEN 32
#define BOT_NAME_LEN 32

#define MAX_TEAMS 16


typedef struct
{
   int  iId;     // weapon ID
   int  iClip;   // amount of ammo in the clip
   int  iAmmo1;  // amount of ammo in primary reserve
   int  iAmmo2;  // amount of ammo in secondary reserve
} bot_current_weapon_t;


class CBaseEntity;
class CParabot;


typedef struct
{
	CParabot *parabot;			// pointer to Parabot instance
	int personality;			// index to personality-table

   edict_t *pEdict;				// Pointer to bot-edict
   bool is_used;				// must be true if BotThink should be called
   int respawn_state;			// used to handle respawns after map changes
   bool need_to_initialize;		// used to handle initialization after bot death
   int not_started;				// true while bot still has to chose team and/or class
   int start_action;			// actions before getting started (chose team/class)
   float menuSelectTime;

// TheFatal - START
   int msecnum;
   float msecdel;
   float msecval;
// TheFatal - END				// ...for runPlayerMove-Timing

   char name[BOT_NAME_LEN+1];
   char skin[BOT_SKIN_LEN+1];

   // things from pev in CBasePlayer...
   int bot_team;
   int bot_class;
   int bot_health;
   int bot_armor;
   int bot_weapons;  // bit map of weapons the bot is carrying
   int bot_money;    // for Counter-Strike

   //edict_t *pBotEnemy;

   float f_max_speed;
   float prev_speed;
   Vector v_prev_origin;
   float f_pause_time;

   float f_move_speed;

   bot_current_weapon_t current_weapon;  // one current weapon
   int m_rgAmmo[MAX_AMMO_SLOTS];  // total ammo amounts

} bot_t;

// new UTIL.CPP functions (from HPB-bot)...
void UTIL_SayText( const char *pText, edict_t *pEdict );
int UTIL_GetTeam(edict_t *pEntity);
int UTIL_GetBotIndex(edict_t *pEdict);
bot_t *UTIL_GetBotPointer(edict_t *pEdict);
void UTIL_SelectItem(edict_t *pEdict, char *item_name);
void UTIL_ShowMenu( edict_t *pEdict, int slots, int displaytime, bool needmore, const char *pText );
CBaseEntity	*UTIL_PlayerByIndex( int playerIndex );


#endif // BOT_H

