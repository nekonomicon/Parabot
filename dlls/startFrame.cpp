///////////////////////////////////////////////////////////////////////////////////
//
//  START FRAME
//
///////////////////////////////////////////////////////////////////////////////////

#include "extdll.h"
//#include "util.h"
#include "entity_state.h"
//#include "cbase.h"

#include "bot.h"
#include "bot_func.h"

#include "animation.h"
#include "pb_mapgraph.h"
#include "pb_mapcells.h"
#include "pb_global.h"
#include "pb_observer.h"
#include "sounds.h"
#include "pb_chat.h"
#include "pb_configuration.h"
//#include "hl_game.h"
//#include "hldm_player.h"



Sounds playerSounds;
PB_Observer observer;
float observerUpdate;
bool fatalParabotError = false;



extern bot_t			bots[32];
extern edict_t			*clients[32];
extern int				welcome_index;			// client to welcome
extern float			bot_check_time;			// for checking if new bots should be created
extern DLL_FUNCTIONS	other_gFunctionTable;
extern bool				g_GameRules;
//extern int				min_bots;
extern bool				pb_pause;
extern PB_Configuration pbConfig;
extern PB_Chat			chat;
extern int				numberOfClients;

extern int debug_engine;
static FILE *fp;

static char		cmd_line[80];
static float	respawn_time = 0.0;
static float	client_update_time = 0.0;
//extern int max_bots;



Vector playerPos;

int mod_id = 0;			// the MOD in which the bot runs
float roundStartTime = 0;

extern float nextAirstrikeTime;

void fixAngle( Vector &angle );
extern int endCam;
extern Vector camPlayerPos, camPlayerAngles, camPlayerVAngle, camPos;
extern int gmsgHideWeapon;
extern int camPlayerModel, camPlayerWeapon;
extern int clientWeapon[32];
extern edict_t *camPlayerLaser;



float welcome_time = 0.0;
#ifdef _DEBUG
	char welcome_msg[] = "You are playing a debug version of Parabot 0.91.\n";
#else
	char welcome_msg[] = "Welcome to Parabot 0.91\n";
#endif



extern PB_MapGraph mapGraph;	// mapgraph for waypoints
extern PB_MapCells map;
extern int botNr;
extern edict_t *camPlayer;
extern Vector camPos;

extern CMarker		glMarker;
extern int   need_init;

extern bool headToBunker;
extern float airStrikeTime;


void UpdateClientData(const struct edict_s *ent, int sendweapons, struct clientdata_s *cd);
// implemented in client.cpp

void saveLevelData();
bool loadLevelData();
// implemented in pb_mapimport.cpp

void debugFile( char *msg );

float srvMaxSpeed = 270;




float serverMaxSpeed()
{
	return srvMaxSpeed;
}


void updateBotClients()
{
	clientdata_s cd;

	if (client_update_time <= gpGlobals->time)	// update bot clients every 0.5 sec
	{
		client_update_time = gpGlobals->time + 0.5;
		
		for (int i=0; i < 32; i++) {
            if (bots[i].is_used) {
				memset(&cd, 0, sizeof(cd));					// assumed by UpdateClientData
				UpdateClientData( bots[i].pEdict, 1, &cd );
				
				// see if a weapon was dropped...
				if (bots[i].bot_weapons != cd.weapons) bots[i].bot_weapons = cd.weapons;
            }
		}
	}
}

#include "parabot.h"


