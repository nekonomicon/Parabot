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

extern int debug_engine;
extern bool speechSynthesis;
//extern ChatList chatGotKilled, chatKilledPlayer, chatGotWeapon, chatReplyUnknown;
extern bot_t bots[32];
extern PB_Configuration pbConfig;
extern PB_Chat chat;

static char g_argv[256];
extern int mod_id;
extern bool g_meta_init;
extern HINSTANCE h_Library;

//#include "hl_game.h"
//extern HL_Game game;

int numberOfClients = 0;
edict_t *playerEnt = 0;
edict_t *clients[32];
DLL_FUNCTIONS gFunctionTable;
DLL_FUNCTIONS other_gFunctionTable;
DLL_GLOBAL const Vector g_vecZero( 0, 0, 0 );
int isFakeClientCommand = 0;
int fake_arg_count;
float bot_check_time = 10.0;	// will be set to correct value when client connects
int min_bots = -1;					
bool g_GameRules = FALSE;		
int welcome_index = -1;	
int wpSpriteTexture, wpBeamTexture, wpSprite2Texture;
int g_hldm_mod = HLDM;
bool gearbox_ctf = false;
char ag_gamemode[8] = {0};
extern Sounds playerSounds;
gamedll_funcs_t gGameDLLFunc;

void StartFrame( void );	// in startframe.cpp
void saveLevelData( void );	// in pb_mapimport.cpp
//void debugFile( char *msg );
void DSaddbot();
void DSsimulate();
void DSlogChat();
void DSrestrictedWeapons();
void DSpeace();

void GameDLLInit( void )
{
	if(mod_id==VALVE_DLL)
	{
		// from jk_botti
		if(CVAR_GET_POINTER("bm_ver") )
			g_hldm_mod = BMOD;
		else if( CVAR_GET_POINTER("mp_giveweapons") && CVAR_GET_POINTER("mp_giveammo") )
			g_hldm_mod = SEVS;
	}
	(*g_engfuncs.pfnAddServerCommand)("addbot", DSaddbot );
	(*g_engfuncs.pfnAddServerCommand)("hidewelcome", DSsimulate );
	(*g_engfuncs.pfnAddServerCommand)("chatlog", DSlogChat );
	(*g_engfuncs.pfnAddServerCommand)("restrictedweapons", DSrestrictedWeapons );
	(*g_engfuncs.pfnAddServerCommand)("peacemode", DSpeace );

   for (int i=0; i<32; i++)
      clients[i] = NULL;
   
   // initialize the bots array of structures...
   memset(bots, 0, sizeof(bots));
	if(!g_meta_init)
		(*other_gFunctionTable.pfnGameInit)();

	RETURN_META(MRES_IGNORED);
}

int DispatchSpawn( edict_t *pent )
{
   if (gpGlobals->deathmatch)
   {
      char *pClassname = (char *)STRING(pent->v.classname);
#ifdef _DEBUG
      if (debug_engine) {
         fp = UTIL_OpenDebugLog();
         fprintf(fp, "%f: DispatchSpawn: %s\n",worldTime(), pClassname );
         if (pent->v.model != 0)
            fprintf(fp, " model=%s\n",STRING(pent->v.model));
         fclose(fp);
      }
#endif
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
		gearbox_ctf = false;

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
         g_GameRules = TRUE;
      }
//	  else if (FStrEq( pClassname, "env_sound" ) ) debugMsg( "DISPATCH env_sound\n" );
//	  else if (FStrEq( pClassname, "env_shake" ) ) debugMsg( "DISPATCH env_shake\n" );
//	  else if (FStrEq( pClassname, "env_explosion" ) ) debugMsg( "DISPATCH env_explosion\n" );
   }
	if(!g_meta_init)
		return (*other_gFunctionTable.pfnSpawn)(pent);

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

void DispatchKeyValue( edict_t *pentKeyvalue, KeyValueData *pkvd )
{
	if( mod_id == AG_DLL )
	{
		strcpy( ag_gamemode, CVAR_GET_STRING( "sv_ag_gamemode" ) );
	}
	else if( mod_id == GEARBOX_DLL )
	{
		if( !gearbox_ctf )
		{
			if( ( FStrEq( pkvd->szKeyName, "classname" ) ) &&
			( FStrEq(pkvd->szValue, "info_ctfdetect") ) )
			{
				gearbox_ctf = true;
			}
		}
	}

	if( !g_meta_init )
		(*other_gFunctionTable.pfnKeyValue)(pentKeyvalue, pkvd);

	RETURN_META(MRES_IGNORED);
}

///////////////////////////////////////////////////////////////////////////////////
//
//  CLIENT HANDLING
//
///////////////////////////////////////////////////////////////////////////////////
BOOL ClientConnect( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ]  )
{ 
	bool connected;
#ifdef _DEBUG
	char buffer[256];
	sprintf( buffer, "%.f: ClientConnect: %s (%s)", worldTime(), STRING(pEntity->v.netname), pszName );
	debugFile( buffer );
#endif
	if (gpGlobals->deathmatch)
	{
#ifdef _DEBUG
		if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp, "ClientConnect: pent=%p name=%s\n",pEntity,pszName); fclose(fp); }
