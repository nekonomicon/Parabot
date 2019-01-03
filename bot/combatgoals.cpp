#include "goals.h"
#include "parabot.h"
#include "sectors.h"
#include "vistable.h"
#include "cell.h"
#include "mapcells.h"

extern int mod_id;
extern int botNr;
extern int clientWeapon[32];

void
goal_huntenemy(CParabot *pb, PERCEPT *item)
// hunt enemy that is trackable but currently not seen
{
	pb->reportEnemySpotted();	// not fully correct since not spotted yet...

	if (pb->roamingIndex >= 0 && pb->huntingFor == item->entity) {
		// already hunting
		pb->setGoalMoveDescr( "HuntEnemy (FollowRoute)" );
		pb->followActualRoute();
	} else {
		short start = mapcells_getcellid(pb->ent);
		short enemyId = mapcells_getcellid(item->entity);
		if (start >= 0 && enemyId >= 0 ) {
			int pl;
			if ((pl = mapcells_getpathtoattack(start, enemyId, pb->roamingRoute)) > 0) {
				pb->setRoamingIndex(pl);
				pb->huntingFor = item->entity;
				// Vector startV = mapcells_getcell( start ).pos();
				// Vector endV;
				DEBUG_MSG( "HuntLength = %i", pl );				
			}
		}
		pb->setGoalMoveDescr( "HuntEnemy (FindRoute)" );
	}
	DEBUG_MSG( "Hunting player\n" );

	//botNr = pb->slot;	// film this!
}

float
weight_huntenemy(CParabot *pb, PERCEPT *item)
{
	assert( item != 0 );
	// only hunt players that haven't been seen yet and are trackable
	if (percept_hasbeenspotted(item) || !(percept_istrackable(item))) return 0;

	return needs_wishforcombat(&pb->needs);
}

void
goal_fleeenemy(CParabot *pb, PERCEPT *item)
// flee taking cover
{
	int pl;

	pb->reportEnemySpotted();
	
	if (pb->roamingIndex>=pb->roamingBreak && pb->fleeingFrom==item->entity) {
		// already fleeing
		pb->setGoalMoveDescr( "FleeEnemy (FollowRoute)" );
		pb->followActualRoute();
	} else {
		short start = mapcells_getcellid( pb->ent );
		short enemyId = mapcells_getcellid(item->entity);
		if (start >= 0 && enemyId >= 0) {
			if (mapcells_lineofsight(start, enemyId)) {
				// get fastest path out of sight
				if ((pl = mapcells_getpathtocover(start, enemyId, pb->roamingRoute)) > 0) {
					pb->setRoamingIndex(pl);
					if (pl > 4) pb->roamingBreak = pl - 4;
					else pb->roamingBreak = 0;
					pb->fleeingFrom = item->entity;
				}
			} else {
				// stay covered and get away
				if ((pl = mapcells_getpathforsneakyescape(start, enemyId, pb->roamingRoute)) > 0) {
					pb->setRoamingIndex(pl);
					if (pl > 4) pb->roamingBreak = pl - 4;
					else pb->roamingBreak = 0;
					pb->fleeingFrom = item->entity;
				}
			}
		}
		pb->setGoalMoveDescr( "FleeEnemy (FindRoute)" );
	}
}

float
weight_fleeenemy(CParabot *pb, PERCEPT *item)
{
	if (item->distance > 150 && item->rating < 0) {
		return (item->rating * item->rating);
	}
	return 0;
}

void
goal_takecover(CParabot *pb, PERCEPT *item)
// move to next cover from enemy position
{
	pb->reportEnemySpotted();
	
	if (pb->roamingIndex >= 0 && pb->fleeingFrom == item->entity) {
		// already heading towards cover
		pb->setGoalMoveDescr("TakeCover (FollowRoute)");
		pb->followActualRoute();
	} else {
		short start = mapcells_getcellid(pb->ent);
		short enemyId = mapcells_getcellid(item->entity);
		if (start >= 0 && enemyId >= 0) {
			if (mapcells_lineofsight(start, enemyId)) {
				pb->setGoalMoveDescr("TakeCover (FindRoute)");
				// get fastest path out of sight
				int pl;
				if ((pl = mapcells_getpathtocover(start, enemyId, pb->roamingRoute)) > 0) {
					pb->setRoamingIndex(pl);
					pb->fleeingFrom = item->entity;
				}
			} else {
				pb->setGoalMoveDescr("TakeCover (Rest)");
			}
		}
	}
}

