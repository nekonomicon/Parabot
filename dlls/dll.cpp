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


extern int debug_engine;
extern bool speechSynthesis;
//extern ChatList chatGotKilled, chatKilledPlayer, chatGotWeapon, chatReplyUnknown;
extern bot_t bots[32];
extern PB_Configuration pbConfig;
extern PB_Chat chat;
extern GETENTITYAPI other_GetEntityAPI;
extern GETNEWDLLFUNCTIONS other_GetNewDLLFunctions; 
static char g_argv[256];
extern int mod_id;
extern bool g_meta_init;

//#include "hl_game.h"
//extern HL_Game game;

int numberOfClients = 0;
edict_t *playerEnt = 0;
edict_t *clients[32];
DLL_FUNCTIONS other_gFunctionTable;
DLL_GLOBAL const Vector g_vecZero = Vector(0,0,0);
int isFakeClientCommand = 0;
int fake_arg_count;
float bot_check_time = 10.0;	// will be set to correct value when client connects
int min_bots = -1;					
bool g_GameRules = FALSE;		
int welcome_index = -1;	
int wpSpriteTexture, wpBeamTexture, wpSprite2Texture;
int g_hldm_mod = HLDM;
bool gearbox_ctf = false;
char ag_gamemode[8] = {};

static FILE *fp;


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
	else
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
      if( FstrEq( pClassname, "worldspawn" ) )
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

         g_GameRules = TRUE;
      }
//	  else if (FStrEq( pClassname, "env_sound" ) ) debugMsg( "DISPATCH env_sound\n" );
//	  else if (FStrEq( pClassname, "env_shake" ) ) debugMsg( "DISPATCH env_shake\n" );
//	  else if (FStrEq( pClassname, "env_explosion" ) ) debugMsg( "DISPATCH env_explosion\n" );
   }
	if(!g_meta_init)
		return (*other_gFunctionTable.pfnSpawn)(pent);
	else
		RETURN_META_VALUE(MRES_IGNORED, 0);
}

void DispatchThink( edict_t *pent )
{
   (*other_gFunctionTable.pfnThink)(pent);
}

void DispatchUse( edict_t *pentUsed, edict_t *pentOther )
{
   (*other_gFunctionTable.pfnUse)(pentUsed, pentOther);
}

void DispatchTouch( edict_t *pentTouched, edict_t *pentOther )
{
   (*other_gFunctionTable.pfnTouch)(pentTouched, pentOther);
}

void DispatchBlocked( edict_t *pentBlocked, edict_t *pentOther )
{
   (*other_gFunctionTable.pfnBlocked)(pentBlocked, pentOther);
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
	else
		RETURN_META(MRES_IGNORED);
}

void DispatchSave( edict_t *pent, SAVERESTOREDATA *pSaveData )
{
   (*other_gFunctionTable.pfnSave)(pent, pSaveData);
}

int DispatchRestore( edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity )
{
   return (*other_gFunctionTable.pfnRestore)(pent, pSaveData, globalEntity);
}

void DispatchObjectCollsionBox( edict_t *pent )
{
   (*other_gFunctionTable.pfnSetAbsBox)(pent);
}

void SaveWriteFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount )
{
   (*other_gFunctionTable.pfnSaveWriteFields)(pSaveData, pname, pBaseData, pFields, fieldCount);
}

void SaveReadFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount )
{
   (*other_gFunctionTable.pfnSaveReadFields)(pSaveData, pname, pBaseData, pFields, fieldCount);
}

void SaveGlobalState( SAVERESTOREDATA *pSaveData )
{
   (*other_gFunctionTable.pfnSaveGlobalState)(pSaveData);
}

void RestoreGlobalState( SAVERESTOREDATA *pSaveData )
{
   (*other_gFunctionTable.pfnRestoreGlobalState)(pSaveData);
}

void ResetGlobalState( void )
{
   (*other_gFunctionTable.pfnResetGlobalState)();
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
		if (FstrEq(pszAddress, "loopback" ) )
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
	if(!g_meta_init)
		connected = (*other_gFunctionTable.pfnClientConnect)(pEntity, pszName, pszAddress, szRejectReason);
	else
		connected = MDLL_ClientConnect(pEntity, pszName, pszAddress, szRejectReason);

	debugFile( "  OK\n" );
	if(!g_meta_init)
		return connected;
	else
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
	if(!g_meta_init)
		(*other_gFunctionTable.pfnClientDisconnect)(pEntity);
	else
		MDLL_ClientDisconnect(pEntity);

	//if (index != -1) FREE_PRIVATE( pEntity );	// fakeclient fix by Leon Hartwig
	debugFile( "  OK\n" );
	if(g_meta_init)
		RETURN_META(MRES_SUPERCEDE);
}

