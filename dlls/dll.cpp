// Based on:
//
// HPB bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// dll.cpp
//

#include "extdll.h"
#include "enginecallback.h"
#include "dllapi.h"
#include "meta_api.h"
#include "bot.h"
#include "bot_func.h"
#include "pb_global.h"
#include "parabot.h"
#include "pb_chat.h"
#include "pb_mapcells.h"
#include "pb_configuration.h"
#include "sounds.h"
#include "studio.h"

extern bool speechSynthesis;
//extern ChatList chatGotKilled, chatKilledPlayer, chatGotWeapon, chatReplyUnknown;
extern bot_t bots[32];
extern PB_Configuration pbConfig;
extern PB_Chat chat;

static char g_argv[256];
extern int mod_id;
extern HINSTANCE h_Library;

//#include "hl_game.h"
//extern HL_Game game;

int numberOfClients;
DLL_FUNCTIONS gFunctionTable;
DLL_FUNCTIONS other_gFunctionTable;
DLL_GLOBAL const Vector g_vecZero;
int isFakeClientCommand;
int fake_arg_count;
float bot_check_time = 10.0;	// will be set to correct value when client connects
int min_bots = -1;					
int welcome_index = -1;	
int wpSpriteTexture, wpBeamTexture, wpSprite2Texture;

const cvar_t *flashlight;
const cvar_t *footsteps;
const cvar_t *freezetime;
const cvar_t *gamemode;
const cvar_t *teamplay;
const cvar_t *maxspeed;

const cvar_t *bm_cbar;
const cvar_t *bm_gluon;
const cvar_t *bm_trip;

extern Sounds playerSounds;
gamedll_funcs_t gGameDLLFunc;

void StartFrame();	// in startframe.cpp
void saveLevelData();	// in pb_mapimport.cpp
void DSaddbot();

void GameDLLInit()
{
	flashlight = CVAR_GET_POINTER( "mp_flashlight" );
	footsteps = CVAR_GET_POINTER( "mp_footsteps" );
	freezetime = CVAR_GET_POINTER( "mp_freezetime" );
	maxspeed = CVAR_GET_POINTER( "sv_maxspeed" );
	teamplay = CVAR_GET_POINTER( "mp_teamplay" );

	if( !teamplay )
	{
		teamplay = CVAR_GET_POINTER( "mp_gameplay" ); // Half-Screwed

		if( !teamplay )
			teamplay = CVAR_GET_POINTER( "mp_gametype" ); // Cold Ice
	}

	switch( mod_id )
	{
	case VALVE_DLL:
		// from jk_botti
		if(CVAR_GET_POINTER( "bm_ver" ) )
		{
			SetBits( g_uiGameFlags, GAME_BMOD );
			bm_cbar = CVAR_GET_POINTER( "bm_cbar_mod" );
			bm_gluon = CVAR_GET_POINTER( "bm_gluon_mod" );
			bm_trip = CVAR_GET_POINTER( "bm_trip_mod" );
		}
		else if( CVAR_GET_POINTER( "mp_giveweapons" ) && CVAR_GET_POINTER("mp_giveammo") )
			SetBits( g_uiGameFlags, GAME_SEVS );
		break;
	case AG_DLL:
		gamemode = CVAR_GET_POINTER( "sv_ag_gamemode" );
		break;
	}

	(*g_engfuncs.pfnAddServerCommand)("add_bot", DSaddbot );

	if( FBitSet( g_uiGameFlags, GAME_METAMOD ) )
		 RETURN_META( MRES_IGNORED );

	(*other_gFunctionTable.pfnGameInit)();
}

