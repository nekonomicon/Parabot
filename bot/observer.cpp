#include "parabot.h"
#include "observer.h"
#include "sectors.h"
#include "kills.h"
#include "focus.h"
#include "pb_mapgraph.h"
#include "vistable.h"
#include "cell.h"
#include "mapcells.h"
#include "bot.h"
#include "sounds.h"


#define MIN_WAYPOINT_DIST		 64		// distance between waypoints
#define CRITICAL_FALL_VELOCITY	550		// more velocity causes damage
#define MIN_CAMP_TIME			5.0		// seconds to be idle for camping


extern PB_MapGraph mapGraph;	// mapgraph for waypoints

extern int mod_id;
extern int clientWeapon[32];
int playerNr = 0;
extern float roundStartTime;
extern bool visualizeCellConnections;
extern bot_t bots[32];

static OBSERVER obs[32];
NAVPOINT *getNearestNavpoint( EDICT *pEdict );


/*
PB_Observer::PB_Observer()
{
	init();
}


PB_Observer::~PB_Observer()
{
	init();
}
*/

static void
observer_clear(int oId)
// clears all records for observer oid and sets active to false
{
	obs[oId].active = false;
	
	for (int i = 0; i < MAX_WPTS; i++) {
		obs[oId].waypoint[i].reset();
	}
#if _DEBUG
	trail[oId].deleteAll();
	while (!markerId[oId].empty()) markerId[oId].pop();	
#endif //_DEBUG
}

void
observer_init()
{
	// DEBUG_MSG( "Init all observation-records\n" );
	for (int i = 0; i < MAX_OBS; i++) {
		observer_clear(i);
		obs[i].player = 0;
	}
}

static void
observer_startobservation(int oId)
{
	observer_clear(oId);	// clear linked lists for markers etc.

	obs[oId].active = true;
	
	obs[oId].leadwaypoint = -1;
	obs[oId].lastplatid = -1;
	obs[oId].platform = 0;
	obs[oId].lastreachednav = 0;

	assert(obs[oId].player != 0);
	vcopy(&obs[oId].player->v.origin, &obs[oId].lastframepos);
	vcopy(&obs[oId].player->v.velocity, &obs[oId].lastframevel);
	obs[oId].frags = obs[oId].player->v.frags;
	obs[oId].health = obs[oId].player->v.health;

	obs[oId].jumppressed = false;
	obs[oId].usepressed = false;
	obs[oId].incombat = false;
	obs[oId].falltime = 0;

	obs[oId].currentcell = NO_CELL_FOUND;
	obs[oId].lastcell = NO_CELL_FOUND;
	obs[oId].lastcelltime = 0;
	
	observer_addwaypoint(oId, &obs[oId].player->v.origin);	// add first waypoint
}

static int
observer_registerplayer(EDICT *player)
{
	// search free slot
	int i = 0;
	while (obs[i].player != 0 && i < MAX_OBS) i++;
	if (i == MAX_OBS) {
		/*FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
		fprintf( dfp, "No free slot in PB_Observer::registerPlayer()!\n" ); 
		fclose( dfp );*/
		i = MAX_OBS - 1;
	}
	obs[i].player = player;
	if (obs[i].player != 0) {
		// init vars:
		observer_startobservation(i);
		DEBUG_MSG("PB_Observer registered %s\n", STRING(obs[i].player->v.netname));
		return i;
	}
	return -1;
}

int
observer_playerid(EDICT *player)
// returns the observer id of player
{
	for (int i = 0; i < MAX_OBS; i++)
		if (obs[i].player == player)
			return i;

	return observer_registerplayer(player);	// if not in list, register for observing
}

void
observer_registerclients()
{
	EDICT *player;

	// DEBUG_MSG("PB_Observer::registerClients()\n");
	for (int i = 1; i <= com.globals->maxclients; i++) {
		player = edictofindex(i);
		if (!playerexists(player)
		    || !is_alive(player))
			continue;

		observer_playerid(player);		// register if not observing yet
	}
}