void ClientKill( edict_t *pEntity )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp, "ClientKill: %p\n",pEntity); fclose(fp); }
#endif
   (*other_gFunctionTable.pfnClientKill)(pEntity);
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
	
	EHANDLE np;
	np.Set( pEntity );
	//HL_World *hlw = (HL_World*)game.world();
	//hlw->addPlayer( np );
	numberOfClients++;
	if(!g_meta_init)
		(*other_gFunctionTable.pfnClientPutInServer)(pEntity);
	else
		RETURN_META(MRES_IGNORED);
}

void ClientUserInfoChanged( edict_t *pEntity, char *infobuffer )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp, "ClientUserInfoChanged: pEntity=%p infobuffer=%s\n", pEntity, infobuffer); fclose(fp); }
#endif
   (*other_gFunctionTable.pfnClientUserInfoChanged)(pEntity, infobuffer);
}


void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
   (*other_gFunctionTable.pfnServerActivate)(pEdictList, edictCount, clientMax);
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


void PlayerPreThink( edict_t *pEntity )
{
   (*other_gFunctionTable.pfnPlayerPreThink)(pEntity);
}


void PlayerPostThink( edict_t *pEntity )
{
   (*other_gFunctionTable.pfnPlayerPostThink)(pEntity);
}


void ParmsNewLevel( void )
{
   (*other_gFunctionTable.pfnParmsNewLevel)();
}


void ParmsChangeLevel( void )
{
   (*other_gFunctionTable.pfnParmsChangeLevel)();
}


const char *GetGameDescription( void )
{
   return (*other_gFunctionTable.pfnGetGameDescription)();
}


void PlayerCustomization( edict_t *pEntity, customization_t *pCust )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp, "PlayerCustomization: %p\n",pEntity); fclose(fp); }
#endif
   (*other_gFunctionTable.pfnPlayerCustomization)(pEntity, pCust);
}


void SpectatorConnect( edict_t *pEntity )
{
   (*other_gFunctionTable.pfnSpectatorConnect)(pEntity);
}


void SpectatorDisconnect( edict_t *pEntity )
{
   (*other_gFunctionTable.pfnSpectatorDisconnect)(pEntity);
}


void SpectatorThink( edict_t *pEntity )
{
   (*other_gFunctionTable.pfnSpectatorThink)(pEntity);
}


void Sys_Error( const char *error_string )
{
#ifdef _DEBUG
	if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp, "Sys_Error: %s\n", error_string); fclose(fp); }
#endif
	(*other_gFunctionTable.pfnSys_Error)(error_string);
}

void PM_Move ( struct playermove_s *ppmove, int server )
{
	(*other_gFunctionTable.pfnPM_Move)(ppmove, server);
}

void PM_Init ( struct playermove_s *ppmove )
{
   (*other_gFunctionTable.pfnPM_Init)(ppmove);
}


char PM_FindTextureType( char *name )
{
   return (*other_gFunctionTable.pfnPM_FindTextureType)(name);
}


void SetupVisibility( edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas )
{
   (*other_gFunctionTable.pfnSetupVisibility)(pViewEntity, pClient, pvs, pas);
}


void UpdateClientData ( const struct edict_s *ent, int sendweapons, struct clientdata_s *cd )
{
   (*other_gFunctionTable.pfnUpdateClientData)(ent, sendweapons, cd);
}


int AddToFullPack( struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet )
{
   return (*other_gFunctionTable.pfnAddToFullPack)(state, e, ent, host, hostflags, player, pSet);
}


void CreateBaseline( int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs )
{
   (*other_gFunctionTable.pfnCreateBaseline)(player, eindex, baseline, entity, playermodelindex, player_mins, player_maxs);
}


void RegisterEncoders( void )
{
   (*other_gFunctionTable.pfnRegisterEncoders)();
}


int GetWeaponData( struct edict_s *player, struct weapon_data_s *info )
{
   return (*other_gFunctionTable.pfnGetWeaponData)(player, info);
}


