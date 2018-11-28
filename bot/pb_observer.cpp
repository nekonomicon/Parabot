#include "parabot.h"
#include "pb_observer.h"
#include "sectors.h"
#include "kills.h"
#include "focus.h"
#include "pb_mapgraph.h"
#include "pb_mapcells.h"
#include "bot.h"
#include "sounds.h"


#define MIN_WAYPOINT_DIST		 64		// distance between waypoints
#define CRITICAL_FALL_VELOCITY	550		// more velocity causes damage
#define MIN_CAMP_TIME			5.0		// seconds to be idle for camping


extern PB_MapGraph mapGraph;	// mapgraph for waypoints
extern PB_MapCells map;

extern int mod_id;
extern int clientWeapon[32];
int playerNr = 0;
extern float roundStartTime;
extern float globalFrameTime;	// set by the bots in action.msec()
extern bool visualizeCellConnections;
extern bot_t bots[32];

PB_Navpoint* getNearestNavpoint( EDICT *pEdict );



PB_Observer::PB_Observer()
{
	init();
}


PB_Observer::~PB_Observer()
{
	init();
}


void PB_Observer::clear( int oId )
// clears all records for observer oid and sets active to false
{
	obs[oId].active = false;
	
	for (int i=0; i < MAX_WPTS; i++) {
		waypoint[oId][i].reset();
	}
#if _DEBUG
	trail[oId].deleteAll();
	while (!markerId[oId].empty()) markerId[oId].pop();	
#endif //_DEBUG
}


void PB_Observer::init()
{
	// DEBUG_MSG( "Init all observation-records\n" );
	for (int i = 0; i<MAX_OBS; i++) {
		clear( i );
		obs[i].player = 0;
	}
}


void PB_Observer::startObservation( int oId )
{
	clear( oId );	// clear linked lists for markers etc.

	obs[oId].active = true;
	
	obs[oId].leadWaypoint = -1;
	obs[oId].lastPlatId = -1;
	obs[oId].platform = 0;
	obs[oId].lastReachedNav = 0;

	assert(obs[oId].player != 0);
	vcopy(&obs[oId].player->v.origin, &obs[oId].lastFramePos);
	vcopy(&obs[oId].player->v.velocity, &obs[oId].lastFrameVel);
	obs[oId].frags = obs[oId].player->v.frags;
	obs[oId].health = obs[oId].player->v.health;

	obs[oId].jumpPressed = false;
	obs[oId].usePressed = false;
	obs[oId].inCombat = false;
	obs[oId].fallTime = 0;

	obs[oId].currentCell = NO_CELL_FOUND;
	obs[oId].lastCell = NO_CELL_FOUND;
	obs[oId].lastCellTime = 0;
	
	addWaypoint( oId, &obs[oId].player->v.origin );	// add first waypoint
}

int PB_Observer::registerPlayer( EDICT *player )
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
		startObservation(i);
		DEBUG_MSG("PB_Observer registered %s\n", STRING(obs[i].player->v.netname));
		return i;
	}
	return -1;
}


int PB_Observer::playerId( EDICT *player )
// returns the observer id of player
{
	for (int i = 0; i < MAX_OBS; i++)
		if (obs[i].player == player)
			return i;

	return registerPlayer(player);	// if not in list, register for observing
}

void PB_Observer::registerClients()
{
	EDICT *player;

	// DEBUG_MSG("PB_Observer::registerClients()\n");
	for (int i = 1; i <= com.globals->maxclients; i++) {
		player = edictofindex(i);
		if (!playerexists(player)
		    || !is_alive(player))
			continue;

		playerId(player);		// register if not observing yet
	}
}


