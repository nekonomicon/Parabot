#pragma warning( disable : 4786 )	// disable graph warnings

#include "parabot.h"
#include "pb_observer.h"
#include "bot.h"
#include "pb_chat.h"
#include "pb_mapcells.h"


extern PB_MapGraph mapGraph;
extern PB_MapCells map;
extern PB_Observer observer;
extern Vector playerPos;		// position of observed player
extern int botNr;				// bot to debug
extern int activeBot;			// bot that's thinking
extern int botTarget;			// target nav id to approach (-1 = nothing)
extern int mod_id;
extern bool pb_pause;
extern int clientWeapon[32];
extern PB_Chat chat;

//extern ChatList chatGotKilled, chatKilledPlayer, chatGotWeapon;
//extern bool botChat;	// from configfiles.cpp
PB_Navpoint* getNearestNavpoint( edict_t *pEdict );




CParabot::CParabot( edict_t *botEnt, int botSlot )
{
	goalMove[0] = 0;
	goalView[0] = 0;
	goalAct[0] = 0;
	ent = botEnt;
	slot = botSlot;
	actualPath = 0;
	aggression = 2.5;
	chatRate = 5;
	lastThink = 0;
}


CParabot::~CParabot()
{
	actualJourney.cancel();
	senses.init( ent );
}


void CParabot::initAfterRespawn()
{
	lastThink = 0;
	campTime = 0;
	lastCamp = worldTime() - 100;
	setRoamingIndex( -1 );
	roamingBreak = 0;
	lastJumpPos = Vector( 0,0,0 );
	makeRoomTime = 0;
	preemptiveFire = false;
	cellTimeOut = 0;
	cellToReach = -1;
	roamingTarget = 0;
	huntingFor = 0;
	fleeingFrom = 0;
	partner = 0;
	botState = PB_NO_TASK;	
	lastRespawn = worldTime();

	action.init( ent );
	pathfinder.init( ent, &action );
	combat.init( slot, ent, &action, &pathfinder );
	senses.init( ent );
	needs.init( this );
	
	mustShootObject = false;
	stoppedForPlat = false;
	if (actualPath) {
		actualPath->reportTargetFailed();
		actualJourney.savePathData();
		actualPath = 0;
	}
	actualJourney.cancel();
	actualNavpoint = getNearestNavpoint( ent );
	
	if (!actualNavpoint) debugMsg( "CParabot::initAfterRespawn() : navpoint=0!\n" );
	else
	if (!actualNavpoint->reached( ent )) {
		actualNavpoint = 0;
		debugMsg( "Not respawned at navpoint!\n" );
	}
	needs.updateWishList();
		
	//debugMsg( "initAfterRespawn called\n" );
}


void CParabot::setGoalViewDescr( char *descr )
{
	strcpy( goalView, descr );
}


void CParabot::setGoalMoveDescr( char *descr )
{
	strcpy( goalMove, descr );
}


void CParabot::setGoalActDescr( char *descr )
{
	strcpy( goalAct, descr );
}


// types of damage to ignore...
#define IGNORE_DAMAGE ( DMG_CRUSH | DMG_FREEZE | DMG_FALL | DMG_SHOCK | DMG_DROWN | \
						DMG_POISON | DMG_NERVEGAS | DMG_RADIATION | DMG_DROWNRECOVER | \
                        DMG_ACID | DMG_SLOWBURN | DMG_SLOWFREEZE)


