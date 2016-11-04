#include "pb_weaponhandling.h"
#include "pb_global.h"
#include "bot.h"
#include "bot_weapons.h"
#include "pm_defs.h"
#include "pb_configuration.h"
#include "parabot.h"

extern bot_t bots[32];
extern int mod_id;
extern bot_weapon_t weapon_defs[MAX_WEAPONS];
extern PB_Configuration pbConfig;	// from configfiles.cpp




PB_WeaponHandling::PB_WeaponHandling()
{
	switch (mod_id)	{
	case VALVE_DLL:		minModWeapon = MIN_VALVE_WEAPONS;
						maxModWeapon = MAX_VALVE_WEAPONS;
						defaultWeapon = VALVE_WEAPON_GLOCK;
						break;
	case HOLYWARS_DLL:	minModWeapon = MIN_HW_WEAPONS;
						maxModWeapon = MAX_HW_WEAPONS;
						defaultWeapon = HW_WEAPON_JACKHAMMER;
						break;
	case DMC_DLL:		minModWeapon = MIN_DMC_WEAPONS;
						maxModWeapon = MAX_DMC_WEAPONS;
						defaultWeapon = DMC_WEAPON_QUAKEGUN;
						break;
	case CSTRIKE_DLL:	minModWeapon = MIN_CS_WEAPONS;
						maxModWeapon = MAX_CS_WEAPONS;
						defaultWeapon = minModWeapon;
						break;
	case TFC_DLL:		minModWeapon = MIN_TFC_WEAPONS;
						maxModWeapon = MAX_TFC_WEAPONS;
						defaultWeapon = minModWeapon;
						break;
	case GEARBOX_DLL:	minModWeapon = MIN_GEARBOX_WEAPONS;
						maxModWeapon = MAX_GEARBOX_WEAPONS;
						defaultWeapon = VALVE_WEAPON_GLOCK;
						break;
	}
}


void PB_WeaponHandling::init( int slot, edict_t *ent, PB_Action *action )
// has to be called with the botSlot before all other methods
{
	weapon.init( slot, ent, action );
	botSlot = slot;
	botEnt = ent;
	botAction = action;
	lastModeSwitch = 0;
	preferredWeaponTimeOut = 0;
	weaponUsable = true;
	bots[botSlot].current_weapon.iId = defaultWeapon;
	armedWeapon = defaultWeapon;
}


void PB_WeaponHandling::initCurrentWeapon()
{
	int cwId = bots[botSlot].current_weapon.iId;
	if( cwId >= minModWeapon && cwId < maxModWeapon ) {
		armedWeapon = cwId;
		weapon.registerArmedWeapon( armedWeapon );
	}
	else {
		// strange values in weapon_id, check model:
		if (botEnt->v.weaponmodel != 0) {
			debugMsg( "CWBug" );
		}
	}
	//int a = weapon.ammo1();
	//debugMsg( "Ammo1 = %i\n", a );
}


bool PB_WeaponHandling::attack( Vector target, float accuracy, Vector relVel )
// attacks in best mode at best time the given position when accuracy is reached
{
	weapon.setCurrentWeapon( currentWeapon() );
	return weapon.attack( target, accuracy, relVel );
}


void PB_WeaponHandling::checkForForcedAttack()
{
	weapon.setCurrentWeapon( currentWeapon() );
	if ( weapon.hasToFinishAttack() ) {
		if ( currentWeapon() != weapon.armedGrenade() ) {
			// if bot picked up other weapon while grenade is still armed:
			switchToWeapon( weapon.armedGrenade() );
		}
		weapon.finishAttack();
	}
}


int PB_WeaponHandling::currentWeapon()
{
	return armedWeapon;
}