static int
observer_checkground(int oId, EDICT **plat)
// checks if bot is standing on a ladder or a moving platform, in latter case adds
// the necessary information to the path
// returns the corresponding Waypoint-Flag (WP_ON_LADDER, WP_ON_PLATFORM or 0)
// if WP_ON_PLATFORM is set, plat returns this platform, needsTrigger if it has to be activated
{
	*plat = 0;

	int flags = 0;
	
	if (is_onladder(obs[oId].player))
		flags |= WP_ON_LADDER;

	EDICT *ground = obs[oId].player->v.groundentity;
	if (ground) {
		const char *groundName = STRING( ground->v.classname );
		if (!Q_STREQ(groundName, "worldspawn")) {
			// we are walking on special ground, check if it is moving:
			if ((Q_STREQ( groundName, "func_door")) ||
				 (Q_STREQ( groundName, "func_plat" ) ) ||
				 (Q_STREQ( groundName, "func_train" ) )   ) 
			{
				// try to find the corresponding navpoint for this groundentity:
				int sId = obs[oId].lastplatid;
				if (sId >= 0) {	// have we got the right one?
					if (navpoint_entity(getNavpoint(sId)) != ground)
						sId = -1;
				}
				if (sId < 0)
					sId = getNavpointIndex(ground);
				if (sId < 0) {
					DEBUG_MSG("ERROR: Couldn't find navpoint for func %s!\n", STRING(ground->v.classname));
				} else {
					// navpoint found, adding necessary information to path:
					flags |= WP_ON_PLATFORM;
					if (obs[oId].platform==0) flags |= WP_AT_PLATFORM_START;
					obs[oId].lastplatid = sId;
					*plat = ground;
//					if (observedPath[oId].isRecording()) 
//						observedPath[oId].addPlatformInfo( sId, ground->v.absmin );
					// DEBUG_MSG( "Added Platform Info!\n" );
					// check if the platform needs a triggering to be valid
					NAVPOINT *platNav = getNavpoint( sId );
					if (navpoint_needstriggering(platNav) && navpoint_istriggered(platNav)) {
						flags |= WP_PLAT_NEEDS_TRIGGER;
#if _DEBUG
						debugsound((obs[oId].player), "weapons/mine_activate.wav");
						DEBUG_MSG("Plat needs triggering by ");
						for (int ni = 0; ni < mapGraph.numberOfNavpoints(); ni++) {
							NAVPOINT t = getNavpoint( ni );
							if (t.isTriggerFor(platNav)) {
								t.print();
								DEBUG_MSG("\n");
							}
						}
#endif
					}
				}
				obs[oId].platform = ground;
			}
			/*else {
				DEBUG_MSG( "Walking unknown ground %s", groundName );
			}*/
		} else {
			// we are on normal ground, check if we just left a platform:
			if (obs[oId].platform) {
				// try to find the corresponding navpoint for this platform:
				int sId = obs[oId].lastplatid;
				if (sId >= 0) {	// have we got the right one?
					if (navpoint_entity(getNavpoint(sId)) != obs[oId].platform) sId = -1;
				}
				if (sId < 0)
					sId = getNavpointIndex(obs[oId].platform);
				if (sId < 0) {
					DEBUG_MSG("ERROR: Couldn't find navpoint for func %s!\n", STRING(obs[oId].platform->v.classname));
				} else {
					// navpoint found, setting flag for waiting
					flags |= (WP_ON_PLATFORM | WP_AT_PLATFORM_END);
					*plat = obs[oId].platform;
					// DEBUG_MSG( "Added Platform End!\n" );
				}
			}
			obs[oId].platform = 0;
		}
	}	
	
	return flags;
}

static bool
observer_shouldobserveplayer(int oId)
// determines if player oId should be observed or not and returns the result
{
	if (obs[oId].player != 0) {
		if (!obs[oId].player->pvPrivateData) {
			// player disconnected
			obs[oId].active = false;
			obs[oId].player = 0;
			return false;
		}
	} else {
		// player disconnected
		obs[oId].active = false;
		return false;
	}

	if (obs[oId].active) {	// check if observation has to stop...
		assert(obs[oId].player != 0);
		if ((obs[oId].player->v.health < 1) || 
		    (obs[oId].player->v.solid == SOLID_NOT)) {
			// ...yes -> mark inactive
			// DEBUG_MSG( "Stopping observation\n" );
			obs[oId].active = false;
		}
	} else if (obs[oId].player != 0) {	// check if observation has to continue...
		if ((obs[oId].player->v.health >= 1)	&& 
		    (obs[oId].player->v.solid != SOLID_NOT)) {
			// DEBUG_MSG("Continue observation...\n");
			observer_startobservation(oId);	// sets active[i] to true
		}
	}
	return obs[oId].active;
}