void CParabot::registerDamage( int amount, Vector origin, int type )
{
/*	const char *name = STRING(ent->v.netname);
	ALERT( at_console, "%s got -%i health, flags = %x\n", name, amount, dmgType );
	ALERT( at_console, "  FallVel=%.f, Origin at (%.f, %.f, %.f)\n", 
		ent->v.flFallVelocity, origin.x, origin.y, origin.z );
	if (dmgType & IGNORE_DAMAGE) return;
*/
	const char *diname;
	CBaseEntity *di = UTIL_FindEntityInSphere( 0, origin, 5 );
	if (di) {
		diname = STRING( di->pev->classname );
		if (strcmp(diname, "player") != 0) {
			if (strcmp(diname, "grenade") == 0) {
			}
			else if (strcmp(diname, "monster_satchel") == 0) {
			}
			else if (strcmp(diname, "monster_tripmine") == 0) {
			}
			else if (strcmp(diname, "rpg_rocket") == 0) {
			}
			else if (strcmp(diname, "bodyque") == 0) {
			}
			else debugMsg( "DAMAGE BY ", diname, "\n" );
		}
		else {
			if (di->pev->origin == botPos()) debugMsg( "SELF-DAMAGE\n" );
			//else debugMsg( "DAMAGE BY PLAYER\n" );
		}
	}
	else {
		debugMsg( "NO ENT FOUND\n" );
	}

	if (type & DMG_SHOWNHUD) {
		int ci = map.getCellId( ent );
		if ( ci != NO_CELL_FOUND) {
			map.cell( ci ).addEnvDamage( amount );
			debugMsg( "ENV_DAMAGE!\n" );
		}
	}

	if (origin == Vector(0,0,0)) return;	// falling etc.

	// who is shooting at the bot?
	bool found = false;
	CBaseEntity *pPlayer = 0;
	for (int i=1; i<=gpGlobals->maxClients; i++) {
		pPlayer = UTIL_PlayerByIndex( i );
		if (!pPlayer) continue;							// skip invalid players
		if ( pPlayer->pev == &(ent->v) ) continue;		// skip self
		if (!isAlive( ENT(pPlayer->pev) )) continue;	// skip player if not alive
		if (pPlayer->pev->solid == SOLID_NOT) continue;	
		
		if ((pPlayer->pev->origin - origin).Length() < 30 ) {
			found = true;
			break;
		}
	}

	if (!found) {	// now we'll have to guess
		//debugMsg( "Guessing!\n" );
		for (int i=1; i<=gpGlobals->maxClients; i++) {
			pPlayer = UTIL_PlayerByIndex( i );
			if (!pPlayer) continue;							// skip invalid players
			if ( pPlayer->pev == &(ent->v) ) continue;		// skip self
			if (!isAlive( ENT(pPlayer->pev) )) continue;	// skip player if not alive
			if (pPlayer->pev->solid == SOLID_NOT) continue;	
		
/*			int usedWeapon = clientWeapon[i];
			switch (mod_id) {
			case VALVE_DLL:		if (! ( usedWeapon==VALVE_WEAPON_RPG ||
										usedWeapon==VALVE_WEAPON_HORNETGUN ||
										usedWeapon==VALVE_WEAPON_CROSSBOW ) ) continue;
								break;
			case HOLYWARS_DLL:	if ( usedWeapon != HW_WEAPON_ROCKETLAUNCHER ) continue;
								break;
			case DMC_DLL:		if (! ( usedWeapon==DMC_WEAPON_GRENLAUNCHER ||
										usedWeapon==DMC_WEAPON_ROCKETLAUNCHER ) ) continue;
								break;

			}*/
			UTIL_MakeVectors( pPlayer->pev->v_angle );
			Vector dir = (botPos() - pPlayer->pev->origin).Normalize();
			float aim = DotProduct( gpGlobals->v_forward, dir );
			//debugMsg( "Aim=%.3f\n", aim );
			if (aim < 0.95) continue;
			if ( !canShootAt( pPlayer->edict(), botPos() ) ) continue;

			found = true;
			break;
		}
	}
	/*
	pPlayer = (CBaseEntity*)GET_PRIVATE( ent->v.dmg_inflictor );
	if ( pPlayer && strlen( STRING(pPlayer->pev->netname) )>0 ) found = true;
*/
	if (found) {
		char *botName = (char *)STRING(ent->v.netname);
		char *inflictorName = (char *)STRING(pPlayer->pev->netname);
		debugMsg( botName, " hurt by ", inflictorName );	debugMsg( "\n" );
		senses.addAttack( pPlayer->edict(), amount );
	}
	else {
		//debugMsg( "No damage inflictor found!\n" );
		senses.addAttack( 0, amount );
	}
}


