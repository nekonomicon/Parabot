#include "parabot.h"

extern int mod_id;

// stores the current weapon for all human clients
int clientWeapon[32];

void
combat_init(COMBAT *combat, int slot, EDICT *ent, ACTION *act, ROAMING *pFinder)
// initializes all necessary variables
{
	combat->botent = ent;
	combat->action = act; 
	combat->pathfinder = pFinder;
	weaponhandling_init(&combat->weapon, slot, ent, act );
	combat->enemycontact = 0;
	combat->nextweaponcheck = 0;
	combat->strafestate = 0;
	combat->nextstrafe = 0;
	combat->nextjump = 0;
}

static void
combat_classifydistance(float dist)
{

}

static void
combat_evade(COMBAT *combat, PERCEPT *perceipt, Vec3D *right)
{
	Vec3D dir;
	float speed;

	if (worldtime() > combat->nextjump) {
		//combat->action->add( BOT_JUMP );
		combat->nextjump = worldtime() + randomfloat(1.0f, 3.0f);
	}

	if (worldtime() > combat->nextstrafe) {
		combat->strafestate++;
		if (combat->strafestate == 4)
			combat->strafestate = 0;
		combat->nextstrafe = worldtime() + randomfloat(0.1f, 1.0f);
	}

	vsub(&perceipt->lastpos, &combat->botent->v.origin, &dir);
	getright(&dir, right);

	if (perceipt->distance > 300)
		speed = 300;
	else
		speed = perceipt->distance;

	if (combat->strafestate == 0) {
		vscale(right, speed, right);
	} else if (combat->strafestate == 2) {
		vscale(right, speed, right);
	} else {
		vcopy(&zerovector, right);
	}
}

float
combat_getrating(COMBAT *combat,  PERCEPT *perceipt)
// returns a value between -5 (dangerous, ignore) and +5 (easy victim, attack)
{
	EDICT *enemy = perceipt->entity;
	assert(enemy != 0);
	float adv = 0;
	float enemyDist = perceipt->distance;
	int botWeapon = weaponhandling_currentweapon(&combat->weapon);

	if (is_invulnerable(perceipt->entity)) return -5;	// don't mess with this one!

	if (percept_isfacingbot(perceipt)) {
		int clientIndex = indexofedict(enemy) - 1;
		assert((clientIndex >= 0) && (clientIndex < 32));
		int enemyWeapon = clientWeapon[clientIndex];

		if ( enemyWeapon != botWeapon ) {	// different weapons: calculate...
			// bot variables
			int botFlags = 0;
			if ( combat->botent->v.waterlevel == 3 ) botFlags |= WF_UNDERWATER;
			float botHitProb = action_targetaccuracy(combat->action);
			// enemy variables
			int enemyFlags = 0;
			if ( enemy->v.waterlevel == 3 ) enemyFlags |= WF_UNDERWATER;
			float enemyHitProb = percept_targetaccuracy(perceipt);
			adv = weaponhandling_getweaponscore(&combat->weapon, botWeapon, enemyDist, botHitProb, botFlags, true )
			    - weaponhandling_getweaponscore(&combat->weapon, enemyWeapon, enemyDist, enemyHitProb, enemyFlags, false );
		}
	} else {
		// bot variables
		int botFlags = 0;
		if ( combat->botent->v.waterlevel == 3 ) botFlags |= WF_UNDERWATER;
		float botHitProb = action_targetaccuracy(combat->action);
		adv = weaponhandling_getweaponscore(&combat->weapon, botWeapon, enemyDist, botHitProb, botFlags, true );
	}
	adv = 2 * adv;	// stronger influence
	if (adv > 5) adv = 5;
	else if (adv < -5) adv = -5;

	return adv;
}

bool
combat_shootatenemy(COMBAT *combat, Vec3D *enemyOrigin, float accuracy)
// picks best place to shoot at and fires
{
	Vec3D firePos, feetPos = {0.0f, 0.0f, -31.0f}, headpos = {0.0f, 0.0f, 28.0f};

	vcopy(enemyOrigin, &firePos);

	if ((action_getaimskill(combat->action) > 6) && (accuracy >= 0.5f)) {
		vadd(&firePos, &headpos, &firePos);	// aim at head
	}
	// TODO: Need ROCKETLAUNCHER flag
	switch(weaponhandling_currentweapon(&combat->weapon)) {
		default:
			break;
		case VALVE_WEAPON_RPG:
		case HW_WEAPON_ROCKETLAUNCHER:
		case DMC_WEAPON_ROCKETLAUNCHER:
			// aim at feet if possible
			vadd(&firePos, &feetPos, &feetPos);
			if (canshootat(combat->botent, &feetPos)) 
				vcopy(&feetPos, &firePos);
			break;
	}
	return weaponhandling_attack(&combat->weapon, &firePos, accuracy, NULL);
}

