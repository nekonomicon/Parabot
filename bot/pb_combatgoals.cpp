#include "pb_goals.h"
#include "parabot.h"
#include "pb_mapcells.h"



extern int mod_id;
extern int botNr;
extern PB_MapGraph mapGraph;
extern PB_MapCells map;
extern int clientWeapon[32];





void goalHuntEnemy( CParabot *pb, PB_Percept*item )
// hunt enemy that is trackable but currently not seen
{
	pb->reportEnemySpotted();	// not fully correct since not spotted yet...

	if ( pb->roamingIndex>=0 && pb->huntingFor==item->entity ) {
		// already hunting
		pb->setGoalMoveDescr( "HuntEnemy (FollowRoute)" );
		pb->followActualRoute();
	}
	else {
		short start = map.getCellId( pb->ent );
		short enemyId = map.getCellId( item->entity );
		if ( start>=0 && enemyId>=0 ) {
			int pl;
			if ( (pl = map.getPathToAttack( start, enemyId, pb->roamingRoute )) > 0 ) {
				pb->setRoamingIndex( pl );
				pb->huntingFor = item->entity;
				Vector startV = map.cell( start ).pos();
				Vector endV;
				debugMsg( "HuntLength = %i", pl );				
			}
		}
		pb->setGoalMoveDescr( "HuntEnemy (FindRoute)" );
	}
	debugMsg( "Hunting player\n" );

	//botNr = pb->slot;	// film this!
}


float weightHuntEnemy( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	// only hunt players that haven't been seen yet and are trackable
	if (item->hasBeenSpotted() || !(item->isTrackable())) return 0;

	return pb->needs.wishForCombat();
}



void goalFleeEnemy( CParabot *pb, PB_Percept*item )
// flee taking cover
{
	int pl;

	pb->reportEnemySpotted();
	
	if ( pb->roamingIndex>=pb->roamingBreak && pb->fleeingFrom==item->entity ) {
		// already fleeing
		pb->setGoalMoveDescr( "FleeEnemy (FollowRoute)" );
		pb->followActualRoute();
	}
	else {
		short start = map.getCellId( pb->ent );
		short enemyId = map.getCellId( item->entity );
		if ( start>=0 && enemyId>=0 ) {
			if (map.lineOfSight( start, enemyId )) {
				// get fastest path out of sight
				if ( (pl = map.getPathToCover( start, enemyId, pb->roamingRoute )) > 0 ) {
					pb->setRoamingIndex( pl );
					if (pl>4) pb->roamingBreak = pl-4;
					else pb->roamingBreak = 0;
					pb->fleeingFrom = item->entity;
				}
			}
			else {
				// stay covered and get away
				if ( (pl = map.getPathForSneakyEscape( start, enemyId, pb->roamingRoute )) > 0 ) {
					pb->setRoamingIndex( pl );
					if (pl>4) pb->roamingBreak = pl-4;
					else pb->roamingBreak = 0;
					pb->fleeingFrom = item->entity;
				}
			}
		}
		pb->setGoalMoveDescr( "FleeEnemy (FindRoute)" );
	}
}


float weightFleeEnemy( CParabot *pb, PB_Percept*item )
{
	if (item->distance > 150 && item->rating < 0) {
		return (item->rating * item->rating);
	}
	return 0;
}



void goalTakeCover( CParabot *pb, PB_Percept*item )
// move to next cover from enemy position
{
	pb->reportEnemySpotted();
	
	if ( pb->roamingIndex>=0 && pb->fleeingFrom==item->entity ) {
		// already heading towards cover
		pb->setGoalMoveDescr( "TakeCover (FollowRoute)" );
		pb->followActualRoute();
	}
	else {
		short start = map.getCellId( pb->ent );
		short enemyId = map.getCellId( item->entity );
		if ( start>=0 && enemyId>=0 ) {
			if (map.lineOfSight( start, enemyId )) {
				pb->setGoalMoveDescr( "TakeCover (FindRoute)" );
				// get fastest path out of sight
				int pl;
				if ( (pl = map.getPathToCover( start, enemyId, pb->roamingRoute )) > 0 ) {
					pb->setRoamingIndex( pl );
					pb->fleeingFrom = item->entity;
				}
			}
			else {
				pb->setGoalMoveDescr( "TakeCover (Rest)" );
			}
		}
	}
}