void CParabot::registerDeath( edict_t *killer, const char *wpnName )
// bot has been killed
{
	// was it one of our fellow players? (not worldspawn)
	if (killer && killer!=this->ent && strlen(STRING(killer->v.netname))>0) {
		int botCell = map.getCellId( ent );
		int killerCell = map.getCellId( killer );
		if ( botCell>=0 && killerCell>=0 && map.lineOfSight( botCell, killerCell ) ) {
			map.cell( botCell ).kills.addDir( killer->v.origin - botPos() );
		}
		chat.registerGotKilled( ent, killer, wpnName );
	}
}


void CParabot::registerKill( edict_t *victim, const char *wpnName )
// bot has killed opponent
{
	chat.registerKilledPlayer( victim, ent, wpnName );
	// check if bot was camping and should camp longer:
	if (actualNavpoint && actualNavpoint->type()==NAV_S_CAMPING) {
		campTime = 0;
	}
}


const char *pfnGetPhysicsKeyValue(const edict_t *pClient, const char *key);
// in engine.cpp

bool CParabot::hasLongJump()
{
	if ( !(mod_id == VALVE_DLL || mod_id == GEARBOX_DLL) ) return false;
	const char *value = pfnGetPhysicsKeyValue( ent, "slj");
	if (strcmp( value, "1" )==0) return true;
	else return false;
}


void CParabot::reportEnemySpotted()
{
	if (actualPath) {
		actualPath->reportEnemySpotted();
		actualPath->cancelAttempt();
		actualJourney.savePathData();
		actualJourney.cancel();
		actualPath = 0;
	}
}


void CParabot::setRoamingIndex( int x )
{
	assert( x < 128 );
	roamingIndex = x;
	huntingFor = 0;
	fleeingFrom = 0;
}


bool CParabot::getJourneyTarget()
{
	int targetNav = -1;
	
	huntingFor = 0;		// we won't be hunting after this!
	fleeingFrom = 0;

	int pathMask = PATH_NORMAL;
	if (mod_id == VALVE_DLL || mod_id == GEARBOX_DLL) {
		if (hasLongJump()) pathMask |= PATH_NEED_LONGJUMP;
		if (combat.hasWeapon( VALVE_WEAPON_GAUSS )) pathMask |= PATH_NEED_GAUSSJUMP;
	}
	if (ent->v.health>80) pathMask |= PATH_CAUSES_DAMAGE;

	if (ent->v.health<30) setJourneyMode( JOURNEY_LONELY );
	else if (needs.wishForCombat() > 7) setJourneyMode( JOURNEY_CROWDED );
	else if (needs.needForItems() > 4) setJourneyMode( JOURNEY_RELIABLE );
	else setJourneyMode( JOURNEY_FAST );
	
	// check if this bot has been assigned a reachable target
	if ( (botNr==slot) && (botTarget>=0) &&	
		 (mapGraph.getJourney( actualNavpoint->id(), botTarget, pathMask, actualJourney)) ) {	
		debugMsg( "Trying to reach assigned target:\n" );
		targetNav = botTarget;
		botTarget = -1;
	}
	else {	// if not, find a wish target
		//initWishList();	// not necessary, but more accurate
		targetNav = mapGraph.getWishJourney( actualNavpoint->id(), needs, pathMask, actualJourney, ent );
	}

	if ( targetNav >= 0 ) {	// found a journey
		PB_Navpoint nav = getNavpoint( targetNav );
		actualPath = actualJourney.getNextPath();
		assert( actualPath != 0 );
		actualPath->startAttempt( worldTime() );
		waypoint = actualPath->getNextWaypoint();
		botState = PB_ON_TOUR;
		//debugMsg( "Switched to ON_TOUR, approach ");  
		//nav.print();  debugMsg( "\n" );
		return true;
	}
	else {
		actualPath = 0;
		return false;
	}
}


