#include "extdll.h"
#include "enginecallback.h"
#include "entity_state.h"
#include "marker.h"
#include <queue>
#include "com_model.h"
#include "pm_defs.h"
#include "bot.h"
#include "bot_func.h"
#include "pb_global.h"
#include "parabot.h"
#include "pb_chat.h"
#include "pb_mapcells.h"
#include "pb_configuration.h"


extern int mod_id;
extern int clientWeapon[32];
extern bool pb_pause;
extern float bot_check_time;
extern enginefuncs_t g_engfuncs;
extern int debug_engine;
extern globalvars_t  *gpGlobals;
extern DLL_FUNCTIONS other_gFunctionTable;
extern bot_t bots[32];
extern int mod_id;
extern bool dbgFile;
extern PB_MapGraph mapGraph;
extern PB_MapCells map;
extern PB_Configuration pbConfig;
extern PB_Chat chat;
extern int numberOfClients;

static FILE *fp;

CMarker		glMarker;
int botNr = 0;			// bot to debug
int botHalt = 0;		// if set >0, breaks for botNr at certain place
int botTarget = -1;		// target nav id to approach (-1 = nothing)
bool visualizeCellConnections = false;
bool oldBotStop = false;

// BotCam-Variables
edict_t *camPlayer = 0;	// points to edict of player with camera view
edict_t *camPlayerLaser = 0;	// laserdot that has to get restored
Vector camPlayerPos, camPlayerAngles, camPlayerVAngle, camPos;
int endCam = 0;
int camPlayerModel, camPlayerWeapon;
int gmsgHideWeapon = 0;

#define NO_MENU		0
#define MAIN_MENU	1
#define MODE_MENU	2
#define CHAT_MENU	3
#define NUMBER_MENU 4
#define SKILL_MENU	5


char dynMenu[512];

int currentMenu;
int menuState = NO_MENU;
int menuChoice;



void saveLevelData( void );	// in pb_mapimport.cpp


void adjustAimSkills()
{
	int minAimSkill = pbConfig.minSkill();
	int maxAimSkill = pbConfig.maxSkill();

	for (int i=0; i<32; i++) if (bots[i].is_used) {
		int aimSkill = pbConfig.personality( bots[i].personality ).aimSkill;
		if (aimSkill < minAimSkill) aimSkill = minAimSkill;
		if (aimSkill > maxAimSkill) aimSkill = maxAimSkill;
		bots[i].parabot->action.setAimSkill( aimSkill );
	}
}



int menuSlot( int number )
{
	int res = 0;
	for (int i=0; i<number; i++) res |= 1<<i;
	return res;
}


void showMainMenu( edict_t *pEntity )
{
	char *main_menu = {"\
	Parabot Configuration\n\n\
	1. Change Number of Bots\n\
	2. Change Botskill\n\
	3. Change Gamemodes\n\
	4. Configure Botchat\n\
	5. Exit\
	"};

	currentMenu = MAIN_MENU;
	UTIL_ShowMenu( pEntity, menuSlot(5), -1, FALSE, main_menu );
}


void showGameModeMenu( edict_t *pEntity )
{
	strcpy( dynMenu, "Change Gamemodes\n\n" );
	if ( pbConfig.onRestrictedWeaponMode() ) 
		 strcat( dynMenu, "1. Disable RestrictedWeapons\n" );
	else strcat( dynMenu, "1. Enable RestrictedWeapons\n" );
	if ( pbConfig.onPeaceMode() ) 
		 strcat( dynMenu, "2. Disable PeaceMode\n" );
	else strcat( dynMenu, "2. Enable PeaceMode\n" );
	strcat( dynMenu, "3. Exit\n" );

	currentMenu = MODE_MENU;
	UTIL_ShowMenu( pEntity, menuSlot(3), -1, FALSE, dynMenu );
}