float
weight_takecover(CParabot *pb, PERCEPT *item)
{
	assert(item != 0);

	if ((worldtime() - item->lastseentime) > 1) return 0;	// not seen for too long
	if (!percept_hasbeenspotted(item)) return 0;				// we haven't seen anything yet
	if (item->distance < 150) return 0;						// too close
	if (item->rating > 0) return 0;							// we have advantage

	float weight = needs_wishforcombat(&pb->needs) - item->rating;

	// are we already in a combat? -> keep on to it!
	if (percept_ishurtingbot(item) || percept_isalert(item)) weight += 3;

	if (percept_hashighpriority(item)) {
		weight -= 2;
	}
	if (percept_hasfocus(item)) weight += 1;	// stick to current enemy

	return weight;
}

void
goal_closecombat(CParabot *pb, PERCEPT *item)
// combat movements for short range
{
	assert( item != 0 );
	pb->reportEnemySpotted();
/*
	float dist = vlen(pb->botPos() - item->entity->v.origin);
	if ((dist < 800) && percept_isvisible(item)) {
		pb->combat.closeCombatMovement( *item );
		pb->setGoalMoveDescr( "CloseCombat" );
	} else {
		if (!percept_isunderpreemptivefire(item)) {
			pb->pathfinder.checkWay(item->lastseenpos);
			pb->setGoalMoveDescr("CloseCombat (Follow)");
		} else {
			pb->setGoalMoveDescr("CloseCombat (Preemptive)");
		}
	}
	*/
	if (item->distance < 100 && percept_isvisible(item)) {
		pb->setGoalMoveDescr("CloseCombat (vaBanque)");
		action_setmovedir(&pb->action, &item->lastseenpos, 0);
		action_setmaxspeed(&pb->action);
	} else if (pb->roamingIndex >= pb->roamingBreak && pb->huntingFor==item->entity ) {
		// already hunting
		pb->setGoalMoveDescr("CloseCombat (FollowRoute)");
		pb->followActualRoute();
	} else {
		short start = mapcells_getcellid(pb->ent);
		short enemyId = mapcells_getcellid(item->entity);
		if (start >= 0 && enemyId >= 0) {
			int clientIndex = indexofedict(pb->ent) - 1;
			assert((clientIndex >= 0) && (clientIndex < 32));
			int wid = clientWeapon[clientIndex];
			WEAPON w;
			weapon_construct2(&w, wid);
			float minDist = weapon_bestdistance(&w);
			int pl;
			if ((pl = mapcells_getoffensivepath(start, enemyId, minDist, pb->roamingRoute)) > 0) {
				pb->setRoamingIndex(pl);
				if (pl > 4) pb->roamingBreak = pl - 4;
				else pb->roamingBreak = 0;
				pb->huntingFor = item->entity;
			}
		}
		pb->setGoalMoveDescr("CloseCombat (FindRoute)");
	}
	goal_shootatenemy(pb, item);
}

float
weight_closecombat(CParabot *pb, PERCEPT *item)
{
	assert(item != 0);

	if ((worldtime() - item->lastseentime) > 5) return 0;	// not seen for too long
	//if (!pb->combat.weapon.bestWeaponUsable()) return 0;
	if (!percept_hasbeenspotted(item)) return 0;				// we haven't seen anything yet
	if (item->distance < 100) return 15 + (item->rating / 5);	// va banque
	if (item->rating < -3) return 0;						// too bad, better don't try that...

	float weight = needs_wishforcombat(&pb->needs) + item->rating;

	// are we already in a combat? -> keep on to it!
	if (percept_ishurtingbot(item) || percept_isalert(item)) weight += 3;
	// check if enemy is vulnerable near charger
	if (!percept_ismoving(item) && (item->entity->v.button & ACTION_USE)) {
		DEBUG_MSG( "ENEMY CHARGING...\n" );
		weight += 2;
	}
	if (percept_hashighpriority(item)) {
		weight += 5;
		//DEBUG_MSG( "Hunting the Saint!\n" );
	}
	if (percept_hasfocus(item)) weight += 1;	// stick to current enemy

	return weight;
}

void
goal_rangeattack(CParabot *pb, PERCEPT *item)
// combat movements for medium to long range
{
}