void CParabot::getRoamingTarget()
{
	huntingFor = 0;
	fleeingFrom = 0;

	// check if this bot has been assigned a target
	if ( (botNr==slot) && (botTarget>=0) ) { 
		roamingTarget = &(getNavpoint( botTarget ));
		botTarget = -1;
		debugMsg( "Trying to reach assigned target:\n" );
	}
	else { // if not, find a roaming navpoint
		/*roamingTarget = mapGraph.getNearestRoamingNavpoint( ent, actualNavpoint );
		if (!roamingTarget) {	// no linked navpoints found -> just get the nearest
			roamingTarget = mapGraph.getNearestNavpoint( botPos() );
		}*/
		short start = map.getCellId( ent );
		if (start>=0) {
			setRoamingIndex( map.getPathToRoamingTarget( start, ent, roamingRoute ) );
			if (roamingIndex >=0 ) {
				roamingTarget = map.cell( roamingRoute[0] ).getNavpoint();
			}
			else {	// no path found:
				roamingTarget = mapGraph.getNearestRoamingNavpoint( ent, actualNavpoint );
				//debugMsg(" E!" );
			}
		}
		else {
			roamingTarget = mapGraph.getNearestRoamingNavpoint( ent, actualNavpoint );
			setRoamingIndex( -1 );
		}
	}
	assert( roamingTarget != 0 );
	pathfinder.reset( roamingTarget->pos() );
	roamingCount = PB_ROAMING_COUNT;
	botState = PB_ROAMING;
	//debugMsg( "Switched to ROAMING, approach " );
	//roamingTarget->print();	debugMsg( "\n" );
	// assert that everything fits
	assert( pathfinder.pev == ent );
	//assert( action.ent == ent );
	assert( pathfinder.action == &action );
}


void CParabot::approachRoamingTarget()
{
	assert( roamingTarget != 0);

	if (roamingIndex >=0 ) {
		followActualRoute();
		return;
	}
	if (roamingTarget->reached( ent )) {
		//debugMsg( "Roaming target reached\n" );
		roamingTarget->reportVisit( ent, worldTime() ); //now in observer
		actualNavpoint = roamingTarget;
		roamingTarget = 0;
	}
	else {
		//if (roamingCount%10 == 0) debugBeam( botPos(), roamingTarget->pos( ent ), 5, 0 );
		roamingCount--;
		if (roamingCount<0) {
			roamingTarget = 0;
		}
		else {
			pathfinder.checkWay( roamingTarget->pos( ent ) );
			action.setViewLikeMove();
			if ( action.gotStuck() || pathfinder.targetNotReachable() ) {
				roamingTarget->doNotVisitBefore( ent, worldTime()+10.0 );
				roamingTarget = 0;
				action.resetStuck();
			}
		}
	}
}




void CParabot::pathFinished()
// called when path is finished
{
//	debugMsg( "Path finished\n" );
	assert( actualPath != 0 );
	actualPath->reportTargetReached( ent, worldTime() );
	actualJourney.savePathData();
	actualNavpoint = &(actualPath->endNav());
	if (needs.newPriorities()) {
		actualJourney.cancel();		// cancel current journey
		actualPath = 0;				// set actualPath to 0 to invoke new journey
		needs.affirmPriorities();	// do this only once
		debugMsg( "Found new item priorities, canceling journey!\n" );
	}
	else if (actualJourney.continues()) {
		actualPath = actualJourney.getNextPath();
		assert( actualPath != 0 );
		actualPath->startAttempt( worldTime() );
		waypoint = actualPath->getNextWaypoint();
		if (actualPath->startNav().type()==NAV_S_BUTTON_SHOT) {
			if (actualPath->startNav().isTriggerFor( actualPath->endNav() )) {
				// bot must shoot this button!
				shootObjectPos = getNavpoint( actualPath->startNav().special() ).pos();
				mustShootObject = true;
			}
		}
		//debugMsg( "Continue journey, Need=%.1f\n", maxWish );
	}
	else {
		actualPath = 0;
	}
}