void showChatMenu( edict_t *pEntity )
{
	strcpy( dynMenu, "Configure Botchat\n\n" );
	if ( pbConfig.usingChat() ) {
		strcat( dynMenu, "1. Disable Botchat\n" );
		if ( pbConfig.onAlwaysRespond() ) 
			 strcat( dynMenu, "2. Disable AlwaysRespond\n" );
		else strcat( dynMenu, "2. Enable AlwaysRespond\n" );
		if ( pbConfig.onChatLog() ) 
			 strcat( dynMenu, "3. Disable ChatLog\n" );
		else strcat( dynMenu, "3. Enable ChatLog\n" );
		strcat( dynMenu, "4. Exit\n" );
	}
	else {
		strcat( dynMenu, "1. Enable Botchat\n\n\n4. Exit\n" );
	}

	currentMenu = CHAT_MENU;
	UTIL_ShowMenu( pEntity, menuSlot(4), -1, FALSE, dynMenu );
}


void showNumberMenu( edict_t *pEntity )
{
	char buffer[64];

	strcpy( dynMenu, "Configure Number of Bots\n\n" );
	if (pbConfig.onServerMode()) {
		sprintf( buffer, "MinBots is %i\n", pbConfig.minBots() );
		strcat( dynMenu, buffer );
		strcat( dynMenu, "  1. -    2. +\n");
		sprintf( buffer, "MaxBots is %i\n", pbConfig.maxBots() );
		strcat( dynMenu, buffer );
		strcat( dynMenu, "  3. -    4. +\n");
		strcat( dynMenu, "5. Disable ServerMode\n");
		strcat( dynMenu, "6. Exit\n" );
	}
	else {
		sprintf( buffer, "NumBots is %i\n", pbConfig.numBots() );
		strcat( dynMenu, buffer );
		strcat( dynMenu, "  1. -    2. +\n\n\n");
		strcat( dynMenu, "5. Enable ServerMode\n");
		strcat( dynMenu, "6. Exit\n" );
	}

	currentMenu = NUMBER_MENU;
	UTIL_ShowMenu( pEntity, menuSlot(6), -1, FALSE, dynMenu );
}


void showSkillMenu( edict_t *pEntity )
{
	char buffer[64];

	strcpy( dynMenu, "Configure Bot Aimskill\n\n" );
	sprintf( buffer, "MinSkill is %i\n", pbConfig.minSkill() );
	strcat( dynMenu, buffer );
	strcat( dynMenu, "  1. -    2. +\n");
	sprintf( buffer, "MaxSkill is %i\n", pbConfig.maxSkill() );
	strcat( dynMenu, buffer );
	strcat( dynMenu, "  3. -    4. +\n");
	strcat( dynMenu, "5. Exit\n" );
	
	currentMenu = SKILL_MENU;
	UTIL_ShowMenu( pEntity, menuSlot(5), -1, FALSE, dynMenu );
}


void DSaddbot() 
{    
	BotCreate();
}


void DSsimulate()
{
	int arg_count = CMD_ARGC();  // number of arguments    
	const char *pcmd = CMD_ARGV(0);    
	if(arg_count == 2) {
		const char *arg1 = CMD_ARGV(1);
		pbConfig.setBoolVar( pcmd, arg1 );
	}
	else {
		pbConfig.setBoolVar( pcmd, 0 );
	}
}


void DSlogChat()
{
	int arg_count = CMD_ARGC();  // number of arguments    
	const char *pcmd = CMD_ARGV(0);    
	if(arg_count == 2) {
		const char *arg1 = CMD_ARGV(1);
		pbConfig.setBoolVar( pcmd, arg1 );
	}
	else {
		pbConfig.setBoolVar( pcmd, 0 );
	}
}


void DSrestrictedWeapons()
{
	int arg_count = CMD_ARGC();  // number of arguments    
	const char *pcmd = CMD_ARGV(0);    
	if(arg_count == 2) {
		const char *arg1 = CMD_ARGV(1);
		pbConfig.setBoolVar( pcmd, arg1 );
	}
	else {
		pbConfig.setBoolVar( pcmd, 0 );
	}
}


void DSpeace()
{
	int arg_count = CMD_ARGC();  // number of arguments    
	const char *pcmd = CMD_ARGV(0);    
	if(arg_count == 2) {
		const char *arg1 = CMD_ARGV(1);
		pbConfig.setBoolVar( pcmd, arg1 );
	}
	else {
		pbConfig.setBoolVar( pcmd, 0 );
	}
}


