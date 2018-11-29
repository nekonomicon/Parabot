#include "parabot.h"
#include "pb_combat.h"


extern int mod_id;

// stores the current weapon for all human clients
int clientWeapon[32];



void PB_Combat::init( int slot, EDICT *ent, ACTION *act, PB_Roaming *pFinder )
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

#if 0
void PB_Combat::classifyDistance( float dist )
{

}
#endif

void PB_Combat::evade(PB_Percept &perceipt, Vec3D *right)
{
	Vec3D dir;
	float speed;

	if (worldtime() > nextJump) {
		//action->add( BOT_JUMP );
		nextJump = worldtime() + randomfloat(1.0f, 3.0f);
	}

	if (worldtime() > nextStrafe) {
		strafeState++;
		if (strafeState == 4)
			strafeState = 0;
		nextStrafe = worldtime() + randomfloat(0.1f, 1.0f);
	}

	vsub(&perceipt.lastPos, &botEnt->v.origin, &dir);
	getright(&dir, right);

	if (perceipt.distance > 300)
		speed = 300;
	else
		speed = perceipt.distance;

	if (strafeState == 0) {
		vscale(right, speed, right);
	} else if (strafeState == 2) {
		vscale(right, speed, right);
	} else {
		vcopy(&zerovector, right);
	}
}


float PB_Combat::getRating( PB_Percept &perceipt )
// returns a value between -5 (dangerous, ignore) and +5 (easy victim, attack)
{
	EDICT *enemy = perceipt.entity;
	assert(enemy != 0);
	float adv = 0;
	float enemyDist = perceipt.distance;
	int botWeapon = weapon.currentWeapon();

	if (is_invulnerable(perceipt.entity)) return -5;	// don't mess with this one!

	if (perceipt.isFacingBot()) {
		int clientIndex = indexofedict(enemy) - 1;
		assert((clientIndex >= 0) && (clientIndex < 32));
		int enemyWeapon = clientWeapon[clientIndex];

		if ( enemyWeapon != botWeapon ) {	// different weapons: calculate...
			// bot variables
			int botFlags = 0;
			if ( botEnt->v.waterlevel == 3 ) botFlags |= WF_UNDERWATER;
			float botHitProb = action_targetaccuracy(action);
			// enemy variables
			int enemyFlags = 0;
			if ( enemy->v.waterlevel == 3 ) enemyFlags |= WF_UNDERWATER;
			float enemyHitProb = perceipt.targetAccuracy();
			adv = weapon.getWeaponScore( botWeapon, enemyDist, botHitProb, botFlags, true )
			    - weapon.getWeaponScore( enemyWeapon, enemyDist, enemyHitProb, enemyFlags, false );
		}
	} else {
		// bot variables
		int botFlags = 0;
		if ( botEnt->v.waterlevel == 3 ) botFlags |= WF_UNDERWATER;
		float botHitProb = action_targetaccuracy(action);
		adv = weapon.getWeaponScore( botWeapon, enemyDist, botHitProb, botFlags, true );
	}
	adv = 2 * adv;	// stronger influence
	if (adv > 5) adv = 5;
	else if (adv < -5) adv = -5;

	return adv;
}


bool PB_Combat::shootAtEnemy( Vec3D *enemyOrigin, float accuracy )
// picks best place to shoot at and fires
{
	Vec3D firePos, feetPos = {0.0f, 0.0f, -31.0f}, headpos = {0.0f, 0.0f, 28.0f};

	vcopy(enemyOrigin, &firePos);

	if ((action_getaimskill(action) > 6) && (accuracy >= 0.5f)) {
		vadd(&firePos, &headpos, &firePos);	// aim at head
	}
	// TODO: Need ROCKETLAUNCHER flag
	switch(weapon.currentWeapon()) {
		default:
			break;
		case VALVE_WEAPON_RPG:
		case HW_WEAPON_ROCKETLAUNCHER:
		case DMC_WEAPON_ROCKETLAUNCHER:
			// aim at feet if possible
			vadd(&firePos, &feetPos, &feetPos);
			if (canshootat(botEnt, &feetPos)) 
				vcopy(&feetPos, &firePos);
			break;
	}
	return weapon.attack(&firePos, accuracy, NULL);
}


