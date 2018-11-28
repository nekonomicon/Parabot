//
// HPB_bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// bot.h
//
#pragma once
#if !defined(BOT_H)
#define BOT_H

#include "sdk_common.h"
#include "pb_global.h"

typedef int (*GETENTITYAPI)(SERVERFUNCS *, int);
typedef int (*GETNEWDLLFUNCTIONS)(SERVERFUNCS2 *, int *); 

typedef void (WINAPI *GIVEFNPTRSTODLL)(ENGINEAPI *, GLOBALENTVARS *);
typedef void (*LINK_ENTITY_FUNC)(ENTVARS *);
typedef int (*SERVER_GETBLENDINGINTERFACE) (int, struct sv_blending_interface **, struct engine_studio_api *, void *, void *);
typedef int (*SERVER_GETPHYSICSINTERFACE) (int, void *, PHYSICS_INTERFACE *);

// define some function prototypes...

void FakeClientCommand(EDICT *pBot, const char *arg1, const char *arg2, const char *arg3);

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


class CParabot;


typedef struct
{
	CParabot *parabot;			// pointer to Parabot instance
	int personality;			// index to personality-table

   EDICT *e;				// Pointer to bot-edict
   bool is_used;				// must be true if BotThink should be called
   int respawn_state;			// used to handle respawns after map changes
   bool need_to_initialize;		// used to handle initialization after bot death
   int not_started;				// true while bot still has to chose team and/or class
   int start_action;			// actions before getting started (chose team/class)
   float menuSelectTime;

   char name[BOT_NAME_LEN+1];
   char skin[BOT_SKIN_LEN+1];

   // things from pev in CBasePlayer...
   int bot_team;
   int bot_class;
   int bot_money;    // for Counter-Strike

   //EDICT *pBotEnemy;

   float prev_speed;
   Vec3D v_prev_origin;
   float f_pause_time;

   float f_move_speed;

   bot_current_weapon_t current_weapon;  // one current weapon
   int m_rgAmmo[32];  // total ammo amounts

} bot_t;

int      getbotindex(EDICT *e);
bot_t   *getbotpointer(EDICT *e);
#endif // BOT_H