void CParabot::pathFailed()
// called when path is *NOT* finished
{
	assert( actualPath != 0 );
	actualPath->print();	debugMsg( " failed\n" );
	actualPath->reportTargetFailed();
	actualJourney.savePathData();
	actualNavpoint = 0;
	actualPath = 0;
	actualJourney.cancel();
}


bool CParabot::positionReached( Vector pos )
	// returns true if bot has reached pos within a distance of PB_REACH_DISTANCE
{
	Vector v = botPos() - pos;
	if (v.Length() < PB_REACH_DISTANCE) return true;
	else return false;
}


void CParabot::pathCheckWay()
// checks if bot has to duck, strafe, or shoot breakable
// if necessary adds these actions
{
	TraceResult tr, trLeft, trRight;
	Vector startTr, endTr, planeAngle;
	
	Vector aDir( 0, action.moveAngleYaw(), 0 );	// use only yaw angle
	UTIL_MakeVectors (aDir);

	// check if bot needs to duck
	if (mod_id != DMC_DLL ) {
		startTr = botPos() + gpGlobals->v_forward * 16 + Vector( 0,0,36 );
		endTr = botPos() + gpGlobals->v_forward * (16+36);
		UTIL_TraceLine( startTr, endTr, ignore_monsters, ent, &tr);
		if (tr.flFraction < 1.0) {
			startTr = botPos() + gpGlobals->v_forward * 16;
			UTIL_TraceLine( startTr, endTr, dont_ignore_monsters, ent, &tr);
			if (tr.flFraction == 1.0) action.add( BOT_DUCK );
			//else debugMsg( "pathCheckWay: completely blocked\n");
		}
	}

	// check if bot needs to strafe
	startTr = botPos() + gpGlobals->v_forward *  8 + gpGlobals->v_right * 16;
	endTr =   botPos() + gpGlobals->v_forward * 50 + gpGlobals->v_right * 16;
	UTIL_TraceLine( startTr, endTr, dont_ignore_monsters, ent, &trRight);
	startTr = botPos() + gpGlobals->v_forward *  8 - gpGlobals->v_right * 16;
	endTr =   botPos() + gpGlobals->v_forward * 50 - gpGlobals->v_right * 16;
	UTIL_TraceLine( startTr, endTr, dont_ignore_monsters, ent, &trLeft);
	if ( (trRight.flFraction < 1.0) && (trLeft.flFraction == 1.0) ) {
		//debugMsg( "Something in right front!\n" );
		planeAngle = UTIL_VecToAngles (trRight.vecPlaneNormal);
		if (planeAngle.x<40) action.add( BOT_STRAFE_LEFT );
	}
	else if ( (trLeft.flFraction < 1.0) && (trRight.flFraction == 1.0) ) {
		//debugMsg( "Something in left front!\n" );
		planeAngle = UTIL_VecToAngles (trLeft.vecPlaneNormal);
		if (planeAngle.x<40) action.add( BOT_STRAFE_RIGHT );
	}

	if ( actualPath ) {
		// check if some breakable object needs to be destroyed
		PB_Navpoint *target = &(actualPath->endNav());
		assert( target != 0 );
		if ( target->type() == NAV_F_BREAKABLE ) {
			if (!target->entity()) {
				debugMsg( "ERROR in pathCheckWay: No entity found!\n" );
				return;
			}
			if (target->entity()->v.health > 0) {	// has to be destroyed
				if (target->visible( ent )) {
					combat.weapon.attack( target->pos(), 0.3 );
				}
				//else debugMsg( "Can't see breakable\n" );
			}
		}
		// check if bot needs to wait for platform
		if (actualPath->waitForPlatform() ) {
			
			Vector lastPos = actualPath->getLastWaypointPos( ent );
			Vector platPos = actualPath->nextPlatformPos();
			if ( (platPos - lastPos).Length() < 50  &&
				  ent->v.groundentity!=0 && 
				  ent->v.groundentity->v.size==Vector(2,2,2) )	// worldSpawn
			{	
				debugMsg(" EVADING PLATFORM\n" );	// lastPos useless
				Vector evDir = botPos() - (platPos - botPos() );
				action.setMoveDir( evDir );
				action.setMaxSpeed();
				//botNr = slot;	// film this!
			}
			else {
				//debugMsg("WAIT FOR PLATFORM!\n" );
				action.add( BOT_STOP_RUNNING );
				action.setSpeed( 0 );
				//action.setMoveDir( lastPos );
				//action.setSpeed( 10*(lastPos-botPos()).Length() );
			}
		}
	}
}

