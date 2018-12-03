#include "parabot.h"
#include "bot.h"
#include "bot_weapons.h"
#include "pb_configuration.h"

extern bot_t bots[32];
extern int mod_id;
extern bot_weapon_t weapon_defs[MAX_WEAPONS];
extern PB_Configuration pbConfig;	// from configfiles.cpp
/*
PB_WeaponHandling::PB_WeaponHandling()
{
	switch (mod_id)	{
	case AG_DLL:
	case VALVE_DLL:		minmodweapon = MIN_VALVE_WEAPONS;
						maxmodweapon = MAX_VALVE_WEAPONS;
						defaultweapon = VALVE_WEAPON_GLOCK;
						break;
	case HOLYWARS_DLL:	minmodweapon = MIN_HW_WEAPONS;
						maxmodweapon = MAX_HW_WEAPONS;
						defaultweapon = HW_WEAPON_JACKHAMMER;
						break;
	case DMC_DLL:		minmodweapon = MIN_DMC_WEAPONS;
						maxmodweapon = MAX_DMC_WEAPONS;
						defaultweapon = DMC_WEAPON_QUAKEGUN;
						break;
	case CSTRIKE_DLL:	minmodweapon = MIN_CS_WEAPONS;
						maxmodweapon = MAX_CS_WEAPONS;
						defaultweapon = minmodweapon;
						break;
	case TFC_DLL:		minmodweapon = MIN_TFC_WEAPONS;
						maxmodweapon = MAX_TFC_WEAPONS;
						defaultweapon = minmodweapon;
						break;
	case HUNGER_DLL:	minmodweapon = MIN_HUNGER_WEAPONS;
						maxmodweapon = MAX_HUNGER_WEAPONS;
						defaultweapon = VALVE_WEAPON_GLOCK;
						break;
	case GEARBOX_DLL:	minmodweapon = MIN_GEARBOX_WEAPONS;
						maxmodweapon = MAX_GEARBOX_WEAPONS;
						defaultweapon = VALVE_WEAPON_GLOCK;
						break;
	}
}
*/

void
weaponhandling_init(WEAPONHANDLING *wh, int slot, EDICT *ent, ACTION *action )
// has to be called with the botSlot before all other methods
{
	weapon_init(&wh->weapon, slot, ent, action );
	wh->botslot = slot;
	wh->botent = ent;
	wh->botaction = action;
	wh->lastmodeswitch = 0;
	wh->preferredweapontimeout = 0;
	wh->weaponusable = true;
	bots[wh->botslot].current_weapon.iId = wh->defaultweapon;
	wh->armedweapon = wh->defaultweapon;
}

void
weaponhandling_initcurrentweapon(WEAPONHANDLING *wh)
{
	int cwId = bots[wh->botslot].current_weapon.iId;
	if( cwId >= wh->minmodweapon && cwId < wh->maxmodweapon) {
		wh->armedweapon = cwId;
		weapon_registerarmedweapon(&wh->weapon, wh->armedweapon);
	} else {
		// strange values in weapon_id, check model:
		if (wh->botent->v.weaponmodel != 0) {
			DEBUG_MSG( "CWBug" );
		}
	}
	//int a = weapon_ammo1(&wh->weapon);
	// DEBUG_MSG( "Ammo1 = %i\n", a );
}

bool
weaponhandling_attack(WEAPONHANDLING *wh, Vec3D *target, float accuracy, Vec3D *relVel)
// attacks in best mode at best time the given position when accuracy is reached
{
	weapon_setcurrentweapon(&wh->weapon, weaponhandling_currentweapon(wh) );
	return weapon_attack(&wh->weapon, target, accuracy, relVel );
}

static void
weaponhandling_switchtoweapon(WEAPONHANDLING *wh, int wId)
{
	char cmd[32];
	weapon_setcurrentweapon(&wh->weapon, wId);

	if (mod_id == DMC_DLL) {
		sprintf(cmd, "slot%d", wId + 1);
		FakeClientCommand(bots[wh->botslot].e, cmd, NULL, NULL);
	} else {
		FakeClientCommand(bots[wh->botslot].e, weapon_name(&wh->weapon), NULL, NULL);
	}

	action_setweaponcone(&bots[wh->botslot].parabot->action, weapon_cone(&wh->weapon));
	weapon_setnextattacktime(&wh->weapon, worldtime() + CHANGE_WEAPON_DELAY);
}

