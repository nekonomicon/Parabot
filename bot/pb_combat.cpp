#include "pb_combat.h"


extern int mod_id;

// stores the current weapon for all human clients
int clientWeapon[32];



void PB_Combat::init( int slot, edict_t *ent, PB_Action *act, PB_Roaming *pFinder )
// initializes all necessary variables
{
	botEnt = ent;
	action = act; 
	pathfinder = pFinder;
	weapon.init( slot, ent, act );
	enemyContact = 0;
	nextWeaponCheck = 0;
	strafeState = 0;
	nextStrafe = 0;
	nextJump = 0;
}


void PB_Combat::classifyDistance( float dist )
{
	
}


Vector UTIL_GetRight( const Vector &vec );

Vector PB_Combat::evade( PB_Percept &perceipt )
{
	if (worldTime()>nextJump) {
		//action->add( BOT_JUMP );
		nextJump = worldTime() + RANDOM_FLOAT( 1, 3 );
	}

	if (worldTime()>nextStrafe) {
		strafeState++;
		if (strafeState==4) strafeState=0;
		nextStrafe = worldTime() + RANDOM_FLOAT( 0.1, 1.0 );
	}

	Vector right = UTIL_GetRight( perceipt.lastPos - botEnt->v.origin );
	float speed;
	if (perceipt.distance > 300) speed = 300;
	else speed = perceipt.distance;

	if (strafeState==0) return(  speed*right );
	if (strafeState==2) return( -speed*right );
	return Vector( 0,0,0 );
}


float PB_Combat::getRating( PB_Percept &perceipt )
// returns a value between -5 (dangerous, ignore) and +5 (easy victim, attack)
{
	edict_t *enemy = perceipt.entity;
	assert( enemy != 0 );
	float adv = 0;
	float enemyDist = perceipt.distance;
	int botWeapon = weapon.currentWeapon();

	if (isInvulnerable( perceipt.entity )) return -5;	// don't mess with this one!

	if ( perceipt.isFacingBot() ) {
		int clientIndex = ENTINDEX( enemy ) - 1;
		assert( (clientIndex >= 0) && (clientIndex < 32) );
		int enemyWeapon = clientWeapon[clientIndex];
		
		if ( enemyWeapon != botWeapon ) {	// different weapons: calculate...
			// bot variables
			int botFlags = 0;
			if ( botEnt->v.waterlevel == 3 ) botFlags |= WF_UNDERWATER;
			float botHitProb = action->targetAccuracy();
			// enemy variables
			int enemyFlags = 0;
			if ( enemy->v.waterlevel == 3 ) enemyFlags |= WF_UNDERWATER;
			float enemyHitProb = perceipt.targetAccuracy();
			adv = weapon.getWeaponScore( botWeapon, enemyDist, botHitProb, botFlags, true )
				- weapon.getWeaponScore( enemyWeapon, enemyDist, enemyHitProb, enemyFlags, false );
		}
	}
	else {
		// bot variables
		int botFlags = 0;
		if ( botEnt->v.waterlevel == 3 ) botFlags |= WF_UNDERWATER;
		float botHitProb = action->targetAccuracy();
		adv = weapon.getWeaponScore( botWeapon, enemyDist, botHitProb, botFlags, true );
	}
	adv = 2*adv;	// stronger influence
	if (adv>5) adv = 5;
	else if (adv<-5) adv = -5;

	return adv;
}