bool PB_WeaponHandling::available( int wId ) 
{
	if (pbConfig.onRestrictedWeaponMode()) {
		// exclude powerful weapons
		switch (mod_id) {

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

	int mask = 1 << wId;
	if (bots[botSlot].bot_weapons & mask) {
		// bot has this weapon - ammo as well?
		PB_Weapon wpn( wId );
		wpn.init( botSlot, botEnt, botAction );
		if (wpn.ammo1() == 0 && wpn.bestDistance() >= 30) return false;
		else return true;
	}
	else return false;
}


int PB_WeaponHandling::getBestWeapon( float distance, float hitProb, int flags )
{
	float score, bestScore = -10;
	int   bestWeapon = defaultWeapon;
	
			
	for (int wId=minModWeapon; wId<maxModWeapon; wId++) {
		if (available( wId )) {
			weapon.setCurrentWeapon( wId );
			score = weapon.getScore( distance, hitProb, flags, true );
			if (score > bestScore) {
				bestScore = score;
				bestWeapon = wId;
			}
		}
	}

	if (bestScore > 0) weaponUsable = true;
	else			   weaponUsable = false;
	
	return bestWeapon;
}

extern int debug_engine;
void CmdStart( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed );
void CmdEnd ( const edict_t *player );


void PB_WeaponHandling::switchToWeapon( int wId )
{
	weapon.setCurrentWeapon( wId );

	if (mod_id==DMC_DLL) {
		usercmd_s cmd;
		cmd.lerp_msec = 0;					// Interpolation time on client
		cmd.msec = 30;						// Duration in ms of command
		cmd.viewangles = botEnt->v.v_angle; // Command view angles.
		// intended velocities
		cmd.forwardmove = 0;				// Forward velocity.
		cmd.sidemove = 0;					// Sideways velocity.
		cmd.upmove = 0;						// Upward velocity.
		cmd.lightlevel = 0;					// Light level at spot where we are standing.
		cmd.buttons = 0;					// Attack buttons
		cmd.impulse = 0;					// Impulse command issued.
		cmd.weaponselect = wId+1;			// Current weapon id ( WEAPON SLOT! )
		CmdStart( botEnt, &cmd, 0 );
		CmdEnd( botEnt );
	}
	else { 
		UTIL_SelectItem( bots[botSlot].pEdict, weapon.name() );
	}

	bots[botSlot].parabot->action.setWeaponCone( weapon.cone() );
	weapon.setNextAttackTime( worldTime() + CHANGE_WEAPON_DELAY );
}


bool PB_WeaponHandling::armBestWeapon( float distance, float hitProb, int flags )
{
	weapon.setCurrentWeapon( currentWeapon() );

	if ( weapon.hasToFinishAttack() ) {
		debugMsg( "Must use grenade!\n" );
		return true;
	}
	
	int bestWeapon;

	if (worldTime() < preferredWeaponTimeOut && available( preferredWeapon )) {
		bestWeapon = preferredWeapon;
		weapon.setAttackMode( preferredMode );
	}
	else {
		bestWeapon = getBestWeapon( distance, hitProb, flags );
	}

	if (currentWeapon() != bestWeapon) {
		switchToWeapon( bestWeapon );
		return false;
	}
	//debugMsg( "Current weapon id = %i, ", currentWeapon() );
	//debugMsg( "clip=%i, ", bots[botSlot].current_weapon.iClip );
	//debugMsg( "ammo1=%i, ", bots[botSlot].current_weapon.iAmmo1 );
	//debugMsg( "ammo2=%i\n", bots[botSlot].current_weapon.iAmmo2 );

	int bestMode = weapon.bestAttackMode();
	
	if (mod_id==VALVE_DLL || mod_id==GEARBOX_DLL) {	// switch to correct weapon-mode
		if ( bestWeapon==VALVE_WEAPON_CROSSBOW || bestWeapon==VALVE_WEAPON_PYTHON ) {	
			if ( bestMode==1 && botEnt->v.fov!=90 && 
				 (lastModeSwitch+0.5)<worldTime() ) {
				botAction->add( BOT_FIRE_SEC );
				lastModeSwitch = worldTime();
				//debugMsg( "Using NORMAL MODE!\n" );
			}
			else if ( bestMode==2 && botEnt->v.fov==90 &&
					  (lastModeSwitch+0.5)<worldTime() ) {
				botAction->add( BOT_FIRE_SEC );
				//debugMsg( "Using ZOOM!\n" );
				lastModeSwitch = worldTime();
			}
		}
		else if ( bestWeapon==VALVE_WEAPON_RPG ) {
			char *rpgClass = (char*)getActiveItem( botEnt );
			if (rpgClass) {
				int  *spotActive = (int*)(rpgClass+124);
				if ( bestMode==1 && (*spotActive)==0 && 
					(lastModeSwitch+0.5)<worldTime() ) {
					botAction->add( BOT_FIRE_SEC );
					lastModeSwitch = worldTime();
					debugMsg( "Using RPG laser mode!\n" );
				}
				else if ( bestMode==2 && (*spotActive)!=0 &&
					(lastModeSwitch+0.5)<worldTime() ) {
					botAction->add( BOT_FIRE_SEC );
					debugMsg( "Using RPG sneak mode!\n" );
					lastModeSwitch = worldTime();
				}
			}
		}
	}
	// check for reload
	if ( weapon.needReload() ) {
		weapon.reload();
		return false;
	}
	return true;
}


void PB_WeaponHandling::setPreferredWeapon( int wId, int mode )
// sets the weapon that will be armed by armBestWeapon during the next 0.5 seconds
{
	if (available( wId )) {
		preferredWeapon = wId;
		preferredWeaponTimeOut = worldTime() + 0.5;
		preferredMode = mode;
	}
}


float PB_WeaponHandling::getWeaponScore( int wId, float distance, float hitProb, int flags, bool checkAmmo )
{
	weapon.setCurrentWeapon( wId );
	return weapon.getScore( distance, hitProb, flags, checkAmmo );
}


float PB_WeaponHandling::bestDistance( int wId )
{
	weapon.setCurrentWeapon( wId );
	return weapon.bestDistance();
}


float PB_WeaponHandling::currentHighAimProb()
{
	weapon.setCurrentWeapon( currentWeapon() );
	return weapon.highAimProb();
}