int PB_Observer::checkGround( int oId, EDICT **plat )
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
				int sId = obs[oId].lastPlatId;
				if (sId >= 0) {	// have we got the right one?
					if (getNavpoint(sId).entity() != ground)
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
					obs[oId].lastPlatId = sId;
					*plat = ground;
//					if (observedPath[oId].isRecording()) 
//						observedPath[oId].addPlatformInfo( sId, ground->v.absmin );
					// DEBUG_MSG( "Added Platform Info!\n" );
					// check if the platform needs a triggering to be valid
					PB_Navpoint platNav = getNavpoint( sId );
					if (platNav.needsTriggering() && platNav.isTriggered()) {
						flags |= WP_PLAT_NEEDS_TRIGGER;
#if _DEBUG
						debugsound((obs[oId].player), "weapons/mine_activate.wav");
						DEBUG_MSG("Plat needs triggering by ");
						for (int ni = 0; ni < mapGraph.numberOfNavpoints(); ni++) {
							PB_Navpoint t = getNavpoint( ni );
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
				int sId = obs[oId].lastPlatId;
				if (sId >= 0) {	// have we got the right one?
					if (getNavpoint(sId).entity() != obs[oId].platform) sId = -1;
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


bool PB_Observer::shouldObservePlayer( int oId )
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
			startObservation(oId);	// sets active[i] to true
		}
	}
	return obs[oId].active;
}

int PB_Observer::getStartIndex( int oId, PB_Navpoint *endNav )
{
	#define MAX_TRIGGERS 16

	int currentIndex = obs[oId].leadWaypoint;
	int lastIndex = currentIndex+1;  if (lastIndex==MAX_WPTS) lastIndex = 0;
	int foundIndex = -1;

	int triggerCount = 0;
	PB_Navpoint *trigger[MAX_TRIGGERS] = {0,};
	bool triggerOk[MAX_TRIGGERS] = {false,};
	if (endNav->needsTriggering()) {
		debugSound( (obs[oId].player), "weapons/mine_activate.wav" );
		trigger[triggerCount] = endNav;	
		triggerOk[triggerCount] = false;
		triggerCount++;
	}
	
	int dbgCnt1 = 0;
	while (foundIndex < 0 && currentIndex != lastIndex && dbgCnt1++ < 1000) {
		int dbgCnt2 = 0;
		while (!waypoint[oId][currentIndex].isNavpoint() && currentIndex != lastIndex && dbgCnt2++ < 1000) {
			if (waypoint[oId][currentIndex].needsTriggerForPlat()) {
				//PB_Navpoint *plat = &(mapGraph[platInfo[oId][currentIndex].data.navId].first);
				PB_Navpoint *plat = &(getNavpoint(platInfo[oId][currentIndex].data.navId));
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
		if (waypoint[oId][currentIndex].isNavpoint()) {
			PB_Navpoint *currentNav = mapGraph.getNearestNavpoint(&waypoint[oId][currentIndex].data.pos);
			bool allTriggersOk = true;
			for (int i=0; i < triggerCount; i++) {
				if (currentNav->isTriggerFor((*trigger[i])))
					triggerOk[i] = true;
				else if (!triggerOk[i]) allTriggersOk = false;
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

void PB_Observer::newNavpointReached( int oId, Vec3D *pos, PB_Navpoint *endNav )
{
	Vec3D dir;
	PB_Path newPath;
	bool passingPlatform = false;
	bool passingTeleporter = false;

	// have we been beamed and need a hint-waypoint for the teleporter?
	if (endNav->type() == NAV_INFO_TELEPORT_DEST) {
		vsub(&obs[oId].lastFramePos, pos, &dir);
		if(vlen(&dir) > 100) {
			// add waypoint pointing in right direction
			Vec3D hintPos;
			vma(&obs[oId].lastFramePos, 2.0f, &obs[oId].lastFrameVel, &hintPos);
			addWaypoint(oId, &hintPos, WP_TELEPORTER_HINT);
			DEBUG_MSG("adding hintpoint\n");
			passingTeleporter = true;
		}
	}

	// only add new paths for human clients:
	if (!(obs[oId].player->v.flags & FL_FAKECLIENT)) {
		// search beginning of path
		int startIndex = getStartIndex( oId, endNav );
		if (startIndex != -1) {	// beginning of this path was recorded
			int pathMode = PATH_NORMAL;
			PB_Navpoint *startNav = mapGraph.getNearestNavpoint(&waypoint[oId][startIndex].data.pos);
			newPath.startRecord( startNav->id(), waypoint[oId][startIndex].data.arrival );
			int currentIndex = startIndex;
			int dbgCnt = 0;
			while (currentIndex != obs[oId].leadWaypoint && dbgCnt++ < 1000) {
				currentIndex++;
				if (currentIndex == MAX_WPTS)
					currentIndex = 0;
				newPath.addWaypoint(&waypoint[oId][currentIndex].data.pos, 
									 waypoint[oId][currentIndex].data.act, 
									 waypoint[oId][currentIndex].data.arrival );
				if (waypoint[oId][currentIndex].isOnPlatform()) {
					passingPlatform = true;
					newPath.addPlatformInfo(platInfo[oId][currentIndex].data.navId,
					&platInfo[oId][currentIndex].data.pos);
				}
				if (waypoint[oId][currentIndex].action() == BOT_LONGJUMP) 
					pathMode |= PATH_NEED_LONGJUMP;
				if (waypoint[oId][currentIndex].causedDamage() )
					pathMode |= PATH_CAUSES_DAMAGE;
			}
			/*if (dbgCnt == 1000) {
				FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
				fprintf( dfp, "Too many loops in PB_Observer::newNavpointReached()!\n" ); 
				fclose( dfp );
			}*/
			if (pathMode & PATH_NEED_LONGJUMP) DEBUG_MSG( "PATH NEEDS LONGJUMP!\n" );
			if (pathMode & PATH_CAUSES_DAMAGE) DEBUG_MSG( "PATH CAUSES DAMAGE!\n" );
			newPath.stopRecord( endNav->id(), worldtime(), pathMode );
			// don't add return paths for teleporters and platforms
			if (passingPlatform || passingTeleporter) 
				mapGraph.addIfImprovement( newPath, false );
			else
				mapGraph.addIfImprovement( newPath );
		}

		// bots report visits in their movement-code, so this is for players only:
		endNav->reportVisit( (obs[oId].player), worldtime() );
	}

	// add this navpoint to waypoint-list:
	addWaypoint( oId, endNav->pos(), WP_IS_NAVPOINT, 2 );				
	obs[oId].lastReachedNav = endNav;	
}

void PB_Observer::checkForJump( int oId, Vec3D *pos )
{
	if (obs[oId].jumpPressed && !(obs[oId].player->v.button & ACTION_JUMP)) {
		obs[oId].jumpPressed = false;		// jump ended
	} else if (!obs[oId].jumpPressed && (obs[oId].player->v.button & ACTION_JUMP)) {
		obs[oId].jumpPressed = true;		// jump started

		int jumpType = 1;

		// it may still be a longjump, so better check:
		if ((obs[oId].player->v.button & ACTION_CROUCH) &&
		    (vlen(&obs[oId].player->v.velocity) > 500))
			jumpType = 2;

		//int flags = checkGround( oId );

		if (jumpType == 1) {
			// normal jump, check if bot should stop before jumping:
			if (vlen2d((Vec2D *)&obs[oId].player->v.velocity) < 50) {
				addWaypoint( oId, pos, BOT_DELAYED_JUMP );
				// DEBUG_MSG( "Stored delayed jump!\n" );
			} else {
				addWaypoint( oId, pos, BOT_JUMP );
				// DEBUG_MSG( "Stored Jump\n" );
			}
		} else if (jumpType == 2) {
			// longjump has to start earlier:
			vma(pos, -globalFrameTime, &obs[oId].player->v.velocity, pos);
			//glMarker.newMarker( pos, 1 );
			addWaypoint( oId, pos, BOT_LONGJUMP );
			// DEBUG_MSG( "Stored Longjump=\n" );
		}
	}
}

void PB_Observer::checkForUse( int oId, Vec3D *pos )
{
	if (obs[oId].usePressed && !(obs[oId].player->v.button & ACTION_USE)) {
		obs[oId].usePressed = false;		// use ended
	} else if (!obs[oId].usePressed && (obs[oId].player->v.button & ACTION_USE)) {
		obs[oId].usePressed = true;		// use started
		
		int navType = -1; 
		if (obs[oId].lastReachedNav) navType = obs[oId].lastReachedNav->type();
		// don't store charger uses
		if (navType == NAV_F_BUTTON || navType == NAV_F_ROT_BUTTON) {
			//int flags = checkGround( oId );
			// assume that player has used a button
			PB_Navpoint *nearestButton;
			if (navType == NAV_F_BUTTON)
				nearestButton = mapGraph.getNearestNavpoint( pos, NAV_F_BUTTON );
			else
				nearestButton = mapGraph.getNearestNavpoint( pos, NAV_F_ROT_BUTTON );
			if (nearestButton) {	// TODO: check if looking at button!!!
				Vec3D usedItemPos;
				vcopy(nearestButton->pos(), &usedItemPos);
				addWaypoint( oId, &usedItemPos, BOT_USE );
			}
		}
		// DEBUG_MSG( "Stored Use\n" );
	}
}

void PB_Observer::checkForMove( int oId, Vec3D *pos )
{
	Vec3D dir;
	float dist;
	bool sharpTurn;

	vsub(pos, &obs[oId].lastWpPos, &dir);
	dist = vlen(&dir);

	if ((dist > MIN_WAYPOINT_DIST)	// sharp turn means >90 degrees yaw change and moved >10 units:
	    || ((fabsf(anglediff(obs[oId].lastWpYaw, obs[oId].player->v.v_angle.y)) > 90)
	    && (dist > 10))) {	// distance reached or angle too big
		// DEBUG_MSG( "x" );
		if ((obs[oId].player->v.flags & FL_ONGROUND)	// on ground
			|| (obs[oId].player->v.waterlevel > 0)	// or in water
			|| (is_onladder(obs[oId].player))) {	// or on ladder
			//int flags = checkGround( oId );
			addWaypoint(oId, pos, 0);
		}
	}
}
	
void PB_Observer::checkForCamping(int oId, Vec3D *pos)
{
	Vec3D dircamp, dirtank;

	if (obs[oId].player->v.frags != obs[oId].frags ) {
		if (obs[oId].player->v.frags > obs[oId].frags) {
			// player has a frag more than before...
			if ((worldtime() - obs[oId].lastWpTime) > MIN_CAMP_TIME) {
				// ... and he seems to be camping
				PB_Navpoint *nearestCamp = mapGraph.getNearestNavpoint( pos, NAV_S_CAMPING );
				PB_Navpoint *nearestTank = mapGraph.getNearestNavpoint( pos, NAV_F_TANKCONTROLS );
				vsub(nearestCamp->pos(), pos, &dircamp);
				vsub(nearestTank->pos(), pos, &dirtank);
				// check if we know the location:
				if( ( nearestCamp && (vlen(&dircamp) < 128 ) )
				    || (nearestTank && (vlen(&dirtank) < 128 ) ) ) 
				{	// ...yes -> let bots camp longer here!
					bot_t *bot = getbotpointer( (obs[oId].player) );
					if (bot) bot->parabot->campTime = 0;	
				} else {
					// ...no -> insert new camping navpoint:
					PB_Navpoint campNav;
					int angleX = (short) obs[oId].player->v.v_angle.x + 360;
					int angleY = (short) obs[oId].player->v.v_angle.y + 360;
					angleY <<= 16;
					int campAngle = angleY | angleX;
					campNav.init( pos, NAV_S_CAMPING, campAngle );
					mapGraph.addNavpoint( campNav );
				}
				DEBUG_MSG( "Player is a camper!\n" );
			}
			
		}
		obs[oId].frags = obs[oId].player->v.frags;
	}
}


void PB_Observer::checkForTripmines( int oId, Vec3D *pos )
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
				PB_Navpoint *nearest = mapGraph.getNearestNavpoint(&minePos, NAV_S_USE_TRIPMINE);
				vsub(nearest->pos(), &minePos, &dir);
				if (nearest && (vlen(&dir) < 128) ) {
					DEBUG_MSG( "Tripmine usage stored nearby!\n" );
				} else {
					// new position for setting up a tripmine: store this!
					DEBUG_MSG( "Adding tripmine hint!\n" );
					PB_Navpoint mineNav;
					mineNav.init(&minePos, NAV_S_USE_TRIPMINE, 0);
					mapGraph.addNavpoint( mineNav );
				}
			} else
				DEBUG_MSG( "No wall in front!\n" );
		}
	}
}

void PB_Observer::checkForButtonShot( int oId, Vec3D *pos )
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
					PB_Navpoint *nearest = mapGraph.getNearestNavpoint( pos, NAV_S_BUTTON_SHOT );
					vsub(nearest->pos(), pos, &dir);
					if (nearest && (vlen(&dir)) < 128) {
						DEBUG_MSG( "Buttonshot stored nearby!\n" );
						// set this navpoint in path:
						addWaypoint( oId, nearest->pos(), WP_IS_NAVPOINT, 2 );				
						obs[oId].lastReachedNav = nearest;
					} else {
						// ...no -> add new navpoint
						DEBUG_MSG( "Adding Buttonshot!\n" );
						PB_Navpoint *button = mapGraph.getNearestNavpoint(&tr.endpos, NAV_F_BUTTON);
						assert( button != 0 );
						PB_Navpoint shotNav;
						shotNav.init( pos, NAV_S_BUTTON_SHOT, button->id() );
						mapGraph.addNavpoint( shotNav );
					}
				}
			}
		}
	}
}