void
weaponhandling_checkforforcedattack(WEAPONHANDLING *wh)
{
	weapon_setcurrentweapon(&wh->weapon, weaponhandling_currentweapon(wh) );
	if ( weapon_hastofinishattack(&wh->weapon) ) {
		if ( weaponhandling_currentweapon(wh) != weapon_armedgrenade(&wh->weapon) ) {
			// if bot picked up other weapon while grenade is still armed:
			weaponhandling_switchtoweapon(wh, weapon_armedgrenade(&wh->weapon) );
		}
		weapon_finishattack(&wh->weapon);
	}
}

int
weaponhandling_currentweapon(WEAPONHANDLING *wh)
{
	return wh->armedweapon;
}

bool
weaponhandling_available(WEAPONHANDLING *wh, int wId)
{
	if (pbConfig.onRestrictedWeaponMode()) {
		// exclude powerful weapons
		switch (mod_id) {
		case AG_DLL:
		case VALVE_DLL:	
			if ( wId == VALVE_WEAPON_MP5		||
				 wId == VALVE_WEAPON_CROSSBOW	||
				 wId == VALVE_WEAPON_SHOTGUN	||
				 wId == VALVE_WEAPON_RPG		||
				 wId == VALVE_WEAPON_GAUSS		||
				 wId == VALVE_WEAPON_EGON			) return false;
			break;

		case HOLYWARS_DLL:
			if ( wId == HW_WEAPON_ROCKETLAUNCHER	||
				 wId == HW_WEAPON_RAILGUN			   ) return false;
			break;

		case DMC_DLL:
			if ( wId == DMC_WEAPON_SUPERNAILGUN		||
				 wId == DMC_WEAPON_GRENLAUNCHER		||
				 wId == DMC_WEAPON_ROCKETLAUNCHER	||
				 wId == DMC_WEAPON_LIGHTNING		   ) return false;
			break;
		case HUNGER_DLL:
                        if ( wId == VALVE_WEAPON_MP5            ||
				wId == VALVE_WEAPON_CHAINGUN	||
				wId == VALVE_WEAPON_CROSSBOW	||
				wId == VALVE_WEAPON_SHOTGUN	||
				wId == VALVE_WEAPON_RPG		||
				wId == VALVE_WEAPON_GAUSS	||
				wId == VALVE_WEAPON_EGON	||
				wId == HUNGER_WEAPON_AP9	||
				wId == HUNGER_WEAPON_TFCSNIPER	||
				wId == HUNGER_WEAPON_SNIPER
				) return false;
                        break;
		case GEARBOX_DLL:
			if ( wId == VALVE_WEAPON_MP5		||
				 wId == VALVE_WEAPON_CROSSBOW	||
				 wId == VALVE_WEAPON_SHOTGUN	||
				 wId == VALVE_WEAPON_RPG		||
				 wId == VALVE_WEAPON_GAUSS		||
				 wId == VALVE_WEAPON_EGON		||
				 wId == GEARBOX_WEAPON_EAGLE	||
				 wId == GEARBOX_WEAPON_M249		||
				 wId == GEARBOX_WEAPON_SHOCKRIFLE	||
				 wId == GEARBOX_WEAPON_SNIPERRIFLE		) return false;
			break;
		}
	}

	int mask = BIT(wId);
	if (bots[wh->botslot].e->v.weapons & mask) {
		// bot has this weapon - ammo as well?
		WEAPON wpn;
		weapon_construct2(&wpn, wId);
		weapon_init(&wpn, wh->botslot, wh->botent, wh->botaction );
		if (weapon_ammo1(&wpn) == 0 && weapon_bestdistance(&wpn) >= 30) return false;
		return true;
	}
	return false;
}

int
weaponhandling_getbestweapon(WEAPONHANDLING *wh, float distance, float hitProb, int flags)
{
	float score, bestScore = -10;
	int   bestWeapon = wh->defaultweapon;
			
	for (int wId = wh->minmodweapon; wId < wh->maxmodweapon; wId++) {
		if (weaponhandling_available(wh, wId)) {
			weapon_setcurrentweapon(&wh->weapon, wId);
			score = weapon_getscore(&wh->weapon, distance, hitProb, flags, true);
			if (score > bestScore) {
				bestScore = score;
				bestWeapon = wId;
			}
		}
	}

	if (bestScore > 0)
		wh->weaponusable = true;
	else
		wh->weaponusable = false;

	return bestWeapon;
}

bool
weaponhandling_bestweaponusable(WEAPONHANDLING *wh)
{
	return wh->weaponusable;
}