void checkForMapChange()
{
	static float previous_time = 1000000; 
	static bool roundNotStarted = false;
	

	// if a new map has started then...
	if (previous_time > gpGlobals->time)
	{
		srvMaxSpeed = CVAR_GET_FLOAT("sv_maxspeed");
		glMarker.deleteAll();
//		need_init = 1;
		camPlayer = 0;
		camPlayerLaser = 0;
		fatalParabotError = !loadLevelData();
		if (fatalParabotError) {
			char buffer[256];
			sprintf( buffer, "The map %s is corrupt and cannot be played with bots!\nPlease exit and pick another one.", STRING(gpGlobals->mapname) );
			errorMsg( buffer );
		}
		observer.init();
		observerUpdate = worldTime() + 0.5;
		// 4..14 minutes
		nextAirstrikeTime = worldTime() + 240.0 + RANDOM_FLOAT( 0.0, 600.0 );
		
		roundStartTime = worldTime();
		client_update_time = 0.0;  // start updating client data again
		
		// if haven't started respawning yet then set the respawn time
		if (respawn_time < 1.0) respawn_time = gpGlobals->time + 5.0;
		
		// set the time to check for automatically adding bots to dedicated servers
		bot_check_time = gpGlobals->time + 10.0;
		
		// if "map" command was used, set bots in use to respawn state...
		for (int i=0; i < 32; i++)
		{
            if ((bots[i].is_used) && (bots[i].respawn_state != RESPAWN_NEED_TO_RESPAWN))
            {
				// bot has already been "kicked" by server so just set flag
				bots[i].respawn_state = RESPAWN_NEED_TO_RESPAWN;
            }
		}
	}
	previous_time = gpGlobals->time;

	if (mod_id==CSTRIKE_DLL) {
		// detect CS-roundstart
		if (worldTime() < roundStartTime) {	
			roundNotStarted = true;
			return;
		}
		if (roundNotStarted) {	// things to do 1x at roundTime=0
			roundNotStarted = false;
			//observer.init();
			//observer.registerClients();
			for (int i=0; i < gpGlobals->maxClients; i++) {
				if (bots[i].is_used && bots[i].respawn_state==RESPAWN_IDLE) 
					bots[i].parabot->initAfterRespawn();	
			}
			debugMsg( "CS ROUND STARTED!\n" );
		}
	}
}

static unsigned short FixedUnsigned16( float value, float scale )
{
	int output;

	output = (int) (value * scale);
	if ( output < 0 )
		output = 0;
	if ( output > 0xFFFF )
		output = 0xFFFF;

	return (unsigned short)output;
}

static short FixedSigned16( float value, float scale )
{
	int output;

	output = (int) (value * scale);

	if ( output > 32767 )
		output = 32767;

	if ( output < -32768 )
		output = -32768;

	return (short)output;
}

void UTIL_HudMessage( CBaseEntity *pEntity, const hudtextparms_t &textparms, const char *pMessage )
{
	if ( !pEntity || !pEntity->IsNetClient() )
		return;

	MESSAGE_BEGIN( MSG_ONE, SVC_TEMPENTITY, NULL, pEntity->edict() );
		WRITE_BYTE( TE_TEXTMESSAGE );
		WRITE_BYTE( textparms.channel & 0xFF );

		WRITE_SHORT( FixedSigned16( textparms.x, 1<<13 ) );
		WRITE_SHORT( FixedSigned16( textparms.y, 1<<13 ) );
		//WRITE_SHORT( -1 );
		//WRITE_SHORT( -1 );
		WRITE_BYTE( textparms.effect );

		WRITE_BYTE( textparms.r1 );
		WRITE_BYTE( textparms.g1 );
		WRITE_BYTE( textparms.b1 );
		WRITE_BYTE( textparms.a1 );

		WRITE_BYTE( textparms.r2 );
		WRITE_BYTE( textparms.g2 );
		WRITE_BYTE( textparms.b2 );
		WRITE_BYTE( textparms.a2 );

		WRITE_SHORT( FixedUnsigned16( textparms.fadeinTime, 1<<8 ) );
		WRITE_SHORT( FixedUnsigned16( textparms.fadeoutTime, 1<<8 ) );
		WRITE_SHORT( FixedUnsigned16( textparms.holdTime, 1<<8 ) );

		if ( textparms.effect == 2 )
			WRITE_SHORT( FixedUnsigned16( textparms.fxTime, 1<<8 ) );
		
		if ( strlen( pMessage ) < 512 )
		{
			WRITE_STRING( pMessage );
		}
		else
		{
			char tmp[512];
			strncpy( tmp, pMessage, 511 );
			tmp[511] = 0;
			WRITE_STRING( tmp );
		}
	MESSAGE_END();
}

int gmsgHudText = 0;