void startBotCam( edict_t *pEntity )
{
	assert( pEntity != 0 );

	if (mod_id==VALVE_DLL || mod_id==GEARBOX_DLL) {
		int clientIndex = ENTINDEX( pEntity ) - 1;
		assert( clientIndex >= 0 );
		assert( clientIndex < 32 );
		if ( clientWeapon[clientIndex]==VALVE_WEAPON_RPG     ||
			 clientWeapon[clientIndex]==GEARBOX_WEAPON_EAGLE    ) 
		{	// kill laserspot
			CBaseEntity *dot = NULL;
			while ( (dot = UTIL_FindEntityByClassname (dot, "laser_spot")) != NULL) {
				if ( laserdotOwner( dot->edict() ) == pEntity ) {
					dot->pev->effects |= EF_NODRAW;
					camPlayerLaser = dot->edict();
					break;
				}
			}
		}
	}

	camPlayerPos = pEntity->v.origin;
	camPlayerModel = pEntity->v.modelindex;
	camPlayerWeapon = pEntity->v.viewmodel;
	camPlayerVAngle = pEntity->v.v_angle;
	camPlayerAngles = pEntity->v.angles;
	
	pEntity->v.flags |= (FL_FROZEN | FL_SPECTATOR);
	pEntity->v.takedamage = DAMAGE_NO;
	pEntity->v.fixangle = TRUE;
	pEntity->v.movetype = MOVETYPE_NONE;
	pEntity->v.solid = SOLID_NOT;					// Remove model & collisions
	pEntity->v.modelindex = 0;
	pEntity->v.viewmodel = 0;
	camPlayer = pEntity;
	camPos = camPlayer->v.origin + Vector(0,0,100);
	// hide all hud
	if (gmsgHideWeapon==0) gmsgHideWeapon = REG_USER_MSG( "HideWeapon", 1 );
	MESSAGE_BEGIN( MSG_ONE, gmsgHideWeapon, NULL, camPlayer );
		WRITE_BYTE( HIDEHUD_ALL );
	MESSAGE_END();
}


void endBotCam()
{
	if (camPlayer) {
		endCam = 2;
	}
}



edict_t *lastGround;
char stepSample[32], stepSound[32];
int stepNr = 1;
float stepVol = 1.0;

void pfnEmitSound( edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch );
// from engine.h

void CmdStart( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed );
void CmdEnd ( const edict_t *player );

short ptBotPos;
short path[128];