bool PB_Combat::shootAtEnemy( Vector enemyOrigin, float accuracy )
// picks best place to shoot at and fires
{
	Vector firePos = enemyOrigin;

    if ( (action->getAimSkill() > 6) && (accuracy >= 0.5) ) {
		firePos = firePos + Vector( 0,0,28);	// aim at head
	}
	switch( mod_id ) {
	case VALVE_DLL:
	case GEARBOX_DLL:
		if (weapon.currentWeapon()==VALVE_WEAPON_RPG) {		
			// aim at feet if possible
			Vector feetPos = firePos + Vector( 0,0,-31 );
			if ( canShootAt( botEnt, feetPos) ) 
				firePos = feetPos;
		}
		break;
	case HOLYWARS_DLL:
		if (weapon.currentWeapon()==HW_WEAPON_ROCKETLAUNCHER) {	
			// aim at feet if possible
			Vector feetPos = firePos + Vector( 0,0,-31 );
			if ( canShootAt( botEnt, feetPos) )
				firePos = feetPos;
		}
		break;
	case DMC_DLL:
		if (weapon.currentWeapon()==DMC_WEAPON_ROCKETLAUNCHER) {	
			// aim at feet if possible
			Vector feetPos = firePos + Vector( 0,0,-31 );
			if ( canShootAt( botEnt, feetPos) )
				firePos = feetPos;
		}
		break;
	}
	return weapon.attack( firePos, accuracy );
}


bool PB_Combat::shootAtEnemy( edict_t *enemy, float accuracy )
// picks best place to shoot at and fires
{
	assert( enemy!=0 );
	Vector firePos = enemy->v.origin;

    if ( (action->getAimSkill() > 6) && (accuracy >= 0.5) ) {
		firePos = firePos + enemy->v.view_ofs;	// aim at head
	}
	
	switch( mod_id ) {
	case VALVE_DLL:
	case GEARBOX_DLL:
		if (weapon.currentWeapon()==VALVE_WEAPON_RPG) {		
			// aim at feet if possible
			Vector feetPos = firePos;	feetPos.z = enemy->v.absmin.z + 1;
			if ( canShootAt( botEnt, feetPos) ) 
				firePos = feetPos;
		}
		break;
	case HOLYWARS_DLL:
		if (weapon.currentWeapon()==HW_WEAPON_ROCKETLAUNCHER) {	
			// aim at feet and predict movement
			Vector feetPos = firePos;	feetPos.z = enemy->v.absmin.z + 1;
			float dist = ((Vector)(botEnt->v.origin-enemy->v.origin)).Length();
			Vector predictedMove = enemy->v.velocity * dist / 1600;
			if ( canShootAt( botEnt, feetPos + predictedMove) )
				firePos = feetPos + predictedMove;
		}
		break;
	case DMC_DLL:
		if (weapon.currentWeapon()==DMC_WEAPON_ROCKETLAUNCHER) {	
			// aim at feet and predict movement
			Vector feetPos = firePos;	feetPos.z = enemy->v.absmin.z + 1;
			float dist = ((Vector)(botEnt->v.origin-enemy->v.origin)).Length();
			Vector predictedMove = enemy->v.velocity * dist / 1200;
			if ( canShootAt( botEnt, feetPos + predictedMove) )
				firePos = feetPos + predictedMove;
		}
		else if (weapon.currentWeapon()==DMC_WEAPON_NAILGUN ||
				 weapon.currentWeapon()==DMC_WEAPON_SUPERNAILGUN) {	
			// predict movement
			float dist = ((Vector)(botEnt->v.origin-enemy->v.origin)).Length();
			Vector predictedMove = enemy->v.velocity * dist / 1200;
			if ( canShootAt( botEnt, firePos + predictedMove) )
				firePos = firePos + predictedMove;
		}
		break;
	}
	return weapon.attack( firePos, accuracy, enemy->v.velocity - botEnt->v.velocity );
}