float weightTakeCover( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );

	if ( (worldTime() - item->lastSeenTime) > 1) return 0;	// not seen for too long
	if ( !item->hasBeenSpotted() ) return 0;				// we haven't seen anything yet
	if (item->distance < 150) return 0;						// too close
	if (item->rating > 0) return 0;							// we have advantage

	float weight = pb->needs.wishForCombat() - item->rating;
	
	// are we already in a combat? -> keep on to it!
	if ( item->isHurtingBot() || item->isAlert() ) weight += 3;
	
	if ( item->hasHighPriority() ) {
		weight -= 2;
	}
	if ( item->hasFocus() ) weight += 1;	// stick to current enemy

	return weight;
}



void goalCloseCombat( CParabot *pb, PB_Percept*item )
// combat movements for short range
{
	assert( item != 0 );
	pb->reportEnemySpotted();
/*
	float dist = (pb->botPos() - item->entity->v.origin).Length();
	if ( (dist < 800) && item->isVisible() ) {
		pb->combat.closeCombatMovement( *item );
		pb->setGoalMoveDescr( "CloseCombat" );
	}
	else {
		if (!item->isUnderPreemptiveFire()) {
			pb->pathfinder.checkWay( item->lastSeenPos );
			pb->setGoalMoveDescr( "CloseCombat (Follow)" );
		}
		else {
			pb->setGoalMoveDescr( "CloseCombat (Preemptive)" );
		}
	}
	*/
	if (item->distance < 100 && item->isVisible()) {
		pb->setGoalMoveDescr( "CloseCombat (vaBanque)" );
		pb->action.setMoveDir( item->lastSeenPos );
		pb->action.setMaxSpeed();
	}
	else if ( pb->roamingIndex>=pb->roamingBreak && pb->huntingFor==item->entity ) {
		// already hunting
		pb->setGoalMoveDescr( "CloseCombat (FollowRoute)" );
		pb->followActualRoute();
	}
	else {
		short start = map.getCellId( pb->ent );
		short enemyId = map.getCellId( item->entity );
		if ( start>=0 && enemyId>=0 ) {
			
			int clientIndex = ENTINDEX( pb->ent ) - 1;
			assert( (clientIndex >= 0) && (clientIndex < 32) );
			int wid = clientWeapon[clientIndex];
			PB_Weapon w( wid );
			float minDist = w.bestDistance();
			int pl;
			if ( (pl = map.getOffensivePath( start, enemyId, minDist, pb->roamingRoute )) > 0 ) {
				pb->setRoamingIndex( pl );
				if (pl>4) pb->roamingBreak = pl-4;
				else pb->roamingBreak = 0;
				pb->huntingFor = item->entity;
			}
		}
		pb->setGoalMoveDescr( "CloseCombat (FindRoute)" );
	}
	goalShootAtEnemy( pb, item );
}


float weightCloseCombat( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );

	if ( (worldTime() - item->lastSeenTime) > 5) return 0;	// not seen for too long
	//if ( !pb->combat.weapon.bestWeaponUsable() ) return 0;
	if ( !item->hasBeenSpotted() ) return 0;				// we haven't seen anything yet
	if (item->distance < 100) return 15 + (item->rating/5);	// va banque
	if (item->rating < -3) return 0;						// too bad, better don't try that...

	float weight = pb->needs.wishForCombat() + item->rating;
	
	// are we already in a combat? -> keep on to it!
	if ( item->isHurtingBot() || item->isAlert() ) weight += 3;
	// check if enemy is vulnerable near charger
	if ( !item->isMoving() && (item->entity->v.button & IN_USE) ) {
		debugMsg( "ENEMY CHARGING...\n" );
		weight += 2;
	}		
	if ( item->hasHighPriority() ) {
		weight += 5;
		//debugMsg( "Hunting the Saint!\n" );
	}
	if ( item->hasFocus() ) weight += 1;	// stick to current enemy

	return weight;
}



void goalRangeAttack( CParabot *pb, PB_Percept*item )
// combat movements for medium to long range
{
}