bool
weaponhandling_armbestweapon(WEAPONHANDLING *wh, float distance, float hitProb, int flags)
{
	weapon_setcurrentweapon(&wh->weapon, weaponhandling_currentweapon(wh));

	if (weapon_hastofinishattack(&wh->weapon) ) {
		DEBUG_MSG( "Must use grenade!\n" );
		return true;
	}

	int bestWeapon;

	if (worldtime() < wh->preferredweapontimeout && weaponhandling_available(wh, wh->preferredweapon)) {
		bestWeapon = wh->preferredweapon;
		weapon_setattackmode(&wh->weapon, wh->preferredmode);
	} else {
		bestWeapon = weaponhandling_getbestweapon(wh, distance, hitProb, flags );
	}

	if (weaponhandling_currentweapon(wh) != bestWeapon) {
		weaponhandling_switchtoweapon(wh, bestWeapon );
		return false;
	}
//	DEBUG_MSG( "Current weapon id = %i, clip=%i, ammo1=%i, ammo2=%i\n",
//	    currentWeapon, bots[botSlot].current_weapon.iClip, bots[botSlot].current_weapon.iAmmo1, bots[botSlot].current_weapon.iAmmo2 );

	int bestMode = weapon_bestattackmode(&wh->weapon);
	
	if (mod_id == VALVE_DLL || mod_id==AG_DLL || mod_id==HUNGER_DLL || mod_id==GEARBOX_DLL) {	// switch to correct weapon-mode
		if (bestWeapon == VALVE_WEAPON_CROSSBOW || bestWeapon==VALVE_WEAPON_PYTHON ) {	
			if ( bestMode==1 && wh->botent->v.fov!=90 && 
				 (wh->lastmodeswitch+0.5)<worldtime() ) {
				action_add(wh->botaction, BOT_FIRE_SEC, NULL);
				wh->lastmodeswitch = worldtime();
				// DEBUG_MSG( "Using NORMAL MODE!\n" );
			} else if ( bestMode == 2 && wh->botent->v.fov == 90 &&
					  (wh->lastmodeswitch + 0.5f) < worldtime() ) {
				action_add(wh->botaction, BOT_FIRE_SEC, NULL);
				// DEBUG_MSG( "Using ZOOM!\n" );
				wh->lastmodeswitch = worldtime();
			}
		} else if ( bestWeapon == VALVE_WEAPON_RPG ) {
			EDICT *pent = NULL;
			bool spotActive = false;
			while((pent = find_entityinsphere(pent, &wh->botent->v.origin, MAX_DIST_VP))) {
				if( Q_STREQ(STRING(pent->v.classname), "laser_spot")) {
					if(laserdotowner(pent) == wh->botent) {
						spotActive = true;
						break;
					}
				}
			}
			if( bestMode == 1 && !spotActive && 
				(wh->lastmodeswitch + 0.5f) < worldtime() ) {
				action_add(wh->botaction, BOT_FIRE_SEC, NULL);
				wh->lastmodeswitch = worldtime();
				DEBUG_MSG( "Using RPG laser mode!\n" );
			} else if( bestMode == 2 && spotActive &&
				(wh->lastmodeswitch + 0.5f) < worldtime() ) {
				action_add(wh->botaction, BOT_FIRE_SEC, NULL);
				DEBUG_MSG( "Using RPG sneak mode!\n" );
				wh->lastmodeswitch = worldtime();
			}
		}
	}
	// check for reload
	if (weapon_needreload(&wh->weapon)) {
		weapon_reload(&wh->weapon);
		return false;
	}
	return true;
}


void
weaponhandling_setpreferredweapon(WEAPONHANDLING *wh, int wId, int mode )
// sets the weapon that will be armed by armBestWeapon during the next 0.5 seconds
{
	if (weaponhandling_available(wh, wId )) {
		wh->preferredweapon = wId;
		wh->preferredweapontimeout = worldtime() + 0.5f;
		wh->preferredmode = mode;
	}
}

float
weaponhandling_getweaponscore(WEAPONHANDLING *wh, int wId, float distance, float hitProb, int flags, bool checkAmmo )
{
	weapon_setcurrentweapon(&wh->weapon, wId );
	return weapon_getscore(&wh->weapon, distance, hitProb, flags, checkAmmo );
}

float
weaponhandling_bestdistance(WEAPONHANDLING *wh, int wId )
{
	weapon_setcurrentweapon(&wh->weapon, wId );
	return weapon_bestdistance(&wh->weapon);
}

float
weaponhandling_currenthighaimprob(WEAPONHANDLING *wh)
{
	weapon_setcurrentweapon(&wh->weapon, weaponhandling_currentweapon(wh) );
	return weapon_highaimprob(&wh->weapon);
}

