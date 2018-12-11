// Based on:
//
// HPB bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// dll.cpp
//

#include "parabot.h"
#include "meta_api.h"
#include "bot.h"
#include "bot_func.h"
#include "chat.h"
#include "sectors.h"
#include "vistable.h"
#include "cell.h"
#include "pb_mapcells.h"
#include "configuration.h"
#include "personalities.h"
#include "observer.h"
#include "dllwrap.h"
#include "sounds.h"
#include "menu.h"
#include "utilityfuncs.h"

void BotPause();
void BotUnPause();

extern bool fatalParabotError;
extern bot_t bots[32];
extern int activeBot;
extern int botNr;

char g_argv[256];
extern int mod_id;

int botTarget;
bool oldBotStop;
extern bool pb_pause;
extern float observerUpdate;
//#include "hl_game.h"
//extern HL_Game game;
void sendWelcomeToNewClients();
void checkForBotRespawn();
void checkForBotCreation();
void checkForAirStrike();
void updateVisTable();
void cachePlayerData();
void checkForMapChange();
int numberOfClients;
extern const Vec3D zerovector;
int isFakeClientCommand;
int fake_arg_count;
float bot_check_time = 10.0;	// will be set to correct value when client connects
int welcome_index = -1;	
int wpSpriteTexture, wpBeamTexture, wpSprite2Texture;
bool visualizeCellConnections;

const CVAR *flashlight;
const CVAR *footsteps;
const CVAR *freezetime;
const CVAR *gamemode;
const CVAR *teamplay;
const CVAR *maxspeed;

const CVAR *bm_cbar;
const CVAR *bm_gluon;
const CVAR *bm_trip;

void saveLevelData();	// in pb_mapimport.cpp

static void
gameinit_wrap()
{
	char filePath[100];

	flashlight = cvar_getpointer("mp_flashlight");
	footsteps = cvar_getpointer("mp_footsteps");
	freezetime = cvar_getpointer("mp_freezetime");
	maxspeed = cvar_getpointer("sv_maxspeed");
	teamplay = cvar_getpointer("mp_teamplay");

	if (!teamplay) {
		teamplay = cvar_getpointer("mp_gameplay"); // Half-Screwed

		if (!teamplay)
			teamplay = cvar_getpointer("mp_gametype"); // Cold Ice
	}

	switch (mod_id)
	{
	case VALVE_DLL:
		// from jk_botti
		if (cvar_getpointer("bm_ver")) {
			com.gamedll_flags |= GAMEDLL_BMOD;
			bm_cbar = cvar_getpointer("bm_cbar_mod");
			bm_gluon = cvar_getpointer("bm_gluon_mod");
			bm_trip = cvar_getpointer("bm_trip_mod");
		} else if (cvar_getpointer("mp_giveweapons")
		    && cvar_getpointer("mp_giveammo"))
			com.gamedll_flags |= GAMEDLL_SEVS;
		break;
	case AG_DLL:
		gamemode = cvar_getpointer("sv_ag_gamemode");
		break;
	}

	strcpy( filePath, com.modname );
	strcat( filePath, "/addons/parabot/config/" );
	strcat( filePath, com.modname );
	strcat( filePath, "/" );
	configuration_init(filePath);
	if(!personalities_init(filePath)) {
		ERROR_MSG( "Couldn't read/write configuration files correctly. Check your write permissions and/or file contents.");
		exit(0);
	}

	// always load chatfile, might be enabled ingame:
	chat_load();

	if(com.gamedll_flags & GAMEDLL_METAMOD)
		 RETURN_META(MRES_IGNORED);

	gameinit_wrap();
}