float weightRangeAttack( CParabot *pb, PB_Percept*item )
{
	if (item->distance < 800) return 0;
	return 0;
}



void goalSilentAttack( CParabot *pb, PB_Percept*item )
// prepared fatal attack
{
//	botNr = pb->slot;
//	if ( !camPlayer ) startBotCam( INDEXENT( 1 ) );
	pb->reportEnemySpotted();

	if (item->distance > 500) {		// too far to approach, just fire from current position
		pb->action.add( BOT_DUCK );
		if ( item->canBeAttackedBest() ) {
			pb->combat.shootAtEnemy( item->lastSeenPos, 0.8 );
			pb->setGoalMoveDescr( "SilentAttack (ShootDistance)" );
		}
		else {
			pb->setGoalMoveDescr( "SilentAttack (SwitchingWpn)" );
		}
	}
	else {
		switch (mod_id) {
			case VALVE_DLL:		pb->combat.weapon.setPreferredWeapon( VALVE_WEAPON_SHOTGUN, 2 );	
								break;
			case HOLYWARS_DLL:	pb->combat.weapon.setPreferredWeapon( HW_WEAPON_DOUBLESHOTGUN, 1 );	
								break;
			case DMC_DLL:		pb->combat.weapon.setPreferredWeapon( DMC_WEAPON_SUPERSHOTGUN, 1 );	
								break;
			case GEARBOX_DLL:	pb->combat.weapon.setPreferredWeapon( VALVE_WEAPON_SHOTGUN, 2 );	
								break;
		}
		if (item->distance > 50) {
			pb->pathfinder.checkWay( item->lastSeenPos );	// get near...
			pb->setGoalMoveDescr( "SilentAttack (Approaching)" );
		}
		else {
			pb->action.add( BOT_DUCK );
			if ( item->canBeAttackedBest() ) {	// ...and attack from there
				bool shot = pb->combat.shootAtEnemy( item->lastSeenPos, pb->combat.weapon.currentHighAimProb() );
				pb->setGoalMoveDescr( "SilentAttack (ShootClose)" );
			}
			else {
				pb->setGoalMoveDescr( "SilentAttack (SwitchingWpn)" );
			}
		}
	}
	item->flags |= PI_FOCUS1;
}


float weightSilentAttack( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	float weight = 0;

	if ( pb->senses.underFire() || !pb->combat.weapon.bestWeaponUsable() ) return 0;
	if ( item->isVisible() && !item->isFacingBot() && !item->isMoving() ) {
		weight = 15;
		if ( item->hasHighPriority() ) weight += 5;
	}
	return weight;
}



void goalPrepareAmbush( CParabot *pb, PB_Percept*item )
// move to ambush position and wait
{
}


float weightPrepareAmbush( CParabot *pb, PB_Percept*item )
{
	return 0;
}