float
weight_rangeattack(CParabot *pb, PERCEPT *item)
{
	if (item->distance < 800) return 0;
	return 0;
}

void
goal_silentattack(CParabot *pb, PERCEPT *item)
// prepared fatal attack
{
//	botNr = pb->slot;
//	if (!camPlayer) startBotCam(edictofindex(1));
	pb->reportEnemySpotted();

	if (item->distance > 500) {		// too far to approach, just fire from current position
		action_add(&pb->action, BOT_DUCK, NULL);
		if (percept_canbeattackedbest(item)) {
			combat_shootatenemy(&pb->combat, &item->lastseenpos, 0.8);
			pb->setGoalMoveDescr("SilentAttack (ShootDistance)");
		} else {
			pb->setGoalMoveDescr("SilentAttack (SwitchingWpn)");
		}
	} else {
		switch (mod_id) {
			case AG_DLL:
			case HUNGER_DLL:
			case GEARBOX_DLL:
			case VALVE_DLL:		weaponhandling_setpreferredweapon(&pb->combat.weapon, VALVE_WEAPON_SHOTGUN, 2 );
								break;
			case HOLYWARS_DLL:	weaponhandling_setpreferredweapon(&pb->combat.weapon, HW_WEAPON_DOUBLESHOTGUN, 1 );
								break;
			case DMC_DLL:		weaponhandling_setpreferredweapon(&pb->combat.weapon, DMC_WEAPON_SUPERSHOTGUN, 1 );
								break;
		}
		if (item->distance > 50) {
			roaming_checkway(&pb->pathfinder, &item->lastseenpos);	// get near...
			pb->setGoalMoveDescr( "SilentAttack (Approaching)" );
		} else {
			action_add(&pb->action, BOT_DUCK, NULL);
			if (percept_canbeattackedbest(item)) {	// ...and attack from there
				//bool shot = pb->combat.shootAtEnemy(item->lastseenpos, weaponhandling_currenthighaimprob(&pb->combat.weapon));
				combat_shootatenemy(&pb->combat, &item->lastseenpos, weaponhandling_currenthighaimprob(&pb->combat.weapon));
				pb->setGoalMoveDescr("SilentAttack (ShootClose)");
			} else {
				pb->setGoalMoveDescr("SilentAttack (SwitchingWpn)");
			}
		}
	}
	item->flags |= PI_FOCUS1;
}

float
weight_silentattack(CParabot *pb, PERCEPT *item)
{
	assert( item != 0 );
	float weight = 0;

	if (perception_underfire(&pb->senses) || !weaponhandling_bestweaponusable(&pb->combat.weapon)) return 0;
	if (percept_isvisible(item) && !percept_isfacingbot(item) && !percept_ismoving(item)) {
		weight = 15;
		if (percept_hashighpriority(item)) weight += 5;
	}
	return weight;
}

#if 0
void
goalprepareambush(CParabot *pb, PERCEPT *item)
// move to ambush position and wait
{
}

float
weight_prepareambush(CParabot *pb, PERCEPT *item)
{
	return 0;
}
#endif