static int
spawn_wrap(EDICT *pent)
{
	const char *pClassname = STRING(pent->v.classname);

	debugFile( "%f: DispatchSpawn: %s\n",worldtime(), pClassname );

	if (pent->v.model != 0)
		debugFile(" model=%s\n",STRING(pent->v.model));

	if (Q_STREQ (pClassname, "worldspawn")) {
         // do level initialization stuff here...
/*
		  if (speechSynthesis) {
			  // precache samples
				int i;
				for (i=0; i<chatGotKilled.size(); i++)
					precache_sound(chatGotKilled[i].text);
				for (i=0; i<chatKilledPlayer.size(); i++)
					precache_sound(chatKilledPlayer[i].text);
				for (i=0; i<chatGotWeapon.size(); i++)
					precache_sound(chatGotWeapon[i].text);
				for (i=0; i<chatReplyUnknown.size(); i++)
					precache_sound(chatReplyUnknown[i].text);
		  }
*/

	com.gamedll_flags &= ~(GAMEDLL_TEAMPLAY | GAMEDLL_DOM | GAMEDLL_CTF);

	if (mod_id == AG_DLL) {
		if (teamplay->value) {
			com.gamedll_flags |= GAMEDLL_TEAMPLAY;
		}

		if (Q_STREQ(gamemode->string, "ctf")) {
			com.gamedll_flags |= GAMEDLL_CTF;
		} else if(Q_STREQ(gamemode->string, "dom")) {
			com.gamedll_flags |= GAMEDLL_DOM;
		}
	}

           precache_sound("weapons/xbow_hit1.wav");      // waypoint add
           precache_sound("weapons/mine_activate.wav");  // waypoint delete
           precache_sound("common/wpn_hudoff.wav");      // path add/delete start
           precache_sound("common/wpn_hudon.wav");       // path add/delete done
           precache_sound("common/wpn_moveselect.wav");  // path add/delete cancel
           precache_sound("common/wpn_denyselect.wav");  // path add/delete error
           wpBeamTexture = precache_model( "sprites/lgtning.spr");
		   wpSpriteTexture = precache_model( "sprites/hotglow.spr");
		   wpSprite2Texture = precache_model( "sprites/laserdot.spr");

      }
//	  else if (Q_STREQ( pClassname, "env_sound" ) ) DEBUG_MSG( "DISPATCH env_sound\n" );
//	  else if (Q_STREQ( pClassname, "env_shake" ) ) DEBUG_MSG( "DISPATCH env_shake\n" );
//	  else if (Q_STREQ( pClassname, "env_explosion" ) ) DEBUG_MSG( "DISPATCH env_explosion\n" );
	if (!(com.gamedll_flags & GAMEDLL_METAMOD))
		return spawn(pent);

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

static void
keyvalue_wrap(EDICT *e, KEYVALUE *kv)
{
	if (mod_id == GEARBOX_DLL ) {
		if (!(com.gamedll_flags & GAMEDLL_CTF)) {
			if((Q_STREQ(kv->keyname, "classname"))
			    && (Q_STREQ(kv->value, "info_ctfdetect"))) {
				com.gamedll_flags |= GAMEDLL_CTF;
			}
		}
	}

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	keyvalue(e, kv);
}

///////////////////////////////////////////////////////////////////////////////////
//
//  CLI HANDLING
//
///////////////////////////////////////////////////////////////////////////////////
static bool
clientconnect_wrap(EDICT *player, const char *name, const char *address, void *rejectreason)
{
	bool connected;

	debugFile("%.f: ClientConnect: %s (%s)", worldtime(), STRING(player->v.netname), name);
	debugFile("ClientConnect: pent=%p name=%s\n", player, name);

	// check if this is NOT a bot joining the server...
	if (!(player->v.flags & FL_FAKECLIENT)) {
		// don't try to add bots for 10 seconds, give client time to get added
		if (bot_check_time < com.globals->time + 10.0f)
			bot_check_time = com.globals->time + 10.0f;
	}

	connected = clientconnect(player, name, address, rejectreason);

	debugFile("  OK\n");
	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
		return connected;

	RETURN_META_VALUE(MRES_SUPERCEDE, connected);
}

static void
clientdisconnect_wrap(EDICT *player)
{
	int i, index = -1;

	debugFile( "%.f: ClientDisconnect: %s ", worldtime(), STRING(player->v.netname) );

	debugFile( "ClientDisconnect: %p\n", player );

	for (i = 0; i < 32; i++) {
		if (bots[i].e == player) {
			index = i;
			break;
		}
	}
		
	if (index != -1) {	// bot is disconnecting
		DEBUG_MSG( "BOT DISCONNECT.\n" );

		bots[index].is_used = false;  // this slot is now free to use
		bots[index].e = 0;
		personalities_leave( bots[index].personality );
		delete (bots[index].parabot);
		bots[index].parabot = 0;			
	}

	debugFile( "...freeing bot" );

	numberOfClients--;

	clientdisconnect(player);

	//if (index != -1) FREE_PRIVATE( player );	// fakeclient fix by Leon Hartwig
	debugFile( "  OK\n" );
	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_SUPERCEDE);
}