int DispatchSpawn( edict_t *pent )
{
   if (gpGlobals->deathmatch)
   {
	const char *pClassname = STRING(pent->v.classname);

         debugFile( "%f: DispatchSpawn: %s\n",worldTime(), pClassname );

         if (pent->v.model != 0)
            debugFile(" model=%s\n",STRING(pent->v.model));

      if( FStrEq( pClassname, "worldspawn" ) )
      {
         // do level initialization stuff here...
/*
		  if (speechSynthesis) {
			  // precache samples
				int i;
				for (i=0; i<chatGotKilled.size(); i++ ) 
					PRECACHE_SOUND( chatGotKilled[i].text );
				for (i=0; i<chatKilledPlayer.size(); i++ ) 
					PRECACHE_SOUND( chatKilledPlayer[i].text );
				for (i=0; i<chatGotWeapon.size(); i++ ) 
					PRECACHE_SOUND( chatGotWeapon[i].text );
				for (i=0; i<chatReplyUnknown.size(); i++ ) 
					PRECACHE_SOUND( chatReplyUnknown[i].text );
		  }
*/

	ClearBits( g_uiGameFlags, GAME_TEAMPLAY | GAME_DOM | GAME_CTF );

	if( mod_id == AG_DLL )
	{
		if( teamplay->value )
		{
			SetBits( g_uiGameFlags, GAME_TEAMPLAY );
		}

		if( FStrEq( gamemode->string, "ctf" ) )
		{
			SetBits( g_uiGameFlags, GAME_CTF );
		}
		else if( FStrEq( gamemode->string, "dom" ) )
		{
			SetBits( g_uiGameFlags, GAME_DOM );
		}
	}

           PRECACHE_SOUND("weapons/xbow_hit1.wav");      // waypoint add
           PRECACHE_SOUND("weapons/mine_activate.wav");  // waypoint delete
           PRECACHE_SOUND("common/wpn_hudoff.wav");      // path add/delete start
           PRECACHE_SOUND("common/wpn_hudon.wav");       // path add/delete done
           PRECACHE_SOUND("common/wpn_moveselect.wav");  // path add/delete cancel
           PRECACHE_SOUND("common/wpn_denyselect.wav");  // path add/delete error
           wpBeamTexture = PRECACHE_MODEL( "sprites/lgtning.spr");
		   wpSpriteTexture = PRECACHE_MODEL( "sprites/hotglow.spr");
		   wpSprite2Texture = PRECACHE_MODEL( "sprites/laserdot.spr");

	playerSounds.init();
      }
//	  else if (FStrEq( pClassname, "env_sound" ) ) debugMsg( "DISPATCH env_sound\n" );
//	  else if (FStrEq( pClassname, "env_shake" ) ) debugMsg( "DISPATCH env_shake\n" );
//	  else if (FStrEq( pClassname, "env_explosion" ) ) debugMsg( "DISPATCH env_explosion\n" );
   }
	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return (*other_gFunctionTable.pfnSpawn)(pent);

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

void DispatchKeyValue( edict_t *pentKeyvalue, KeyValueData *pkvd )
{
	if( mod_id == GEARBOX_DLL )
	{
		if( !FBitSet( g_uiGameFlags, GAME_CTF ) )
		{
			if( ( FStrEq( pkvd->szKeyName, "classname" ) ) &&
			( FStrEq(pkvd->szValue, "info_ctfdetect") ) )
			{
				SetBits( g_uiGameFlags, GAME_CTF );
			}
		}
	}

	if( FBitSet( g_uiGameFlags, GAME_METAMOD ) )
		RETURN_META(MRES_IGNORED);

	(*other_gFunctionTable.pfnKeyValue)(pentKeyvalue, pkvd);
}

///////////////////////////////////////////////////////////////////////////////////
//
//  CLIENT HANDLING
//
///////////////////////////////////////////////////////////////////////////////////
BOOL ClientConnect( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ]  )
{
	bool connected;

	debugFile( "%.f: ClientConnect: %s (%s)", worldTime(), STRING(pEntity->v.netname), pszName );

	if (gpGlobals->deathmatch)
	{
		debugFile( "ClientConnect: pent=%p name=%s\n", pEntity, pszName );

		// check if this is NOT a bot joining the server...
		if ( !FBitSet( pEntity->v.flags, FL_FAKECLIENT ) ) {
			// don't try to add bots for 10 seconds, give client time to get added
			if (bot_check_time < gpGlobals->time + 10.0) bot_check_time = gpGlobals->time + 10.0;
		}
	}

	connected = MDLL_ClientConnect(pEntity, pszName, pszAddress, szRejectReason);

	debugFile( "  OK\n" );
	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return connected;

	RETURN_META_VALUE(MRES_SUPERCEDE, connected);
}

void ClientDisconnect( edict_t *pEntity )
{
	int i, index = -1;

	debugFile( "%.f: ClientDisconnect: %s ", worldTime(), STRING(pEntity->v.netname) );

	if (gpGlobals->deathmatch) {
		debugFile( "ClientDisconnect: %p\n", pEntity );

		for (i=0; i < 32; i++) {
			if (bots[i].pEdict == pEntity) {
				index = i;
				break;
			}
		}
		
		if (index != -1) {	// bot is disconnecting
			debugMsg( "BOT DISCONNECT.\n" );

			bots[index].is_used = FALSE;  // this slot is now free to use
			bots[index].pEdict = 0;
			pbConfig.personalityLeaves( bots[index].personality );
			delete (bots[index].parabot);	bots[index].parabot = 0;			
		}
	}

	debugFile( "...freeing bot" );

	numberOfClients--;

	MDLL_ClientDisconnect(pEntity);

	//if (index != -1) FREE_PRIVATE( pEntity );	// fakeclient fix by Leon Hartwig
	debugFile( "  OK\n" );
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_SUPERCEDE);
}

void ClientPutInServer( edict_t *pEntity )
{
	debugFile( "ClientPutInServer: %p\n",pEntity);

	// check if this is NOT a bot joining the server...
	if (UTIL_GetBotIndex( pEntity ) == -1) {
		// next welcome message to this client:
		if (welcome_index == -1) welcome_index = ENTINDEX(pEntity);
	}
	
	numberOfClients++;
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*other_gFunctionTable.pfnClientPutInServer)(pEntity);
}

void ServerDeactivate()
{
	debugFile( "ServerDeactivate\n" );

	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		(*other_gFunctionTable.pfnServerDeactivate)();

	saveLevelData();		// save last level's data

	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);
}