void goalShootAtEnemy( CParabot *pb, PB_Percept*item )
// shooting
{
	bool fired = false;

	assert( item != 0 );
	//debugFile( "ShootAtEnemy" );
	if ( item->isVisible() ) {
		if ( item->isAimingAtBot() || item->isAlert() )	{
		// enemy is aiming or alert: shoot with low accuracy
			fired = pb->combat.shootAtEnemy( item->entity, (pb->combat.weapon.currentHighAimProb()-0.2) / 2.0 );
			pb->setGoalViewDescr( "ShootAtEnemy (LowAcc)" );
		}
		else {
		// else take time to aim well
			if ( item->canBeAttackedBest() ) {
				fired = pb->combat.shootAtEnemy( item->entity, pb->combat.weapon.currentHighAimProb() );
				pb->setGoalViewDescr( "ShootAtEnemy (HighAcc)" );
			}
			else {
				pb->setGoalViewDescr( "ShootAtEnemy (SwitchingWpn)" );
			}
			if ( !pb->senses.underFire() ) {
				pb->action.setSpeed( 0 );
			}
		}
		if (fired) item->flags |= PI_ALERT;
	}
	else {			// enemy is not visible:
		if ( item->hasJustDisappeared() ) {
			// start debug
/*			short path[128];
			short start = map.getCellId( pb->ent );
			short enemyId = map.getCellId( item->entity );
			if ( start>=0 && enemyId>=0 ) {
				int pl;
				if ( pl = map.predictPlayerPos( enemyId, start, path ) ) {
					Vector startV = map.cell( enemyId ).pos();
					Vector endV;
					if (pl != -1) {
						for (int l=(pl-1); l>=0; l--) {
							endV = map.cell( path[l] ).pos();
							debugBeam( startV, endV, 50 );
							startV = endV;
						}
					}
				}
			}*/
			// end debug
			UTIL_MakeVectors( pb->ent->v.v_angle );
			Vector botpos = pb->ent->v.origin + pb->ent->v.view_ofs;
			Vector pos = item->lastSeenPos + item->entity->v.view_ofs;
			Vector dir = (pos - botpos).Normalize();
			float dot = DotProduct( gpGlobals->v_forward, dir );
			if ( (dot > 0.7) && (item->distance > 250)) {
				// check if bot should throw grenade at last position:
				int flags = WF_NEED_GRENADE;
				if (pb->ent->v.waterlevel == 3) flags |= WF_UNDERWATER;
				int bestWeapon = pb->combat.weapon.getBestWeapon( item->distance, pb->action.targetAccuracy(), flags );
				float bestScore = pb->combat.weapon.getWeaponScore( bestWeapon, item->distance, pb->action.targetAccuracy(), flags, true );
				Vector prePos = item->lastSeenPos - (0.05*item->lastSeenVelocity);
				if ( (bestScore > 0) && canShootAt( pb->ent, prePos ) ) {
					//botNr = pb->slot;
					//if ( !camPlayer ) startBotCam( INDEXENT( 1 ) );

					item->flags &= ~PI_BEST_ARMED;
					item->flags |= PI_PREEMPTIVE;
					pb->preemptiveWeapon = bestWeapon;
					if ((mod_id==VALVE_DLL || mod_id==GEARBOX_DLL) && bestWeapon==VALVE_WEAPON_RPG) 
						pb->preemptiveMode = 2;		// switch off RPG laserdot
					else 
						pb->preemptiveMode = 1;
					debugMsg( "Enemy disappereared, using preemptive fire\n" );
				}
				else {
					//pb->preemptiveFire = false;
					debugMsg( "No grenade or bot has moved\n" );
				}
			}
			else debugMsg( "Bot turned round or too near!\n" );
			pb->setGoalViewDescr( "ShootAtEnemy (JustDisappeared)" );
		}
		if (item->isUnderPreemptiveFire()) {
			if ( !pb->senses.underFire() ) {
				//pb->action.setSpeed( 0 );
			}
			pb->combat.weapon.setPreferredWeapon( pb->preemptiveWeapon, pb->preemptiveMode );
			Vector prePos = item->lastSeenPos - (0.05*item->lastSeenVelocity);
			if ( item->canBeAttackedBest() ) {
				fired = pb->combat.shootAtEnemy( prePos, 0.5 );
				if (fired) pb->preemptiveFire = false;	// shoot only once
				pb->setGoalViewDescr( "ShootAtEnemy (Preemptive:Shoot)" );
			}
			else {
				debugMsg( "Waiting for bestWeapon..." );
				pb->action.setAimDir( prePos );
				pb->setGoalViewDescr( "ShootAtEnemy (Preemptive:SwitchingWpn)" );
			}
		}
		else {
			if (item->isTrackable()) {
				pb->action.setAimDir( item->predictedAppearance( pb->botPos() ) );
				pb->setGoalViewDescr( "ShootAtEnemy (Disappeared:PredictiveAiming)" );
			}
			else {
				goalLookAround( pb, item );
				pb->setGoalViewDescr( "ShootAtEnemy (Disappeared:LookAround)" );
			}
		}
	}
	//debugFile( " ok\n" );
	item->flags |= PI_FOCUS1;	// focus on this one (arm weapons...)
}