void
goal_shootatenemy(CParabot *pb, PERCEPT *item)
// shooting
{
	bool fired = false;

	assert( item != 0 );
	//debugFile("ShootAtEnemy" );
	if (percept_isvisible(item)) {
		if (percept_isaimingatbot(item) || percept_isalert(item))	{
		// enemy is aiming or alert: shoot with low accuracy
			fired = combat_shootatenemy2(&pb->combat, item->entity, (weaponhandling_currenthighaimprob(&pb->combat.weapon) - 0.2f) / 2.0f );
			pb->setGoalViewDescr( "ShootAtEnemy (LowAcc)" );
		} else {
		// else take time to aim well
			if (percept_canbeattackedbest(item)) {
				fired = combat_shootatenemy2(&pb->combat, item->entity, weaponhandling_currenthighaimprob(&pb->combat.weapon));
				pb->setGoalViewDescr("ShootAtEnemy (HighAcc)");
			} else {
				pb->setGoalViewDescr("ShootAtEnemy (SwitchingWpn)");
			}
			if (!perception_underfire(&pb->senses)) {
				action_setspeed(&pb->action, 0);
			}
		}
		if (fired) item->flags |= PI_ALERT;
	} else {			// enemy is not visible:
		if (percept_hasjustdisappeared(item)) {
			// start debug
/*			short path[128];
			short start = mapcells_getcellid( pb->ent );
			short enemyId = mapcells_getcellid( item->entity );
			if ( start>=0 && enemyId>=0 ) {
				int pl;
				if ( pl = mapcells_predictplayerpos( enemyId, start, path ) ) {
					Vector startV = mapcells_getcell( enemyId ).pos();
					Vector endV;
					if (pl != -1) {
						for (int l=(pl-1); l>=0; l--) {
							endV = mapcells_getcell( path[l] ).pos();
							debugBeam( startV, endV, 50 );
							startV = endV;
						}
					}
				}
			}*/
			// end debug
			makevectors(&pb->ent->v.v_angle);
			Vec3D botpos, pos, dir, prePos;
			eyepos(pb->ent, &botpos);
			vadd(&item->lastseenpos, &item->entity->v.view_ofs, &pos);
			vsub(&pos, &botpos, &dir);
			normalize(&dir);
			float dot = dotproduct(&com.globals->fwd, &dir);
			if ((dot > 0.7f) && (item->distance > 250.0f)) {
				// check if bot should throw grenade at last position:
				int flags = WF_NEED_GRENADE;
				if (pb->ent->v.waterlevel == 3) flags |= WF_UNDERWATER;
				int bestWeapon = weaponhandling_getbestweapon(&pb->combat.weapon, item->distance, action_targetaccuracy(&pb->action), flags );
				float bestScore = weaponhandling_getweaponscore(&pb->combat.weapon, bestWeapon, item->distance, action_targetaccuracy(&pb->action), flags, true );
				vma(&item->lastseenpos, -0.05f, &item->lastseenvelocity, &prePos);
				if ((bestScore > 0) && canshootat(pb->ent, &prePos)) {
					//botNr = pb->slot;
					//if ( !camPlayer ) startBotCam( edictofindex( 1 ) );

					item->flags &= ~PI_BEST_ARMED;
					item->flags |= PI_PREEMPTIVE;
					pb->preemptiveWeapon = bestWeapon;
					if ((mod_id==VALVE_DLL || mod_id == AG_DLL || mod_id == HUNGER_DLL || mod_id==GEARBOX_DLL) && bestWeapon==VALVE_WEAPON_RPG) 
						pb->preemptiveMode = 2;		// switch off RPG laserdot
					else
						pb->preemptiveMode = 1;
					DEBUG_MSG("Enemy disappereared, using preemptive fire\n");
				} else {
					//pb->preemptiveFire = false;
					DEBUG_MSG("No grenade or bot has moved\n");
				}
			}
			else DEBUG_MSG("Bot turned round or too near!\n");
			pb->setGoalViewDescr("ShootAtEnemy (JustDisappeared)");
		}
		if (percept_isunderpreemptivefire(item)) {
			if (!perception_underfire(&pb->senses)) {
				//pb->action.setSpeed( 0 );
			}
			weaponhandling_setpreferredweapon(&pb->combat.weapon, pb->preemptiveWeapon, pb->preemptiveMode);
			Vec3D prePos;
			vma(&item->lastseenpos, -0.05f, &item->lastseenvelocity, &prePos);
			if (percept_canbeattackedbest(item)) {
				fired = combat_shootatenemy(&pb->combat, &prePos, 0.5f);
				if (fired) pb->preemptiveFire = false;	// shoot only once
				pb->setGoalViewDescr("ShootAtEnemy (Preemptive:Shoot)");
			} else {
				DEBUG_MSG("Waiting for bestWeapon...");
				action_setaimdir(&pb->action, &prePos, NULL);
				pb->setGoalViewDescr("ShootAtEnemy (Preemptive:SwitchingWpn)");
			}
		} else {
			if (percept_istrackable(item)) {
				action_setaimdir(&pb->action, percept_predictedappearance(item, pb->botPos()), NULL);
				pb->setGoalViewDescr("ShootAtEnemy (Disappeared:PredictiveAiming)");
			} else {
				goal_lookaround(pb, item);
				pb->setGoalViewDescr("ShootAtEnemy (Disappeared:LookAround)");
			}
		}
	}
	//debugFile( " ok\n" );
	item->flags |= PI_FOCUS1;	// focus on this one (arm weapons...)
}