void sendWelcomeToNewClients()
{
	if (welcome_index != -1) {
		
		if (welcome_time == 0) {
			// are they out of observer mode yet?
			if (isAlive( clients[welcome_index] )) {
				welcome_time = worldTime() + 5.0;  // welcome in 5 seconds
			}
		}
		else if (worldTime() > welcome_time) {
			if (worldTime() > 30.0)		// if game has already started
				chat.parseMessage( clients[welcome_index], " Hi " );	// make bots chat
			if (!pbConfig.onTouringMode()) {
				if (gmsgHudText==0)	gmsgHudText = REG_USER_MSG("HudText", -1);
				MESSAGE_BEGIN( MSG_ONE, gmsgHudText, NULL, clients[welcome_index] );
				WRITE_STRING( welcome_msg );
				MESSAGE_END();
			}
			welcome_time = 0;
			welcome_index = -1;
		}

	}
}

 
void checkForBotRespawn()
// are we currently respawning bots and is it time to spawn one yet?
{
	if ((respawn_time >= 1.0) && (respawn_time <= gpGlobals->time))
	{
		int index = 0;
		
		// set the time to check for automatically adding bots to dedicated servers
		// bot_check_time = gpGlobals->time + 5.0;
		
		// find bot needing to be respawned...
		while ( (index < 32) && (bots[index].respawn_state != RESPAWN_NEED_TO_RESPAWN) )
			index++;
		
		if (index < 32)
		{
            bots[index].respawn_state = RESPAWN_IS_RESPAWNING;
            bots[index].is_used = FALSE;      // free up this slot
			
            // respawn 1 bot then wait a while (otherwise engine crashes)
        	BotCreate( bots[index].personality );
            			
			bot_check_time = gpGlobals->time + 10.0;	// don't add new bots
            respawn_time = gpGlobals->time + 1.0;  // set next respawn time
		}
		else
		{
            respawn_time = 0.0;
		}
	}
}


void addRandomBot()
{
	BotCreate();
}


void kickRandomBot()
{
	int cnt = 0;
	while (cnt++ < 500) {
		int i = RANDOM_LONG( 0, 32-1 );
		if (bots[i].is_used) {
			char cmd[80];
			sprintf(cmd, "kick \"%s\"\n", bots[i].name);
			SERVER_COMMAND( cmd );  // kick the bot using (kick "name")
			bots[i].is_used = false;
			break;
		}
	}
}


void checkForBotCreation()
// check if time to see if a bot needs to be created...
{
	if ( !pbConfig.onServerMode() ) {
		// standard mode with fixed number of bots:
		if (bot_check_time < gpGlobals->time) {
			bot_check_time = gpGlobals->time + 5.0;		// add/kick bots with 5 sec. delay
			
			int numBots = 0;
			for (int b=0; b<32; b++) if (bots[b].is_used) numBots++;
		
			if ( numBots < pbConfig.numBots() && numberOfClients < gpGlobals->maxClients) addRandomBot();
			else if ( numBots > pbConfig.numBots() ) kickRandomBot();
		}
	}
	else {
		// server mode
		if (bot_check_time < gpGlobals->time) {
				
			int numBots = 0;
			for (int b=0; b<32; b++) if (bots[b].is_used) numBots++;

			float avgTime = 2.0 * pbConfig.stayTime() / ( pbConfig.minBots() + pbConfig.maxBots() );
			bot_check_time = gpGlobals->time + RANDOM_FLOAT( 0.5*avgTime, 1.5*avgTime );
			float rnd = RANDOM_FLOAT( 0, 100 );

			if ( numberOfClients == gpGlobals->maxClients ) {
				// hold one slot free for human players
				if ( numBots > 0 ) kickRandomBot();
			}
			else if ( numBots < pbConfig.minBots() ) {
				if ( numberOfClients < (gpGlobals->maxClients-1) ) {
					addRandomBot();
					if ( numBots < (pbConfig.minBots()-1) )
						bot_check_time = gpGlobals->time + 5.0;		// add bots faster
				}
			}
			else if ( numBots == pbConfig.minBots() ) {
				if ( rnd < 50 && numberOfClients < (gpGlobals->maxClients-1) ) addRandomBot(); 
			}
			else if ( numBots < pbConfig.maxBots() ) {
				if ( rnd < 30 && numberOfClients < (gpGlobals->maxClients-1) ) addRandomBot();
				else if ( rnd > 70 && numBots > 0 ) kickRandomBot();
			}
			else if ( numBots == pbConfig.maxBots() ) {
				if ( rnd < 50 && numBots > 0 ) kickRandomBot();
			}
			else if ( numBots > pbConfig.maxBots() ) {
				kickRandomBot();
				if ( numBots > (pbConfig.maxBots()+1) )
					bot_check_time = gpGlobals->time + 5.0;		// kick bots faster
			}
		}
	}
}


float worldTime()
{
	return gpGlobals->time;
}