static int
observer_getstartindex(int oId, NAVPOINT *endNav)
{
	#define MAX_TRIGGERS 16

	int currentIndex = obs[oId].leadwaypoint;
	int lastIndex = currentIndex+1;  if (lastIndex==MAX_WPTS) lastIndex = 0;
	int foundIndex = -1;

	int triggerCount = 0;
	NAVPOINT *trigger[MAX_TRIGGERS] = {0,};
	bool triggerOk[MAX_TRIGGERS] = {false,};
	if (navpoint_needstriggering(endNav)) {
		debugSound( (obs[oId].player), "weapons/mine_activate.wav" );
		trigger[triggerCount] = endNav;	
		triggerOk[triggerCount] = false;
		triggerCount++;
	}
	
	int dbgCnt1 = 0;
	while (foundIndex < 0 && currentIndex != lastIndex && dbgCnt1++ < 1000) {
		int dbgCnt2 = 0;
		while (!obs[oId].waypoint[currentIndex].isNavpoint() && currentIndex != lastIndex && dbgCnt2++ < 1000) {
			if (obs[oId].waypoint[currentIndex].needsTriggerForPlat()) {
				//NAVPOINT *plat = &mapGraph[obs[oId].platinfo[currentIndex].data.navId].first;
				NAVPOINT *plat = getNavpoint(obs[oId].platinfo[currentIndex].data.navId);
				bool platRegistered = false;
				for (int i = (triggerCount - 1); i >= 0; i--) {
					if (trigger[i] == plat) {
						platRegistered = true;
						break;
					}
				}
				if (!platRegistered) {
					trigger[triggerCount] = plat;
					triggerOk[triggerCount] = false;
					triggerCount++;
				}
			}
			currentIndex--;
			if (currentIndex < 0)
				currentIndex = MAX_WPTS - 1;
		}
		if (obs[oId].waypoint[currentIndex].isNavpoint()) {
			NAVPOINT *currentNav = mapGraph.getNearestNavpoint(&obs[oId].waypoint[currentIndex].data.pos);
			bool allTriggersOk = true;
			for (int i = 0; i < triggerCount; i++) {
				if (navpoint_istriggerfor(currentNav, trigger[i]))
					triggerOk[i] = true;
				else if (!triggerOk[i])
					allTriggersOk = false;
			}
			if (allTriggersOk) {
				foundIndex = currentIndex;
			} else if (currentIndex != lastIndex) {
				currentIndex--;
				if (currentIndex < 0)
					currentIndex = MAX_WPTS - 1;
			}
		}
		/*if (dbgCnt2==1000 || dbgCnt1==999) {
			FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
			if (dbgCnt2==1000) fprintf( dfp, "Too many loops2 in PB_Observer::getStartIndex()!\n" ); 
			else fprintf( dfp, "Too many loops1 in PB_Observer::getStartIndex()!\n" ); 
			fclose( dfp );
		}*/
	}
	return foundIndex;
}

static void
observer_newnavpointreached(int oId, Vec3D *pos, NAVPOINT *endNav)
{
	Vec3D dir;
	PB_Path newPath;
	bool passingPlatform = false;
	bool passingTeleporter = false;

	// have we been beamed and need a hint-waypoint for the teleporter?
	if (navpoint_type(endNav) == NAV_INFO_TELEPORT_DEST) {
		vsub(&obs[oId].lastframepos, pos, &dir);
		if(vlen(&dir) > 100) {
			// add waypoint pointing in right direction
			Vec3D hintPos;
			vma(&obs[oId].lastframepos, 2.0f, &obs[oId].lastframevel, &hintPos);
			observer_addwaypoint(oId, &hintPos, WP_TELEPORTER_HINT);
			DEBUG_MSG("adding hintpoint\n");
			passingTeleporter = true;
		}
	}

	// only add new paths for human clients:
	if (!(obs[oId].player->v.flags & FL_FAKECLIENT)) {
		// search beginning of path
		int startIndex = observer_getstartindex( oId, endNav );
		if (startIndex != -1) {	// beginning of this path was recorded
			int pathMode = PATH_NORMAL;
			NAVPOINT *startNav = mapGraph.getNearestNavpoint(&obs[oId].waypoint[startIndex].data.pos);
			newPath.startRecord( navpoint_id(startNav), obs[oId].waypoint[startIndex].data.arrival );
			int currentIndex = startIndex;
			int dbgCnt = 0;
			while (currentIndex != obs[oId].leadwaypoint && dbgCnt++ < 1000) {
				currentIndex++;
				if (currentIndex == MAX_WPTS)
					currentIndex = 0;
				newPath.addWaypoint(&obs[oId].waypoint[currentIndex].data.pos, 
									 obs[oId].waypoint[currentIndex].data.act, 
									 obs[oId].waypoint[currentIndex].data.arrival );
				if (obs[oId].waypoint[currentIndex].isOnPlatform()) {
					passingPlatform = true;
					newPath.addPlatformInfo(obs[oId].platinfo[currentIndex].data.navId,
					&obs[oId].platinfo[currentIndex].data.pos);
				}
				if (obs[oId].waypoint[currentIndex].action() == BOT_LONGJUMP) 
					pathMode |= PATH_NEED_LONGJUMP;
				if (obs[oId].waypoint[currentIndex].causedDamage() )
					pathMode |= PATH_CAUSES_DAMAGE;
			}
			/*if (dbgCnt == 1000) {
				FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
				fprintf( dfp, "Too many loops in PB_Observer::newNavpointReached()!\n" ); 
				fclose( dfp );
			}*/
			if (pathMode & PATH_NEED_LONGJUMP) DEBUG_MSG( "PATH NEEDS LONGJUMP!\n" );
			if (pathMode & PATH_CAUSES_DAMAGE) DEBUG_MSG( "PATH CAUSES DAMAGE!\n" );
			newPath.stopRecord(navpoint_id(endNav), worldtime(), pathMode );
			// don't add return paths for teleporters and platforms
			if (passingPlatform || passingTeleporter) 
				mapGraph.addIfImprovement( newPath, false );
			else
				mapGraph.addIfImprovement( newPath );
		}

		// bots report visits in their movement-code, so this is for players only:
		navpoint_reportvisit(endNav, obs[oId].player, worldtime() );
	}

	// add this navpoint to waypoint-list:
	observer_addwaypoint( oId, navpoint_pos(endNav), WP_IS_NAVPOINT, 2 );				
	obs[oId].lastreachednav = endNav;	
}