void fixAngle( Vector &angle );

void CParabot::checkForTripmines()
{
	// TODO: only check when beam visible
	TraceResult trAll, trHalf;
	edict_t *mine = senses.getNearestTripmine();
	
	if (!mine) return;
	char *mineClass = (char*)GET_PRIVATE( mine );
	if (!mineClass) return;
	
	Vector mine_vecDir = mine->v.angles;
	fixAngle( mine_vecDir );
	UTIL_MakeVectors( mine_vecDir );
	mine_vecDir = gpGlobals->v_forward;
	Vector moveDir = action.getMoveDir();
	Vector fakeStart = mine->v.origin - 64*moveDir;
	Vector fakeEnd = fakeStart +  512*mine_vecDir;
	
	//debugBeam( fakeStart, fakeEnd, 250, 0 );
	//debugMsg( "BOTS PAUSED!\n" );
	//pb_pause = true;
	
	bool tAll=false;

	// HACKHACK Set simple box using this really nice global!
	gpGlobals->trace_flags = FTRACE_SIMPLEBOX;
	UTIL_TraceLine( fakeStart, fakeEnd, dont_ignore_monsters, mine, &trAll );
	if (trAll.pHit == ent) tAll = true;

	if (tAll) {
		if ( mine_vecDir.z == 0 ) { 
			if (trAll.vecEndPos.z < (ent->v.absmin.z+40)) {
				action.add( BOT_JUMP );
				debugMsg( "JUMP - Trying to evade Tripmine ALL\n" );
			}
			else {
				action.add( BOT_DUCK_LONG );
				debugMsg( "DUCK - Trying to evade Tripmine ALL\n" );
			}
		}
		else debugMsg( "Tripmine vertical, can't evade\n" );
	}

	if (actualPath) {	// check if tripmine pathtarget
		if (actualPath->endNav().type() == NAV_S_USE_TRIPMINE) {
			float mineDist = (actualPath->endNav().pos() - mine->v.origin).Length();
			float botDist = (actualPath->endNav().pos() - ent->v.origin).Length();
			if ( (mineDist < 50) && (botDist < 100) ) {
				debugMsg( "Canceling path - tripmine found at end!\n" );
				pathFinished();
			}
		}
	}
}