void ClientCommand( edict_t *pEntity )
{
	const char *pcmd = Cmd_Argv(0);
	const char *arg1 = Cmd_Argv(1);
	const char *arg2 = Cmd_Argv(2);
	const char *arg3 = Cmd_Argv(3);
	const char *arg4 = Cmd_Argv(4);


	// chat message analysis:
	if (FStrEq( pcmd, "say" )) {
		chat.parseMessage( pEntity, (char*)arg1 );	// no return!!!
	}

	// these client commands aren't allow in single player mode or
	// on dedicated servers
	
	if ((gpGlobals->deathmatch) && (!IS_DEDICATED_SERVER()))
	{	
		CParabot *pb = bots[botNr].parabot;
		
		if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"ClientCommand: %s %s %s\n",pcmd, arg1, arg2); fclose(fp); }

		// mapchange redefined:
		if (FStrEq( pcmd, "map" )) {	
			char *newmap = (char*) arg1;
			if ( !IS_MAP_VALID(newmap) ) {
				debugMsg( "Map not valid!\n" );
				return;
			}
			debugFile( "Changing map...\n" );
			FakeClientCommand( pEntity, "hideconsole", 0, 0 );
			CHANGE_LEVEL( newmap, NULL ); 
			return;
		}
		// bot commands:
		else if (FStrEq(pcmd, "botmenu"))
		{
			oldBotStop = pb_pause;
			pb_pause = true;		// while in menu, bots don't move
			showMainMenu( pEntity );
			return;
		}
		else if (FStrEq(pcmd, "menuselect"))
		{
			menuChoice = atoi( arg1 );
			switch( currentMenu ) {

			case  MAIN_MENU:
				switch( menuChoice ) {
				case 1:	showNumberMenu( pEntity );
						break;
				case 2: showSkillMenu( pEntity );
						break;							
				case 3: showGameModeMenu( pEntity );	// change gamemodes
						break;
				case 4:	showChatMenu( pEntity );
						break;
				default:currentMenu = NO_MENU;			// exit		
						pb_pause = oldBotStop; 
				}
				break;

			case NUMBER_MENU:
				if (pbConfig.onServerMode()) {
					switch( menuChoice ) {
					case 1: pbConfig.setIntVar( "MinBots", pbConfig.minBots()-1, 0, 32 );
							showNumberMenu( pEntity );
							break;
					case 2: pbConfig.setIntVar( "MinBots", pbConfig.minBots()+1, 0, pbConfig.maxBots() );
							showNumberMenu( pEntity );
							break;
					case 3: pbConfig.setIntVar( "MaxBots", pbConfig.maxBots()-1, pbConfig.minBots(), 32 );
							showNumberMenu( pEntity );
							break;
					case 4: pbConfig.setIntVar( "MaxBots", pbConfig.maxBots()+1, 0, gpGlobals->maxClients );
							showNumberMenu( pEntity );
							break;
					case 5:	pbConfig.setBoolVar( "ServerMode", "Off" );
							bot_check_time = gpGlobals->time + 5.0;
							showNumberMenu( pEntity );
							break;
					default:showMainMenu( pEntity );
					}
				}
				else {
					switch( menuChoice ) {
					case 1: pbConfig.setIntVar( "NumBots", pbConfig.numBots()-1, 0, 32 );
							showNumberMenu( pEntity );
							break;
					case 2: pbConfig.setIntVar( "NumBots", pbConfig.numBots()+1, 0, gpGlobals->maxClients );
							showNumberMenu( pEntity );
							break;
					case 3: 
					case 4: showNumberMenu( pEntity );
							break;
					case 5:	pbConfig.setBoolVar( "ServerMode", "On" );
							bot_check_time = gpGlobals->time + 5.0;
							showNumberMenu( pEntity );
							break;
					default:showMainMenu( pEntity );
				}
				break;

			case SKILL_MENU:
				switch( menuChoice ) {
					case 1: pbConfig.setIntVar( "MinAimSkill", pbConfig.minSkill()-1, 1, 10 );
							showSkillMenu( pEntity );
							break;
					case 2: pbConfig.setIntVar( "MinAimSkill", pbConfig.minSkill()+1, 1, pbConfig.maxSkill() );
							showSkillMenu( pEntity );
							break;
					case 3: pbConfig.setIntVar( "MaxAimSkill", pbConfig.maxSkill()-1, pbConfig.minSkill(), 10 );
							showSkillMenu( pEntity );
							break;
					case 4: pbConfig.setIntVar( "MaxAimSkill", pbConfig.maxSkill()+1, 1, 10 );
							showSkillMenu( pEntity );
							break;
					
					default:showMainMenu( pEntity );
				}
				adjustAimSkills();
				break;

			case MODE_MENU:
				switch( menuChoice ) {
				case 1: if (pbConfig.onRestrictedWeaponMode()) pbConfig.setBoolVar( "RestrictedWeapons", "Off" );
						else pbConfig.setBoolVar( "RestrictedWeapons", "On" );
						showGameModeMenu( pEntity );
						break;
				case 2: if (pbConfig.onPeaceMode()) pbConfig.setBoolVar( "PeaceMode", "Off" );
						else pbConfig.setBoolVar( "PeaceMode", "On" );
						showGameModeMenu( pEntity );
						break;
				default:showMainMenu( pEntity );
				}
				break;

			case CHAT_MENU:
				switch( menuChoice ) {
				case 1:	if (pbConfig.usingChat()) pbConfig.setBoolVar( "BotChat", "Off" );
						else pbConfig.setBoolVar( "BotChat", "On" );
						showChatMenu( pEntity );
						break;
				case 2: if (pbConfig.usingChat()) {
							if (pbConfig.onAlwaysRespond()) pbConfig.setBoolVar( "AlwaysRespond", "Off" );
							else pbConfig.setBoolVar( "AlwaysRespond", "On" );
						}
						showChatMenu( pEntity );
						break;
				case 3: if (pbConfig.usingChat()) {
							if (pbConfig.onChatLog()) pbConfig.setBoolVar( "ChatLog", "Off" );
							else pbConfig.setBoolVar( "ChatLog", "On" );
						}
						showChatMenu( pEntity );
						break;
				default:showMainMenu( pEntity );
				}
				break;
			}

			default: break;

			}
			return;			
		}
		else if (FStrEq(pcmd, "addbot"))
		{
			BotCreate();
			return;
		}
		else if (FStrEq( pcmd, "peacemode" )) {
			DSpeace();
			return;
		}
		else if (FStrEq( pcmd, "restrictedweapons" )) {
			DSrestrictedWeapons();
			return;
		}
		else if (FStrEq( pcmd, "hidewelcome" )) {
			DSsimulate();
			return;
		}
		else if (FStrEq( pcmd, "chatlog" )) {
			DSlogChat();
			return;
		}
		else if (FStrEq( pcmd, "botstop" )) {
			pb_pause = true;
			return;
		}
		else if (FStrEq( pcmd, "botgo" )) {
			pb_pause = false;
			// call resetStuck() for all bots
			for (int i=0; i<gpGlobals->maxClients; i++) {
				if (bots[i].is_used && bots[i].respawn_state==RESPAWN_IDLE) {
					assert( bots[i].parabot != 0 );
					bots[i].parabot->action.resetStuck();
				}
			}
			return;
		}
		else if (FStrEq( pcmd, "botcam" )) {
			if (!camPlayer)	startBotCam( pEntity );
			else {	// switch to next bot
				int count = 32;
				do {
					count--;
					botNr++;
					if (botNr==32) botNr = 0;
				}
				while ( !bots[botNr].is_used && count>0 );
			}
			return;
		}
		else if (FStrEq( pcmd, "camstop" )) {
			endBotCam();
			return;
		}