static void
clientputinserver_wrap(EDICT *player)
{
	debugFile( "ClientPutInServer: %p\n", player);

	// check if this is NOT a bot joining the server...
	if (getbotindex(player) == -1) {
		// next welcome message to this client:
		if (welcome_index == -1) welcome_index = indexofedict(player);
	}
	
	numberOfClients++;
	if(com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	clientputinserver(player);
}

static void
serverdeactivate_wrap()
{
	debugFile("ServerDeactivate\n");

	if (!(com.gamedll_flags & GAMEDLL_METAMOD))
		serverdeactivate();

	saveLevelData();		// save last level's data

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);
}

static void
startframe_wrap()
{
	cachePlayerData();
					
	checkForMapChange();

	if (!fatalParabotError) {
		sounds_getAllClientSounds();
		
		print3dDebugInfo();
		
		// call BotThink for each active bot
		for (int b = 0; b < 32; b++) {
			if (bots[b].is_used && bots[b].respawn_state == RESPAWN_IDLE)
				BotThink(&bots[b]);	
		}
		
		activeBot = botNr;	// print out global debug messages
		
		if (worldtime() > observerUpdate) {
			observer_registerclients();
			observerUpdate = worldtime() + 3.0;//0.5;
			/*
			// PIA Test
			for (int pi=0; pi<game.world()->numberOfPlayers(); pi++) {
				PIA_Player *dmp = game.world()->player( pi );
				if ( dmp->isValid() && !(dmp->isBot()) ) {
					PIA_Weapon *pw = dmp->firstWeapon();
					if (pw) {
						DEBUG_MSG( "%s\n", pw->name() );
						while (pw=dmp->nextWeapon()) DEBUG_MSG( "%s\n",pw->name() );
					}
					break;
				}
			}
			*/
		}
		observer_observeall();
		updateVisTable();
		checkForAirStrike();
		sendWelcomeToNewClients();
		checkForBotRespawn();
		checkForBotCreation();
		if (!pb_pause) chat_check();
#if _DEBUG
		glMarker.drawMarkers();
#endif //_DEBUG
	}
	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	startframe();
}