void CmdStart( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed )
{
#ifdef _DEBUG
	/*if (debug_engine) { 
		fp = UTIL_OpenDebugLog(); 
		fprintf(fp, "CmdStart: ed=%p, lms=%i, msec=%i, bts=%i, imp=%i, wps=%i\n",
			player, cmd->lerp_msec, cmd->msec, cmd->buttons, cmd->impulse, cmd->weaponselect ); 
		fclose(fp); 
	}*/
#endif
/*	short	lerp_msec;      // Interpolation time on client
	byte	msec;           // Duration in ms of command
	vec3_t	viewangles;     // Command view angles.

// intended velocities
	float	forwardmove;    // Forward velocity.
	float	sidemove;       // Sideways velocity.
	float	upmove;         // Upward velocity.
	byte	lightlevel;     // Light level at spot where we are standing.
	unsigned short  buttons;  // Attack buttons
	byte    impulse;          // Impulse command issued.
	byte	weaponselect;	// Current weapon id
*/
   (*other_gFunctionTable.pfnCmdStart)(player, cmd, random_seed);
}


void CmdEnd ( const edict_t *player )
{
#ifdef _DEBUG
	/*if (debug_engine) { 
		fp = UTIL_OpenDebugLog(); 
		fprintf(fp, "CmdEnd: ed=%p\n", player ); 
		fclose(fp); 
	}*/
#endif
	//debugMsg( "CmdEnd\n" );
   (*other_gFunctionTable.pfnCmdEnd)(player);
}


int ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
   return (*other_gFunctionTable.pfnConnectionlessPacket)(net_from, args, response_buffer, response_buffer_size);
}


int GetHullBounds( int hullnumber, float *mins, float *maxs )
{
   return (*other_gFunctionTable.pfnGetHullBounds)(hullnumber, mins, maxs);
}


void CreateInstancedBaselines( void )
{
   (*other_gFunctionTable.pfnCreateInstancedBaselines)();
}


int InconsistentFile( const edict_t *player, const char *filename, char *disconnect_message )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp, "InconsistentFile: %p filename=%s\n",player,filename); fclose(fp); }
#endif
   return (*other_gFunctionTable.pfnInconsistentFile)(player, filename, disconnect_message);
}


int AllowLagCompensation( void )
{
   return (*other_gFunctionTable.pfnAllowLagCompensation)();
}


DLL_FUNCTIONS gFunctionTable =
{
   GameDLLInit,               //pfnGameInit
   DispatchSpawn,             //pfnSpawn
   DispatchThink,             //pfnThink
   DispatchUse,               //pfnUse
   DispatchTouch,             //pfnTouch
   DispatchBlocked,           //pfnBlocked
   DispatchKeyValue,          //pfnKeyValue
   DispatchSave,              //pfnSave
   DispatchRestore,           //pfnRestore
   DispatchObjectCollsionBox, //pfnAbsBox

   SaveWriteFields,           //pfnSaveWriteFields
   SaveReadFields,            //pfnSaveReadFields

   SaveGlobalState,           //pfnSaveGlobalState
   RestoreGlobalState,        //pfnRestoreGlobalState
   ResetGlobalState,          //pfnResetGlobalState

   ClientConnect,             //pfnClientConnect
   ClientDisconnect,          //pfnClientDisconnect
   ClientKill,                //pfnClientKill
   ClientPutInServer,         //pfnClientPutInServer
   ClientCommand,             //pfnClientCommand
   ClientUserInfoChanged,     //pfnClientUserInfoChanged
   ServerActivate,            //pfnServerActivate
   ServerDeactivate,          //pfnServerDeactivate

   PlayerPreThink,            //pfnPlayerPreThink
   PlayerPostThink,           //pfnPlayerPostThink

   StartFrame,                //pfnStartFrame
   ParmsNewLevel,             //pfnParmsNewLevel
   ParmsChangeLevel,          //pfnParmsChangeLevel

   GetGameDescription,        //pfnGetGameDescription    Returns string describing current .dll game.
   PlayerCustomization,       //pfnPlayerCustomization   Notifies .dll of new customization for player.

   SpectatorConnect,          //pfnSpectatorConnect      Called when spectator joins server
   SpectatorDisconnect,       //pfnSpectatorDisconnect   Called when spectator leaves the server
   SpectatorThink,            //pfnSpectatorThink        Called when spectator sends a command packet (usercmd_t)

   Sys_Error,                 //pfnSys_Error          Called when engine has encountered an error

   PM_Move,                   //pfnPM_Move
   PM_Init,                   //pfnPM_Init            Server version of player movement initialization
   PM_FindTextureType,        //pfnPM_FindTextureType

   SetupVisibility,           //pfnSetupVisibility        Set up PVS and PAS for networking for this client
   UpdateClientData,          //pfnUpdateClientData       Set up data sent only to specific client
   AddToFullPack,             //pfnAddToFullPack
   CreateBaseline,            //pfnCreateBaseline        Tweak entity baseline for network encoding, allows setup of player baselines, too.
   RegisterEncoders,          //pfnRegisterEncoders      Callbacks for network encoding
   GetWeaponData,             //pfnGetWeaponData
   CmdStart,                  //pfnCmdStart
   CmdEnd,                    //pfnCmdEnd
   ConnectionlessPacket,      //pfnConnectionlessPacket
   GetHullBounds,             //pfnGetHullBounds
   CreateInstancedBaselines,  //pfnCreateInstancedBaselines
   InconsistentFile,          //pfnInconsistentFile
   AllowLagCompensation,      //pfnAllowLagCompensation
};