extern "C" EXPORT int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion )
{
	// check if engine's pointer is valid and version is correct...
	if( !pFunctionTable || interfaceVersion != INTERFACE_VERSION )
		return FALSE;

	memset( pFunctionTable, 0, sizeof( DLL_FUNCTIONS ) );

	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
	{
		// pass other DLLs engine callbacks to function table
		if( !(*(GETENTITYAPI)GetProcAddress( h_Library, "GetEntityAPI" )) (&other_gFunctionTable, INTERFACE_VERSION) )
			return FALSE;  // error initializing function table!!!

		gGameDLLFunc.dllapi_table = &other_gFunctionTable;
		gpGamedllFuncs = &gGameDLLFunc;
		memcpy( pFunctionTable, &other_gFunctionTable, sizeof( DLL_FUNCTIONS ) );
	}

	pFunctionTable->pfnGameInit = GameDLLInit;
	pFunctionTable->pfnSpawn = DispatchSpawn;
	pFunctionTable->pfnKeyValue = DispatchKeyValue;
	pFunctionTable->pfnClientConnect = ClientConnect;
	pFunctionTable->pfnClientDisconnect = ClientDisconnect;
	pFunctionTable->pfnClientPutInServer = ClientPutInServer;
	pFunctionTable->pfnServerDeactivate = ServerDeactivate;
	pFunctionTable->pfnStartFrame = StartFrame;
	pFunctionTable->pfnClientCommand = ClientCommand;

	return TRUE;
}

extern "C" EXPORT int GetNewDLLFunctions( NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion ) 
{ 
	static GETNEWDLLFUNCTIONS other_GetNewDLLFunctions = NULL;
	static bool missing = FALSE;

	if( !FBitSet( g_uiGameFlags, GAME_METAMOD ) )
		return TRUE;

	// if the new DLL functions interface has been formerly reported as missing, give up
	if( missing )
		return FALSE;

	// do we NOT know if the new DLL functions interface is provided ? if so, look for its address
	if( other_GetNewDLLFunctions == NULL )
		other_GetNewDLLFunctions = (GETNEWDLLFUNCTIONS) GetProcAddress( h_Library, "GetNewDLLFunctions" );

	// have we NOT found it ?
	if( other_GetNewDLLFunctions == NULL )
	{
		missing = TRUE; // then mark it as missing, no use to look for it again in the future
		return FALSE; // and give up
	}

	gGameDLLFunc.newapi_table = pFunctionTable;

	// else call the function that provides the new DLL functions interface on request
	return ( !(*other_GetNewDLLFunctions)( pFunctionTable, interfaceVersion ) );
}

void FakeClientCommand(edict_t *pBot, const char *arg1, const char *arg2, const char *arg3)
{
   int length;

   isFakeClientCommand = 1;
   memset( g_argv, 0, sizeof(g_argv) );

   if ((arg1 == NULL) || (*arg1 == 0)) return;

   if ((arg2 == NULL) || (*arg2 == 0))
   {
      length = sprintf(&g_argv[0], "%s", arg1);
      fake_arg_count = 1;
   }
   else if ((arg3 == NULL) || (*arg3 == 0))
   {
      length = sprintf(&g_argv[0], "%s %s", arg1, arg2);
      fake_arg_count = 2;
   }
   else
   {
      length = sprintf(&g_argv[0], "%s %s %s", arg1, arg2, arg3);
      fake_arg_count = 3;
   }

   g_argv[length] = 0;  // null terminate just in case

   strcpy(&g_argv[64], arg1);

   if (arg2)
      strcpy(&g_argv[128], arg2);

   if (arg3)
      strcpy(&g_argv[192], arg3);

	// allow the MOD DLL to execute the ClientCommand...
	MDLL_ClientCommand(pBot);

   isFakeClientCommand = 0;
}

const char *Cmd_Args()
{
   if (isFakeClientCommand)
   {
	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return &g_argv[0];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[0]);
   }
   else
   {
	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return (*g_engfuncs.pfnCmd_Args)();

	RETURN_META_VALUE(MRES_IGNORED, 0);
   }
}

const char *Cmd_Argv( int argc )
{
   if (isFakeClientCommand)
   {
      if (argc == 0)
      {
	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return &g_argv[64];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[64]);
      }
      else if (argc == 1)
      {
	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return &g_argv[128];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[128]);
      }
      else if (argc == 2)
      {
	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return &g_argv[192];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[192]);
      }
      else
      {
	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return "???";

	RETURN_META_VALUE(MRES_SUPERCEDE, "???");
      }
   }
   else
   {
		const char *pargv = (*g_engfuncs.pfnCmd_Argv)(argc);
		if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
			return pargv;

		RETURN_META_VALUE(MRES_SUPERCEDE, pargv);
   }
}

int Cmd_Argc()
{
   if (isFakeClientCommand)
   {
	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return fake_arg_count;

	RETURN_META_VALUE(MRES_SUPERCEDE, fake_arg_count);
   }
   else
   {
	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return (*g_engfuncs.pfnCmd_Argc)();

	RETURN_META_VALUE(MRES_IGNORED, 0);
   }
}