bool PB_Combat::shootAtEnemy( EDICT *enemy, float accuracy )
// picks best place to shoot at and fires
{
	Vec3D firePos, feetPos, testPos, predictedMove, dir, vel;
	float dist;

	assert(enemy != 0);

	vcopy(&enemy->v.origin, &firePos);
	if ((action_getaimskill(action) > 6) && (accuracy >= 0.5f)) {
		vadd(&firePos, &enemy->v.view_ofs, &firePos); // aim at head
	}

	// TODO: Need ROCKETLAUNCHER and NAILGUN flags
	switch(mod_id) {
	default:
		if (weapon.currentWeapon() == VALVE_WEAPON_RPG) {		
			// aim at feet if possible
			vcopy(&firePos, &feetPos);
			feetPos.z = enemy->v.absmin.z + 1.0f;
			if (canshootat(botEnt, &feetPos)) 
				vcopy(&feetPos, &firePos);
		}
		break;
	case HOLYWARS_DLL:
		if (weapon.currentWeapon() == HW_WEAPON_ROCKETLAUNCHER) {	
			// aim at feet and predict movement
			vcopy(&firePos, &feetPos);
			feetPos.z = enemy->v.absmin.z + 1.0f;
			vsub(&botEnt->v.origin, &enemy->v.origin, &dir);
			dist = vlen(&dir);
			vscale(&enemy->v.velocity, dist / 1600.0f, &predictedMove);
			vadd(&feetPos, &predictedMove, &testPos);
			if (canshootat(botEnt, &testPos))
				vcopy(&testPos, &firePos);
		}
		break;
	case DMC_DLL:
		if (weapon.currentWeapon() == DMC_WEAPON_ROCKETLAUNCHER) {	
			// aim at feet and predict movement
			vcopy(&firePos, &feetPos);
			feetPos.z = enemy->v.absmin.z + 1.0f;
			vsub(&botEnt->v.origin, &enemy->v.origin, &dir);
			dist = vlen(&dir);
			vscale(&enemy->v.velocity, dist / 1200.0f, &predictedMove);
			vadd(&feetPos, &predictedMove, &testPos);
			if (canshootat(botEnt, &testPos))
				vcopy(&testPos, &firePos);
		} else if (weapon.currentWeapon() == DMC_WEAPON_NAILGUN ||
			    weapon.currentWeapon() == DMC_WEAPON_SUPERNAILGUN) {
			// predict movement
			vsub(&botEnt->v.origin, &enemy->v.origin, &dir);
			dist = vlen(&dir);
			vscale(&enemy->v.velocity, dist / 1200.0f, &predictedMove);
			vadd(&firePos, &predictedMove, &testPos);
			if (canshootat(botEnt, &testPos))
				vcopy(&testPos, &firePos);
		}
		break;
	}

	vsub(&enemy->v.velocity, &botEnt->v.velocity, &vel);

	return weapon.attack(&firePos, accuracy, &vel);
}


void PB_Combat::closeCombatMovement( PB_Percept &perceipt )
// decides which reaction is most appropiate and calls either engage() or retreat()
{
	EDICT *enemy = perceipt.entity;
	assert( enemy != 0 );
	if (enemy==0) debugFile( " ENEMY=0 " );
	if (botEnt==0) debugFile( " BOT=0 " );
	
	// init distance to enemy
	float enemyDist = perceipt.distance;
		
	// init bot variables
	int botWeapon = weapon.currentWeapon();
	int botFlags = WF_FAST_ATTACK;
	if ( botEnt->v.waterlevel == 3 ) {
		botFlags |= WF_UNDERWATER;
		DEBUG_MSG( "underwater\n" );
	}
	float botHitProb = action_targetaccuracy(action);
	
	// init enemy variables
	int clientIndex = indexofedict( enemy ) - 1;
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
		} else {	// same weapons: go to optimum
			if  (weapon.bestDistance( botWeapon ) > (enemyDist+25)) 
				gainDistance = true;	// only for non-sniper-weapons
			else if (weapon.bestDistance( botWeapon ) < enemyDist) 
				closeUp = true;
		}
	}
	Vec3D evadeMove = {0, 0, 0}, tDir;

	// if enemy tries to shoot at bot
	int closeDistanceWeapon = 0;
	switch (mod_id) {
		case AG_DLL:
		case HUNGER_DLL:
		case GEARBOX_DLL:
		case VALVE_DLL:		closeDistanceWeapon = VALVE_WEAPON_CROWBAR;
							break;
		case HOLYWARS_DLL:	closeDistanceWeapon = HW_WEAPON_JACKHAMMER;
							break;
		case DMC_DLL:		closeDistanceWeapon = DMC_WEAPON_CROWBAR;
							break;
	}
	if (perceipt.isVisible() && perceipt.isAimingAtBot()
	    && (botWeapon != closeDistanceWeapon))	// don't evade with this
		evade(perceipt, &evadeMove);
	if (gainDistance) {	// bigger distance better
		vsub(&enemy->v.origin, &botEnt->v.origin, &tDir);
		vsub(&botEnt->v.origin, &tDir, &tDir);
		vadd(&evadeMove, &tDir, &tDir);
		pathfinder->checkWay(&tDir);
	} else if (closeUp) {	// closer distance better
		vadd(&enemy->v.origin, &evadeMove, &tDir);
		pathfinder->checkWay(&tDir);
	} else if (vlen(&evadeMove) > 0) {	// just evade
		vadd(&botEnt->v.origin, &evadeMove, &tDir);
		pathfinder->checkWay(&tDir);
	} else {	// no move -> duck
		action_add(action, BOT_DUCK, NULL);
	}
}


void PB_Combat::retreat(EDICT *enemy )
// flees from the enemy
{
	Vec3D tDir;

	assert( enemy != 0 );

	vsub(&enemy->v.origin, &botEnt->v.origin, &tDir);
	vsub(&botEnt->v.origin, &tDir, &tDir);
	pathfinder->checkWay(&tDir);
}


void PB_Combat::idleActions()
// manages weapon actions when no enemy is around
{
/*	if ( worldtime() > nextWeaponCheck ) {
		float dist = 200;	// short distance
		float hitProb = 200/dist;
		if (hitProb>1) hitProb = 1;
		int flags = 0;
		nextWeaponCheck = worldtime() + CHECK_WEAPON_IDLE;
		weapon.armBestWeapon( dist, hitProb, flags );
	}*/
}