void PB_Observer::checkPlayerHealth( int oId )
// check if player lost health by fall-damage or attack
// sets PATH_CAUSES_DAMAGE-flag and the inCombat-variable
{
	// check for fall damage
	if (obs[oId].player->v.FallVelocity > CRITICAL_FALL_VELOCITY) {
		obs[oId].fallTime = worldtime();
		//DEBUG_MSG( "falling at %.f\n", observedPlayer->pev->FallVelocity);
	}
	if (obs[oId].player->v.dmg_take > 0) {
		if ((worldtime() - obs[oId].fallTime) < 0.3f) {
			waypoint[oId][obs[oId].leadWaypoint].data.act |= WP_DMG_OCURRED;
			//observedPathMode[oId] |= PATH_CAUSES_DAMAGE;
			//DEBUG_MSG( "Fall " );
		} else if ((obs[oId].player->v.watertype == CONTENTS_LAVA) ||
			(obs[oId].player->v.watertype == CONTENTS_SLIME)) {
			waypoint[oId][obs[oId].leadWaypoint].data.act |= WP_DMG_OCURRED;
			//observedPathMode[oId] |= PATH_CAUSES_DAMAGE;
			//DEBUG_MSG( "Slime/Lava " );
		}
		obs[oId].health = obs[oId].player->v.health;
		//DEBUG_MSG( "damage\n");
	} else if (obs[oId].player->v.health < obs[oId].health) {
		// must have some cause
		obs[oId].health = obs[oId].player->v.health;
		obs[oId].inCombat = true;
	}
}