void CParabot::followActualPath()
{
	assert( actualPath != 0 );

	if ( actualPath->finished( ent ) ) {
		pathFinished();
	}
	else {
		if (mustShootObject) {
			switch (mod_id) {
				case VALVE_DLL:		combat.weapon.setPreferredWeapon( VALVE_WEAPON_GLOCK, 1 );		
									break;
				case HOLYWARS_DLL:	combat.weapon.setPreferredWeapon( HW_WEAPON_DOUBLESHOTGUN, 1 );	
									break;
				case DMC_DLL:		combat.weapon.setPreferredWeapon( DMC_WEAPON_QUAKEGUN, 1 );		
									break;
				case GEARBOX_DLL:	combat.weapon.setPreferredWeapon( VALVE_WEAPON_GLOCK, 1 );		
									break;
			}
			// arm preferred weapon, params don't really matter...
			bool bestArmed = combat.weapon.armBestWeapon( 200, 0.95, 0 );
			if (bestArmed) {
				if (combat.weapon.attack( shootObjectPos, 0.95 )) mustShootObject = false;
				else return;	// don't do anything else...
			}
			else return;	// don't do anything else...
		}
		if (waypoint.reached( ent )) {
			action.add( actualPath->getNextAction(), &(waypoint.pos( ent )) );	// if there's something to do...
			actualPath->reportWaypointReached();		// confirm waypoint
			Vector oldWP = waypoint.pos( ent );
			waypoint = actualPath->getNextWaypoint();	// get next one
			debugBeam( waypoint.pos( ent ), oldWP, 50, 1 );
			float wpd = (waypoint.pos( ent ) - oldWP).Length();
			//debugMsg( "Reached new WP after %.f\n", wpd );
		}
		int prior;
		Vector proposedViewPos = actualPath->getViewPos( ent, prior );
		action.setViewDir( proposedViewPos, prior  );	// set viewAngle
		action.setMoveDir( waypoint.pos( ent ) );		// set moveAngle and speed

		Vector xyzDir = waypoint.pos( ent ) - botPos();	
		if ( !( isUnderwater( ent ) || waypoint.isOnLadder() ) ) {
			Vector xyDir = xyzDir;	xyDir.z = 0;
			float xyDist = xyDir.Length();
			float xyzDist = xyzDir.Length();
			if ( xyDist<30 && xyzDist>50) action.setSpeed( 8 * xyDist );
			else action.setMaxSpeed();
		}
		else action.setMaxSpeed();
		pathCheckWay();
		if (mod_id == VALVE_DLL || mod_id == GEARBOX_DLL) checkForTripmines();
		if (!actualPath) return;	// maybe tripmine canceled path

		if (actualPath->cannotBeContinued( ent )) {
			debugMsg( "Path failed.\n" );
			pathFailed();
		}
		else if ( action.gotStuck() ) {
			pathFailed();
			action.resetStuck();
		}

	}
}


void CParabot::followActualRoute()
{

	short botCell = map.getCellId( ent );
	if (botCell < 0 ) return;
	short targetCell = roamingRoute[roamingIndex];
	if (cellToReach != targetCell) {
		cellToReach = targetCell;
		cellTimeOut = worldTime() + 1.0;
	}
	Vector target = map.cell( targetCell ).pos();

	//debugMsg( "." );
	if (botCell == targetCell) {
		//debugMsg( "Reached index %i\n", roamingIndex );
		if ( roamingIndex > 0 ) {
			roamingIndex--;
			debugBeam( target, map.cell( roamingRoute[roamingIndex] ).pos(), 50, 2 );
		}
		else {
			//debugMsg( "TARGET REACHED.\n" );
			PB_Navpoint *nav = map.cell( botCell ).getNavpoint();
			if ( nav && nav == roamingTarget ) {
				target = nav->pos( ent );
				if (nav->reached( ent )) {
					nav->reportVisit( ent, worldTime() );
					actualNavpoint = nav;
					roamingTarget = 0;
					setRoamingIndex( -1 );
					return;
				}
			}
			else {
				roamingTarget = 0;
				setRoamingIndex( -1 );
				return;
			}
		}
	}
	else if (botCell != roamingRoute[roamingIndex+1]) {
		float dist2target = (target-map.cell( botCell ).pos()).Length();
		float normDist = (target-map.cell( roamingRoute[roamingIndex+1] ).pos()).Length();
		if (dist2target > (normDist+250)) {
			//botNr = slot;
			debugMsg( "ROUTE ERROR in ", goalMove, "!\n" );
			debugBeam( target, map.cell( botCell ).pos(), 250, 0 );
			setRoamingIndex( -1 );
			return;
		}
	}
	
	action.setViewDir( target );	// set viewAngle
	action.setMoveDir( target );		// set moveAngle and speed
	Vector xyzDir = target - botPos();	
	
	if ( ! isUnderwater( ent ) ) {
		Vector xyDir = xyzDir;	xyDir.z = 0;
		float xyDist = xyDir.Length();
		float xyzDist = xyzDir.Length();
		if ( xyDist<30 && xyzDist>50) action.setSpeed( 8 * xyDist );
		else action.setMaxSpeed();
	}
	else action.setMaxSpeed();

	pathCheckWay();
	
	if ( action.gotStuck() || (worldTime() > cellTimeOut) ) {
		action.resetStuck();
		if ( (lastJumpPos-botPos()).Length() > 50 ) {
			action.add( BOT_JUMP );
			lastJumpPos = botPos();
			cellTimeOut = worldTime() + 1.0;
		}
		else {
			// bisherigen traffic auf Teilstrecke auswerten, falls <3 Verbindung löschen
			if ( map.cell( botCell ).getTraffic( targetCell ) < 3 ) {
				if ( map.cell( botCell ).delNeighbour( targetCell ) ) {
					debugMsg( "Deleted cell neighbour.\n" );
					Vector c1 = map.cell( botCell ).pos() +Vector(0,0,8);
					Vector c2 = map.cell( targetCell ).pos();
					debugBeam( c1, c2, 250, 0 );
					debugMarker( c2, 250 );
				}
				else {
					debugMsg( "Could not delete cell neighbour.\n" );
					Vector c0 = map.cell( botCell ).pos() -Vector(0,0,8);
					Vector c1 = map.cell( roamingRoute[roamingIndex+1] ).pos() +Vector(0,0,8);
					Vector c2 = map.cell( targetCell ).pos();
					debugBeam( c1, c2, 250, 0 );	// rot vom Vorgänger
					debugBeam( c0, c2, 250, 1 );	// grün vom bot aus
					debugMarker( c2, 250 );
					map.cell( roamingRoute[roamingIndex+1] ).delNeighbour( targetCell );
				}
			}
			PB_Cell tc = map.cell( botCell );
			if (roamingTarget) roamingTarget->doNotVisitBefore( ent, worldTime()+10.0 );
			lastJumpPos = Vector( 0,0,0 );
			roamingTarget = 0;
			setRoamingIndex( -1 );
			return;
		}
	}
}