// rest only for debugmode!
#ifdef _DEBUG
		else if (FStrEq(pcmd, "debug_engine"))
		{
			debug_engine = 1;
			ClientPrint( VARS(pEntity), HUD_PRINTNOTIFY, "debug_engine enabled!\n");
			return;
		}
		else if (FStrEq(pcmd, "debug_off"))
		{
			debug_engine = 0;
			ClientPrint( VARS(pEntity), HUD_PRINTNOTIFY, "debug_engine disabled!\n");
			return;
		}
		else if (FStrEq( pcmd, "dbgfile" )) {
			if (FStrEq( arg1, "on" )) {
				dbgFile = true;
			}
			else if (FStrEq( arg1, "off" )) {
				dbgFile = false;
			}
			return;
		}
		else if (FStrEq(pcmd, "debugTrace"))
		{
			PB_Action at;
			PB_Roaming rt;
			at.init( pEntity );
			rt.init( pEntity, &at );
			UTIL_MakeVectors( pEntity->v.v_angle );
			Vector pos = pEntity->v.origin + 200*gpGlobals->v_forward;
			at.setMoveAngle( pEntity->v.v_angle );
			rt.checkWay( pos );
			return;
		}
		else if (FStrEq(pcmd, "setstep")) {
			char *newSample = (char*) arg1;
			strcpy( stepSample, newSample );
			sprintf( stepSound, "player/pl_%s%i.wav", stepSample, stepNr );
			debugMsg( "New stepSound = ", stepSound, "\n" );
			return;
		}
		else if (FStrEq(pcmd, "setstepnr")) {
			char *newNr = (char*) arg1;
			stepNr = atoi( newNr );
			sprintf( stepSound, "player/pl_%s%i.wav", stepSample, stepNr );
			debugMsg( "New stepSound = ", stepSound, "\n" );
			return;
		}
		else if (FStrEq(pcmd, "setvol")) {
			char *newVol = (char*) arg1;
			int iVol = atoi( newVol );
			stepVol = (float)iVol;	stepVol/=100;
			if (stepVol > 1.0) stepVol = 1.0;
			debugMsg( "New stepVolume = %.2f\n", stepVol );
			return;
		}
		else if (FStrEq(pcmd, "playstep")) {
			//EMIT_SOUND( pb->ent, CHAN_BODY, stepSound, stepVol, ATTN_NORM);
			pfnEmitSound( pb->ent, CHAN_BODY, stepSound, stepVol, ATTN_NORM, 0, 100 );
			return;
		}
		else if (FStrEq(pcmd, "playsound")) {
			char *sound = (char*) arg1;
			pfnEmitSound( pEntity, CHAN_BODY, sound, 1.0, ATTN_NORM, 0, 100 );
			return;
		}
		else if (FStrEq(pcmd, "debugbreak"))
		{
			char *reason = (char*) arg1;
			if ( FStrEq(reason, "weapon") ) botHalt = BREAK_WEAPON;
			else if ( FStrEq(reason, "goals") ) botHalt = BREAK_GOALS;
			return;
		}
		else if (FStrEq( pcmd, "setbotnr" )) {
			char *stopstr;
			botNr = strtol( arg1, &stopstr, 10 );
			debugMsg( "OK.\n" );
			return;
		}
		else if (FStrEq( pcmd, "botdist" )) {
			Vector p = pb->ent->v.origin - pEntity->v.origin;
			debugMsg( "Distance = %.f\n", p.Length() );
			return;
		}
		else if (FStrEq( pcmd, "pathinfo" )) {
			if (pb->actualPath) {
				debugMsg( "Actual ");  pb->actualPath->print();  debugMsg( "\n" );
			}
			return;
		}
		else if (FStrEq( pcmd, "wpinfo" )) {
			if (pb->actualPath) {
				Vector p = pb->waypoint.pos();
				debugMsg( "Heading for waypoint (%.f, %.f, %.f)\n", p.x, p.y, p.z);
			}
			return;
		}
		else if (FStrEq( pcmd, "ground" )) {
			assert( pEntity != 0 );
			if (pEntity->v.groundentity==0) {
				debugMsg( "No ground entity!\n" );
				return;
			}
			lastGround = pEntity->v.groundentity;
			assert( lastGround != 0 );
			Vector p = lastGround->v.absmin;
			ALERT( at_console, "%s, min at(%.f, %.f, %.f), ", STRING(lastGround->v.classname), p.x, p.y, p.z);
			p = lastGround->v.absmax;
			ALERT( at_console, "max at(%.f, %.f, %.f), solid=%i\n", p.x, p.y, p.z,
				lastGround->v.solid );
			return;
		}
		else if (FStrEq( pcmd, "lastground" )) {
			if (lastGround==0) {
				debugMsg( "No ground entity!\n" );
				return;
			}
			Vector p = lastGround->v.absmin;
			ALERT( at_console, "%s, min at(%.f, %.f, %.f), ", STRING(lastGround->v.classname), p.x, p.y, p.z);
			p = lastGround->v.absmax;
			ALERT( at_console, "max at(%.f, %.f, %.f), solid=%i\n", p.x, p.y, p.z,
				lastGround->v.solid );
			return;
		}
		else if (FStrEq( pcmd, "navinfo" ) && (pb->actualNavpoint)) {
			debugMsg( "Actual navpoint is ");  pb->actualNavpoint->print();  
			debugMsg( ", Linkage=%i\n", mapGraph.linkedNavpointsFrom( pb->actualNavpoint ) );
			return;
		}
		else if (FStrEq( pcmd, "playernav" )) {
			PB_Navpoint *nav = mapGraph.getNearestNavpoint( pEntity->v.origin );
			//PB_Navpoint::Nav2Classname( str, nav->type() );
			Vector p = pEntity->v.origin;
			debugMsg( "Player at (%.f, %.f, %.f), ", p.x, p.y, p.z);
			p = nav->pos();
			debugMsg( "nearest navpoint is ", nav->classname() );
			debugMsg( " id=%i", nav->id() );
			debugMsg( " at (%.f, %.f, %.f)\n", p.x, p.y, p.z);
			debugMsg( "Linkage=%i\n", mapGraph.linkedNavpointsFrom( nav ) );
			if (nav->reached( pEntity )) debugMsg( "Navpoint reached\n" );
			if (nav->entity()) {
				debugMsg( "NavHealth = %.f, ", nav->entity()->v.health );
				debugMsg( "solid = %i\n", nav->entity()->v.solid );
			}
			if (nav->type()==NAV_F_DOOR || nav->type()==NAV_F_PLAT || nav->type()==NAV_F_TRAIN) 
				debugMsg( "Targetname = ", STRING(nav->entity()->v.targetname), "\n" );
			if (nav->type()==NAV_F_BUTTON) {
				debugMsg( "Target = ", STRING(nav->entity()->v.target), "\n" );
				edict_t *pTarget = FIND_ENTITY_BY_TARGETNAME(0, STRING(nav->entity()->v.target));
				if (pTarget) debugMsg( "Target1 = ", STRING(pTarget->v.classname), "\n" );
				pTarget = FIND_ENTITY_BY_TARGETNAME(pTarget, STRING(nav->entity()->v.target));
				if (pTarget) debugMsg( "Target2 = ", STRING(pTarget->v.classname), "\n" );
				pTarget = FIND_ENTITY_BY_TARGETNAME(pTarget, STRING(nav->entity()->v.target));
				if (pTarget) debugMsg( "Target3 = ", STRING(pTarget->v.classname), "\n" );
				pTarget = FIND_ENTITY_BY_TARGETNAME(pTarget, STRING(nav->entity()->v.target));
				if (pTarget) debugMsg( "Target4 = ", STRING(pTarget->v.classname), "\n" );
			}
			return;
		}
		else if (FStrEq( pcmd, "items" )) {
			Vector pos = pEntity->v.origin;
			CBaseEntity *ent = 0;
			while (ent = UTIL_FindEntityInSphere( ent, pos, 100 )) 
			{
				Vector p = (ent->pev->absmax + ent->pev->absmin) * 0.5;
				const char *iname = STRING(ent->pev->classname);
				const char *modelname = STRING(ent->pev->model);
				ALERT( at_console, "%s at (%.f, %.f, %.f), pev=%x\n", 
					iname, p.x, p.y, p.z, ent->pev );
				if (strcmp( iname, "monster_tripmine" )==0) {
					char *tmClass = (char*)ent;// rpgClass = pointer to CWeaponRPG
					char **ownerEd = (char**)(tmClass+680);
					edict_t *tmOwner = (edict_t*)(*ownerEd);
					edict_t *tmOwner2 = *((edict_t**)(ent+680));
				}
				/*if (strcmp( iname, "func_button" )==0) {
					CBaseButton *buttonClass = (CBaseButton*)ent;// rpgClass = pointer to CWeaponRPG
					buttonClass->TriggerAndWait();
					//char **targetPev = (char**)(buttonClass+500);	// (*targetPev) = pointer to Pev
					//ALERT( at_console, "   TargetPev = %x\n", (*targetPev) );
				}*/
			}
			return;
		}
		else if (FStrEq( pcmd, "botpos" )) {
			Vector p = pb->botPos();
			debugMsg( "Botpos = (%.f, %.f, %.f)\n", p.x, p.y, p.z);
			return;
		}
		else if (FStrEq( pcmd, "setbotpos" )) {
			char *stopstr;
			int id = strtol( arg1, &stopstr, 10 );
			Vector p = getNavpoint( id ).pos();
			p.z += 36;	// don't stuck bot in earth
			pb->ent->v.origin = p;
			UTIL_SetOrigin (&(pb->ent->v), p);
			pb->actualNavpoint = &(getNavpoint( id ));
			pb->actualPath = 0;
			pb->actualJourney.cancel();
			return;
		}
		else if (FStrEq( pcmd, "bottarget" )) {
			if (pb->botState==PB_ON_TOUR) {
				debugMsg( "Path-target = ");  pb->actualPath->endNav().printPos();  debugMsg( "\n" );
			}
			else if (pb->botState==PB_ROAMING) {
				debugMsg( "Roaming-target = ");  pb->roamingTarget->printPos();  debugMsg( "\n" );
			}
			return;
		}
		else if (FStrEq( pcmd, "setbottarget" )) {
			char *stopstr;
			botTarget = strtol( arg1, &stopstr, 10 );
			debugMsg( "Affirmative\n" );
			return;
		}
		else if (FStrEq( pcmd, "nopartner" )) {
			pb->partner = 0;
			pb->botState = PB_NO_TASK;
			return;
		}	
		else if (FStrEq( pcmd, "ptbot" )) {
			ptBotPos = map.getCellId( pEntity );
			return;
		}	
		else if (FStrEq( pcmd, "ptcover" )) {
			short plId = map.getCellId( pEntity );
			int pl = map.getPathToCover( ptBotPos, plId, path );
			Vector start = map.cell( ptBotPos ).pos();
			Vector end;
			if (pl != -1) {
				for (int l=(pl-1); l>=0; l--) {
					end = map.cell( path[l] ).pos();
					debugBeam( start, end, 50 );
					start = end;
				}
			}
			return;
		}	
		else if (FStrEq( pcmd, "ptattack" )) {
			short plId = map.getCellId( pEntity );
			int pl = map.getPathToAttack( ptBotPos, plId, path );
			Vector start = map.cell( ptBotPos ).pos();
			Vector end;
			if (pl != -1) {
				for (int l=(pl-1); l>=0; l--) {
					end = map.cell( path[l] ).pos();
					debugBeam( start, end, 50 );
					start = end;
				}
			}
			return;
		}
		else if (FStrEq( pcmd, "showcells" )) {
			if (FStrEq( arg1, "on" )) {
				visualizeCellConnections = true;
			}
			else if (FStrEq( arg1, "off" )) {
				visualizeCellConnections = false;
			}
			return;
		}
		else if (FStrEq( pcmd, "markvis" )) {
			glMarker.deleteAll();
			int homeCell = map.getCellId( pEntity->v.origin );
			if (homeCell != -1) {
				for (int i=0; i<map.numberOfCells(); i++)
					if (map.lineOfSight( homeCell, i )) {
						if (i==homeCell) glMarker.newMarker( map.cell(i).pos(), 2 );
						else glMarker.newMarker( map.cell(i).pos(), 1 );
					}
			}
			return;
		}	
		else if (FStrEq( pcmd, "markfocus" )) {
			glMarker.deleteAll();
			int homeCell = map.getCellId( pEntity->v.origin );
			if (homeCell != -1) {
				UTIL_MakeVectors( pEntity->v.v_angle );
				Vector dir = gpGlobals->v_forward;
				for (int i=0; i<map.numberOfCells(); i++)
					if (map.lineOfSight( homeCell, i )) {
						if (map.cell(i).focus.forDir( dir ) > 5) 
							glMarker.newMarker( map.cell(i).pos(), 1 );
						else 
							glMarker.newMarker( map.cell(i).pos(), 2 );
					}
			}
			return;
		}	
		else if (FStrEq( pcmd, "markenv" )) {
			char *thresh = (char*) arg1;
			float fThresh = atof( thresh );
			glMarker.deleteAll();
			for (int i=0; i<map.numberOfCells(); i++) {
				if (map.cell(i).getEnvDamage() > fThresh) {
					glMarker.newMarker( map.cell(i).pos(), 1 );
				}
			}
			return; 
		}	
		else if (FStrEq( pcmd, "savemap" )) {
			saveLevelData();
			ClientPrint( VARS(pEntity), HUD_PRINTNOTIFY, "Map data saved.\n" );
			return;
		}
		else if (FStrEq( pcmd, "delmarkers" )) {
			glMarker.deleteAll();
			return;
		}	
		else if (FStrEq( pcmd, "numclients" )) {
			debugMsg( "Number of clients = %i\n", numberOfClients );
			return;
		}
// end of debug mode commands...
#endif

	}
	
	(*other_gFunctionTable.pfnClientCommand)(pEntity);
}