static void
clientcommand_wrap(EDICT *player)
{
	const char *pcmd = cmd_argv(0);
	const char *arg1 = cmd_argv(1);
#if _DEBUG
	const char *arg2 = cmd_argv(2);
	const char *arg3 = cmd_argv(3);
	const char *arg4 = cmd_argv(4);
#endif
	// chat message analysis:
	if (Q_STREQ(pcmd, "say")) {
		chat_parsemsg( player, (char*)arg1 );	// no return!!!
	} else if ((!is_dedicatedserver()) && is_hostowner(player)) {
		// these client commands aren't allow
		// on dedicated servers and for not host owner
#if _DEBUG
		CParabot *pb = bots[botNr].parabot;
		debugFile("ClientCommand: %s %s %s\n",pcmd, arg1, arg2);
#endif
		// mapchange redefined:
		if (Q_STREQ( pcmd, "map" )) {
			char *newmap = (char*) arg1;
			if(!is_mapvalid(newmap)) {
				DEBUG_MSG( "Map not valid!\n" );
			} else {
				debugFile( "Changing map...\n" );
				FakeClientCommand( player, "hideconsole", 0, 0 );
				changelevel(newmap, NULL);
			}
			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ(pcmd, "botmenu")) {
			// bot commands:
			oldBotStop = pb_pause;
			BotPause();		// while in menu, bots don't move
			menu_main(player);
			if (!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ(pcmd, "menuselect")) {
			menu_selectitem(player, atoi(arg1));
			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ(pcmd, "add_bot")) {
			BotCreate();
			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "botstop" )) {
			BotPause();
			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "botgo" )) {
			BotUnPause();
			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		}

// rest only for debugmode!
#if _DEBUG
		else if (Q_STREQ( pcmd, "dbgfile" )) {
			if (Q_STREQ( arg1, "on")) {
				com.gamedll_flags |= GAMEDLL_DEBUG;
			} else if (Q_STREQ( arg1, "off")) {
				com.gamedll_flags &= ~ GAMEDLL_DEBUG;
			}

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ(pcmd, "debugTrace")) {
			PB_Action at;
			PB_Roaming rt;
			at.init( player );
			rt.init( player, &at );
			UTIL_MakeVectors(player->v.v_angle);
			Vector pos = pEntity->v.origin + 200.0f * com.globals->v_forward;
			at.setMoveAngle( pEntity->v.v_angle );
			rt.checkWay( pos );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ(pcmd, "setstep")) {
			char *newSample = (char*) arg1;
			strcpy( stepSample, newSample );
			sprintf( stepSound, "player/pl_%s%i.wav", stepSample, stepNr );
			DEBUG_MSG( "New stepSound = %s\n", stepSound );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ(pcmd, "setstepnr")) {
			char *newNr = (char*) arg1;
			stepNr = atoi( newNr );
			sprintf( stepSound, "player/pl_%s%i.wav", stepSample, stepNr );
			DEBUG_MSG( "New stepSound = %s\n", stepSound );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ(pcmd, "setvol")) {
			char *newVol = (char*) arg1;
			int iVol = atoi( newVol );
			stepVol = (float)iVol;	stepVol/=100;
			if (stepVol > 1.0) stepVol = 1.0;
			DEBUG_MSG( "New stepVolume = %.2f\n", stepVol );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META(MRES_IGNORED);
		} else if (Q_STREQ(pcmd, "playstep")) {
			//EMIT_SOUND( pb->ent, CHAN_BODY, stepSound, stepVol, ATTN_NORM);
			pfnEmitSound( pb->ent, CHAN_BODY, stepSound, stepVol, ATTN_NORM, 0, 100 );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ(pcmd, "playsound")) {
			char *sound = (char*) arg1;
			pfnEmitSound( pEntity, CHAN_BODY, sound, 1.0, ATTN_NORM, 0, 100 );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ(pcmd, "debugbreak")) {
			char *reason = (char*) arg1;
			if ( Q_STREQ(reason, "weapon") ) botHalt = BREAK_WEAPON;
			else if ( Q_STREQ(reason, "goals") ) botHalt = BREAK_GOALS;

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "setbotnr" )) {
			char *stopstr;
			botNr = strtol( arg1, &stopstr, 10 );
			DEBUG_MSG( "OK.\n" );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "botdist" )) {
			Vector p = pb->ent->v.origin - player->v.origin;
			DEBUG_MSG( "Distance = %.f\n", p.Length() );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "pathinfo" )) {
			if (pb->actualPath) {
				DEBUG_MSG( "Actual "); pb->actualPath->print(); DEBUG_MSG( "\n" );
			}

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "wpinfo" )) {
			if (pb->actualPath) {
				Vector p = pb->waypoint.pos();
				DEBUG_MSG( "Heading for waypoint (%.f, %.f, %.f)\n", p.x, p.y, p.z);
			}

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "ground" )) {
			assert( player != 0 );
			if (player->v.groundentity==0) {
				DEBUG_MSG( "No ground entity!\n" );

				if(!(com.gamedll_flags & GAMEDLL_METAMOD))
					return;

				RETURN_META( MRES_IGNORED );
			}
			lastGround = player->v.groundentity;
			assert( lastGround != 0 );
			Vector p = lastGround->v.absmin;
			ALERT( at_console, "%s, min at(%.f, %.f, %.f), ", STRING(lastGround->v.classname), p.x, p.y, p.z);
			p = lastGround->v.absmax;
			ALERT( at_console, "max at(%.f, %.f, %.f), solid=%i\n", p.x, p.y, p.z,
				lastGround->v.solid );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "lastground" )) {
			if (lastGround==0) {
				DEBUG_MSG( "No ground entity!\n" );

				if(!(com.gamedll_flags & GAMEDLL_METAMOD))
					return;

				RETURN_META( MRES_IGNORED );
			}
			Vector p = lastGround->v.absmin;
			ALERT( at_console, "%s, min at(%.f, %.f, %.f), ", STRING(lastGround->v.classname), p.x, p.y, p.z);
			p = lastGround->v.absmax;
			ALERT( at_console, "max at(%.f, %.f, %.f), solid=%i\n", p.x, p.y, p.z,
				lastGround->v.solid );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "navinfo" ) && (pb->actualNavpoint)) {
			DEBUG_MSG( "Actual navpoint is "); pb->actualPath->print();
			DEBUG_MSG( ", Linkage=%i\n", mapGraph.linkedNavpointsFrom( pb->actualNavpoint ) );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "playernav" )) {
			PB_Navpoint *nav = mapGraph.getNearestNavpoint( player->v.origin );
			//PB_Navpoint::Nav2Classname( str, nav->type() );
			Vec3D p = player->v.origin;
			DEBUG_MSG( "Player at (%.f, %.f, %.f), ", p.x, p.y, p.z);
			p = nav->pos();
			DEBUG_MSG( "nearest navpoint is %s", nav->classname() );
			DEBUG_MSG( " id=%i", nav->id() );
			DEBUG_MSG( " at (%.f, %.f, %.f)\n", p.x, p.y, p.z);
			DEBUG_MSG( "Linkage=%i\n", mapGraph.linkedNavpointsFrom( nav ) );
			if (nav->reached( player )) DEBUG_MSG( "Navpoint reached\n" );
			if (nav->entity()) {
				DEBUG_MSG( "NavHealth = %.f, solid = %i\n", nav->entity()->v.health, nav->entity()->v.solid );
			}
			if (nav->type()==NAV_F_DOOR || nav->type()==NAV_F_PLAT || nav->type()==NAV_F_TRAIN) 
				DEBUG_MSG( "Targetname = %s\n", STRING(nav->entity()->v.targetname) );
			if (nav->type()==NAV_F_BUTTON) {
				DEBUG_MSG( "Target = %s\n", STRING(nav->entity()->v.target) );
				EDICT *pTarget = FIND_ITY_BY_TARGETNAME(0, STRING(nav->entity()->v.target));
				if (pTarget) DEBUG_MSG( "Target1 = %s\n", STRING(pTarget->v.classname) );
				pTarget = FIND_ITY_BY_TARGETNAME(pTarget, STRING(nav->entity()->v.target));
				if (pTarget) DEBUG_MSG( "Target2 = %s\n", STRING(pTarget->v.classname) );
				pTarget = FIND_ITY_BY_TARGETNAME(pTarget, STRING(nav->entity()->v.target));
				if (pTarget) DEBUG_MSG( "Target3 = %s\n", STRING(pTarget->v.classname) );
				pTarget = FIND_ITY_BY_TARGETNAME(pTarget, STRING(nav->entity()->v.target));
				if (pTarget) DEBUG_MSG( "Target4 = %s\n", STRING(pTarget->v.classname) );
			}

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "items" )) {
			Vec3D pos = player->v.origin;
			EDICT *ent = 0;
			while (FNullEnt(ent = FIND_ITY_IN_SPHERE( ent, pos, 100 ))) {
				Vec3D p;
				boxcenter(ent, p);
				const char *iname = STRING(ent->v.classname);
				const char *modelname = STRING(ent->v.model);
				INFO_MSG( "%s at (%.f, %.f, %.f), pev=%x\n", 
					iname, p.x, p.y, p.z, &ent->v );
				/*if (Q_STREQ( iname, "monster_tripmine" )) {
					char *tmClass = (char*)ent;// rpgClass = pointer to CWeaponRPG
					char **ownerEd = (char**)(tmClass+680);
					EDICT *tmOwner = (EDICT*)(*ownerEd);
					EDICT *tmOwner2 = *((EDICT**)(ent+680));
				}*/
				/*if (FstrEq( iname, "func_button" )) {
					CBaseButton *buttonClass = (CBaseButton*)ent;// rpgClass = pointer to CWeaponRPG
					buttonClass->TriggerAndWait();
					//char **targetPev = (char**)(buttonClass+500);	// (*targetPev) = pointer to Pev
					//ALERT( at_console, "   TargetPev = %x\n", (*targetPev) );
				}*/
			}

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "botpos" )) {
			Vector p = pb->botPos();
			DEBUG_MSG( "Botpos = (%.f, %.f, %.f)\n", p.x, p.y, p.z);

			if(!(com.gamedll_flags, GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "setbotpos" )) {
			char *stopstr;
			int id = strtol( arg1, &stopstr, 10 );
			Vector p = getNavpoint( id ).pos();
			p.z += 36;	// don't stuck bot in earth
			pb->ent->v.origin = p;
			UTIL_SetOrigin (&(pb->ent->v), p);
			pb->actualNavpoint = &(getNavpoint( id ));
			pb->actualPath = 0;
			pb->actualJourney.cancel();

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "bottarget" )) {
			if (pb->botState==PB_ON_TOUR) {
				DEBUG_MSG( "Path-target = ");  pb->actualPath->endNav().printPos();  DEBUG_MSG( "\n" );
			} else if (pb->botState==PB_ROAMING) {
				DEBUG_MSG( "Roaming-target = ");  pb->roamingTarget->printPos();  DEBUG_MSG( "\n" );
			}

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "setbottarget" )) {
			char *stopstr;
			botTarget = strtol( arg1, &stopstr, 10 );
			DEBUG_MSG( "Affirmative\n" );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "nopartner" )) {
			pb->partner = 0;
			pb->botState = PB_NO_TASK;

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "ptbot" )) {
			ptBotPos = map.getCellId( player );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "ptcover" )) {
			short plId = map.getCellId( player );
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

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "ptattack" )) {
			short plId = map.getCellId( player );
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

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "showcells" )) {
			if (Q_STREQ( arg1, "on" )) {
				visualizeCellConnections = true;
			} else if (Q_STREQ( arg1, "off" )) {
				visualizeCellConnections = false;
			}

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "markvis" )) {
			glMarker.deleteAll();
			int homeCell = map.getCellId( player->v.origin );
			if (homeCell != -1) {
				for (int i = 0; i < map.numberOfCells(); i++)
					if (map.lineOfSight( homeCell, i )) {
						if (i==homeCell) glMarker.newMarker( map.cell(i).pos(), 2 );
						else glMarker.newMarker( map.cell(i).pos(), 1 );
					}
			}

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "markfocus" )) {
			glMarker.deleteAll();
			int homeCell = map.getCellId( player->v.origin );
			if (homeCell != -1) {
				makevectors( player->v.v_angle );
				Vector dir = com.globals->v_forward;
				for (int i=0; i<map.numberOfCells(); i++)
					if (map.lineOfSight( homeCell, i )) {
						if (map.cell(i).focus.forDir( dir ) > 5) 
							glMarker.newMarker( map.cell(i).pos(), 1 );
						else 
							glMarker.newMarker( map.cell(i).pos(), 2 );
					}
			}

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "markenv" )) {
			char *thresh = (char*) arg1;
			float fThresh = atof( thresh );
			glMarker.deleteAll();
			for (int i=0; i<map.numberOfCells(); i++) {
				if (map.cell(i).getEnvDamage() > fThresh) {
					glMarker.newMarker( map.cell(i).pos(), 1 );
				}
			}

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "savemap" )) {
			saveLevelData();
			INFO_MSG( "Map data saved.\n" );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "delmarkers" )) {
			glMarker.deleteAll();

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META( MRES_IGNORED );
		} else if (Q_STREQ( pcmd, "numclients" )) {
			DEBUG_MSG( "Number of clients = %i\n", numberOfClients );

			if(!(com.gamedll_flags & GAMEDLL_METAMOD))
				return;

			RETURN_META(MRES_IGNORED);
		}