float weightShootAtEnemy( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	float weight;

	if ( !pb->combat.weapon.bestWeaponUsable() ) return 0;

	if ( !item->isTrackable() ) {	// try to look at it:
		if ( item->isHurtingBot() ) weight = 5;		// hidden attacker -> dangerous!
		else weight = weightReactToUnidentified( pb, item );
	}
	else {	// we see or saw this one:
		if ( item->isHurtingBot() || item->isAimingAtBot() ) weight = 5;	// shoot if he attacks us
		else if ( item->rating < -3 && !item->isFacingBot() ) weight = 0;	// too bad, don't evoke attention
		else weight = 2;							// default

		if ( !item->isVisible() && !item->isUnderPreemptiveFire()) weight /= 2;	// still look in direction if no other enemy around
		if ( item->hasHighPriority() ) weight += 5;
	}
	if ( weight>0 && item->hasFocus() ) weight += 1;	// stick to current enemy
	return weight;
}


float weightShootAtSnark( CParabot *pb, PB_Percept*item )
{
	float weight;
	if ( item->isVisible() ) weight = (500/item->distance);
	if (weight>3) weight = 3;
	return weight;
}



void goalBunnyHop( CParabot *pb, PB_Percept*item )
{
	static float nextHop[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	int i = pb->slot;
	if (worldTime() >= nextHop[i]) {
		pb->action.add( BOT_JUMP );
		nextHop[i] = worldTime() + RANDOM_FLOAT( 0.5, 1.5 );
	}
	else if (worldTime()+5.0 < nextHop[i]) {
		// level change
		nextHop[i] = worldTime();
	}
	pb->setGoalActDescr( "BunnyHop" );
}


float weightBunnyHop( CParabot *pb, PB_Percept*item )
{
	if (item->inflictorKnown()) return 1;
	return 5;
}



void goalArmBestWeapon( CParabot *pb, PB_Percept*item )
// chose best weapon
{
	float dist; 
	float hitProb;
	int botFlags = 0;

	if (item && item->isTrackable()) {
		dist = item->distance;
		hitProb = pb->action.targetAccuracy();
		if ( item->isFacingBot() ) {
			if ( hitProb>0.2 && item->targetAccuracy()>0.2 ) 
				botFlags |= WF_IMMEDIATE_ATTACK;	// both can shoot
			else
				botFlags |= WF_FAST_ATTACK;			// hurry up
		}
		else {
			botFlags |= WF_SINGLE_SHOT_KILL;	// so that enemy gets no chance to see us
		}
		if ( item->predictedAppearance(pb->botPos()).z > (pb->botPos().z+20) ) botFlags |= WF_ENEMY_ABOVE;
		else if ( item->predictedAppearance(pb->botPos()).z < (pb->botPos().z-80) ) botFlags |= WF_ENEMY_ABOVE;
		pb->combat.nextWeaponCheck = worldTime() + CHECK_WEAPON_COMBAT;
	}
	else {
		dist = 250;		// expect medium distance
		hitProb = 0.2;	// and low accuracy
		pb->combat.nextWeaponCheck = worldTime() + CHECK_WEAPON_IDLE;
	}

	if ( pb->ent->v.waterlevel == 3 ) botFlags |= WF_UNDERWATER;
		
	// if best weapon already armed, store in item:
	checkForBreakpoint( BREAK_WEAPON );
	bool bestArmed = pb->combat.weapon.armBestWeapon( dist, hitProb, botFlags );
	if (!bestArmed) pb->combat.nextWeaponCheck = worldTime() + CHANGE_WEAPON_DELAY;
	if (bestArmed && item) item->flags |= PI_BEST_ARMED;
	pb->setGoalActDescr( "ArmBestWeapon" );
}


float weightArmBestWeapon( CParabot *pb, PB_Percept*item )
{
	float weight = 0;
		
	if ( item && item->isTrackable() && 
		 (worldTime()-item->firstDetection > 0.5) ) {	// aim a while before decision
		if ( ( (worldTime()+CHECK_WEAPON_COMBAT) < pb->combat.nextWeaponCheck ) ||
		     ( worldTime() > pb->combat.nextWeaponCheck ) ) weight = 5;
		if ( item->hasFocus() ) weight = 10;
	}
	else {
		if ( worldTime() > pb->combat.nextWeaponCheck ) weight = 5;
	}
	return weight;
}
