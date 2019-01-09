#pragma warning( disable : 4786 )	// disable graph warnings

#include "parabot.h"
#include "observer.h"
#include "bot.h"
#include "chat.h"
#include "sectors.h"
#include "kills.h"
#include "vistable.h"
#include "cell.h"
#include "mapcells.h"


int botNr;				// bot to debug
extern int activeBot;			// bot that's thinking
extern int botTarget;			// target nav id to approach (-1 = nothing)
extern int mod_id;
extern bool pb_pause;
extern int clientWeapon[32];

//extern ChatList chatGotKilled, chatKilledPlayer, chatGotWeapon;
//extern bool botChat;	// from configfiles.cpp
NAVPOINT *getNearestNavpoint( EDICT *pEdict );

CParabot::CParabot( EDICT *botEnt, int botSlot )
{
#if DEBUG
	goalMove[0] = 0;
	goalView[0] = 0;
	goalAct[0] = 0;
#endif
	ent = botEnt;
	slot = botSlot;
	actualPath = 0;
	aggression = 2.5;
	chatRate = 5;
	lastThink = 0;
}


CParabot::~CParabot()
{
	journey_cancel(&actualJourney);
	perception_init(&senses, ent);
}


void CParabot::initAfterRespawn()
{
	lastThink = 0;
	campTime = 0;
	lastCamp = worldtime() - 100;
	setRoamingIndex( -1 );
	roamingBreak = 0;
	lastJumpPos = zerovector;
	makeRoomTime = 0;
	preemptiveFire = false;
	cellTimeOut = 0;
	cellToReach = -1;
	roamingTarget = 0;
	huntingFor = 0;
	fleeingFrom = 0;
	partner = 0;
	botState = PB_NO_TASK;	
	lastRespawn = worldtime();

	action_init(&action, ent);
	roaming_init(&pathfinder, ent, &action );
	combat_init(&combat, slot, ent, &action, &pathfinder );
	perception_init(&senses, ent);
	needs_init(&needs, this);
	
	mustShootObject = false;
	stoppedForPlat = false;
	if (actualPath) {
		path_reporttargetfailed(actualPath);
		journey_savepathdata(&actualJourney);
		actualPath = 0;
	}
	journey_cancel(&actualJourney);
	actualNavpoint = getNearestNavpoint( ent );
	
	if (!actualNavpoint)
		DEBUG_MSG( "CParabot::initAfterRespawn() : navpoint=0!\n" );
	else if (!navpoint_reached(actualNavpoint, ent)) {
		actualNavpoint = 0;
		DEBUG_MSG( "Not respawned at navpoint!\n" );
	}
	needs_updatewishlist(&needs);

	// DEBUG_MSG( "initAfterRespawn called\n" );
}

#if DEBUG
void CParabot::setGoalViewDescr( const char *descr )
{
	strcpy( goalView, descr );
}


void CParabot::setGoalMoveDescr( const char *descr )
{
	strcpy( goalMove, descr );
}


void CParabot::setGoalActDescr( const char *descr )
{
	strcpy( goalAct, descr );
}
#endif

// types of damage to ignore...
#define IGNORE_DAMAGE (~(0x82))