bool
combat_shootatenemy2(COMBAT *combat, EDICT *enemy, float accuracy)
// picks best place to shoot at and fires
{
	Vec3D firePos, feetPos, testPos, predictedMove, dir, vel;
	float dist;

	assert(enemy != 0);

	vcopy(&enemy->v.origin, &firePos);
	if ((action_getaimskill(combat->action) > 6) && (accuracy >= 0.5f)) {
		vadd(&firePos, &enemy->v.view_ofs, &firePos); // aim at head
	}

	// TODO: Need ROCKETLAUNCHER and NAILGUN flags
	switch(mod_id) {
	default:
		if (weaponhandling_currentweapon(&combat->weapon) == VALVE_WEAPON_RPG) {		
			// aim at feet if possible
			vcopy(&firePos, &feetPos);
			feetPos.z = enemy->v.absmin.z + 1.0f;
			if (canshootat(combat->botent, &feetPos)) 
				vcopy(&feetPos, &firePos);
		}
		break;
	case HOLYWARS_DLL:
		if (weaponhandling_currentweapon(&combat->weapon) == HW_WEAPON_ROCKETLAUNCHER) {	
			// aim at feet and predict movement
			vcopy(&firePos, &feetPos);
			feetPos.z = enemy->v.absmin.z + 1.0f;
			vsub(&combat->botent->v.origin, &enemy->v.origin, &dir);
			dist = vlen(&dir);
			vscale(&enemy->v.velocity, dist / 1600.0f, &predictedMove);
			vadd(&feetPos, &predictedMove, &testPos);
			if (canshootat(combat->botent, &testPos))
				vcopy(&testPos, &firePos);
		}
		break;
	case DMC_DLL:
		if (weaponhandling_currentweapon(&combat->weapon) == DMC_WEAPON_ROCKETLAUNCHER) {	
			// aim at feet and predict movement
			vcopy(&firePos, &feetPos);
			feetPos.z = enemy->v.absmin.z + 1.0f;
			vsub(&combat->botent->v.origin, &enemy->v.origin, &dir);
			dist = vlen(&dir);
			vscale(&enemy->v.velocity, dist / 1200.0f, &predictedMove);
			vadd(&feetPos, &predictedMove, &testPos);
			if (canshootat(combat->botent, &testPos))
				vcopy(&testPos, &firePos);
		} else if (weaponhandling_currentweapon(&combat->weapon) == DMC_WEAPON_NAILGUN ||
			    weaponhandling_currentweapon(&combat->weapon) == DMC_WEAPON_SUPERNAILGUN) {
			// predict movement
			vsub(&combat->botent->v.origin, &enemy->v.origin, &dir);
			dist = vlen(&dir);
			vscale(&enemy->v.velocity, dist / 1200.0f, &predictedMove);
			vadd(&firePos, &predictedMove, &testPos);
			if (canshootat(combat->botent, &testPos))
				vcopy(&testPos, &firePos);
		}
		break;
	}

	vsub(&enemy->v.velocity, &combat->botent->v.velocity, &vel);

	return weaponhandling_attack(&combat->weapon, &firePos, accuracy, &vel);
}