static void
observer_checkforjump(int oId, Vec3D *pos)
{
	if (obs[oId].jumppressed && !(obs[oId].player->v.button & ACTION_JUMP)) {
		obs[oId].jumppressed = false;		// jump ended
	} else if (!obs[oId].jumppressed && (obs[oId].player->v.button & ACTION_JUMP)) {
		obs[oId].jumppressed = true;		// jump started

		int jumpType = 1;

		// it may still be a longjump, so better check:
		if ((obs[oId].player->v.button & ACTION_CROUCH) &&
		    (vlen(&obs[oId].player->v.velocity) > 500))
			jumpType = 2;

		//int flags = checkGround( oId );

		if (jumpType == 1) {
			// normal jump, check if bot should stop before jumping:
			if (vlen2d((Vec2D *)&obs[oId].player->v.velocity) < 50) {
				observer_addwaypoint( oId, pos, BOT_DELAYED_JUMP );
				// DEBUG_MSG( "Stored delayed jump!\n" );
			} else {
				observer_addwaypoint( oId, pos, BOT_JUMP );
				// DEBUG_MSG( "Stored Jump\n" );
			}
		} else if (jumpType == 2) {
			// longjump has to start earlier:
			vma(pos, -getframerate(), &obs[oId].player->v.velocity, pos);
			//glMarker.newMarker( pos, 1 );
			observer_addwaypoint( oId, pos, BOT_LONGJUMP );
			// DEBUG_MSG( "Stored Longjump=\n" );
		}
	}
}

static void
observer_checkforuse(int oId, Vec3D *pos)
{
	if (obs[oId].usepressed && !(obs[oId].player->v.button & ACTION_USE)) {
		obs[oId].usepressed = false;		// use ended
	} else if (!obs[oId].usepressed && (obs[oId].player->v.button & ACTION_USE)) {
		obs[oId].usepressed = true;		// use started
		
		int navType = -1; 
		if (obs[oId].lastreachednav) navType = navpoint_type(obs[oId].lastreachednav);
		// don't store charger uses
		if (navType == NAV_F_BUTTON || navType == NAV_F_ROT_BUTTON) {
			//int flags = checkGround( oId );
			// assume that player has used a button
			NAVPOINT *nearestButton;
			if (navType == NAV_F_BUTTON)
				nearestButton = mapGraph.getNearestNavpoint( pos, NAV_F_BUTTON );
			else
				nearestButton = mapGraph.getNearestNavpoint( pos, NAV_F_ROT_BUTTON );
			if (nearestButton) {	// TODO: check if looking at button!!!
				Vec3D usedItemPos;
				vcopy(navpoint_pos(nearestButton), &usedItemPos);
				observer_addwaypoint( oId, &usedItemPos, BOT_USE );
			}
		}
		// DEBUG_MSG( "Stored Use\n" );
	}
}

static void
observer_checkformove(int oId, Vec3D *pos)
{
	Vec3D dir;
	float dist;
	bool sharpTurn;

	vsub(pos, &obs[oId].lastwppos, &dir);
	dist = vlen(&dir);

	if ((dist > MIN_WAYPOINT_DIST)	// sharp turn means >90 degrees yaw change and moved >10 units:
	    || ((fabsf(anglediff(obs[oId].lastwpyaw, obs[oId].player->v.v_angle.y)) > 90)
	    && (dist > 10))) {	// distance reached or angle too big
		// DEBUG_MSG( "x" );
		if ((obs[oId].player->v.flags & FL_ONGROUND)	// on ground
			|| (obs[oId].player->v.waterlevel > 0)	// or in water
			|| (is_onladder(obs[oId].player))) {	// or on ladder
			//int flags = checkGround( oId );
			observer_addwaypoint(oId, pos, 0);
		}
	}
}