void CParabot::registerDamage( int amount, Vec3D *origin, int type )
{
	const char *diname;
	EDICT *di = find_entityinsphere(0, origin, 5);
	if (di) {
		diname = STRING( di->v.classname );
		if (!Q_STREQ(diname, "player")) {
			if (Q_STREQ(diname, "grenade")) {
			} else if (Q_STREQ(diname, "monster_satchel")) {
			} else if (Q_STREQ(diname, "monster_tripmine")) {
			} else if (Q_STREQ(diname, "rpg_rocket")) {
			} else if (Q_STREQ(diname, "bodyque")) {
			} else
				DEBUG_MSG("DAMAGE BY %s\n", diname);
		} else {
			if (vcomp(&di->v.origin, botPos())) DEBUG_MSG("SELF-DAMAGE\n");
			//else DEBUG_MSG("DAMAGE BY PLAYER\n");
		}
	} else {
		DEBUG_MSG("NO  FOUND\n");
	}

	if (type & IGNORE_DAMAGE) {
		int ci = mapcells_getcellid(ent);
		if ( ci != NO_CELL_FOUND) {
			cell_addenvdamage(mapcells_getcell(ci), amount);
			DEBUG_MSG( "ENV_DAMAGE!\n" );
		}
	}

	if (vcomp(origin, &zerovector))	// falling etc.
		return;

	// who is shooting at the bot?
	bool found = false;
	EDICT *player = 0;
	for (int i = 1; i <= com.globals->maxclients; i++) {
		Vec3D dir;
		float dist;

		player = edictofindex( i );
		if (!is_alive(player)	// skip player if not alive
		    || (player == ent)		// skip self
		    || (player->v.solid == SOLID_NOT))
			continue;

		vsub(&player->v.origin, origin, &dir);
		dist = vlen(&dir);
		if (dist < 30 ) {
			found = true;
			break;
		}
	}

	if (!found) {	// now we'll have to guess
		//DEBUG_MSG( "Guessing!\n" );
		for (int i = 1; i <= com.globals->maxclients; i++) {
			player = edictofindex( i );
			if (!is_alive(player)	// skip player if not alive
			    || (player == ent)	// skip self
			    || (player->v.solid == SOLID_NOT)) continue;	

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
			makevectors(&player->v.v_angle);
			Vec3D dir;
			vsub(botPos(), &player->v.origin, &dir);
			normalize(&dir);
			float aim = dotproduct(&com.globals->fwd, &dir );
			//DEBUG_MSG( "Aim=%.3f\n", aim );
			if ((aim < 0.95)
			    || !canshootat(player, botPos()))
				continue;

			found = true;
			break;
		}
	}
	/*
	player = ent->v.dmg_inflictor;
	if ( player && strlen(STRING(player->v.netname)) > 0) found = true;
*/

	if (found) {
#if _DEBUG
		const char *botName = STRING(ent->v.netname);
		const char *inflictorName = STRING(player->v.netname);
		DEBUG_MSG( "%s hurt by %s\n", botName, inflictorName );
#endif
	} else {
		// DEBUG_MSG( "No damage inflictor found!\n" );
		player = NULL;
	}
	perception_addattack(&senses, player, amount);
}


void CParabot::registerDeath( EDICT *killer, const char *wpnName )
// bot has been killed
{
	// was it one of our fellow players? (not worldspawn)
	if (killer && killer != this->ent && strlen(STRING(killer->v.netname)) > 0) {
		int botCell = mapcells_getcellid(ent);
		int killerCell = mapcells_getcellid(killer);
		if (botCell >= 0 && killerCell >= 0 && mapcells_lineofsight(botCell, killerCell)) {
			Vec3D dir;

			vadd(&killer->v.origin, botPos(), &dir);
			kills_adddir(&dir, mapcells_getcell(botCell)->data.sectors);
		}
		chat_registergotkilled(ent, killer, wpnName);
	}
}

void CParabot::registerKill( EDICT *victim, const char *wpnName )
// bot has killed opponent
{
	chat_registerkilledplayer(victim, ent, wpnName);
	// check if bot was camping and should camp longer:
	if (actualNavpoint && navpoint_type(actualNavpoint) == NAV_S_CAMPING) {
		campTime = 0;
	}
}

bool CParabot::hasLongJump()
{
	if ( !(mod_id == VALVE_DLL || mod_id == AG_DLL || mod_id == HUNGER_DLL || mod_id == GEARBOX_DLL) )
		return false;
	// const char *value = g_engfuncs.pfnGetPhysicsKeyValue( ent, "slj");

	// if (Q_STREQ(value, "1"))
	//	return true;

	return false;
}