float
weight_shootatenemy(CParabot *pb, PERCEPT *item)
{
	assert(item != 0);
	float weight;

	if (!weaponhandling_bestweaponusable(&pb->combat.weapon)) return 0;

	if (!percept_istrackable(item)) {	// try to look at it:
		if (percept_ishurtingbot(item)) weight = 5;		// hidden attacker -> dangerous!
		else weight = weight_reacttounidentified(pb, item);
	} else {	// we see or saw this one:
		if (percept_ishurtingbot(item) || percept_isaimingatbot(item)) weight = 5;	// shoot if he attacks us
		else if (item->rating < -3 && !percept_isfacingbot(item)) weight = 0;	// too bad, don't evoke attention
		else weight = 2;							// default

		if (!percept_isvisible(item) && !percept_isunderpreemptivefire(item)) weight *= 0.5f;	// still look in direction if no other enemy around
		if (percept_hashighpriority(item) ) weight += 5;
	}
	if (weight > 0 && percept_hasfocus(item)) weight += 1;	// stick to current enemy

	return weight;
}

float
weight_shootatsnark(CParabot *pb, PERCEPT *item)
{
	float weight = 0.0f;
	if (percept_isvisible(item)) weight = (500 / item->distance);
	if (weight>3) weight = 3;
	return weight;
}

void
goal_bunnyhop(CParabot *pb, PERCEPT *item)
{
	static float nextHop[32];

	int i = pb->slot;
	if (worldtime() >= nextHop[i]) {
		action_add(&pb->action, BOT_JUMP, NULL);
		nextHop[i] = worldtime() + randomfloat(0.5, 1.5);
	} else if (worldtime() + 5.0f < nextHop[i]) {
		// level change
		nextHop[i] = worldtime();
	}
	pb->setGoalActDescr( "BunnyHop" );
}

float
weight_bunnyhop(CParabot *pb, PERCEPT *item)
{
	if (percept_inflictorknown(item)) return 1;
	return 5;
}

void
goal_armbestweapon(CParabot *pb, PERCEPT *item)
// chose best weapon
{
	float dist; 
	float hitProb;
	int botFlags = 0;

	if (item && percept_istrackable(item)) {
		dist = item->distance;
		hitProb = action_targetaccuracy(&pb->action);
		if (percept_isfacingbot(item)) {
			if ( hitProb > 0.2f && percept_targetaccuracy(item) > 0.2f ) 
				botFlags |= WF_IMMEDIATE_ATTACK;	// both can shoot
			else
				botFlags |= WF_FAST_ATTACK;			// hurry up
		} else {
			botFlags |= WF_SINGLE_SHOT_KILL;	// so that enemy gets no chance to see us
		}
		Vec3D botPos;
		vcopy(pb->botPos(), &botPos);
		if (percept_predictedappearance(item, &botPos)->z > (botPos.z + 20)) botFlags |= WF_ENEMY_ABOVE;
		else if (percept_predictedappearance(item, &botPos)->z < (botPos.z - 80)) botFlags |= WF_ENEMY_ABOVE;
		pb->combat.nextweaponcheck = worldtime() + CHECK_WEAPON_COMBAT;
	} else {
		dist = 250;		// expect medium distance
		hitProb = 0.2;	// and low accuracy
		pb->combat.nextweaponcheck = worldtime() + CHECK_WEAPON_IDLE;
	}

	if (pb->ent->v.waterlevel == 3) botFlags |= WF_UNDERWATER;

	// if best weapon already armed, store in item:
	checkForBreakpoint( BREAK_WEAPON );
	bool bestArmed = weaponhandling_armbestweapon(&pb->combat.weapon, dist, hitProb, botFlags );
	if (!bestArmed) pb->combat.nextweaponcheck = worldtime() + CHANGE_WEAPON_DELAY;
	if (bestArmed && item) item->flags |= PI_BEST_ARMED;
	pb->setGoalActDescr( "ArmBestWeapon" );
}

float
weight_armbestweapon(CParabot *pb, PERCEPT *item)
{
	float weight = 0;

	if (item && percept_istrackable(item) &&
		 (worldtime() - item->firstdetection > 0.5f)) {	// aim a while before decision
		if (((worldtime() + CHECK_WEAPON_COMBAT) < pb->combat.nextweaponcheck) ||
		     (worldtime() > pb->combat.nextweaponcheck ) ) weight = 5;
		if (percept_hasfocus(item)) weight = 10;
	} else {
		if (worldtime() > pb->combat.nextweaponcheck) weight = 5;
	}
	return weight;
}