// end of debug mode commands...
#endif
	}

	if(com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	clientcommand(player);
}

extern "C" EXPORT int GetEntityAPI(SERVERFUNCS *functiontable, int interfaceversion)
{
	// check if engine's pointer is valid...
	if(!functiontable)
		return false;

	memset(functiontable, 0, sizeof(SERVERFUNCS));

	if(!(com.gamedll_flags & GAMEDLL_METAMOD)) {
		// pass other DLLs engine callbacks to function table
		com.gamedll->funcs = (SERVERFUNCS *)malloc(sizeof(SERVERFUNCS));
		if(!(*(GETENTITYAPI)GetProcAddress(com.gamedll_handle, "GetEntityAPI")) (com.gamedll->funcs, interfaceversion))
			return false;  // error initializing function table!!!

		memcpy(functiontable, com.gamedll->funcs, sizeof(SERVERFUNCS));
	}

	functiontable->GameInit = gameinit_wrap;
	functiontable->Spawn = spawn_wrap;
	functiontable->KeyValue = keyvalue_wrap;
	functiontable->ClientConnect = clientconnect_wrap;
	functiontable->ClientDisconnect = clientdisconnect_wrap;
	functiontable->ClientPutinServer = clientputinserver_wrap;
	functiontable->ServerDeactivate = serverdeactivate_wrap;
	functiontable->StartFrame = startframe_wrap;
	functiontable->ClientCommand = clientcommand_wrap;

	return true;
}