void CParabot::executeGoals()
{
	tGoalFunc	goalFunction;
	PB_Percept	*trigger;

	goalFinder.check();
	goalFinder.synchronize();
	
	checkForBreakpoint( BREAK_GOALS );

	//debugFile( (char*) STRING(ent->v.netname) );
	for (int i=0; i<MAX_GOALS; i++) {
		goalFunction = goalFinder.bestGoal( i );
		if (goalFunction) {
			trigger		 = goalFinder.trigger( i );
			if (trigger!=0) assert( trigger->pClass > 0 && trigger->pClass <= MAX_PERCEPTION );
			(*goalFunction)( this, trigger );
		}
	}
	//debugFile( "exec ok\n" );
}



void CParabot::botThink()
{
	assert( ent != 0 );
	
	CBaseEntity *wpn = getActiveItem( ent );
	if (wpn) {
		const char *wpncl = STRING( wpn->pev->classname );
	}

	activeBot = slot;

	// execute in 10Hz steps:
	float difTime = worldTime()-lastThink;
	if ( difTime >= 0 && difTime < 0.1) {
		action.perform(); // execute planned actions
		if (action.pausing()) cellTimeOut = worldTime() + 1.0;	// adjust cellTimeOut if pausing
		return;
	}
	lastThink = worldTime();
	
	if (actualNavpoint) {	// check for navpoint
		if (!actualNavpoint->reached( ent )) actualNavpoint = 0;
	}

	combat.weapon.initCurrentWeapon();

	needs.updateWishList();

	action.reset();		// initializes action flag and other variables

	senses.collectData();
	goalFinder.init( this );
	goalFinder.analyzeUnconditionalGoals();
	goalFinder.analyze( senses );
	//goalFinder.analyze( tactics );

	executeGoals();
	//debugMsg( "%i enemies\n", senses.numEnemies );

	// check if any grenades have to b thrown (overrides former actions)
	combat.weapon.checkForForcedAttack();

	action.perform(); // execute planned actions
	
	if (action.pausing()) cellTimeOut = worldTime() + 1.0;	// adjust cellTimeOut if pausing
}