static void
observer_checkforcamping(int oId, Vec3D *pos)
{
	Vec3D dircamp, dirtank;

	if (obs[oId].player->v.frags != obs[oId].frags ) {
		if (obs[oId].player->v.frags > obs[oId].frags) {
			// player has a frag more than before...
			if ((worldtime() - obs[oId].lastwptime) > MIN_CAMP_TIME) {
				// ... and he seems to be camping
				NAVPOINT *nearestCamp = mapGraph.getNearestNavpoint( pos, NAV_S_CAMPING );
				NAVPOINT *nearestTank = mapGraph.getNearestNavpoint( pos, NAV_F_TANKCONTROLS );
				vsub(navpoint_pos(nearestCamp), pos, &dircamp);
				vsub(navpoint_pos(nearestTank), pos, &dirtank);
				// check if we know the location:
				if( ( nearestCamp && (vlen(&dircamp) < 128 ) )
				    || (nearestTank && (vlen(&dirtank) < 128 ) ) ) 
				{	// ...yes -> let bots camp longer here!
					bot_t *bot = getbotpointer( (obs[oId].player) );
					if (bot) bot->parabot->campTime = 0;	
				} else {
					// ...no -> insert new camping navpoint:
					NAVPOINT campNav;
					int angleX = (short) obs[oId].player->v.v_angle.x + 360;
					int angleY = (short) obs[oId].player->v.v_angle.y + 360;
					angleY <<= 16;
					int campAngle = angleY | angleX;
					navpoint_init(&campNav, pos, NAV_S_CAMPING, campAngle );
					mapGraph.addNavpoint(&campNav);
				}
				DEBUG_MSG( "Player is a camper!\n" );
			}
			
		}
		obs[oId].frags = obs[oId].player->v.frags;
	}
}

static void
observer_checkfortripmines(int oId, Vec3D *pos)
// check for setting up tripmine
{
	Vec3D dir;

	if (mod_id==VALVE_DLL || mod_id==AG_DLL || mod_id==HUNGER_DLL || mod_id==GEARBOX_DLL) {
		// find weapon the player is handling:
		int clientIndex = indexofedict( (obs[oId].player) ) - 1;
		assert( (clientIndex >= 0) && (clientIndex < 32) );
		int playerWeapon = clientWeapon[clientIndex];

		if (((obs[oId].player->v.button & ACTION_ATTACK1)
		    && (playerWeapon == VALVE_WEAPON_TRIPMINE))
		    || ((com.gamedll_flags & GAMEDLL_BMOD)
		    && (obs[oId].player->v.button & ACTION_ATTACK2)
		    && (playerWeapon == VALVE_WEAPON_TRIPMINE
		    || playerWeapon == VALVE_WEAPON_SNARK)) ){	// player is trying set up a tripmine, check if possible:
			makevectors(&obs[oId].player->v.v_angle);
			TRACERESULT tr;
			Vec3D startTrace;
			Vec3D endTrace;
			vadd(pos, &obs[oId].player->v.view_ofs, &startTrace);
			vma(&startTrace, 100.0f, &com.globals->fwd, &endTrace);
			trace_line(&startTrace, &endTrace, true, false, 0, &tr );
			if (tr.fraction < 1.0) {
				// ...yes -> check if we know this spot: 
				Vec3D minePos = tr.endpos;
				vcopy(&tr.endpos, &minePos);
				NAVPOINT *nearest = mapGraph.getNearestNavpoint(&minePos, NAV_S_USE_TRIPMINE);
				vsub(navpoint_pos(nearest), &minePos, &dir);
				if (nearest && (vlen(&dir) < 128) ) {
					DEBUG_MSG( "Tripmine usage stored nearby!\n" );
				} else {
					// new position for setting up a tripmine: store this!
					DEBUG_MSG( "Adding tripmine hint!\n" );
					NAVPOINT mineNav;
					navpoint_init(&mineNav, &minePos, NAV_S_USE_TRIPMINE, 0);
					mapGraph.addNavpoint(&mineNav);
				}
			} else
				DEBUG_MSG( "No wall in front!\n" );
		}
	}
}

static void
observer_checkforbuttonshot(int oId, Vec3D *pos)
// check for setting up tripmine
{
	Vec3D dir;

	if (mod_id==DMC_DLL) {
		// find weapon the player is handling:
		int clientIndex = indexofedict( (obs[oId].player) ) - 1;
		assert( (clientIndex >= 0) && (clientIndex < 32) );
		int playerWeapon = clientWeapon[clientIndex];

		if ( (obs[oId].player->v.button & ACTION_ATTACK1)
		    && (playerWeapon!=DMC_WEAPON_CROWBAR)) {	// player is shooting at something, check at what:
			makevectors(&obs[oId].player->v.v_angle);
			TRACERESULT tr;
			Vec3D startTrace;
			Vec3D endTrace;
			vadd(pos, &obs[oId].player->v.view_ofs, &startTrace);
			vma(&startTrace, 1024.0f, &com.globals->fwd, &endTrace);
			trace_line(&startTrace, &endTrace, false, false, (obs[oId].player), &tr );
			if (tr.fraction < 1.0f && tr.hit != 0) {
				const char *hitClass = STRING( tr.hit->v.classname );
				if (tr.hit->v.health > 0 && Q_STREQ(hitClass, "func_button")) {
					// player shot at button, check if navpoint is already stored nearby
					NAVPOINT *nearest = mapGraph.getNearestNavpoint( pos, NAV_S_BUTTON_SHOT );
					vsub(navpoint_pos(nearest), pos, &dir);
					if (nearest && (vlen(&dir)) < 128) {
						DEBUG_MSG( "Buttonshot stored nearby!\n" );
						// set this navpoint in path:
						observer_addwaypoint( oId, navpoint_pos(nearest), WP_IS_NAVPOINT, 2 );				
						obs[oId].lastreachednav = nearest;
					} else {
						// ...no -> add new navpoint
						DEBUG_MSG( "Adding Buttonshot!\n" );
						NAVPOINT *button = mapGraph.getNearestNavpoint(&tr.endpos, NAV_F_BUTTON);
						assert( button != 0 );
						NAVPOINT shotNav;
						navpoint_init(&shotNav, pos, NAV_S_BUTTON_SHOT, navpoint_id(button) );
						mapGraph.addNavpoint( &shotNav );
					}
				}
			}
		}
	}
}