void updateBotCam() 
{
	if (!camPlayer) return;

	if (endCam==1) {
		// show all hud
		char hudState = 0;
		if (CVAR_GET_FLOAT( "mp_flashlight" ) == 0) hudState = HIDEHUD_FLASHLIGHT;
		if (gmsgHideWeapon==0) gmsgHideWeapon = REG_USER_MSG( "HideWeapon", 1 );
		MESSAGE_BEGIN( MSG_ONE, gmsgHideWeapon, NULL, camPlayer );
			WRITE_BYTE( hudState );
		MESSAGE_END();
		camPlayer->v.flags &= ~(FL_FROZEN | FL_SPECTATOR);
		camPlayer->v.takedamage = DAMAGE_AIM;
		camPlayer->v.fixangle = false;
		camPlayer->v.movetype = MOVETYPE_WALK;
		camPlayer->v.solid = SOLID_SLIDEBOX;		
		camPlayer->v.modelindex = camPlayerModel;
		camPlayer->v.viewmodel = camPlayerWeapon;
		UTIL_SetOrigin (&(camPlayer->v), camPlayerPos);	// restore player position
		
		if (camPlayerLaser) {	
			// restore laserdot:
			camPlayerLaser->v.effects &= ~EF_NODRAW;
			camPlayerLaser = 0;
		}

		camPlayer = 0;		
		endCam = 0;
		return;
	}

	if (bots[botNr].is_used) {
		edict_t *ent = bots[botNr].pEdict;
		//UTIL_MakeVectors (ent->v.v_angle);
		//Vector camPos = ent->v.origin - 40*gpGlobals->v_forward + 45*gpGlobals->v_up;
		Vector camAngle;
		bool chaseCam = true;
		if (chaseCam) {
			Vector playerDir = ent->v.origin - camPos;
			float camDist = playerDir.Length();
			if (camDist == 0) {
				errorMsg( "updateBotCam!" );
				return;
			}
			Vector camMove = ((camDist-100) / camDist) * playerDir;
			camPos = camPos + camMove;
			camAngle = UTIL_VecToAngles( playerDir );
			camAngle.x = -camAngle.x;
		}
		else {
			//camPlayer->v.viewmodel = ent->v.viewmodel;	// show correct weapon
			UTIL_MakeVectors (ent->v.v_angle);
			camPos = ent->v.origin + 16*gpGlobals->v_forward;
			camAngle = ent->v.v_angle;
		}

		fixAngle( camAngle );
		
		camPlayer->v.flags |= (FL_FROZEN | FL_SPECTATOR);
		camPlayer->v.takedamage = DAMAGE_NO;
		camPlayer->v.fixangle = TRUE;
		camPlayer->v.movetype = MOVETYPE_NONE;
		camPlayer->v.solid = SOLID_NOT;
		
		if (endCam==0) {
			camPlayer->v.v_angle = camAngle;
			camPlayer->v.angles  = camAngle;
			camPlayer->v.origin = camPos;
		}
		else {
			camPlayer->v.v_angle = camPlayerVAngle;
			camPlayer->v.angles  = camPlayerVAngle;
			camPlayer->v.origin = camPlayerPos;
			endCam = 1;
		}
		UTIL_SetOrigin( &(camPlayer->v), camPos );		// set player position to camPos

	}
}


void checkForAirStrike()
{
	if (airStrikeTime==0) return;
	if (worldTime()<airStrikeTime) return;

	CBaseEntity *pPlayer = 0;
	for (int i=1; i<=gpGlobals->maxClients; i++) {
		pPlayer = UTIL_PlayerByIndex( i );
		if (!pPlayer) continue;							// skip invalid players
		if (!isAlive( ENT(pPlayer->pev) )) continue;	// skip player if not alive
		if (pPlayer->pev->solid == SOLID_NOT) continue;	

		bot_t *bot = UTIL_GetBotPointer( pPlayer->edict() );
		if ( bot == 0 ) continue;
		if ( (worldTime() - bot->parabot->lastRespawn) < 1.0 ) continue;
		
		const char *name = STRING(pPlayer->pev->netname);
		debugMsg( name, " was save at airstrike!\n" );
		Vector pos = pPlayer->pev->origin;
		PB_Navpoint *nearest = mapGraph.getNearestNavpoint( pos, NAV_S_AIRSTRIKE_COVER );
		if ( nearest && ((nearest->pos()-pos).Length() < 256) ) {
			debugMsg( "Airstrike cover stored nearby!\n" );
		}
		else {
			nearest = mapGraph.getNearestNavpoint( pos, NAV_INFO_PLAYER_DM );
			if ( !nearest || (nearest->pos()-pos).Length() > 64 ) {
				debugMsg( "Adding airstrike cover!\n" );
				PB_Navpoint coverNav;
				coverNav.init( pos, NAV_S_AIRSTRIKE_COVER, 0 );
				mapGraph.addNavpoint( coverNav );
			}
		}
	}

	airStrikeTime = 0;
	headToBunker = false;
}


extern int activeBot;
void pb2dMsg( int x, int y, const char *msg );
void pb3dMsg( Vector pos, const char *msg );