void CParabot::reportEnemySpotted()
{
	if (actualPath) {
		path_reportenemyspotted(actualPath);
		path_cancelattempt(actualPath);
		journey_savepathdata(&actualJourney);
		journey_cancel(&actualJourney);
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
	if (mod_id == VALVE_DLL || mod_id == AG_DLL || mod_id == HUNGER_DLL || mod_id == GEARBOX_DLL) {
		if (hasLongJump())
			pathMask |= PATH_NEED_LONGJUMP;

		if (combat_hasweapon(&combat, VALVE_WEAPON_GAUSS))
			pathMask |= PATH_NEED_GAUSSJUMP;
	}
	if (ent->v.health > 80) pathMask |= PATH_CAUSES_DAMAGE;

	if (ent->v.health < 30)
		setJourneyMode( JOURNEY_LONELY );
	else if (needs_wishforcombat(&needs) > 7)
		setJourneyMode( JOURNEY_CROWDED );
	else if (needs_wishforitems(&needs) > 4)
		setJourneyMode( JOURNEY_RELIABLE );
	else
		setJourneyMode( JOURNEY_FAST );
	
	// check if this bot has been assigned a reachable target
	if ((botNr == slot)
	    && (botTarget >= 0)
	    && (mapgraph_getjourney(navpoint_id(actualNavpoint), botTarget, pathMask, &actualJourney))) {
		DEBUG_MSG("Trying to reach assigned target:\n");
		targetNav = botTarget;
		botTarget = -1;
	} else { // if not, find a wish target
		//initWishList();	// not necessary, but more accurate
		targetNav = mapgraph_getwishjourney(navpoint_id(actualNavpoint), &needs, pathMask, &actualJourney, ent );
	}

	if (targetNav >= 0) {	// found a journey
		// NAVPOINT *nav = getNavpoint( targetNav );
		actualPath = journey_getnextpath(&actualJourney);
		assert( actualPath != 0 );
		path_startattempt(actualPath, worldtime());
		path_getnextwaypoint(actualPath, &waypoint);
		botState = PB_ON_TOUR;
		// DEBUG_MSG( "Switched to ON_TOUR, approach ");  
		//nav.print();  DEBUG_MSG( "\n" );
		return true;
	} else {
		actualPath = 0;
		return false;
	}
}


void CParabot::getRoamingTarget()
{
	huntingFor = 0;
	fleeingFrom = 0;

	// check if this bot has been assigned a target
	if ((botNr == slot) && (botTarget >= 0)) { 
		roamingTarget = getNavpoint(botTarget);
		botTarget = -1;
		DEBUG_MSG( "Trying to reach assigned target:\n" );
	} else { // if not, find a roaming navpoint
		/*roamingTarget = mapgraph_getnearestroamingnavpoint( ent, actualNavpoint );
		if (!roamingTarget) {	// no linked navpoints found -> just get the nearest
			roamingTarget = mapgraph_getnearestnavpoint( botPos() );
		}*/
		short start = mapcells_getcellid(ent);
		if (start >= 0) {
			setRoamingIndex(mapcells_getpathtoroamingtarget(start, ent, roamingRoute));
			if (roamingIndex >= 0) {
				roamingTarget = cell_getnavpoint(mapcells_getcell(roamingRoute[0]));
			} else {	// no path found:
				roamingTarget = mapgraph_getnearestroamingnavpoint(ent, actualNavpoint);
				// DEBUG_MSG(" E!" );
			}
		} else {
			roamingTarget = mapgraph_getnearestroamingnavpoint(ent, actualNavpoint);
			setRoamingIndex(-1);
		}
	}
	assert( roamingTarget != 0 );
	roaming_reset(&pathfinder, navpoint_pos(roamingTarget));
	roamingCount = PB_ROAMING_COUNT;
	botState = PB_ROAMING;
	// DEBUG_MSG( "Switched to ROAMING, approach " );
	//roamingTarget->print(); DEBUG_MSG( "\n" );
	// assert that everything fits
	assert( pathfinder.pev == ent );
	//assert( action.ent == ent );
	assert( pathfinder.action == &action );
}


void CParabot::approachRoamingTarget()
{
	assert(roamingTarget != 0);

	if (roamingIndex >= 0) {
		followActualRoute();
		return;
	}
	if (navpoint_reached(roamingTarget, ent)) {
		// DEBUG_MSG( "Roaming target reached\n" );
		navpoint_reportvisit(roamingTarget, ent, worldtime()); //now in observer
		actualNavpoint = roamingTarget;
		roamingTarget = 0;
	} else {
		//if (roamingCount % 10 == 0) debugBeam(botPos(), roamingTarget->pos(ent), 5, 0);
		roamingCount--;
		if (roamingCount < 0) {
			roamingTarget = 0;
		} else {
			Vec3D rtPos;

			navpoint_pos(roamingTarget, ent, &rtPos);
			roaming_checkway(&pathfinder, &rtPos);
			action_setviewlikemove(&action);
			if (action_gotstuck(&action) || roaming_targetnotreachable(&pathfinder)) {
				navpoint_donotvisitbefore(roamingTarget, ent, worldtime() + 10.0f);
				roamingTarget = 0;
				action_resetstuck(&action);
			}
		}
	}
}

void CParabot::pathFinished()
// called when path is finished
{
	// DEBUG_MSG( "Path finished\n" );
	assert(actualPath != 0);
	path_reporttargetreached(actualPath, ent, worldtime());
	journey_savepathdata(&actualJourney);
	actualNavpoint = path_endnav(actualPath);
	if (needs_newpriorities(&needs)) {
		journey_cancel(&actualJourney);		// cancel current journey
		actualPath = 0;				// set actualPath to 0 to invoke new journey
		needs_affirmpriorities(&needs);	// do this only once
		DEBUG_MSG("Found new item priorities, canceling journey!\n");
	} else if (journey_continues(&actualJourney)) {
		actualPath = journey_getnextpath(&actualJourney);
		assert(actualPath != 0);
		path_startattempt(actualPath, worldtime());
		path_getnextwaypoint(actualPath, &waypoint);
		if (navpoint_type(path_startnav(actualPath)) == NAV_S_BUTTON_SHOT) {
			if (navpoint_istriggerfor(path_startnav(actualPath), path_endnav(actualPath))) {
				// bot must shoot this button!
				vcopy(navpoint_pos(getNavpoint(navpoint_special(path_startnav(actualPath)))), &shootObjectPos);
				mustShootObject = true;
			}
		}
		// DEBUG_MSG( "Continue journey, Need=%.1f\n", maxWish );
	} else {
		actualPath = 0;
	}
}


void CParabot::pathFailed()
// called when path is *NOT* finished
{
	assert( actualPath != 0 );
#if _DEBUG
	actualPath->print();
	DEBUG_MSG( " failed\n" );
#endif
	path_reporttargetfailed(actualPath);
	journey_savepathdata(&actualJourney);
	actualNavpoint = 0;
	actualPath = 0;
	journey_cancel(&actualJourney);
}


bool CParabot::positionReached(Vec3D *pos)
	// returns true if bot has reached pos within a distance of PB_REACH_DISTANCE
{
	Vec3D dir;
	float dist;

	vsub(botPos(), pos, &dir);
	dist = vlen(&dir);

	if (dist < PB_REACH_DISTANCE)
		return true;

	return false;
}


void CParabot::pathCheckWay()
// checks if bot has to duck, strafe, or shoot breakable
// if necessary adds these actions
{
	TRACERESULT tr, trLeft, trRight;
	Vec3D startTr, endTr, planeAngle, fwd1, fwd2, right;
	
	Vec3D aDir = {0, action_moveangleyaw(&action), 0};	// use only yaw angle
	makevectors(&aDir);

	// check if bot needs to duck
	if (mod_id != DMC_DLL ) {
		vma(botPos(), 16.0f, &com.globals->fwd, &startTr);
		startTr.z += 36.0f;
		vma(botPos(), (16.0f + 36.0f), &com.globals->fwd, &endTr);
		trace_line(&startTr, &endTr, true, false, ent, &tr);
		if (tr.fraction < 1.0) {
			vma(botPos(), 16.0f, &com.globals->fwd, &startTr);
			trace_line(&startTr, &endTr, false, false, ent, &tr);
			if (tr.fraction == 1.0f)
				action_add(&action, BOT_DUCK, NULL);
			//else DEBUG_MSG( "pathCheckWay: completely blocked\n");
		}
	}

	// check if bot needs to strafe
	vma(botPos(), 8.0f, &com.globals->fwd, &fwd1);
	vma(botPos(), 50.0f, &com.globals->fwd, &fwd2);
	vscale(&com.globals->right, 16.0f, &right);
	vadd(&fwd1, &right, &startTr);
	vadd(&fwd2, &right, &endTr);
	trace_line(&startTr, &endTr, false, false, ent, &trRight);
	vinv(&right);
	vadd(&fwd1, &right, &startTr);
	vadd(&fwd2, &right, &endTr);
	trace_line(&startTr, &endTr, false, false, ent, &trLeft);
	if ((trRight.fraction < 1.0f) && (trLeft.fraction == 1.0f)) {
		// DEBUG_MSG("Something in right front!\n");
		vectoangles(&trRight.planenormal, &planeAngle);
		if (planeAngle.x < 40.0f)
			action_add(&action, BOT_STRAFE_LEFT, NULL);
	} else if ((trLeft.fraction < 1.0f) && (trRight.fraction == 1.0f)) {
		// DEBUG_MSG("Something in left front!\n");
		vectoangles(&trLeft.planenormal, &planeAngle);
		if (planeAngle.x < 40.0f)
			action_add(&action, BOT_STRAFE_RIGHT, NULL);
	}

	if ( actualPath ) {
		// check if some breakable object needs to be destroyed
		NAVPOINT *target = path_endnav(actualPath);
		assert( target != 0 );
		if (navpoint_type(target) == NAV_F_BREAKABLE ) {
			if (!navpoint_entity(target)) {
				DEBUG_MSG( "ERROR in pathCheckWay: No entity found!\n" );
				return;
			}
			if (navpoint_entity(target)->v.health > 0) {	// has to be destroyed
				if (navpoint_visible(target, ent )) {
					weaponhandling_attack(&combat.weapon, navpoint_pos(target), 0.3f, NULL);
				}
				//else DEBUG_MSG( "Can't see breakable\n" );
			}
		}
		// check if bot needs to wait for platform
		if (path_waitforplatform(actualPath)) {
			Vec3D lastPos, platPos, dir, size = {2.0f, 2.0f, 2.0f};
			path_getlastwaypointpos(actualPath, ent, &lastPos);
			path_nextplatformpos(actualPath, &platPos);
			vsub(&platPos, &lastPos, &dir);

			if (vlen(&dir) < 50  &&
			    ent->v.groundentity != 0 && 
				vcomp(&ent->v.groundentity->v.size, &size))	// worldSpawn
			{
				DEBUG_MSG(" EVADING PLATFORM\n" );	// lastPos useless
				vsub(&platPos, botPos(), &dir);
				vsub(botPos(), &dir, &dir);
				action_setmovedir(&action, &dir, 0);
				action_setmaxspeed(&action);
				//botNr = slot;	// film this!
			} else {
				// DEBUG_MSG("WAIT FOR PLATFORM!\n" );
				action_add(&action, BOT_STOP_RUNNING, NULL);
				action_setspeed(&action, 0);
				//action.setMoveDir(lastPos);
				//action.setSpeed(10 * vlen(vsub(lastPos, botPos())));
			}
		}
	}
}

void CParabot::checkForTripmines()
{
	// TODO: only check when beam visible
	TRACERESULT trAll, trHalf;
	EDICT *mine = perception_getnearesttripmine(&senses);
	
	if (!mine)
		return;
	
	Vec3D mine_vecDir, moveDir, fakeStart, fakeEnd;
	vcopy(&mine->v.angles, &mine_vecDir);
	fixangle(&mine_vecDir);
	makevectors(&mine_vecDir);
	vcopy(&com.globals->fwd, &mine_vecDir);
	action_getmovedir(&action, &moveDir);
	vma(&mine->v.origin, -64.0f, &moveDir, &fakeStart);
	vma(&fakeStart, 512.0f, &mine_vecDir, &fakeEnd);
	
	//debugBeam( fakeStart, fakeEnd, 250, 0 );
	// DEBUG_MSG( "BOTS PAUSED!\n" );
	//pb_pause = true;
	
	bool tAll = false;

	trace_line(&fakeStart, &fakeEnd, false, false, mine, &trAll);
	if (trAll.hit == ent) tAll = true;

	if (tAll) {
		if (mine_vecDir.z == 0) { 
			if (trAll.endpos.z < (ent->v.absmin.z + 40.0f)) {
				action_add(&action, BOT_JUMP, NULL);
				DEBUG_MSG( "JUMP - Trying to evade Tripmine ALL\n" );
			} else {
				action_add(&action, BOT_DUCK_LONG, NULL);
				DEBUG_MSG( "DUCK - Trying to evade Tripmine ALL\n" );
			}
		} else
			DEBUG_MSG( "Tripmine vertical, can't evade\n" );
	}

	if (actualPath) {	// check if tripmine pathtarget
		if (navpoint_type(path_endnav(actualPath)) == NAV_S_USE_TRIPMINE) {
			Vec3D dir;
			float mineDist, botDist;
			vsub(navpoint_pos(path_endnav(actualPath)), &mine->v.origin, &dir);
			mineDist = vlen(&dir);
			vsub(navpoint_pos(path_endnav(actualPath)), &ent->v.origin, &dir);
			botDist = vlen(&dir);
			if ( (mineDist < 50) && (botDist < 100) ) {
				DEBUG_MSG( "Canceling path - tripmine found at end!\n" );
				pathFinished();
			}
		}
	}
}

void CParabot::followActualPath()
{
	assert( actualPath != 0 );

	if (path_finished(actualPath, ent)) {
		pathFinished();
	} else {
		if (mustShootObject) {
			switch (mod_id) {
				case AG_DLL:
				case HUNGER_DLL:
				case GEARBOX_DLL:
				case VALVE_DLL:		weaponhandling_setpreferredweapon(&combat.weapon, VALVE_WEAPON_GLOCK, 1 );		
									break;
				case HOLYWARS_DLL:	weaponhandling_setpreferredweapon(&combat.weapon, HW_WEAPON_DOUBLESHOTGUN, 1 );	
									break;
				case DMC_DLL:		weaponhandling_setpreferredweapon(&combat.weapon, DMC_WEAPON_QUAKEGUN, 1 );		
									break;
			}
			// arm preferred weapon, params don't really matter...
			bool bestArmed = weaponhandling_armbestweapon(&combat.weapon, 200, 0.95f, 0 );
			if (bestArmed) {
				if (weaponhandling_attack(&combat.weapon, &shootObjectPos, 0.95f, NULL))
					mustShootObject = false;
				else
					return;	// don't do anything else...
			} else
				return;	// don't do anything else...
		}
		if (path_waypoint_reached(&waypoint, ent)) {
			Vec3D wpos;
			vcopy(path_waypoint_pos(&waypoint, ent), &wpos);
			action_add(&action, path_getnextaction(actualPath), &wpos);	// if there's something to do...
			path_reportwaypointreached(actualPath);		// confirm waypoint
#if _DEBUG
			Vec3D oldWP = waypoint.pos( ent );
#endif
			path_getnextwaypoint(actualPath, &waypoint);	// get next one
#if _DEBUG
			debugBeam( waypoint.pos( ent ), oldWP, 50, 1 );
			float wpd = vlen(vsub(waypoint.pos(ent), oldWP));
			// DEBUG_MSG( "Reached new WP after %.f\n", wpd );
#endif
		}
		int prior;
		Vec3D proposedViewPos;
		path_getviewpos(actualPath, ent, &prior, &proposedViewPos);
		action_setviewdir(&action, &proposedViewPos, prior);	// set viewAngle
		action_setmovedir(&action, path_waypoint_pos(&waypoint, ent), 0);		// set moveAngle and speed

		if (!(is_underwater(ent) || path_waypoint_isonladder(&waypoint))) {
			Vec3D dir;
			float dist2d, dist;
			
			vsub(path_waypoint_pos(&waypoint, ent), botPos(), &dir);

			dist2d = vlen2d((Vec2D *)&dir);
			dist = vlen(&dir);

			if (dist2d < 30 && dist > 50)
				action_setspeed(&action, 8 * dist2d);
			else
				action_setmaxspeed(&action);
		} else
			action_setmaxspeed(&action);
		pathCheckWay();
		if (mod_id == VALVE_DLL || mod_id == AG_DLL || mod_id == HUNGER_DLL || mod_id == GEARBOX_DLL)
			checkForTripmines();
		if (!actualPath) return;	// maybe tripmine canceled path

		if (path_cannotbecontinued(actualPath, ent)) {
			DEBUG_MSG( "Path failed.\n" );
			pathFailed();
		} else if (action_gotstuck(&action)) {
			pathFailed();
			action_resetstuck(&action);
		}

	}
}


void CParabot::followActualRoute()
{
	short botCell = mapcells_getcellid( ent );
	if (botCell < 0 ) return;
	short targetCell = roamingRoute[roamingIndex];
	if (cellToReach != targetCell) {
		cellToReach = targetCell;
		cellTimeOut = worldtime() + 1.0;
	}
	Vec3D target;

	cell_pos(mapcells_getcell(targetCell), &target);

	// DEBUG_MSG( "." );
	if (botCell == targetCell) {
		// DEBUG_MSG( "Reached index %i\n", roamingIndex );
		if ( roamingIndex > 0 ) {
			roamingIndex--;
			debugBeam( target, mapcells_getcell( roamingRoute[roamingIndex] ).pos(), 50, 2 );
		} else {
			// DEBUG_MSG( "TARGET REACHED.\n" );
			NAVPOINT *nav = cell_getnavpoint(mapcells_getcell(botCell));
			if ( nav && nav == roamingTarget ) {
				navpoint_pos(nav, ent, &target);
				if (navpoint_reached(nav, ent)) {
					navpoint_reportvisit(nav, ent, worldtime());
					actualNavpoint = nav;
					roamingTarget = 0;
					setRoamingIndex( -1 );
					return;
				}
			} else {
				roamingTarget = 0;
				setRoamingIndex( -1 );
				return;
			}
		}
	} else if (botCell != roamingRoute[roamingIndex+1]) {
		Vec3D dir;
		float dist2target, normDist;

		vsub(&target, cell_pos2(mapcells_getcell(botCell)), &dir);
		dist2target = vlen(&dir);
		vsub(&target, cell_pos2(mapcells_getcell(roamingRoute[roamingIndex + 1])), &dir);
		normDist = vlen(&dir);
		if (dist2target > (normDist + 250)) {
			//botNr = slot;
			DEBUG_MSG( "ROUTE ERROR in %s!\n", goalMove );
			debugBeam( target, cell_pos(mapcells_getcell(botCell)), 250, 0 );
			setRoamingIndex( -1 );
			return;
		}
	}
	
	action_setviewdir(&action, &target, 0);	// set viewAngle
	action_setmovedir(&action, &target, 0);	// set moveAngle and speed
	if(!is_underwater(ent)) {
		Vec3D dir;
		vsub(&target, botPos(), &dir);
		float dist2d = vlen2d((Vec2D *)&dir);
		float dist = vlen(&dir);
		if (dist2d < 30 && dist > 50)
			action_setspeed(&action, 8.0f * dist2d);
		else
			action_setmaxspeed(&action);
	} else
		action_setmaxspeed(&action);

	pathCheckWay();
	
	if (action_gotstuck(&action) || (worldtime() > cellTimeOut) ) {
		Vec3D dir;

		action_resetstuck(&action);
		vsub(&lastJumpPos, botPos(), &dir);
		if (vlen(&dir) > 50) {
			action_add(&action, BOT_JUMP, NULL);
			vcopy(botPos(), &lastJumpPos);
			cellTimeOut = worldtime() + 1.0;
		} else {
			// bisherigen traffic auf Teilstrecke auswerten, falls <3 Verbindung löschen
			if ( cell_gettraffic(mapcells_getcell(botCell), targetCell) < 3 ) {
				if (cell_delneighbour(mapcells_getcell(botCell), targetCell)) {
#if _DEBUG
					DEBUG_MSG( "Deleted cell neighbour.\n" );
					Vec3D c1 = cell_pos(mapcells_getcell(botCell)) + Vector(0,0,8);
					Vec3D c2 = cell_pos(mapcells_getcell(targetCell));
					debugBeam( c1, c2, 250, 0 );
					debugMarker( c2, 250 );
#endif
				} else {
#if _DEBUG
					DEBUG_MSG( "Could not delete cell neighbour.\n" );
					Vec3D c0 = cell_pos(mapcells_getcell(botCell)) -Vector(0,0,8);
					Vec3D c1 = cell_pos(mapcells_getcell(roamingRoute[roamingIndex + 1])) +Vector(0,0,8);
					Vec3D c2 = cell_pos(mapcells_getcell(targetCell));
					debugBeam( c1, c2, 250, 0 );	// rot vom Vorgänger
					debugBeam( c0, c2, 250, 1 );	// grün vom bot aus
					debugMarker( c2, 250 );
#endif
					cell_delneighbour(mapcells_getcell(roamingRoute[roamingIndex + 1]), targetCell);
				}
			}
			// PB_Cell tc = mapcells_getcell( botCell );
			if (roamingTarget)
				navpoint_donotvisitbefore(roamingTarget, ent, worldtime() + 10.0f);
			lastJumpPos = zerovector;
			roamingTarget = 0;
			setRoamingIndex( -1 );
			return;
		}
	}
}

void CParabot::executeGoals()
{
	tGoalFunc	goalFunction;
	PERCEPT		*trigger;

	goalfinder_check(&goalFinder);
	goalfinder_synchronize(&goalFinder);
	
	checkForBreakpoint( BREAK_GOALS );

	//debugFile( STRING(ent->v.netname) );
	for (int i=0; i<MAX_GOALS; i++) {
		goalFunction = goalfinder_bestgoal(&goalFinder, i);
		if (goalFunction) {
			trigger		 = goalfinder_trigger(&goalFinder, i);
			if (trigger!=0) assert( trigger->pClass > 0 && trigger->pClass <= MAX_PERCEPTION );
			(*goalFunction)( this, trigger );
		}
	}
	//debugFile( "exec ok\n" );
}

void CParabot::botThink()
{
	assert( ent != 0 );

	/*CBaseEntity *wpn = getActiveItem( ent );
	if (wpn) {
		const char *wpncl = STRING( wpn->pev->classname );
	}*/

	activeBot = slot;

	// execute in 10Hz steps:
	float difTime = worldtime()-lastThink;
	if ( difTime >= 0 && difTime < 0.1) {
		action_perform(&action); // execute planned actions
		if (action_pausing(&action))
			cellTimeOut = worldtime() + 1.0;	// adjust cellTimeOut if pausing
		return;
	}
	lastThink = worldtime();
	
	if (actualNavpoint) {	// check for navpoint
		if (!navpoint_reached(actualNavpoint, ent)) actualNavpoint = 0;
	}

	weaponhandling_initcurrentweapon(&combat.weapon);

	needs_updatewishlist(&needs);

	action_reset(&action);		// initializes action flag and other variables

	perception_collectdata(&senses);
	goalfinder_init(&goalFinder, this);
	goalfinder_analyzeunconditionalgoals(&goalFinder);
	goalfinder_analyze(&goalFinder, &senses);
	//goalfinder_analyze(&goalFinder, tactics);

	executeGoals();
	// DEBUG_MSG( "%i enemies\n", senses.numEnemies );

	// check if any grenades have to b thrown (overrides former actions)
	weaponhandling_checkforforcedattack(&combat.weapon);

	action_perform(&action); // execute planned actions
	
	if (action_pausing(&action))
		cellTimeOut = worldtime() + 1.0;	// adjust cellTimeOut if pausing
}