#endif
		// check if this client is the listen server client
		if (FStrEq(pszAddress, "loopback" ) )
		{
			// save the edict of the listen server client...
			playerEnt = pEntity;
		}
		// check if this is NOT a bot joining the server...
		if ( !FStrEq( pszAddress, "127.0.0.1" ) ) {
			// don't try to add bots for 10 seconds, give client time to get added
			if (bot_check_time < gpGlobals->time + 10.0) bot_check_time = gpGlobals->time + 10.0;
		}
	}

	connected = MDLL_ClientConnect(pEntity, pszName, pszAddress, szRejectReason);

	debugFile( "  OK\n" );
	if(!g_meta_init)
		return connected;

	RETURN_META_VALUE(MRES_SUPERCEDE, connected);
}


void ClientDisconnect( edict_t *pEntity )
{
	int i, index = -1;
#ifdef _DEBUG
	char buffer[256];
	sprintf( buffer, "%.f: ClientDisconnect: %s ", worldTime(), STRING(pEntity->v.netname) );
#endif
	if (gpGlobals->deathmatch) {
#ifdef _DEBUG
		if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp, "ClientDisconnect: %p\n",pEntity); fclose(fp); }
#endif
		i = 0;
		while ((i < 32) && (clients[i] != pEntity))	i++;
		if (i < 32)	clients[i] = NULL;
		
		for (i=0; i < 32; i++) {
			if (bots[i].pEdict == pEntity) {
				index = i;
				break;
			}
		}
		
		if (index != -1) {	// bot is disconnecting
#ifdef _DEBUG
			debugMsg( "BOT DISCONNECT.\n" );
			strcat( buffer, "...freeing bot" );
#endif
			bots[index].is_used = FALSE;  // this slot is now free to use
			bots[index].pEdict = 0;
			pbConfig.personalityLeaves( bots[index].personality, worldTime() );
			delete (bots[index].parabot);	bots[index].parabot = 0;			
		}
	}
#ifdef _DEBUG
	debugFile( buffer );
#endif
	numberOfClients--;

	MDLL_ClientDisconnect(pEntity);

	//if (index != -1) FREE_PRIVATE( pEntity );	// fakeclient fix by Leon Hartwig
	debugFile( "  OK\n" );
	if(g_meta_init)
		RETURN_META(MRES_SUPERCEDE);
}

void ClientPutInServer( edict_t *pEntity )
{
#ifdef _DEBUG
	if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp, "ClientPutInServer: %p\n",pEntity); fclose(fp); }
#endif
	int index = 0;
	while ((index < 32) && (clients[index] != NULL)) index++;
	if (index < 32) clients[index] = pEntity;
	/*else {
		FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
		fprintf( dfp, "32 clients in ClientPutInServer()!\n" ); 
		fclose( dfp );
	}*/
	// check if this is NOT a bot joining the server...
	if (UTIL_GetBotIndex( pEntity ) == -1) {
		// next welcome message to this client:
		if (welcome_index == -1) welcome_index = index;
	}
	
	numberOfClients++;
	if(!g_meta_init)
		(*other_gFunctionTable.pfnClientPutInServer)(pEntity);
	else
		RETURN_META(MRES_IGNORED);
}

void ServerDeactivate( void )
{
#ifdef _DEBUG
	if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp, "ServerDeactivate\n"); fclose(fp); }
#endif
	if(!g_meta_init)
		(*other_gFunctionTable.pfnServerDeactivate)();

	saveLevelData();		// save last level's data
	if(g_meta_init)
		RETURN_META(MRES_IGNORED);
}


extern "C" EXPORT int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion )
{
	// check if engine's pointer is valid and version is correct...
	if( !pFunctionTable || interfaceVersion != INTERFACE_VERSION )
		return FALSE;

	memset( pFunctionTable, 0, sizeof( DLL_FUNCTIONS ) );

	if(!g_meta_init)
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

	if( !g_meta_init )
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
	if(!g_meta_init)
		return &g_argv[0];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[0]);
   }
   else
   {
	if(!g_meta_init)
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
	if(!g_meta_init)
		return &g_argv[64];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[64]);
      }
      else if (argc == 1)
      {
	if(!g_meta_init)
		return &g_argv[128];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[128]);
      }
      else if (argc == 2)
      {
	if(!g_meta_init)
		return &g_argv[192];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[192]);
      }
      else
      {
	if(!g_meta_init)
		return "???";

	RETURN_META_VALUE(MRES_SUPERCEDE, "???");
      }
   }
   else
   {
		const char *pargv = (*g_engfuncs.pfnCmd_Argv)(argc);
		if(!g_meta_init)
			return pargv;

		RETURN_META_VALUE(MRES_SUPERCEDE, pargv);
   }
}

int Cmd_Argc()
{
   if (isFakeClientCommand)
   {
	if(!g_meta_init)
		return fake_arg_count;

	RETURN_META_VALUE(MRES_SUPERCEDE, fake_arg_count);
   }
   else
   {
	if(!g_meta_init)
		return (*g_engfuncs.pfnCmd_Argc)();

	RETURN_META_VALUE(MRES_IGNORED, 0);
   }
}