void drawCube( EDICT *ent, Vec3D pos, float size );

void PB_Observer::updateCellInfo( int i )
{
	Vec3D obsPos, dir;
	eyepos(obs[i].player, &obsPos);
	// check if new cell has to be added:
	short obsCell = map.getCellId(&obsPos);
	if (obsCell == NO_CELL_FOUND) {
		if ((obs[i].player->v.waterlevel > 0)			// or in water
		    || (is_onladder(obs[i].player)) 		// or on ladder
		    || ((obs[i].player->v.flags & FL_ONGROUND)	// on ground
		    && (pointcontents(&obsPos) != CONTENTS_SOLID)))	// Bugfix!!!
			obsCell = map.addCell( PB_Cell(obs[i].player), true, obs[i].lastCell );
	}

	// init lastCell (after spawning)
	if (obs[i].lastCell == NO_CELL_FOUND) obs[i].lastCell = obsCell;

	// currentCell still holds value from last frame!
	if (obs[i].currentCell != NO_CELL_FOUND) {

		vsub(&obsPos, map.cell(obs[i].currentCell).pos(), &dir);
		float distToCurrentCell = vlen(&dir);
		vsub(&obsPos, map.cell(obs[i].lastCell).pos(), &dir);
		float distToLastCell = vlen(&dir);
		// client has reached a new cell
		if ((obs[i].lastCell != obs[i].currentCell && distToCurrentCell < 25)
		    || (distToLastCell >= 1.5 * CELL_SIZE) ) {
			// transversal from one cell to another
			float neededTime = worldtime() - obs[i].lastCellTime;
			map.cell( obs[i].lastCell ).addTraffic( obs[i].currentCell, neededTime );

			// set new viewdir for bots:
			int bNr = getbotindex(obs[i].player);
			if (bNr>=0) {
				Vec3D moveDir, area = {1.0f, 0.0f, 0.0f};
				vcopy(bots[bNr].parabot->action.getMoveDir(), &moveDir);
				float fd1 = (focus_cellsfordir(&area, map.cell(obs[i].currentCell).data.sectors)
				    + 3.0f * kills_fordir(&area, map.cell(obs[i].currentCell).data.sectors))
				    * (1.5f + moveDir.x);
				area.x = 0.0f;
				area.y = 1.0f;
				float fd2 = (focus_cellsfordir(&area, map.cell(obs[i].currentCell).data.sectors)
				    + 3.0f * kills_fordir(&area, map.cell(obs[i].currentCell).data.sectors))
				    * (1.5f + moveDir.y);
				area.x = -1.0f;
				area.y = 0.0f;
				float fd3 = (focus_cellsfordir(&area, map.cell(obs[i].currentCell).data.sectors)
				    + 3.0f * kills_fordir(&area, map.cell(obs[i].currentCell).data.sectors))
				    * (1.5f - moveDir.x);
				area.x = 0.0f;
				area.y = -1.0f;
				float fd4 = (focus_cellsfordir(&area, map.cell(obs[i].currentCell).data.sectors)
				    + 3.0f * kills_fordir(&area, map.cell(obs[i].currentCell).data.sectors))
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
				vcopy(map.cell(obs[i].currentCell).pos(), &start);
				vcopy(map.cell(obs[i].lastCell).pos(), &end);
				start.z -= 30;
				end.z -= 30;
				debugBeam(&start, &end, 100, 0);
				// draw beams to neighbours
				if (obs[i].currentCell != NO_CELL_FOUND && obs[i].currentCell != obs[i].lastCell) {
					for (int nb = 0; nb < 10; nb++) {
						short nbId = map.cell(obs[i].currentCell).getNeighbour(nb);
						if (nbId == -1)
							break;
						vcopy(map.cell(nbId).pos(), &end);
						start.z -= 2;
						end.z -= 32;
						debugBeam(&start, &end, 100);
					}
				}
			}
#endif // _DEBUG
			obs[i].lastCell = obs[i].currentCell;
			obs[i].lastCellTime = worldtime();
		}
	}
	obs[i].currentCell = obsCell;
}