static void
observer_checkplayerhealth(int oId)
// check if player lost health by fall-damage or attack
// sets PATH_CAUSES_DAMAGE-flag and the incombat-variable
{
	// check for fall damage
	if (obs[oId].player->v.FallVelocity > CRITICAL_FALL_VELOCITY) {
		obs[oId].falltime = worldtime();
		//DEBUG_MSG( "falling at %.f\n", observedPlayer->pev->FallVelocity);
	}
	if (obs[oId].player->v.dmg_take > 0) {
		if ((worldtime() - obs[oId].falltime) < 0.3f) {
			obs[oId].waypoint[obs[oId].leadwaypoint].data.act |= WP_DMG_OCURRED;
			//observedPathMode[oId] |= PATH_CAUSES_DAMAGE;
			//DEBUG_MSG( "Fall " );
		} else if ((obs[oId].player->v.watertype == CONTENTS_LAVA) ||
			(obs[oId].player->v.watertype == CONTENTS_SLIME)) {
			obs[oId].waypoint[obs[oId].leadwaypoint].data.act |= WP_DMG_OCURRED;
			//observedPathMode[oId] |= PATH_CAUSES_DAMAGE;
			//DEBUG_MSG( "Slime/Lava " );
		}
		obs[oId].health = obs[oId].player->v.health;
		//DEBUG_MSG( "damage\n");
	} else if (obs[oId].player->v.health < obs[oId].health) {
		// must have some cause
		obs[oId].health = obs[oId].player->v.health;
		obs[oId].incombat = true;
	}
}

void drawCube( EDICT *ent, Vec3D pos, float size );