void PB_Combat::closeCombatMovement( PB_Percept &perceipt )
// decides which reaction is most appropiate and calls either engage() or retreat()
{
	edict_t *enemy = perceipt.entity;
	assert( enemy != 0 );
	if (enemy==0) debugFile( " ENEMY=0 " );
	if (botEnt==0) debugFile( " BOTENT=0 " );
	
	// init distance to enemy
	float enemyDist = perceipt.distance;
		
	// init bot variables
	int botWeapon = weapon.currentWeapon();
	int botFlags = WF_FAST_ATTACK;
	if ( botEnt->v.waterlevel == 3 ) {
		botFlags |= WF_UNDERWATER;
		debugMsg( "underwater\n" );
	}
	float botHitProb = action->targetAccuracy();
	
	// init enemy variables
	int clientIndex = ENTINDEX( enemy ) - 1;
	assert( clientIndex >= 0 );
	assert( clientIndex < 32 );
	int enemyWeapon = clientWeapon[clientIndex];
	int enemyFlags = WF_FAST_ATTACK;
	if ( enemy->v.waterlevel == 3 ) enemyFlags |= WF_UNDERWATER;
	float enemyHitProb = perceipt.targetAccuracy();

	// chose move direction
	closeUp=false; gainDistance=false;
	if ( enemyDist < 1000 ) {	// don't give up sniper position
		if ( enemyWeapon != botWeapon ) {	// different weapons: calculate...
			float currentAdv = weapon.getWeaponScore( botWeapon, enemyDist, botHitProb, botFlags, true )
				- weapon.getWeaponScore( enemyWeapon, enemyDist, enemyHitProb, enemyFlags, false );
			float nearAdv = weapon.getWeaponScore( botWeapon, enemyDist-25, botHitProb, botFlags, true )
				- weapon.getWeaponScore( enemyWeapon, enemyDist-25, enemyHitProb, enemyFlags, false );
			float farAdv = weapon.getWeaponScore( botWeapon, enemyDist+25, botHitProb, botFlags, true )
				- weapon.getWeaponScore( enemyWeapon, enemyDist+25, enemyHitProb, enemyFlags, false );
			
			if ( (farAdv>currentAdv) && (farAdv>nearAdv) ) gainDistance = true;
			if ( (nearAdv>currentAdv) && (nearAdv>farAdv) ) closeUp = true;
		}
		else {	// same weapons: go to optimum
			if  (weapon.bestDistance( botWeapon ) > (enemyDist+25)) 
				gainDistance = true;	// only for non-sniper-weapons
			else if (weapon.bestDistance( botWeapon ) < enemyDist) 
				closeUp = true;
		}
	}
	Vector evadeMove (0,0,0);
	// if enemy tries to shoot at bot
	int closeDistanceWeapon = 0;
	switch (mod_id) {
		case VALVE_DLL:		closeDistanceWeapon = VALVE_WEAPON_CROWBAR;
							break;
		case HOLYWARS_DLL:	closeDistanceWeapon = HW_WEAPON_JACKHAMMER;
							break;
		case DMC_DLL:		closeDistanceWeapon = DMC_WEAPON_CROWBAR;
							break;
		case GEARBOX_DLL:	closeDistanceWeapon = VALVE_WEAPON_CROWBAR;
							break;
	}
	if ( perceipt.isVisible() && perceipt.isAimingAtBot() && 
		 (botWeapon!=closeDistanceWeapon) )	// don't evade with this
		evadeMove = evade( perceipt );
	if (gainDistance) {	// bigger distance better
		Vector tDir = botEnt->v.origin - (enemy->v.origin - botEnt->v.origin);
		pathfinder->checkWay( tDir + evadeMove);
	}
	else if (closeUp) {	// closer distance better
		pathfinder->checkWay( enemy->v.origin + evadeMove );
	}
	else if (evadeMove.Length() > 0) {	// just evade
		pathfinder->checkWay( botEnt->v.origin + evadeMove );
	}
	else {	// no move -> duck
		action->add( BOT_DUCK );
	}
}


void PB_Combat::retreat( edict_t *enemy )
// flees from the enemy
{
	assert( enemy != 0 );
	Vector tDir = botEnt->v.origin - (enemy->v.origin - botEnt->v.origin);
	pathfinder->checkWay( tDir );
}


void PB_Combat::idleActions()
// manages weapon actions when no enemy is around
{
/*	if ( worldTime() > nextWeaponCheck ) {
		float dist = 200;	// short distance
		float hitProb = 200/dist;
		if (hitProb>1) hitProb = 1;
		int flags = 0;
		nextWeaponCheck = worldTime() + CHECK_WEAPON_IDLE;
		weapon.armBestWeapon( dist, hitProb, flags );
	}*/
}