void PB_Observer::observeAll()
// observes all human clients currently registered
{
	Vec3D		pos;	// current position of observed player
	PB_Navpoint *nav;	// nearest navpoint to observed player

	// don't observe while round not started
	if (worldtime() < roundStartTime) return;

	for (int i = 0; i < MAX_OBS; i++) {		
		if (shouldObservePlayer( i )) {	// still observing this one
			assert (obs[i].player != 0 );
			obs[i].inCombat = false;	// only true for one frame

			pos = obs[i].player->v.origin;
			nav = getNearestNavpoint( obs[i].player );
			assert( nav != 0 );

			if( !nav )
				continue;
			if (nav->reached(obs[i].player)) {
				// check if reached new navpoint	
				if ((nav != obs[i].lastReachedNav) &&
				    (nav->entity() != obs[i].player->v.groundentity)) {	// take care with plats,
					newNavpointReached(i, &pos, nav);		// bot has to wait before approaching
				}
			}

			updateCellInfo( i );

			// insert waypoints?
			checkForJump( i, &pos );
			checkForUse( i, &pos );
			checkForMove( i, &pos );
			
			// insert navpoints?
			checkForCamping( i, &pos );
			checkForTripmines( i, &pos );
			checkForButtonShot( i, &pos );

			// modify path-flags?
			checkPlayerHealth( i );

		/*	if (obs[i].player->v.groundentity) {
				const char *ground = STRING(obs[i].player->v.groundentity->v.classname);
				if ( !Q_STREQ( ground, "world_spawn" ) ) {
					DEBUG_MSG( "Ground entity = %s\n", ground );
				}
			}				 
			trail[i].drawMarkers();
			*/

			// remember these things for teleporters:
			obs[i].lastFramePos = obs[i].player->v.origin;
			obs[i].lastFrameVel = obs[i].player->v.velocity;
		}
	}
}