static void
observer_updatecellinfo(int i)
{
	Vec3D obsPos, dir;
	eyepos(obs[i].player, &obsPos);
	// check if new cell has to be added:
	short obsCell = mapcells_getcellid(&obsPos);
	if (obsCell == NO_CELL_FOUND) {
		if ((obs[i].player->v.waterlevel > 0)			// or in water
		    || (is_onladder(obs[i].player)) 		// or on ladder
		    || ((obs[i].player->v.flags & FL_ONGROUND)	// on ground
		    && (pointcontents(&obsPos) != CONTENTS_SOLID))) {	// Bugfix!!!
			CELL cell;
			cell_construct(&cell, obs[i].player);
			obsCell = mapcells_addcell(cell, true, obs[i].lastcell);
		}
	}

	// init lastcell (after spawning)
	if (obs[i].lastcell == NO_CELL_FOUND) obs[i].lastcell = obsCell;

	// currentcell still holds value from last frame!
	if (obs[i].currentcell != NO_CELL_FOUND) {
		vsub(&obsPos, cell_pos2(mapcells_getcell(obs[i].currentcell)), &dir);
		float distToCurrentCell = vlen(&dir);
		vsub(&obsPos, cell_pos2(mapcells_getcell(obs[i].lastcell)), &dir);
		float distToLastCell = vlen(&dir);
		// client has reached a new cell
		if ((obs[i].lastcell != obs[i].currentcell && distToCurrentCell < 25)
		    || (distToLastCell >= 1.5 * CELL_SIZE) ) {
			// transversal from one cell to another
			float neededTime = worldtime() - obs[i].lastcelltime;
			cell_addtraffic(mapcells_getcell(obs[i].lastcell), obs[i].currentcell, neededTime);

			// set new viewdir for bots:
			int bNr = getbotindex(obs[i].player);
			if (bNr>=0) {
				Vec3D moveDir, area = {1.0f, 0.0f, 0.0f};
				action_getmovedir(&bots[bNr].parabot->action, &moveDir);
				float fd1 = (focus_cellsfordir(&area, mapcells_getcell(obs[i].currentcell)->data.sectors)
				    + 3.0f * kills_fordir(&area, mapcells_getcell(obs[i].currentcell)->data.sectors))
				    * (1.5f + moveDir.x);
				area.x = 0.0f;
				area.y = 1.0f;
				float fd2 = (focus_cellsfordir(&area, mapcells_getcell(obs[i].currentcell)->data.sectors)
				    + 3.0f * kills_fordir(&area, mapcells_getcell(obs[i].currentcell)->data.sectors))
				    * (1.5f + moveDir.y);
				area.x = -1.0f;
				area.y = 0.0f;
				float fd3 = (focus_cellsfordir(&area, mapcells_getcell(obs[i].currentcell)->data.sectors)
				    + 3.0f * kills_fordir(&area, mapcells_getcell(obs[i].currentcell)->data.sectors))
				    * (1.5f - moveDir.x);
				area.x = 0.0f;
				area.y = -1.0f;
				float fd4 = (focus_cellsfordir(&area, mapcells_getcell(obs[i].currentcell)->data.sectors)
				    + 3.0f * kills_fordir(&area, mapcells_getcell(obs[i].currentcell)->data.sectors))
				    * (1.5f - moveDir.y);

				Vec3D bp = obs[i].player->v.origin;
				vcopy(&obs[i].player->v.origin, &bp);
				if (fd1 > fd2 && fd1 > fd3 && fd1 > fd4) {
					area.x = 1000.0f;
					area.y = 0.0f;
					bots[bNr].parabot->senses.addNewArea(&area);
					//DEBUG_MSG("  VD=1  ");
				} else if (fd2 > fd1 && fd2 > fd3 && fd2 > fd4) {
					area.y = 1000.0f;
					bots[bNr].parabot->senses.addNewArea(&area);
					//DEBUG_MSG("  VD=2  ");
				} else if (fd3 > fd1 && fd3 > fd2 && fd3 > fd4) {
					area.x = -1000.0f;
					area.y = 0.0f;
					bots[bNr].parabot->senses.addNewArea(&area);
					//DEBUG_MSG("  VD=3  ");
				} else if (fd4 > fd1 && fd4 > fd2 && fd4 > fd3) {
					area.y = -1000.0f;
					bots[bNr].parabot->senses.addNewArea(&area);
					// DEBUG_MSG("  VD=4  ");
				}
			}
#if _DEBUG
			if (visualizeCellConnections) {
				Vec3D start, end;

				// draw traffic beam
				vcopy(mapcells_getcell(obs[i].currentcell).pos(), &start);
				vcopy(mapcells_getcell(obs[i].lastcell).pos(), &end);
				start.z -= 30;
				end.z -= 30;
				debugBeam(&start, &end, 100, 0);
				// draw beams to neighbours
				if (obs[i].currentcell != NO_CELL_FOUND && obs[i].currentcell != obs[i].lastcell) {
					for (int nb = 0; nb < 10; nb++) {
						short nbId = mapcells_getcell(obs[i].currentcell).getNeighbour(nb);
						if (nbId == -1)
							break;
						vcopy(mapcells_getcell(nbId).pos(), &end);
						start.z -= 2;
						end.z -= 32;
						debugBeam(&start, &end, 100);
					}
				}
			}
#endif // _DEBUG
			obs[i].lastcell = obs[i].currentcell;
			obs[i].lastcelltime = worldtime();
		}
	}
	obs[i].currentcell = obsCell;
}

void observer_observeall()
// observes all human clients currently registered
{
	Vec3D		pos;	// current position of observed player
	NAVPOINT *nav;	// nearest navpoint to observed player

	// don't observe while round not started
	if (worldtime() < roundStartTime) return;

	for (int i = 0; i < MAX_OBS; i++) {		
		if (observer_shouldobserveplayer(i)) {	// still observing this one
			assert (obs[i].player != 0 );
			obs[i].incombat = false;	// only true for one frame

			pos = obs[i].player->v.origin;
			nav = getNearestNavpoint( obs[i].player );
			assert( nav != 0 );

			if( !nav )
				continue;
			if (navpoint_reached(nav, obs[i].player)) {
				// check if reached new navpoint	
				if ((nav != obs[i].lastreachednav) &&
				    (navpoint_entity(nav) != obs[i].player->v.groundentity)) {	// take care with plats,
					observer_newnavpointreached(i, &pos, nav);		// bot has to wait before approaching
				}
			}

			observer_updatecellinfo(i);

			// insert waypoints?
			observer_checkforjump(i, &pos);
			observer_checkforuse(i, &pos);
			observer_checkformove(i, &pos);

			// insert navpoints?
			observer_checkforcamping(i, &pos);
			observer_checkfortripmines(i, &pos);
			observer_checkforbuttonshot(i, &pos);

			// modify path-flags?
			observer_checkplayerhealth(i);

		/*	if (obs[i].player->v.groundentity) {
				const char *ground = STRING(obs[i].player->v.groundentity->v.classname);
				if ( !Q_STREQ( ground, "world_spawn" ) ) {
					DEBUG_MSG( "Ground entity = %s\n", ground );
				}
			}				 
			trail[i].drawMarkers();
			*/

			// remember these things for teleporters:
			obs[i].lastframepos = obs[i].player->v.origin;
			obs[i].lastframevel = obs[i].player->v.velocity;
		}
	}
}