bool isOnScreen( edict_t *ent, edict_t *player )
{
	TraceResult tr;

	// check if in visible distance
	Vector playerpos = player->v.origin + player->v.view_ofs;
	Vector pos = ent->v.origin + ent->v.view_ofs;
	float dist = (pos - playerpos).Length();
	if (dist > 1500) return false;

	// check if in viewcone
	Vector dir = (pos - playerpos).Normalize();
	float dot = DotProduct( gpGlobals->v_forward, dir );
	if (  dot > 0.7 ) {
		UTIL_TraceLine( playerpos, pos, dont_ignore_monsters, ignore_glass, player, &tr);	
		if ( (tr.flFraction == 1.0) || (tr.pHit == ent) ) {
			return true;
		}
	}
	return false;
}


void print3dDebugInfo()
{
	// draw 3d msg
	edict_t *player = INDEXENT( 1 );
	UTIL_MakeVectors( player->v.v_angle );
	for (int i=0; i < gpGlobals->maxClients; i++) {
		if (bots[i].is_used && bots[i].respawn_state==RESPAWN_IDLE) {
			if (isOnScreen( bots[i].pEdict, player )) {
				char buffer[256];
				strcpy( buffer, STRING(bots[i].pEdict->v.netname) );
				strcat( buffer, "\n" );
				strcat( buffer, bots[i].parabot->goalMove );
				strcat( buffer, "\n" );
				strcat( buffer, bots[i].parabot->goalView );
				strcat( buffer, "\n" );
				strcat( buffer, bots[i].parabot->goalAct );
				pb3dMsg( bots[i].pEdict->v.origin+bots[i].pEdict->v.view_ofs, buffer );
			}
		}
	}
}


void updateVisTable()
{
	int trCount = map.updateVisibility( 128 );
	if (trCount > 0) {
		char buffer[80];
		sprintf( buffer, "Tracing Visibility (%i/%i)...", map.lastVisUpdate(), map.numberOfCells() );
		pb2dMsg( 20, 100, buffer );
	}
}


PB_Navpoint* cashedNavpoint[32+1];


void invalidateMapGraphCash()
{
	for (int i=0; i<=32; i++) cashedNavpoint[i] = 0;
}


void cachePlayerData()
{
	for (int i=1; i<=gpGlobals->maxClients; i++) {
		edict_t *pPlayer = INDEXENT( i );
		if (playerExists( pPlayer )) {
			cashedNavpoint[i] = mapGraph.getNearestNavpoint( pPlayer->v.origin );
		}
	}
}


PB_Navpoint* getNearestNavpoint( edict_t *pEdict )
{
	int i = ENTINDEX( pEdict );
	if (i<0 || i>32) return 0;
	return cashedNavpoint[i];
}


extern edict_t *playerEnt;

//HL_Game game( HL_Game::DM );


void StartFrame( void )
{
	if (gpGlobals->deathmatch) {

		cachePlayerData();
						
		/*CBaseEntity *wpn = getActiveItem( playerEnt );
		if (wpn) debugMsg( "carrying ", STRING( wpn->pev->classname ), "\n" );
		else debugMsg( "no weapon!\n" );*/
		
		checkForMapChange();

		if (!fatalParabotError) {
			updateBotClients();
			playerSounds.getAllClientSounds();
			
			print3dDebugInfo();
			
			// call BotThink for each active bot
			for (int b=0; b < 32; b++) {
				if (bots[b].is_used && bots[b].respawn_state==RESPAWN_IDLE) BotThink(&bots[b]);	
			}
			
			activeBot = botNr;	// print out global debug messages
			
			updateBotCam();
			if (worldTime() > observerUpdate) {
				observer.registerClients();
				observerUpdate = worldTime() + 3.0;//0.5;
				/*
				// PIA Test
				for (int pi=0; pi<game.world()->numberOfPlayers(); pi++) {
					PIA_Player *dmp = game.world()->player( pi );
					if ( dmp->isValid() && !(dmp->isBot()) ) {
						PIA_Weapon *pw = dmp->firstWeapon();
						if (pw) {
							debugMsg( pw->name(), "\n" );
							while (pw=dmp->nextWeapon()) debugMsg( pw->name(), "\n" );
						}
						break;
					}
				}
				*/
			}
			observer.observeAll();
			updateVisTable();
			checkForAirStrike();
			sendWelcomeToNewClients();
			checkForBotRespawn();
			checkForBotCreation();
			if (!pb_pause) chat.check();
			glMarker.drawMarkers();
		}
	}
	
	(*other_gFunctionTable.pfnStartFrame)();
}