void PB_Observer::reportPartner( int botId, int oId )
// links botId with oId
{
	partner[botId] = oId;
	//assert( !waypoint[observerId].empty() );
	currentWaypoint[botId] = obs[oId].leadWaypoint;
}

void PB_Observer::addWaypoint( int oId, Vec3D *pos, int action, int col )
// adds a waypoint for followers
{
	EDICT *plat;
	int flags = checkGround( oId, &plat );
	PB_Path_Waypoint wp( pos, (action | flags), worldtime() );
	obs[oId].leadWaypoint++;
	if (obs[oId].leadWaypoint==MAX_WPTS) obs[oId].leadWaypoint = 0;
	// DEBUG_MSG( "Added WP %i at (%.f, %.f)\n", obs[oId].leadWaypoint, pos.x, pos.y );
	waypoint[oId][obs[oId].leadWaypoint] = wp;
	if( wp.isOnPlatform() && plat )
	{
		PB_Path_Platform pf(obs[oId].lastPlatId, &plat->v.absmin );
		platInfo[oId][obs[oId].leadWaypoint] = pf;
	}
	obs[oId].lastWpTime = worldtime();
	obs[oId].lastWpPos = obs[oId].player->v.origin;
	obs[oId].lastWpYaw = obs[oId].player->v.v_angle.y;
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

PB_Path_Waypoint PB_Observer::getNextWaypoint( int botId )
// returns the next waypoint to follow player with observer id
{
	int nr = partner[botId];
	assert( obs[nr].player != 0 );
	/*
	if ( currentWaypoint[botId] > leadWaypoint[nr] ) {
		DEBUG_MSG( " NoWP " );
		return PB_Path_Waypoint( observedPlayer[nr]->pev->origin, WP_NOT_REACHABLE );
	}*/
	return waypoint[nr][currentWaypoint[botId]];
}

void PB_Observer::reportWaypointReached( int botId )
{
	int nr = partner[botId];
	if (currentWaypoint[botId] != obs[nr].leadWaypoint) {
//		DEBUG_MSG( "Reached FWP %i\n", currentWaypoint[botId] );
		currentWaypoint[botId]++;
		if (currentWaypoint[botId]==MAX_WPTS) currentWaypoint[botId] = 0;
	}
	/*	int nr = partner[botId];
	assert( !waypoint[nr].empty() );
	waypoint[nr].pop_front();
	int mid = markerId[nr].front();
	trail[nr].deleteMarker( mid );
	markerId[nr].pop();		*/
	// DEBUG_MSG( " r%i ", waypoint[nr].size() );
}

bool PB_Observer::shouldFollow( int botId, EDICT *bot )
// returns true if bot should move towards partner
{
	Vec3D dir;

	// always execute jump to avoid slow-downs
	assert( bot != 0 );
	if ((getNextWaypoint( botId ).action() == BOT_JUMP) ||
	    !(bot->v.flags & FL_ONGROUND))
		return true;

	int nr = partner[botId];
	// if partner not valid don't follow
	if (!partnerValid(nr))
		return false;

	vsub(&obs[nr].player->v.origin, &bot->v.origin, &dir);
	float dist = vlen(&dir);
	// follow if distance > ...
	if (dist > 100.0f)
		return true;

	return false;	
}

bool PB_Observer::canNotFollow( int botId )
// if bot isn't succesful in following frees waypoints and returns true
{
	int nr = partner[botId];
	int nextLeadWpt = obs[nr].leadWaypoint + 1;
	if (nextLeadWpt==MAX_WPTS) nextLeadWpt = 0;
	if (currentWaypoint[botId] == nextLeadWpt) {
		DEBUG_MSG( "Following queue near overflow!\n");
		return true;
	}
	return false;
}


bool PB_Observer::partnerInCombat( int botId )
// returns true if bots registered partner is involved in a combat
{
	int nr = partner[botId];
	return obs[nr].inCombat;
}


bool PB_Observer::partnerValid( int botId )
// returns true if bots registered partner is valid
{
	int nr = partner[botId];
	if ( obs[nr].active && (obs[nr].player != 0) )
		return true;

	return false;
}