void
observer_reportpartner(int botId, int oId)
// links botId with oId
{
	obs[botId].partner = oId;
	//assert( !waypoint[observerId].empty() );
	obs[botId].currentwaypoint = obs[oId].leadwaypoint;
}

void
observer_addwaypoint(int oId, Vec3D *pos, int action, int col)
// adds a waypoint for followers
{
	EDICT *plat;
	int flags = observer_checkground(oId, &plat);
	PB_Path_Waypoint wp( pos, (action | flags), worldtime() );
	obs[oId].leadwaypoint++;
	if (obs[oId].leadwaypoint==MAX_WPTS) obs[oId].leadwaypoint = 0;
	// DEBUG_MSG( "Added WP %i at (%.f, %.f)\n", obs[oId].leadwaypoint, pos.x, pos.y );
	obs[oId].waypoint[obs[oId].leadwaypoint] = wp;
	if( wp.isOnPlatform() && plat )
	{
		PB_Path_Platform pf(obs[oId].lastplatid, &plat->v.absmin );
		obs[oId].platinfo[obs[oId].leadwaypoint] = pf;
	}
	obs[oId].lastwptime = worldtime();
	obs[oId].lastwppos = obs[oId].player->v.origin;
	obs[oId].lastwpyaw = obs[oId].player->v.v_angle.y;
	/*
	if (action & WP_ON_PLATFORM) col = 1;
	else col = 2;
	int mid = trail[oId].newMarker( pos, col );
	markerId[oId].push( mid );
	
	if (markerId[oId].size() > MAX_WPTS) {
		int	mid = markerId[oId].front();
		trail[oId].deleteMarker( mid );
		markerId[oId].pop();
	}
	*/
}

PB_Path_Waypoint
observer_getnextwaypoint(int botId)
// returns the next waypoint to follow player with observer id
{
	int nr = obs[botId].partner;
	assert( obs[nr].player != 0 );
	/*
	if ( obs[botId].currentwaypoint > leadwaypoint[nr] ) {
		DEBUG_MSG( " NoWP " );
		return PB_Path_Waypoint( observedPlayer[nr]->pev->origin, WP_NOT_REACHABLE );
	}*/
	return obs[nr].waypoint[obs[botId].currentwaypoint];
}

void
observer_reportwaypointreached(int botId)
{
	int nr = obs[botId].partner;
	if (obs[botId].currentwaypoint != obs[nr].leadwaypoint) {
//		DEBUG_MSG("Reached FWP %i\n", obs[botId].currentwaypoint);
		obs[botId].currentwaypoint++;
		if (obs[botId].currentwaypoint == MAX_WPTS) obs[botId].currentwaypoint = 0;
	}
	/*	int nr = obs[botId].partner;
	assert(!waypoint[nr].empty());
	waypoint[nr].pop_front();
	int mid = markerId[nr].front();
	trail[nr].deleteMarker(mid);
	markerId[nr].pop();		*/
	// DEBUG_MSG( " r%i ", waypoint[nr].size() );
}

bool
observer_shouldfollow(int botId, EDICT *bot)
// returns true if bot should move towards partner
{
	Vec3D dir;

	// always execute jump to avoid slow-downs
	assert(bot != 0);
	if ((observer_getnextwaypoint(botId).action() == BOT_JUMP) ||
	    !(bot->v.flags & FL_ONGROUND))
		return true;

	int nr = obs[botId].partner;
	// if partner not valid don't follow
	if (!observer_partnervalid(nr))
		return false;

	vsub(&obs[nr].player->v.origin, &bot->v.origin, &dir);
	float dist = vlen(&dir);
	// follow if distance > ...
	if (dist > 100.0f)
		return true;

	return false;	
}

bool
observer_cannotfollow(int botId)
// if bot isn't succesful in following frees waypoints and returns true
{
	int nr = obs[botId].partner;
	int nextLeadWpt = obs[nr].leadwaypoint + 1;
	if (nextLeadWpt==MAX_WPTS) nextLeadWpt = 0;
	if (obs[botId].currentwaypoint == nextLeadWpt) {
		DEBUG_MSG( "Following queue near overflow!\n");
		return true;
	}
	return false;
}

bool
observer_partnerincombat(int botId)
// returns true if bots registered partner is involved in a combat
{
	int nr = obs[botId].partner;
	return obs[nr].incombat;
}

bool
observer_partnervalid(int botId)
// returns true if bots registered partner is valid
{
	int nr = obs[botId].partner;
	if (obs[nr].active && (obs[nr].player != 0))
		return true;

	return false;
}
