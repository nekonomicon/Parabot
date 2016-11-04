#include "pb_observer.h"
#include "pb_mapgraph.h"
#include "pb_mapcells.h"
#include "animation.h"
#include "bot.h"
#include "parabot.h"
#include "sounds.h"


#define MIN_WAYPOINT_DIST		 64		// distance between waypoints
#define CRITICAL_FALL_VELOCITY	550		// more velocity causes damage
#define MIN_CAMP_TIME			5.0		// seconds to be idle for camping


extern PB_MapGraph mapGraph;	// mapgraph for waypoints
extern PB_MapCells map;

extern int mod_id;
extern int clientWeapon[32];
int playerNr = 0;
extern Vector playerPos;
extern float roundStartTime;
extern float globalFrameTime;	// set by the bots in action.msec()
extern Sounds playerSounds;
extern bool visualizeCellConnections;
extern bot_t bots[32];;

PB_Navpoint* getNearestNavpoint( edict_t *pEdict );



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
	
	for (int i=0; i<MAX_WPTS; i++) {
		waypoint[oId][i].reset();
	}

	trail[oId].deleteAll();
	while (!markerId[oId].empty()) markerId[oId].pop();	
}


void PB_Observer::init()
{
	//debugMsg( "Init all observation-records\n" );
	for (int i=0; i<MAX_OBS; i++) {
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

	assert( obs[oId].player != 0 );
	obs[oId].lastFramePos = obs[oId].player->pev->origin;
	obs[oId].lastFrameVel = obs[oId].player->pev->velocity;
	obs[oId].frags = obs[oId].player->pev->frags;
	obs[oId].health = obs[oId].player->pev->health;

	obs[oId].jumpPressed = false;
	obs[oId].usePressed = false;
	obs[oId].inCombat = false;
	obs[oId].fallTime = 0;

	obs[oId].currentCell = NO_CELL_FOUND;
	obs[oId].lastCell = NO_CELL_FOUND;
	obs[oId].lastCellTime = 0;
	
	addWaypoint( oId, obs[oId].player->pev->origin );	// add first waypoint
}



int PB_Observer::registerPlayer( edict_t *player )
{
	// search free slot
	int i=0;
	while (	obs[i].player!=0 && i<MAX_OBS) i++;
	if (i==MAX_OBS) {
		FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
		fprintf( dfp, "No free slot in PB_Observer::registerPlayer()!\n" ); 
		fclose( dfp );
		i = MAX_OBS - 1;
	}
	obs[i].player = CBaseEntity::Instance( player );
	if ( obs[i].player != 0 ) {
		// init vars:
		startObservation( i );
		debugMsg( "PB_Observer registered ", STRING(obs[i].player->pev->netname), "\n" );
		return i;
	}
	return -1;
}


int PB_Observer::playerId( edict_t *player )
// returns the observer id of player
{
	for (int i=0; i<MAX_OBS; i++) if (obs[i].player.Get() == player) return i;

	return registerPlayer( player );	// if not in list, register for observing
}

	
void PB_Observer::registerClients()
{
	edict_t *player;

	//debugMsg( "PB_Observer::registerClients()\n" );
    for (int i=1; i<=gpGlobals->maxClients; i++)
    {
		player = INDEXENT( i );
		if (!playerExists( player )) continue;
		if (!isAlive( player )) continue;
		
		playerId( player );		// register if not observing yet
    }
}


int PB_Observer::checkGround( int oId, edict_t **plat )
// checks if bot is standing on a ladder or a moving platform, in latter case adds
// the necessary information to the path
// returns the corresponding Waypoint-Flag (WP_ON_LADDER, WP_ON_PLATFORM or 0)
// if WP_ON_PLATFORM is set, plat returns this platform, needsTrigger if it has to be activated
{
	*plat = 0;

	int flags = 0;
	
	if (obs[oId].player->pev->movetype == MOVETYPE_FLY) flags |= WP_ON_LADDER;

	edict_t *ground = obs[oId].player->pev->groundentity;
	if (ground) {
		const char *groundName = STRING( ground->v.classname );
		if (strcmp( groundName, "worldspawn" ) != 0) {
			// we are walking on special ground, check if it is moving:
			if ( (strcmp( groundName, "func_door" ) == 0) ||
				 (strcmp( groundName, "func_plat" ) == 0) ||
				 (strcmp( groundName, "func_train" ) == 0)   ) 
			{
				// try to find the corresponding navpoint for this groundentity:
				int sId = obs[oId].lastPlatId;
				if ( sId >= 0) {	// have we got the right one?
					if ( getNavpoint( sId ).entity() != ground ) sId = -1;
				}
				if ( sId < 0 ) sId = getNavpointIndex( ground );
				if (sId<0) {
					debugMsg( "ERROR: Couldn't find navpoint for func ", STRING( ground->v.classname ), " !\n" );
				}
				else {
					// navpoint found, adding necessary information to path:
					flags |= WP_ON_PLATFORM;
					if (obs[oId].platform==0) flags |= WP_AT_PLATFORM_START;
					obs[oId].lastPlatId = sId;
					*plat = ground;
//					if (observedPath[oId].isRecording()) 
//						observedPath[oId].addPlatformInfo( sId, ground->v.absmin );
					//debugMsg( "Added Platform Info!\n" );
					// check if the platform needs a triggering to be valid
					PB_Navpoint platNav = getNavpoint( sId );
					if (platNav.needsTriggering() && platNav.isTriggered()) {
						flags |= WP_PLAT_NEEDS_TRIGGER;
						debugSound( ENT(obs[oId].player->pev), "weapons/mine_activate.wav" );
						debugMsg( "Plat needs triggering by " );
						for (int ni=0; ni<mapGraph.numberOfNavpoints(); ni++) {
							PB_Navpoint t = getNavpoint( ni );
							if (t.isTriggerFor( platNav )) {
								t.print();
								debugMsg( "\n" );
							}
						}
					}
				}
				obs[oId].platform = ground;
			}
			/*else {
				debugMsg( "Walking unknown ground ");
				debugMsg( groundName ); debugMsg( "\n" );
			}*/
		}
		else {
			// we are on normal ground, check if we just left a platform:
			if (obs[oId].platform) {	
				// try to find the corresponding navpoint for this platform:
				int sId = obs[oId].lastPlatId;
				if ( sId >= 0) {	// have we got the right one?
					if ( getNavpoint( sId ).entity() != obs[oId].platform ) sId = -1;
				}
				if ( sId < 0 ) sId = getNavpointIndex( obs[oId].platform );
				if (sId<0) {
					debugMsg( "ERROR: Couldn't find navpoint for func ", STRING( obs[oId].platform->v.classname ), " !\n" );
				}
				else {
					// navpoint found, setting flag for waiting
					flags |= (WP_ON_PLATFORM | WP_AT_PLATFORM_END);
					*plat = obs[oId].platform;
					//debugMsg( "Added Platform End!\n" );
				}
			}
			obs[oId].platform = 0;
		}
	}	
	
	return flags;
}

extern CMarker glMarker;


bool PB_Observer::shouldObservePlayer( int oId )
// determines if player oId should be observed or not and returns the result
{
	if ( obs[oId].player != 0 ) {
		if ( obs[oId].player.Get() == 0 || GET_PRIVATE(obs[oId].player.Get()) == 0 ) {
			// player disconnected
			obs[oId].active = false;
			obs[oId].player = 0;
			return false;
		}
	}

	if (obs[oId].active) {	// check if observation has to stop...
		assert (obs[oId].player != 0 );
		if ( (obs[oId].player->pev->health < 1)		|| 
			 (obs[oId].player->pev->solid == SOLID_NOT) ) 
		{	// ...yes -> mark inactive
			//debugMsg( "Stopping observation\n" );
			obs[oId].active = false;
		}
	}
	else if (obs[oId].player !=0 ) {	// check if observation has to continue...
		if ( (obs[oId].player->pev->health >= 1)	&& 
			 (obs[oId].player->pev->solid != SOLID_NOT) ) 
		{
			//debugMsg( "Continue observation...\n" );
			startObservation( oId );	// sets active[i] to true
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
	PB_Navpoint *trigger[MAX_TRIGGERS];
	bool triggerOk[MAX_TRIGGERS];
	for (int i=0; i<MAX_TRIGGERS; i++) {
		trigger[i] = 0;
		triggerOk[i] = true;
	}
	if (endNav->needsTriggering()) {
		debugSound( ENT(obs[oId].player->pev), "weapons/mine_activate.wav" );
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
				PB_Navpoint *plat = &(getNavpoint( platInfo[oId][currentIndex].data.navId ));
				bool platRegistered = false;
				for (int i=(triggerCount-1); i>=0; i--) if (trigger[i]==plat) {
					platRegistered = true;  break;
				}
				if (!platRegistered) {
					trigger[triggerCount] = plat;
					triggerOk[triggerCount] = false;
					triggerCount++;
				}
			}
			currentIndex--;  if (currentIndex<0) currentIndex = MAX_WPTS-1;
		}
		if (waypoint[oId][currentIndex].isNavpoint()) {
			PB_Navpoint *currentNav = mapGraph.getNearestNavpoint( waypoint[oId][currentIndex].data.pos );
			bool allTriggersOk = true;
			for (int i=0; i<triggerCount; i++) {
				if (currentNav->isTriggerFor( (*trigger[i]) )) triggerOk[i] = true;
				else if (!triggerOk[i]) allTriggersOk = false;
			}
			if (allTriggersOk) {
				foundIndex = currentIndex;
			}
			else if (currentIndex != lastIndex) {
				currentIndex--;  if (currentIndex<0) currentIndex = MAX_WPTS-1;
			}
		}
		if (dbgCnt2==1000 || dbgCnt1==999) {
			FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
			if (dbgCnt2==1000) fprintf( dfp, "Too many loops2 in PB_Observer::getStartIndex()!\n" ); 
			else fprintf( dfp, "Too many loops1 in PB_Observer::getStartIndex()!\n" ); 
			fclose( dfp );
		}
	}
	return foundIndex;
}


void PB_Observer::newNavpointReached( int oId, Vector &pos, PB_Navpoint *endNav )
{
	PB_Path newPath;
	bool passingPlatform = false;
	bool passingTeleporter = false;

	// have we been beamed and need a hint-waypoint for the teleporter?
	if ( endNav->type() == NAV_INFO_TELEPORT_DEST &&
		(obs[oId].lastFramePos - pos).Length() > 100   ) {
		// add waypoint pointing in right direction
		Vector hintPos = obs[oId].lastFramePos + 2*obs[oId].lastFrameVel;
		addWaypoint( oId, hintPos, WP_TELEPORTER_HINT );
		debugMsg( "adding hintpoint\n" );
		passingTeleporter = true;
	}

	// only add new paths for human clients:
	if (!FBitSet( obs[oId].player->pev->flags, FL_FAKECLIENT )) {
		// search beginning of path
		int startIndex = getStartIndex( oId, endNav );
		if (startIndex != -1) {	// beginning of this path was recorded
			int pathMode = PATH_NORMAL;
			PB_Navpoint *startNav = mapGraph.getNearestNavpoint( waypoint[oId][startIndex].data.pos );
			newPath.startRecord( startNav->id(), waypoint[oId][startIndex].data.arrival );
			int currentIndex = startIndex;
			int dbgCnt = 0;
			while (currentIndex != obs[oId].leadWaypoint && dbgCnt++ < 1000) {
				currentIndex++;  if (currentIndex==MAX_WPTS) currentIndex = 0;
				newPath.addWaypoint( waypoint[oId][currentIndex].data.pos, 
									 waypoint[oId][currentIndex].data.act, 
									 waypoint[oId][currentIndex].data.arrival );
				if (waypoint[oId][currentIndex].isOnPlatform()) {
					passingPlatform = true;
					newPath.addPlatformInfo( platInfo[oId][currentIndex].data.navId,
											 platInfo[oId][currentIndex].data.pos    );
				}
				if (waypoint[oId][currentIndex].action() == BOT_LONGJUMP) 
					pathMode |= PATH_NEED_LONGJUMP;
				if (waypoint[oId][currentIndex].causedDamage() )
					pathMode |= PATH_CAUSES_DAMAGE;
			}
			if (dbgCnt == 1000) {
				FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
				fprintf( dfp, "Too many loops in PB_Observer::newNavpointReached()!\n" ); 
				fclose( dfp );
			}
			if (pathMode & PATH_NEED_LONGJUMP) debugMsg( "PATH NEEDS LONGJUMP!\n" );
			if (pathMode & PATH_CAUSES_DAMAGE) debugMsg( "PATH CAUSES DAMAGE!\n" );
			newPath.stopRecord( endNav->id(), worldTime(), pathMode );
			// don't add return paths for teleporters and platforms
			if (passingPlatform || passingTeleporter) 
				mapGraph.addIfImprovement( newPath, false );
			else
				mapGraph.addIfImprovement( newPath );
		}

		// bots report visits in their movement-code, so this is for players only:
		endNav->reportVisit( ENT(obs[oId].player->pev), worldTime() );
	}

	// add this navpoint to waypoint-list:
	addWaypoint( oId, endNav->pos(), WP_IS_NAVPOINT, 2 );				
	obs[oId].lastReachedNav = endNav;	
}


void PB_Observer::checkForJump( int oId, Vector &pos )
{
	if (obs[oId].jumpPressed && !(obs[oId].player->pev->button & IN_JUMP)) {
		obs[oId].jumpPressed = false;		// jump ended
	}

	if (!obs[oId].jumpPressed && (obs[oId].player->pev->button & IN_JUMP)) {
		obs[oId].jumpPressed = true;		// jump started
		
		void *pmodel	 = GET_MODEL_PTR( ENT(obs[oId].player->pev) );
		int jumpSeq      = LookupActivity( pmodel, obs[oId].player->pev, ACT_HOP );
		int longjumpSeq  = LookupActivity( pmodel, obs[oId].player->pev, ACT_LEAP );
		int seq			 = obs[oId].player->pev->sequence;
		int jumpType	 = 0;
		
		
		if (seq == jumpSeq) {
			// it may still be a longjump, so better check:
			if ( (obs[oId].player->pev->button & IN_DUCK) &&
				( ((Vector)obs[oId].player->pev->velocity).Length() > 500 ) )
				jumpType = 2;
			else jumpType = 1;
		}
		else if	(seq == longjumpSeq) jumpType = 2;
		
		//int flags = checkGround( oId );
		
		if (jumpType==1) {
			// normal jump, check if bot should stop before jumping:
			Vector vel = obs[oId].player->pev->velocity;
			vel.z = 0;
			if (vel.Length() < 50) {
				addWaypoint( oId, pos, BOT_DELAYED_JUMP );
				//debugMsg( "Stored delayed jump!\n" );
			}
			else {
				addWaypoint( oId, pos, BOT_JUMP );
				//debugMsg( "Stored Jump\n" );
			}
		}
		else if (jumpType==2) {
			// longjump has to start earlier:
			pos = pos - globalFrameTime*obs[oId].player->pev->velocity;
			//glMarker.newMarker( pos, 1 );
			addWaypoint( oId, pos, BOT_LONGJUMP );
			//debugMsg( "Stored Longjump=\n" );
		}
	}
}


void PB_Observer::checkForUse( int oId, Vector &pos )
{
	if (obs[oId].usePressed && !(obs[oId].player->pev->button & IN_USE)) {
		obs[oId].usePressed = false;		// use ended
	}
	
	if (!obs[oId].usePressed && (obs[oId].player->pev->button & IN_USE)) {
		obs[oId].usePressed = true;		// use started
		
		int navType = -1; 
		if (obs[oId].lastReachedNav) navType = obs[oId].lastReachedNav->type();
		// don't store charger uses
		if (navType==NAV_F_BUTTON || navType==NAV_F_ROT_BUTTON) {
			//int flags = checkGround( oId );
			// assume that player has used a button
			PB_Navpoint *nearestButton;
			if (navType==NAV_F_BUTTON) nearestButton = mapGraph.getNearestNavpoint( pos, NAV_F_BUTTON );
			else					   nearestButton = mapGraph.getNearestNavpoint( pos, NAV_F_ROT_BUTTON );
			if (nearestButton) {	// TODO: check if looking at button!!!
				Vector usedItemPos = nearestButton->pos();
				addWaypoint( oId, usedItemPos, BOT_USE );
			}
		}
		//debugMsg( "Stored Use\n" );
	}
}


void PB_Observer::checkForMove( int oId, Vector &pos )
{
	// sharp turn means >90 degrees yaw change and moved >10 units:
	bool sharpTurn = ((abs( UTIL_AngleDiff(obs[oId].lastWpYaw, obs[oId].player->pev->v_angle.y) ) > 90) &&
					 (pos-obs[oId].lastWpPos).Length() > 10);

	if ( ((pos-obs[oId].lastWpPos).Length() > MIN_WAYPOINT_DIST) || sharpTurn )
	{	// distance reached or angle too big
		//debugMsg( "x" );
		if ( FBitSet( obs[oId].player->pev->flags, FL_ONGROUND )	// on ground
			|| (obs[oId].player->pev->waterlevel > 0 )				// or in water
			|| (obs[oId].player->pev->movetype == MOVETYPE_FLY) )	// or on ladder
		{
			//int flags = checkGround( oId );
			addWaypoint( oId, pos, 0 );
		}
	}
}
	

void PB_Observer::checkForCamping( int oId, Vector &pos )
{
	if (obs[oId].player->pev->frags != obs[oId].frags ) {
		if (obs[oId].player->pev->frags > obs[oId].frags) {
			// player has a frag more than before...
			if ((worldTime() - obs[oId].lastWpTime) > MIN_CAMP_TIME) {
				// ... and he seems to be camping
				PB_Navpoint *nearestCamp = mapGraph.getNearestNavpoint( pos, NAV_S_CAMPING );
				PB_Navpoint *nearestTank = mapGraph.getNearestNavpoint( pos, NAV_F_TANKCONTROLS );
				// check if we know the location:
				if ( nearestCamp && ((nearestCamp->pos()-pos).Length() < 128) ||
					 nearestTank && ((nearestTank->pos()-pos).Length() < 128)    ) 
				{	// ...yes -> let bots camp longer here!
					bot_t *bot = UTIL_GetBotPointer( ENT(obs[oId].player->pev) );
					if (bot) bot->parabot->campTime = 0;	
				}
				else {
					// ...no -> insert new camping navpoint:
					PB_Navpoint campNav;
					int angleX = (short) obs[oId].player->pev->v_angle.x + 360;
					int angleY = (short) obs[oId].player->pev->v_angle.y + 360;
					angleY <<= 16;
					int campAngle = angleY | angleX;
					campNav.init( pos, NAV_S_CAMPING, campAngle );
					mapGraph.addNavpoint( campNav );
				}
				debugMsg( "Player is a camper!\n" );
			}
			
		}
		obs[oId].frags = obs[oId].player->pev->frags;
	}
}


void PB_Observer::checkForTripmines( int oId, Vector &pos )
// check for setting up tripmine
{
	if (mod_id==VALVE_DLL || mod_id==GEARBOX_DLL) {
		// find weapon the player is handling:
		int clientIndex = ENTINDEX( ENT(obs[oId].player->pev) ) - 1;
		assert( (clientIndex >= 0) && (clientIndex < 32) );
		int playerWeapon = clientWeapon[clientIndex];

		if ( (obs[oId].player->pev->button & IN_ATTACK) && 
			 (playerWeapon==VALVE_WEAPON_TRIPMINE)				) 
		{	// player is trying set up a tripmine, check if possible:
			UTIL_MakeVectors( obs[oId].player->pev->v_angle );
			TraceResult tr;
			Vector startTrace = pos + obs[oId].player->pev->view_ofs;
			Vector endTrace = startTrace + 100 * gpGlobals->v_forward;
			UTIL_TraceLine( startTrace, endTrace, ignore_monsters, 0, &tr );
			if (tr.flFraction < 1.0) {
				// ...yes -> check if we know this spot: 
				Vector minePos = tr.vecEndPos;
				PB_Navpoint *nearest = mapGraph.getNearestNavpoint( minePos, NAV_S_USE_TRIPMINE );
				if ( nearest && ((nearest->pos()-minePos).Length() < 128) ) {
					debugMsg( "Tripmine usage stored nearby!\n" );
				}
				else {
					// new position for setting up a tripmine: store this!
					debugMsg( "Adding tripmine hint!\n" );
					PB_Navpoint mineNav;
					mineNav.init( minePos, NAV_S_USE_TRIPMINE, 0 );
					mapGraph.addNavpoint( mineNav );
				}
			}
			else debugMsg( "No wall in front!\n" );
		}
	}
}


void PB_Observer::checkForButtonShot( int oId, Vector &pos )
// check for setting up tripmine
{
	if (mod_id==DMC_DLL) {
		// find weapon the player is handling:
		int clientIndex = ENTINDEX( ENT(obs[oId].player->pev) ) - 1;
		assert( (clientIndex >= 0) && (clientIndex < 32) );
		int playerWeapon = clientWeapon[clientIndex];

		if ( (obs[oId].player->pev->button & IN_ATTACK) && 
			 (playerWeapon!=DMC_WEAPON_CROWBAR)				) 
		{	// player is shooting at something, check at what:
			UTIL_MakeVectors( obs[oId].player->pev->v_angle );
			TraceResult tr;
			Vector startTrace = pos + obs[oId].player->pev->view_ofs;
			Vector endTrace = startTrace + 1024 * gpGlobals->v_forward;
			UTIL_TraceLine( startTrace, endTrace, dont_ignore_monsters, ENT(obs[oId].player->pev), &tr );
			if (tr.flFraction<1.0 && tr.pHit!=0) {
				const char *hitClass = STRING( tr.pHit->v.classname );
				if ( strcmp( hitClass, "func_button" ) == 0 &&
					 tr.pHit->v.health > 0						)
				{
					// player shot at button, check if navpoint is already stored nearby
					PB_Navpoint *nearest = mapGraph.getNearestNavpoint( pos, NAV_S_BUTTON_SHOT );
					if ( nearest && ((nearest->pos()-pos).Length() < 128) ) {
						debugMsg( "Buttonshot stored nearby!\n" );
						// set this navpoint in path:
						addWaypoint( oId, nearest->pos(), WP_IS_NAVPOINT, 2 );				
						obs[oId].lastReachedNav = nearest;	
					}
					else {
						// ...no -> add new navpoint
						debugMsg( "Adding Buttonshot!\n" );
						PB_Navpoint *button = mapGraph.getNearestNavpoint( tr.vecEndPos, NAV_F_BUTTON );
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
	if (obs[oId].player->pev->flFallVelocity > CRITICAL_FALL_VELOCITY) {
		obs[oId].fallTime = worldTime();
		//debugMsg( "falling at %.f\n", observedPlayer->pev->flFallVelocity);
	}
	if (obs[oId].player->pev->dmg_take > 0) {
		if ( (worldTime()-obs[oId].fallTime) < 0.3 ) {
			waypoint[oId][obs[oId].leadWaypoint].data.act |= WP_DMG_OCURRED;
			//observedPathMode[oId] |= PATH_CAUSES_DAMAGE;
			//debugMsg( "Fall " );
		}
		else if ( (obs[oId].player->pev->watertype == CONTENT_LAVA) ||
			(obs[oId].player->pev->watertype == CONTENT_SLIME)	 ) {
			waypoint[oId][obs[oId].leadWaypoint].data.act |= WP_DMG_OCURRED;
			//observedPathMode[oId] |= PATH_CAUSES_DAMAGE;
			//debugMsg( "Slime/Lava " );
		}
		obs[oId].health = obs[oId].player->pev->health;
		//debugMsg( "damage\n");
	}
	else if (obs[oId].player->pev->health < obs[oId].health) {
		// must have some cause
		obs[oId].health = obs[oId].player->pev->health;
		obs[oId].inCombat = true;
	}
}

void drawCube( edict_t *ent, Vector pos, float size );

void PB_Observer::updateCellInfo( int i )
{
	edict_t *obsEdict = ENT( obs[i].player->pev );
	Vector obsPos = PB_Cell::makePos( obsEdict );
	// check if new cell has to be added:
	short obsCell = map.getCellId( obsPos );
	if (obsCell == NO_CELL_FOUND) {
		if (   UTIL_PointContents( obsPos ) != CONTENTS_SOLID			// Bugfix!!!
			&& (	FBitSet( obs[i].player->pev->flags, FL_ONGROUND )	// on ground
				|| (obs[i].player->pev->waterlevel > 0 )				// or in water
				|| (obs[i].player->pev->movetype == MOVETYPE_FLY) 		// or on ladder
				)
			)
			obsCell = map.addCell( PB_Cell( obsEdict ), true, obs[i].lastCell );
	}

	// init lastCell (after spawning)
	if (obs[i].lastCell == NO_CELL_FOUND) obs[i].lastCell = obsCell;

	// currentCell still holds value from last frame!
	if (obs[i].currentCell != NO_CELL_FOUND) {
		float distToCurrentCell = (obsPos - map.cell( obs[i].currentCell ).pos()).Length();
		float distToLastCell = (obsPos - map.cell( obs[i].lastCell ).pos()).Length();
		// client has reached a new cell
		if ( (obs[i].lastCell != obs[i].currentCell && distToCurrentCell < 25) || 
			 (distToLastCell >= 1.5*CELL_SIZE) )
		{
			// transversal from one cell to another
			float neededTime = worldTime() - obs[i].lastCellTime;
			map.cell( obs[i].lastCell ).addTraffic( obs[i].currentCell, neededTime );

			// set new viewdir for bots:
			int bNr = UTIL_GetBotIndex( obsEdict );
			if (bNr>=0) {
				Vector moveDir = bots[bNr].parabot->action.getMoveDir();
				float fd1 = ( map.cell( obs[i].currentCell ).focus.cellsForDir( Vector( 1,0,0 ) ) +
							  3*map.cell( obs[i].currentCell ).kills.forDir( Vector( 1,0,0 ) )     )
					* (1.5 + moveDir.x);
				float fd2 = ( map.cell( obs[i].currentCell ).focus.cellsForDir( Vector( 0,1,0 ) ) +
							  3*map.cell( obs[i].currentCell ).kills.forDir( Vector( 0,1,0 ) )     )
					* (1.5 + moveDir.y);
				float fd3 = ( map.cell( obs[i].currentCell ).focus.cellsForDir( Vector(-1,0,0 ) ) +
							  3*map.cell( obs[i].currentCell ).kills.forDir( Vector(-1,0,0 ) )     )
					* (1.5 - moveDir.x);
				float fd4 = ( map.cell( obs[i].currentCell ).focus.cellsForDir( Vector( 0,-1,0 ) )+
							  3*map.cell( obs[i].currentCell ).kills.forDir( Vector( 0,-1,0 ) )    )
					* (1.5 - moveDir.y);
				
				Vector bp = obsEdict->v.origin;
				if (fd1>fd2 && fd1>fd3 && fd1>fd4) {
					bots[bNr].parabot->senses.addNewArea( Vector( 1000, 0, 0 ) );
					//debugMsg("  VD=1  ");
				}
				else if (fd2>fd1 && fd2>fd3 && fd2>fd4) {
					bots[bNr].parabot->senses.addNewArea( Vector( 0, 1000, 0 ) );
					//debugMsg("  VD=2  ");
				}
				else if (fd3>fd1 && fd3>fd2 && fd3>fd4) {
					bots[bNr].parabot->senses.addNewArea( Vector( -1000, 0, 0 ) );
					//debugMsg("  VD=3  ");
				}
				else if (fd4>fd1 && fd4>fd2 && fd4>fd3) {
					bots[bNr].parabot->senses.addNewArea( Vector( 0, -1000, 0 ) );
					//debugMsg("  VD=4  ");
				}
			}
						
			if (visualizeCellConnections) {
				// draw traffic beam
				debugBeam(  map.cell( obs[i].currentCell ).pos() + Vector(0,0,-30), 
					map.cell( obs[i].lastCell ).pos() + Vector(0,0,-30), 100, 0 );
				// draw beams to neighbours
				if (obs[i].currentCell != NO_CELL_FOUND && obs[i].currentCell != obs[i].lastCell) {
					for (int nb=0; nb<10; nb++) {
						short nbId = map.cell( obs[i].currentCell ).getNeighbour( nb );
						if (nbId == -1) break;
						debugBeam(  map.cell( obs[i].currentCell ).pos() + Vector(0,0,-32), 
							map.cell( nbId ).pos() + Vector(0,0,-32), 100 );
					}
				}
			}

			obs[i].lastCell = obs[i].currentCell;
			obs[i].lastCellTime = worldTime();
		}
	}
	obs[i].currentCell = obsCell;
}


void PB_Observer::observeAll()
// observes all human clients currently registered
{
	Vector		pos;	// current position of observed player
	PB_Navpoint *nav;	// nearest navpoint to observed player

	// don't observe while round not started
	if (worldTime() < roundStartTime) return;

	for (int i=0; i<MAX_OBS; i++) {
		
		if (shouldObservePlayer( i )) {	// still observing this one
			assert (obs[i].player != 0 );
			obs[i].inCombat = false;	// only true for one frame

			pos = obs[i].player->pev->origin;
			if (i==playerNr) playerPos = pos;
			nav = getNearestNavpoint( obs[i].player->edict() );
			assert( nav != 0 );

			if ( nav->reached( obs[i].player->edict() ) ) {
				// check if reached new navpoint	
				if ( (nav!=obs[i].lastReachedNav) &&
					(nav->entity() != obs[i].player->pev->groundentity) )	// take care with plats,
				{																// bot has to wait before approaching
					newNavpointReached( i, pos, nav );
				}
			}

			updateCellInfo( i );

			// insert waypoints?
			checkForJump( i, pos );
			checkForUse( i, pos );
			checkForMove( i, pos );
			
			// insert navpoints?
			checkForCamping( i, pos );
			checkForTripmines( i, pos );
			checkForButtonShot( i, pos );

			// modify path-flags?
			checkPlayerHealth( i );

		/*	if (obs[i].player->pev->groundentity) {
				const char *ground = STRING(obs[i].player->pev->groundentity->v.classname);
				if (strcmp( ground, "world_spawn" ) != 0) {
					debugMsg( "Ground entity = ", ground, "\n" );
				}
			}				 
			trail[i].drawMarkers();
			*/

			// remember these things for teleporters:
			obs[i].lastFramePos = obs[i].player->pev->origin;
			obs[i].lastFrameVel = obs[i].player->pev->velocity;
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


void PB_Observer::addWaypoint( int oId, Vector pos, int action, int col )
// adds a waypoint for followers
{
	edict_t *plat;
	int flags = checkGround( oId, &plat );
	PB_Path_Waypoint wp( pos, (action | flags), worldTime() );
	obs[oId].leadWaypoint++;
	if (obs[oId].leadWaypoint==MAX_WPTS) obs[oId].leadWaypoint = 0;
	//debugMsg( "Added WP %i", obs[oId].leadWaypoint );
	//debugMsg( " at (%.f, %.f)\n", pos.x, pos.y );
	waypoint[oId][obs[oId].leadWaypoint] = wp;
	if (wp.isOnPlatform()) {
		PB_Path_Platform pf( obs[oId].lastPlatId, plat->v.absmin );
		platInfo[oId][obs[oId].leadWaypoint] = pf;
	}
	obs[oId].lastWpTime = worldTime();
	obs[oId].lastWpPos = obs[oId].player->pev->origin;
	obs[oId].lastWpYaw = obs[oId].player->pev->v_angle.y;
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
		debugMsg( " NoWP " );
		return PB_Path_Waypoint( observedPlayer[nr]->pev->origin, WP_NOT_REACHABLE );
	}*/
	return waypoint[nr][currentWaypoint[botId]];
}


void PB_Observer::reportWaypointReached( int botId )
{
	int nr = partner[botId];
	if (currentWaypoint[botId] != obs[nr].leadWaypoint) {
//		debugMsg( "Reached FWP %i\n", currentWaypoint[botId] );
		currentWaypoint[botId]++;
		if (currentWaypoint[botId]==MAX_WPTS) currentWaypoint[botId] = 0;
	}
	/*	int nr = partner[botId];
	assert( !waypoint[nr].empty() );
	waypoint[nr].pop_front();
	int mid = markerId[nr].front();
	trail[nr].deleteMarker( mid );
	markerId[nr].pop();		*/
	//debugMsg( " r%i ", waypoint[nr].size() );
}


bool PB_Observer::shouldFollow( int botId, edict_t *bot )
// returns true if bot should move towards partner
{
	// always execute jump to avoid slow-downs
	assert( bot != 0 );
	if ( ( getNextWaypoint( botId ).action() == BOT_JUMP ) ||
		 !FBitSet( bot->v.flags, FL_ONGROUND )		) return true;

	int nr = partner[botId];
	// if partner not valid don't follow
	if (!partnerValid(nr)) return false;

	float dist = (obs[nr].player->pev->origin - bot->v.origin).Length();
	// follow if distance > ...
	if ( dist > 100 ) return true;
	else return false;	
}


bool PB_Observer::canNotFollow( int botId )
// if bot isn't succesful in following frees waypoints and returns true
{
	int nr = partner[botId];
	int nextLeadWpt = obs[nr].leadWaypoint + 1;
	if (nextLeadWpt==MAX_WPTS) nextLeadWpt = 0;
	if (currentWaypoint[botId] == nextLeadWpt) {
		debugMsg( "Following queue near overflow!\n");
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
	if ( obs[nr].active && (obs[nr].player != 0) ) return true;
	else return false;
}