extern "C" EXPORT int GetNewDLLFunctions(SERVERFUNCS2 *functiontable, int *interfaceversion)
{ 
	static GETNEWDLLFUNCTIONS other_GetNewDLLFunctions = NULL;
	static bool missing = false;

	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
		return true;

	// if the new DLL functions interface has been formerly reported as missing, give up
	if(missing)
		return false;

	// do we NOT know if the new DLL functions interface is provided ? if so, look for its address
	if (!other_GetNewDLLFunctions)
		other_GetNewDLLFunctions = (GETNEWDLLFUNCTIONS)GetProcAddress(com.gamedll_handle, "GetNewDLLFunctions");

	// have we NOT found it ?
	if (!other_GetNewDLLFunctions) {
		missing = true; // then mark it as missing, no use to look for it again in the future
		return false; // and give up
	}

	com.gamedll->newfuncs = functiontable;

	// else call the function that provides the new DLL functions interface on request
	return (!(*other_GetNewDLLFunctions)(functiontable, interfaceversion));
}

extern "C" EXPORT int Server_GetBlendingInterface( int version, struct sv_blending_interface **ppinterface, struct engine_studio_api *pstudio, void *rotationmatrix, void *bonetransform)
{
	static SERVER_GETBLENDINGINTERFACE other_Server_GetBlendingInterface = NULL;
	static bool missing = false;

	// if the blending interface has been formerly reported as missing, give up
	if (missing)
		return false;

	// do we NOT know if the blending interface is provided ? if so, look for its address
	if (other_Server_GetBlendingInterface == NULL)
		other_Server_GetBlendingInterface = (SERVER_GETBLENDINGINTERFACE) GetProcAddress (com.gamedll_handle, "Server_GetBlendingInterface");

	// have we NOT found it ?
	if (!other_Server_GetBlendingInterface)
	{
		missing = true; // then mark it as missing, no use to look for it again in the future
		return false; // and give up
	}

	// else call the function that provides the blending interface on request
	return ((other_Server_GetBlendingInterface) (version, ppinterface, pstudio, rotationmatrix, bonetransform));
}

void FakeClientCommand(EDICT *pBot, const char *arg1, const char *arg2, const char *arg3)
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
	clientcommand(pBot);

   isFakeClientCommand = 0;
}