static void
combat_closecombatmovement(COMBAT *combat, PERCEPT *perceipt)
// decides which reaction is most appropiate and calls either engage() or retreat()
{
	EDICT *enemy = perceipt->entity;
	assert( enemy != 0 );
	if (enemy==0) debugFile( " ENEMY=0 " );
	if (combat->botent==0) debugFile( " BOT=0 " );
	
	// init distance to enemy
	float enemyDist = perceipt->distance;
		
	// init bot variables
	int botWeapon = weaponhandling_currentweapon(&combat->weapon);
	int botFlags = WF_FAST_ATTACK;
	if ( combat->botent->v.waterlevel == 3 ) {
		botFlags |= WF_UNDERWATER;
		DEBUG_MSG( "underwater\n" );
	}
	float botHitProb = action_targetaccuracy(combat->action);

	// init enemy variables
	int clientIndex = indexofedict( enemy ) - 1;
	assert( clientIndex >= 0 );
	assert( clientIndex < 32 );
	int enemyWeapon = clientWeapon[clientIndex];
	int enemyFlags = WF_FAST_ATTACK;
	if ( enemy->v.waterlevel == 3 ) enemyFlags |= WF_UNDERWATER;
	float enemyHitProb = percept_targetaccuracy(perceipt);

	// chose move direction
	combat->closeup=false; combat->gaindistance=false;
	if ( enemyDist < 1000 ) {	// don't give up sniper position
		if ( enemyWeapon != botWeapon ) {	// different weapons: calculate...
			float currentAdv = weaponhandling_getweaponscore(&combat->weapon, botWeapon, enemyDist, botHitProb, botFlags, true )
				- weaponhandling_getweaponscore(&combat->weapon, enemyWeapon, enemyDist, enemyHitProb, enemyFlags, false );
			float nearAdv = weaponhandling_getweaponscore(&combat->weapon, botWeapon, enemyDist-25, botHitProb, botFlags, true )
				- weaponhandling_getweaponscore(&combat->weapon, enemyWeapon, enemyDist-25, enemyHitProb, enemyFlags, false );
			float farAdv = weaponhandling_getweaponscore(&combat->weapon, botWeapon, enemyDist+25, botHitProb, botFlags, true )
				- weaponhandling_getweaponscore(&combat->weapon, enemyWeapon, enemyDist+25, enemyHitProb, enemyFlags, false );

			if ( (farAdv>currentAdv) && (farAdv>nearAdv) ) combat->gaindistance = true;
			if ( (nearAdv>currentAdv) && (nearAdv>farAdv) ) combat->closeup = true;
		} else {	// same weapons: go to optimum
			if  (weaponhandling_bestdistance(&combat->weapon, botWeapon ) > (enemyDist+25)) 
				combat->gaindistance = true;	// only for non-sniper-weapons
			else if (weaponhandling_bestdistance(&combat->weapon, botWeapon ) < enemyDist) 
				combat->closeup = true;
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
	if (percept_isvisible(perceipt) && percept_isaimingatbot(perceipt)
	    && (botWeapon != closeDistanceWeapon))	// don't evade with this
		combat_evade(combat, perceipt, &evadeMove);
	if (combat->gaindistance) {	// bigger distance better
		vsub(&enemy->v.origin, &combat->botent->v.origin, &tDir);
		vsub(&combat->botent->v.origin, &tDir, &tDir);
		vadd(&evadeMove, &tDir, &tDir);
		roaming_checkway(combat->pathfinder, &tDir);
	} else if (combat->closeup) {	// closer distance better
		vadd(&enemy->v.origin, &evadeMove, &tDir);
		roaming_checkway(combat->pathfinder, &tDir);
	} else if (vlen(&evadeMove) > 0) {	// just evade
		vadd(&combat->botent->v.origin, &evadeMove, &tDir);
		roaming_checkway(combat->pathfinder, &tDir);
	} else {	// no move -> duck
		action_add(combat->action, BOT_DUCK, NULL);
	}
}

static void
combat_retreat(COMBAT *combat, EDICT *enemy)
// flees from the enemy
{
	Vec3D tDir;

	assert( enemy != 0 );

	vsub(&enemy->v.origin, &combat->botent->v.origin, &tDir);
	vsub(&combat->botent->v.origin, &tDir, &tDir);
	roaming_checkway(combat->pathfinder, &tDir);
}

bool
combat_hasweapon(COMBAT *combat, int wId)
{
	return weaponhandling_available(&combat->weapon, wId);
}

static void
combat_idleactions(COMBAT *combat)
// manages weapon actions when no enemy is around
{
/*	if ( worldtime() > combat->nextweaponcheck ) {
		float dist = 200;	// short distance
		float hitProb = 200/dist;
		if (hitProb>1) hitProb = 1;
		int flags = 0;
		combat->nextweaponcheck = worldtime() + CHECK_WEAPON_IDLE;
		weaponhandling_armbestweapon(&weapon, dist, hitProb, flags );
	}*/
}