extern "C" EXPORT int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion )
{
	if(!g_meta_init)
	{
		// check if engine's pointer is valid and version is correct...
		if ( !pFunctionTable || interfaceVersion != INTERFACE_VERSION )
			return FALSE;

		// pass engine callback function table to engine...
		memcpy( pFunctionTable, &gFunctionTable, sizeof( DLL_FUNCTIONS ) );

		// pass other DLLs engine callbacks to function table...
		if (!(*other_GetEntityAPI)(&other_gFunctionTable, INTERFACE_VERSION))
		{
			return FALSE;  // error initializing function table!!!
		}
	}
	else
	{
		memset(pFunctionTable, 0, sizeof(DLL_FUNCTIONS));
		pFunctionTable->pfnGameInit = GameDLLInit;
		pFunctionTable->pfnSpawn = DispatchSpawn;
		pFunctionTable->pfnKeyValue = DispatchKeyValue;
		pFunctionTable->pfnClientConnect = ClientConnect;
		pFunctionTable->pfnClientDisconnect = ClientDisconnect;
		pFunctionTable->pfnClientPutInServer = ClientPutInServer;
		pFunctionTable->pfnServerDeactivate = ServerDeactivate;
		pFunctionTable->pfnPM_Move = PM_Move;
		pFunctionTable->pfnStartFrame = StartFrame;
		pFunctionTable->pfnClientCommand = ClientCommand;
	}
   return TRUE;
}

extern "C" EXPORT int GetNewDLLFunctions( NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion ) 
{ 
	// pass other DLLs engine callbacks to function table... 
	if (!other_GetNewDLLFunctions) return FALSE;
	if (!(*other_GetNewDLLFunctions)(pFunctionTable, interfaceVersion)) 
	{ 
		return FALSE;  // error initializing function table!!! 
	} 

	return TRUE; 
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
	if(!g_meta_init)
		ClientCommand(pBot);
	else
		MDLL_ClientCommand(pBot);

   isFakeClientCommand = 0;
}


const char *Cmd_Args( void )
{
   if (isFakeClientCommand)
   {
	if(!g_meta_init)
		return &g_argv[0];
	else
		RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[0]);
   }
   else
   {
	if(!g_meta_init)
		return (*g_engfuncs.pfnCmd_Args)();
	else
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
	else
		RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[64]);
      }
      else if (argc == 1)
      {
	if(!g_meta_init)
		return &g_argv[128];
	else
		RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[128]);
      }
      else if (argc == 2)
      {
	if(!g_meta_init)
		return &g_argv[192];
	else
		RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[192]);
      }
      else
      {
	if(!g_meta_init)
		return "???";
	else
		RETURN_META_VALUE(MRES_SUPERCEDE, "???");
      }
   }
   else
   {
		const char *pargv = (*g_engfuncs.pfnCmd_Argv)(argc);
		if(!g_meta_init)
			return pargv;
		else
			RETURN_META_VALUE(MRES_SUPERCEDE, pargv);
   }
}


int Cmd_Argc( void )
{
   if (isFakeClientCommand)
   {
	if(!g_meta_init)
		return fake_arg_count;
	else
		RETURN_META_VALUE(MRES_SUPERCEDE, fake_arg_count);
   }
   else
   {
	if(!g_meta_init)
		return (*g_engfuncs.pfnCmd_Argc)();
	else
		RETURN_META_VALUE(MRES_IGNORED, 0);
   }
}
