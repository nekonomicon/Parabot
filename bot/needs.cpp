#include "parabot.h"
#include "pb_global.h"
#include "bot.h"

extern int mod_id;
extern bool haloOnBase;
extern bool headToBunker;
extern float nextAirstrikeTime;

void
needs_init(NEEDS *needs, CParabot *botClass)
{
	needs->airstrikeknown = false;
	needs->bot = botClass;
	needs->haloknownonbase = false;
	needs->maxwish = 0;
	needs->newitempriorities = false;
	needs->weaponwish = 0;
	memset( &needs->wish, 0, sizeof needs->wish );
	needs->wishupdate = -100;
}

float
needs_desirefor(NEEDS *needs, int navId)
{
	return needs->wish[navId];
}

float
needs_wishforitems(NEEDS *needs)
{
	return needs->maxwish;
}

float
needs_wishforweapons(NEEDS *needs)
{
	return needs->weaponwish;
}

float
needs_wishforhealth(NEEDS *needs)
// returns a value between 0 and 10 indicating the need for health
{ 
	#define MIN_HEALTH 20.0	// need=10 if health is below this value
	
	float need = (100.0 - needs->bot->ent->v.health) * (10.0 / (100.0 - MIN_HEALTH));
	if (need < 0)
		need = 0;
	else if (need > 10)
		need = 10;
	// DEBUG_MSG( "VHealth=%.1f   ", ent->v.health );
	// DEBUG_MSG( "Health=%.1f\n", need );
	return need;
}

float
needs_wishforarmor(NEEDS *needs)
// returns a value between 0 and 10 indicating the need for armor
{ 
	#define	MAX_ARMOR_WISH 4.0	// need for armor when armorvalue==0

	float need = (100.0 - needs->bot->ent->v.armorvalue) * (0.01 * MAX_ARMOR_WISH);
	if (need < 0)
		need = 0;
	// DEBUG_MSG( "Armor=%.1f\n", need );
	return need;
}


float
needs_wishforcombat(NEEDS *needs)
// returns a value between 0 and 10 indicating the wish for enemy encounter
{
	// health
	float health = (needs->bot->ent->v.health - 10) / (70 - 10);
	if (health < 0)
		health = 0;
	else if (health > 1.2)
		health = 1.2;
	// armor
	float armor = (needs->bot->ent->v.armorvalue / 100) + 0.9;
	if (armor > 1.4)
		armor = 1.4;
	// weapon
	float weapon = 0.1;

	switch (mod_id) {
	case AG_DLL:
	case VALVE_DLL:
		if(combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_EGON) 
		    && !(com.gamedll_flags & (GAMEDLL_BMOD | GAMEDLL_SEVS))) {
			weapon = 1;
		} else if(combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_EGON) 
		    && bm_gluon
		    && !bm_gluon->value) {
			weapon = 1;	
		} else if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_GAUSS))
			weapon = 1;
		else if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_MP5)
		    || combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_RPG))
			weapon = 0.8;
		else if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_SHOTGUN))
			weapon = 0.6;
		else if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_HORNETGUN)
		    || combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_PYTHON)
		    || combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_CROSSBOW))
			weapon = 0.4;
		break;
	case HOLYWARS_DLL:
		if (combat_hasweapon(&needs->bot->combat, HW_WEAPON_MACHINEGUN)
		    || combat_hasweapon(&needs->bot->combat, HW_WEAPON_ROCKETLAUNCHER))
			weapon = 1;
		else if (combat_hasweapon(&needs->bot->combat, HW_WEAPON_RAILGUN))
			weapon = 0.8;
		else if (combat_hasweapon(&needs->bot->combat, HW_WEAPON_DOUBLESHOTGUN))
			weapon = 0.7;
		break;
	case DMC_DLL:
		if (combat_hasweapon(&needs->bot->combat, DMC_WEAPON_LIGHTNING)
		    || combat_hasweapon(&needs->bot->combat, DMC_WEAPON_SUPERNAILGUN))
			weapon = 1;
		else if (combat_hasweapon(&needs->bot->combat, DMC_WEAPON_NAILGUN)
		    || combat_hasweapon(&needs->bot->combat, DMC_WEAPON_ROCKETLAUNCHER))
			weapon = 0.8;
		else if (combat_hasweapon(&needs->bot->combat, DMC_WEAPON_SUPERSHOTGUN)
		    || combat_hasweapon(&needs->bot->combat, DMC_WEAPON_GRENLAUNCHER))
			weapon = 0.6;
		break;
	case HUNGER_DLL:
	case GEARBOX_DLL:
		weapon = 0.8;
		break;
	}
	// wish
	float wish_c = health * armor * weapon * 10 + 0.5 * needs->bot->aggression;
	if (wish_c > 10.0f)
		wish_c = 10.0f;
	return wish_c;
}

float
needs_wishforsniping(NEEDS *needs, bool weaponCheck)
// returns a value between 0 and 10 indicating the wish for sniping
{
	// health
	float health = (needs->bot->ent->v.health - 10) / (80 - 10);
	if (health < 0)
		health = 0;
	else if (health > 1)
		health = 1;
	// weapon
	float weapon = 0;
	if (weaponCheck) {
		switch (mod_id) {
			case AG_DLL:
			case VALVE_DLL:
				if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_CROSSBOW))
					weapon = 1;
				if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_PYTHON))
					weapon = 0.5; 
				break;
			case HOLYWARS_DLL:
				if (combat_hasweapon(&needs->bot->combat, HW_WEAPON_RAILGUN))
					weapon = 1;
				break;
			case DMC_DLL:
				if (combat_hasweapon(&needs->bot->combat, DMC_WEAPON_LIGHTNING))
					weapon = 1;
				break;
			case GEARBOX_DLL:
				if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_CROSSBOW))
					weapon = 1;
				break;
			case HUNGER_DLL:
				if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_CROSSBOW)
				    || combat_hasweapon(&needs->bot->combat, HUNGER_WEAPON_TFCSNIPER)
				    || combat_hasweapon(&needs->bot->combat, HUNGER_WEAPON_SNIPER))
					weapon = 1;
				break;
		}
	} else
		weapon = 1.5;	// turrets
			  
	// wish
	float wish_s = health * weapon * 5 + (5 - needs->bot->aggression);
	
	const float outTime = 40;// time after which camping gives 0 points
	float x = outTime + (worldtime() - needs->bot->lastCamp) - needs->bot->campTime;
	// while camping x=outTime-campTime, else x increasing
	if (x > outTime) {
		if (needs->bot->aggression < 2.5)
			x = outTime + (x - outTime) / (10 * needs->bot->aggression);
		else
			x = outTime;
	}
	float timeFactor = x / outTime;
//	if (timeFactor > 0) timeFactor = 1;	// not linear!

	return (timeFactor * wish_s);
}

bool
needs_newpriorities(NEEDS *needs)
{
	return needs->newitempriorities;
}

void
needs_affirmpriorities(NEEDS *needs)
{
	needs->newitempriorities = false;
}

static void
needs_valvewishlist(NEEDS *needs)
{
	int i;

	if (headToBunker) {
		needs->wish[NAV_S_AIRSTRIKE_COVER] = 20;
		needs->maxwish = needs->wish[NAV_S_AIRSTRIKE_COVER];
		if (!needs->airstrikeknown) {
			needs->newitempriorities = true;
			needs->airstrikeknown = true;
		}

		if (mapgraph_getnearestnavpoint(&zerovector, NAV_S_AIRSTRIKE_COVER ))
			return;	// only head for bunker if cover exists!
	}
	else if (needs->airstrikeknown) {
		needs->newitempriorities = true;
		needs->airstrikeknown = false;
	}

	if (worldtime() > nextAirstrikeTime) needs->wish[NAV_S_AIRSTRIKE_BUTTON] = 2;

	needs->wish[NAV_I_HEALTHKIT]     = needs_wishforhealth(needs);
	needs->wish[NAV_F_HEALTHCHARGER] = needs_wishforhealth(needs);
	if (needs->bot->senses.numEnemies > 0) needs->wish[NAV_F_HEALTHCHARGER] = 0;
	needs->wish[NAV_I_BATTERY]		  = needs_wishforarmor(needs);
	needs->wish[NAV_F_RECHARGE]	  = needs_wishforarmor(needs);
	if (needs->bot->senses.numEnemies > 0) needs->wish[NAV_F_RECHARGE] = 0;

	needs->wish[NAV_S_CAMPING] = needs_wishforsniping(needs, true) - 0.5;
	needs->wish[NAV_F_TANKCONTROLS] = needs_wishforsniping(needs, false) - 0.5;

	if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_TRIPMINE )) needs->wish[NAV_S_USE_TRIPMINE] = 2;

	if (!needs->bot->hasLongJump()) needs->wish[NAV_I_LONGJUMP] = 5;

	if ( !( combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_MP5 )		|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_SHOTGUN )	|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_GAUSS )		|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_EGON )			))
	{	// no big gun at hand...
		needs->wish[NAV_W_MP5]		= 9;
		needs->wish[NAV_W_SHOTGUN] = 9;
		needs->wish[NAV_W_GAUSS]	= 9;
		needs->wish[NAV_W_EGON]	= 9;
	}
	else
	{	// have one but want more :-)
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_EGON		)) needs->wish[NAV_W_EGON]			= 5;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_GAUSS		)) needs->wish[NAV_W_GAUSS]		= 5;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_MP5			)) needs->wish[NAV_W_MP5]			= 4;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_SHOTGUN		)) needs->wish[NAV_W_SHOTGUN]		= 3;
	}
	// rest of armatory...
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_CROSSBOW	)) needs->wish[NAV_W_CROSSBOW]		= 3;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_HORNETGUN	)) needs->wish[NAV_W_HORNETGUN]	= 1.5;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_PYTHON		)) needs->wish[NAV_W_PYTHON]		= 2;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_RPG			)) needs->wish[NAV_W_RPG]			= 4.5;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_TRIPMINE	)) needs->wish[NAV_W_TRIPMINE]		= 2;

	if(com.gamedll_flags & GAMEDLL_BMOD) {
		if( !combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_CROWBAR ) )
			needs->wish[NAV_W_CROWBAR] = 9;
	}

	// copy identical ids
	needs->wish[NAV_W_9MMAR]	= needs->wish[NAV_W_MP5];
	needs->wish[NAV_W_357]		= needs->wish[NAV_W_PYTHON];

	// grenades
	needs->wish[NAV_W_HANDGRENADE] = 1;
	needs->wish[NAV_W_SATCHEL] = 1.5;
	needs->wish[NAV_W_SNARK] = 2;

	// ammo
	needs->wish[NAV_A_MP5GRENADES] = 1.5;
	needs->wish[NAV_A_MP5CLIP] = 0.8;
	needs->wish[NAV_A_EGONCLIP] = 0.8;
	needs->wish[NAV_A_RPGCLIP] = 0.8;
	needs->wish[NAV_A_RPG_ROCKET] = 0.4;
	needs->wish[NAV_A_CROSSBOW_BOLT] = 0.4;
	needs->wish[NAV_A_BUCKSHOT] = 0.4;	
	needs->wish[NAV_A_357] = 0.4;

	// copy identical ids
	needs->wish[NAV_A_ARGRENADES]	= needs->wish[NAV_A_MP5GRENADES];
	needs->wish[NAV_A_9MMCLIP]		= needs->wish[NAV_A_MP5CLIP];
	needs->wish[NAV_A_9MMAR]		= needs->wish[NAV_A_MP5CLIP];
	needs->wish[NAV_A_GAUSSCLIP]	= needs->wish[NAV_A_EGONCLIP];

	needs->maxwish = 0;
	for (i = 0; i < MAX_NAV_TYPES; i++) 
		if (mapgraph_itemavailable(i) && (needs->wish[i] > needs->maxwish))
			needs->maxwish = needs->wish[i];
	needs->weaponwish = 0;
	for (i = NAV_W_CROSSBOW; i <= NAV_A_GLOCKCLIP; i++) {
		if (mapgraph_itemavailable(i))
			needs->weaponwish += needs->wish[i];
	}
}

static void
needs_hwwishlist(NEEDS *needs)
{
	int i;

	if (haloOnBase) {
		needs->wish[NAV_HW_HALOBASE] = 20;
		needs->maxwish = needs->wish[NAV_HW_HALOBASE];
		if (!needs->haloknownonbase) {
			needs->bot->senses.resetPlayerClassifications();
			needs->newitempriorities = true;
			needs->haloknownonbase = true;
		}
		return;	// no other targets...
	} else if (needs->haloknownonbase) {
		needs->bot->senses.resetPlayerClassifications();
		needs->newitempriorities = true;
		needs->haloknownonbase = false;
	}

	needs->wish[NAV_I_HEALTHKIT]     = needs_wishforhealth(needs);
	needs->wish[NAV_F_HEALTHCHARGER] = needs_wishforhealth(needs);
	if (needs->bot->senses.numEnemies > 0) needs->wish[NAV_F_HEALTHCHARGER] = 0;
	needs->wish[NAV_I_BATTERY]		  = needs_wishforarmor(needs);

	needs->wish[NAV_S_CAMPING] = needs_wishforsniping(needs, true)-0.5;

	if (!combat_hasweapon(&needs->bot->combat, HW_WEAPON_DOUBLESHOTGUN	)) needs->wish[NAV_HWW_DOUBLESHOTGUN]	= 3;
	if (!combat_hasweapon(&needs->bot->combat, HW_WEAPON_MACHINEGUN		)) needs->wish[NAV_HWW_MACHINEGUN]		= 5;
	if (!combat_hasweapon(&needs->bot->combat, HW_WEAPON_ROCKETLAUNCHER	)) needs->wish[NAV_HWW_ROCKETLAUNCHER]	= 5;
	if (!combat_hasweapon(&needs->bot->combat, HW_WEAPON_RAILGUN		)) needs->wish[NAV_HWW_RAILGUN]		= 4;
	
	// ammo
	needs->wish[NAV_HWA_DOUBLESHOTGUN] = 0.4;
	needs->wish[NAV_HWA_MACHINEGUN] = 0.8;
	needs->wish[NAV_HWA_ROCKETLAUNCHER] = 0.4;
	needs->wish[NAV_HWA_RAILGUN] = 0.4;

	needs->maxwish = 0;
	for (i = 0; i < MAX_NAV_TYPES; i++) 
		if (mapgraph_itemavailable(i) && (needs->wish[i] > needs->maxwish))
			needs->maxwish = needs->wish[i];
	needs->weaponwish = 0;
	for (i = NAV_HWW_DOUBLESHOTGUN; i <= NAV_HWA_ROCKETLAUNCHER; i++) 
		if (mapgraph_itemavailable(i)) needs->weaponwish += needs->wish[i];
}

static void
needs_dmcwishlist(NEEDS *needs)
{
	float need;
	int i;

	needs->wish[NAV_DMCI_HEALTH_NORM]  = needs_wishforhealth(needs);
	needs->wish[NAV_DMCI_HEALTH_SMALL] = needs->wish[NAV_DMCI_HEALTH_NORM] * 0.6;
	needs->wish[NAV_DMCI_HEALTH_LARGE] = 3 + needs_wishforhealth(needs);
		need = (100 - needs->bot->ent->v.armorvalue) / 40;
	if (need < 0)
		need = 0;
	needs->wish[NAV_DMCI_ARMOR1] = need;
		need = (150 - needs->bot->ent->v.armorvalue) / 40;
	if (need < 0)
		need = 0;
	needs->wish[NAV_DMCI_ARMOR2]	  = need;
	needs->wish[NAV_DMCI_ARMOR3]	  = 20;
		need = (200 - needs->bot->ent->v.armorvalue) / 40;
	if (need < 0)
		need = 0;
	needs->wish[NAV_DMCI_ARMOR_INV]  = need;
	needs->wish[NAV_DMCI_ENVIROSUIT]  = 20;
	needs->wish[NAV_DMCI_INVISIBILITY]  = 3;
	needs->wish[NAV_DMCI_INVULNERABILITY]  = 3;
	needs->wish[NAV_DMCI_SUPERDAMAGE]  = 3;

	needs->wish[NAV_S_CAMPING] = needs_wishforsniping(needs, true) - 0.5;

	if ( !( combat_hasweapon(&needs->bot->combat, DMC_WEAPON_NAILGUN )			|| 
			combat_hasweapon(&needs->bot->combat, DMC_WEAPON_SUPERNAILGUN )	|| 
			combat_hasweapon(&needs->bot->combat, DMC_WEAPON_ROCKETLAUNCHER )	|| 
			combat_hasweapon(&needs->bot->combat, DMC_WEAPON_LIGHTNING )			))
	{	// no big gun at hand...
		needs->wish[DMC_WEAPON_SUPERNAILGUN]	= 9;
		needs->wish[DMC_WEAPON_ROCKETLAUNCHER]	= 9;
		needs->wish[DMC_WEAPON_LIGHTNING]		= 9;
	}
	else {		
		if (!combat_hasweapon(&needs->bot->combat, DMC_WEAPON_SUPERNAILGUN	)) needs->wish[NAV_DMCW_SUPERNAILGUN]	= 3.5;
		if (!combat_hasweapon(&needs->bot->combat, DMC_WEAPON_ROCKETLAUNCHER)) needs->wish[NAV_DMCW_ROCKETLAUNCHER]= 5;
		if (!combat_hasweapon(&needs->bot->combat, DMC_WEAPON_LIGHTNING		)) needs->wish[NAV_DMCW_LIGHTNING]		= 5;
	}
	if (!combat_hasweapon(&needs->bot->combat, DMC_WEAPON_NAILGUN		)) needs->wish[NAV_DMCW_NAILGUN]		= 2.5;
	if (!combat_hasweapon(&needs->bot->combat, DMC_WEAPON_SUPERSHOTGUN	)) needs->wish[NAV_DMCW_SUPERSHOTGUN]	= 2;
	if (!combat_hasweapon(&needs->bot->combat, DMC_WEAPON_GRENLAUNCHER	)) needs->wish[NAV_DMCW_GRENLAUNCHER]	= 3.5;

	// ammo
	needs->wish[NAV_DMCI_SHELLS] = 0.4;
	needs->wish[NAV_DMCI_SPIKES] = 0.4;
	needs->wish[NAV_DMCI_ROCKETS] = 0.8;
	needs->wish[NAV_DMCI_CELLS] = 0.8;

	needs->maxwish = 0;
	for (i=0; i<MAX_NAV_TYPES; i++) 
		if ( mapgraph_itemavailable(i) && (needs->wish[i] > needs->maxwish) ) needs->maxwish = needs->wish[i];
	needs->weaponwish = 0;
	for (i=NAV_DMCW_QUAKEGUN; i<=NAV_DMCW_LIGHTNING; i++) 
		if ( mapgraph_itemavailable(i) ) needs->weaponwish += needs->wish[i];
}

static void
needs_gearboxwishlist(NEEDS *needs)
{
	int i;

	needs->wish[NAV_I_HEALTHKIT]     = needs_wishforhealth(needs);
	needs->wish[NAV_F_HEALTHCHARGER] = needs_wishforhealth(needs);
	if (needs->bot->senses.numEnemies > 0) needs->wish[NAV_F_HEALTHCHARGER] = 0;
	needs->wish[NAV_I_BATTERY]		  = needs_wishforarmor(needs);
	needs->wish[NAV_F_RECHARGE]	  = needs_wishforarmor(needs);
	if (needs->bot->senses.numEnemies > 0) needs->wish[NAV_F_RECHARGE] = 0;

	needs->wish[NAV_S_CAMPING] = needs_wishforsniping(needs, true) - 0.5;	
	needs->wish[NAV_F_TANKCONTROLS] = needs_wishforsniping(needs, false) - 0.5;	
	
	if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_TRIPMINE )) needs->wish[NAV_S_USE_TRIPMINE] = 2;
	
	if (!needs->bot->hasLongJump()) needs->wish[NAV_I_LONGJUMP] = 5;
	
	if ( !( combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_MP5 )		|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_SHOTGUN )	|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_GAUSS )		|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_EGON ) 		|| 
			combat_hasweapon(&needs->bot->combat, GEARBOX_WEAPON_M249 )	|| 
			combat_hasweapon(&needs->bot->combat, GEARBOX_WEAPON_EAGLE )	|| 
			combat_hasweapon(&needs->bot->combat, GEARBOX_WEAPON_SHOCKRIFLE ) ))
	{	// no big gun at hand...
		needs->wish[NAV_W_MP5]		= 9;
		needs->wish[NAV_W_SHOTGUN] = 9;
		needs->wish[NAV_W_GAUSS]	= 9;
		needs->wish[NAV_W_EGON]	= 9;
		needs->wish[NAV_OFW_M249]	= 9;
		needs->wish[NAV_OFW_EAGLE]	= 9;
		needs->wish[NAV_OFW_SHOCKRIFLE]= 9;
	}
	else
	{	// have one but want more :-)
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_EGON		)) needs->wish[NAV_W_EGON]			= 5;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_GAUSS		)) needs->wish[NAV_W_GAUSS]		= 5;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_MP5			)) needs->wish[NAV_W_MP5]			= 4;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_SHOTGUN		)) needs->wish[NAV_W_SHOTGUN]		= 3;
		if (!combat_hasweapon(&needs->bot->combat, GEARBOX_WEAPON_M249		)) needs->wish[NAV_OFW_M249]		= 4;
		if (!combat_hasweapon(&needs->bot->combat, GEARBOX_WEAPON_EAGLE		)) needs->wish[NAV_OFW_EAGLE]		= 3;
		if (!combat_hasweapon(&needs->bot->combat, GEARBOX_WEAPON_SHOCKRIFLE)) needs->wish[NAV_OFW_SHOCKRIFLE]	= 4;
	}
	// rest of armatory...
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_CROSSBOW		)) needs->wish[NAV_W_CROSSBOW]		= 3;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_HORNETGUN		)) needs->wish[NAV_W_HORNETGUN]	= 1.5;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_PYTHON			)) needs->wish[NAV_W_PYTHON]		= 2;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_RPG				)) needs->wish[NAV_W_RPG]			= 4.5;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_TRIPMINE		)) needs->wish[NAV_W_TRIPMINE]		= 2;
	if (!combat_hasweapon(&needs->bot->combat, GEARBOX_WEAPON_GRAPPLE		)) needs->wish[NAV_OFW_GRAPPLE]	= 4;
	if (!combat_hasweapon(&needs->bot->combat, GEARBOX_WEAPON_SPORELAUNCHER	)) needs->wish[NAV_OFW_SPORELAUNCHER]= 3;
	if (!combat_hasweapon(&needs->bot->combat, GEARBOX_WEAPON_SNIPERRIFLE	)) needs->wish[NAV_OFW_SNIPERRIFLE]= 4;
	if (!combat_hasweapon(&needs->bot->combat, GEARBOX_WEAPON_KNIFE			)) needs->wish[NAV_OFW_KNIFE]		= 0.5;
	
	// copy identical ids
	needs->wish[NAV_W_9MMAR]	= needs->wish[NAV_W_MP5];
	needs->wish[NAV_W_357]		= needs->wish[NAV_W_PYTHON];

	// grenades
	needs->wish[NAV_W_HANDGRENADE] = 1;
	needs->wish[NAV_W_SATCHEL] = 1.5;
	needs->wish[NAV_W_SNARK] = 2;
	needs->wish[NAV_OFW_PENGUIN] = 2;

	// ammo
	needs->wish[NAV_A_MP5GRENADES] = 1.5;
	needs->wish[NAV_A_MP5CLIP] = 0.8;
	needs->wish[NAV_A_EGONCLIP] = 0.8;
	needs->wish[NAV_A_RPGCLIP] = 0.8;
	needs->wish[NAV_A_RPG_ROCKET] = 0.4;
	needs->wish[NAV_A_CROSSBOW_BOLT] = 0.4;
	needs->wish[NAV_A_BUCKSHOT] = 0.4;	
	needs->wish[NAV_A_357] = 0.4;
	needs->wish[NAV_OFA_556] = 0.8;
	needs->wish[NAV_OFA_762] = 0.8;
	needs->wish[NAV_OFA_EAGLECLIP] = 0.4;
	needs->wish[NAV_OFA_SPORE] = 0.4;

	// copy identical ids
	needs->wish[NAV_A_ARGRENADES]	= needs->wish[NAV_A_MP5GRENADES];
	needs->wish[NAV_A_9MMCLIP]		= needs->wish[NAV_A_MP5CLIP];
	needs->wish[NAV_A_9MMAR]		= needs->wish[NAV_A_MP5CLIP];
	needs->wish[NAV_A_GAUSSCLIP]	= needs->wish[NAV_A_EGONCLIP];

	needs->maxwish = 0;
	for (i=0; i<MAX_NAV_TYPES; i++) 
		if ( mapgraph_itemavailable(i) && (needs->wish[i] > needs->maxwish) )
			needs->maxwish = needs->wish[i];
	needs->weaponwish = 0;
	for (i = NAV_W_CROSSBOW; i <= NAV_A_GLOCKCLIP; i++) 
		if ( mapgraph_itemavailable(i) ) needs->weaponwish += needs->wish[i];
	for (i=NAV_OFW_GRAPPLE; i<=NAV_OFA_SPORE; i++) 
		if ( mapgraph_itemavailable(i) ) needs->weaponwish += needs->wish[i];

}

static void
needs_hungerwishlist(NEEDS *needs)
{
	int i;

	needs->wish[NAV_I_HEALTHKIT]     = needs_wishforhealth(needs);
	needs->wish[NAV_F_HEALTHCHARGER] = needs_wishforhealth(needs);
	if (needs->bot->senses.numEnemies > 0) needs->wish[NAV_F_HEALTHCHARGER] = 0;

	needs->wish[NAV_S_CAMPING] = needs_wishforsniping(needs, true)-0.5;
	needs->wish[NAV_F_TANKCONTROLS] = needs_wishforsniping(needs, false)-0.5;

	if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_TRIPMINE )) needs->wish[NAV_S_USE_TRIPMINE] = 2;

	if (!needs->bot->hasLongJump()) needs->wish[NAV_I_LONGJUMP] = 5;
	
	if ( !( combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_MP5 )		|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_SHOTGUN )	|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_GAUSS )	|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_EGON )	||
			combat_hasweapon(&needs->bot->combat, HUNGER_WEAPON_CHAINGUN )	||
			combat_hasweapon(&needs->bot->combat, HUNGER_WEAPON_AP9 ) 			))
	{	// no big gun at hand...
		needs->wish[NAV_W_MP5]		= 9;
		needs->wish[NAV_W_SHOTGUN] = 9;
		needs->wish[NAV_W_GAUSS]	= 9;
		needs->wish[NAV_W_EGON]	= 9;
		needs->wish[NAV_THW_CHAINGUN]	= 9;
		needs->wish[NAV_THW_AP9]	= 9;
	}
	else
	{	// have one but want more :-)
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_EGON		)) needs->wish[NAV_W_EGON]			= 5;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_GAUSS		)) needs->wish[NAV_W_GAUSS]		= 5;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_MP5			)) needs->wish[NAV_W_MP5]			= 4;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_SHOTGUN		)) needs->wish[NAV_W_SHOTGUN]		= 3;
		if (!combat_hasweapon(&needs->bot->combat, HUNGER_WEAPON_CHAINGUN ))  needs->wish[NAV_THW_CHAINGUN]  = 5;
		if (!combat_hasweapon(&needs->bot->combat, HUNGER_WEAPON_AP9 )) needs->wish[NAV_THW_AP9] = 4;
	}
	// rest of armatory...
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_CROSSBOW	)) needs->wish[NAV_W_CROSSBOW]		= 3;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_PYTHON		)) needs->wish[NAV_W_PYTHON]		= 2;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_RPG			)) needs->wish[NAV_W_RPG]			= 4.5;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_TRIPMINE	)) needs->wish[NAV_W_TRIPMINE]		= 2;
	if (!combat_hasweapon(&needs->bot->combat, HUNGER_WEAPON_SNIPER       )) needs->wish[NAV_THW_SNIPER]         = 3;
	if (!combat_hasweapon(&needs->bot->combat, HUNGER_WEAPON_TFCSNIPER       )) needs->wish[NAV_THW_EINAR1]         = 3;
	if (!combat_hasweapon(&needs->bot->combat, HUNGER_WEAPON_TAURUS       )) needs->wish[NAV_THA_TAURUS] = 2;

	// copy identical ids
	needs->wish[NAV_W_9MMAR]	= needs->wish[NAV_W_MP5];
	needs->wish[NAV_W_357]		= needs->wish[NAV_W_PYTHON];

	// grenades
	needs->wish[NAV_W_HANDGRENADE] = 1;
	needs->wish[NAV_W_SATCHEL] = 1.5;
	needs->wish[NAV_W_SNARK] = 2;

	// ammo
	needs->wish[NAV_A_MP5GRENADES] = 1.5;
	needs->wish[NAV_A_MP5CLIP] = 0.8;
	needs->wish[NAV_A_EGONCLIP] = 0.8;
	needs->wish[NAV_A_RPGCLIP] = 0.8;
	needs->wish[NAV_A_RPG_ROCKET] = 0.4;
	needs->wish[NAV_A_CROSSBOW_BOLT] = 0.4;
	needs->wish[NAV_A_BUCKSHOT] = 0.4;	
	needs->wish[NAV_A_357] = 0.4;
	needs->wish[NAV_THA_SNIPER] = 0.4;
	needs->wish[NAV_THA_TAURUS] = 0.4;
	needs->wish[NAV_THA_AP9] = 0.4;

	// copy identical ids
	needs->wish[NAV_A_ARGRENADES]	= needs->wish[NAV_A_MP5GRENADES];
	needs->wish[NAV_A_9MMCLIP]		= needs->wish[NAV_A_MP5CLIP];
	needs->wish[NAV_A_9MMAR]		= needs->wish[NAV_A_MP5CLIP];
	needs->wish[NAV_A_GAUSSCLIP]	= needs->wish[NAV_A_EGONCLIP];

	needs->maxwish = 0;
	for (i=0; i<MAX_NAV_TYPES; i++) 
		if ( mapgraph_itemavailable(i) && (needs->wish[i]>needs->maxwish) )
			needs->maxwish = needs->wish[i];
	needs->weaponwish = 0;
	for (i=NAV_W_CROSSBOW; i<=NAV_A_GLOCKCLIP; i++) 
		if ( mapgraph_itemavailable(i) ) needs->weaponwish += needs->wish[i];
	for (i=NAV_THW_AP9; i<=NAV_THA_TAURUS; i++)
                if ( mapgraph_itemavailable(i) ) needs->weaponwish += needs->wish[i];
}

static void
needs_agwishlist(NEEDS *needs)
{
	EDICT *pent = NULL;
	int i;

	if (headToBunker) {
		needs->wish[NAV_S_AIRSTRIKE_COVER] = 20;
		needs->maxwish = needs->wish[NAV_S_AIRSTRIKE_COVER];
		if (!needs->airstrikeknown) {
			needs->newitempriorities = true;
			needs->airstrikeknown = true;
		}
		if (mapgraph_getnearestnavpoint(&zerovector, NAV_S_AIRSTRIKE_COVER ))
			return;	// only head for bunker if cover exists!
	}
	else if (needs->airstrikeknown) {
		needs->newitempriorities = true;
		needs->airstrikeknown = false;
	}

	if (worldtime() > nextAirstrikeTime) needs->wish[NAV_S_AIRSTRIKE_BUTTON] = 2;

	needs->wish[NAV_I_HEALTHKIT]     = needs_wishforhealth(needs);
	needs->wish[NAV_F_HEALTHCHARGER] = needs_wishforhealth(needs);
	if (needs->bot->senses.numEnemies > 0) needs->wish[NAV_F_HEALTHCHARGER] = 0;
	needs->wish[NAV_I_BATTERY]		  = needs_wishforarmor(needs);
	needs->wish[NAV_F_RECHARGE]	  = needs_wishforarmor(needs);
	if (needs->bot->senses.numEnemies > 0) needs->wish[NAV_F_RECHARGE] = 0;

	needs->wish[NAV_S_CAMPING] = needs_wishforsniping(needs, true)-0.5;	
	needs->wish[NAV_F_TANKCONTROLS] = needs_wishforsniping(needs, false)-0.5;	
	
	if (combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_TRIPMINE )) needs->wish[NAV_S_USE_TRIPMINE] = 2;

	if (!needs->bot->hasLongJump()) needs->wish[NAV_I_LONGJUMP] = 5;

	if ( !( combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_MP5 )		|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_SHOTGUN )	|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_GAUSS )		|| 
			combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_EGON )			))
	{	// no big gun at hand...
		needs->wish[NAV_W_MP5]		= 9;
		needs->wish[NAV_W_SHOTGUN] = 9;
		needs->wish[NAV_W_GAUSS]	= 9;
		needs->wish[NAV_W_EGON]	= 9;
	}
	else
	{	// have one but want more :-)
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_EGON		)) needs->wish[NAV_W_EGON]			= 5;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_GAUSS		)) needs->wish[NAV_W_GAUSS]		= 5;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_MP5			)) needs->wish[NAV_W_MP5]			= 4;
		if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_SHOTGUN		)) needs->wish[NAV_W_SHOTGUN]		= 3;
	}
	// rest of armatory...
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_CROSSBOW	)) needs->wish[NAV_W_CROSSBOW]		= 3;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_HORNETGUN	)) needs->wish[NAV_W_HORNETGUN]	= 1.5;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_PYTHON		)) needs->wish[NAV_W_PYTHON]		= 2;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_RPG			)) needs->wish[NAV_W_RPG]			= 4.5;
	if (!combat_hasweapon(&needs->bot->combat, VALVE_WEAPON_TRIPMINE	)) needs->wish[NAV_W_TRIPMINE]		= 2;

	// copy identical ids
	needs->wish[NAV_W_9MMAR]	= needs->wish[NAV_W_MP5];
	needs->wish[NAV_W_357]		= needs->wish[NAV_W_PYTHON];

	// grenades
	needs->wish[NAV_W_HANDGRENADE] = 1;
	needs->wish[NAV_W_SATCHEL] = 1.5;
	needs->wish[NAV_W_SNARK] = 2;

	// ammo
	needs->wish[NAV_A_MP5GRENADES] = 1.5;
	needs->wish[NAV_A_MP5CLIP] = 0.8;
	needs->wish[NAV_A_EGONCLIP] = 0.8;
	needs->wish[NAV_A_RPGCLIP] = 0.8;
	needs->wish[NAV_A_RPG_ROCKET] = 0.4;
	needs->wish[NAV_A_CROSSBOW_BOLT] = 0.4;
	needs->wish[NAV_A_BUCKSHOT] = 0.4;	
	needs->wish[NAV_A_357] = 0.4;

	// copy identical ids
	needs->wish[NAV_A_ARGRENADES]	= needs->wish[NAV_A_MP5GRENADES];
	needs->wish[NAV_A_9MMCLIP]		= needs->wish[NAV_A_MP5CLIP];
	needs->wish[NAV_A_9MMAR]		= needs->wish[NAV_A_MP5CLIP];
	needs->wish[NAV_A_GAUSSCLIP]	= needs->wish[NAV_A_EGONCLIP];

	// DOM
	if(com.gamedll_flags & GAMEDLL_DOM)
		needs->wish[NAV_AGI_DOM_CONTROLPOINT] = 20;

	// CTF
	if(com.gamedll_flags & GAMEDLL_CTF)
	{
		if( getteam( needs->bot->ent ) )
		{
			while((pent = find_entitybyclassname( pent, "carried_flag_team1" ) ) )
			{
				if( pent->v.owner == needs->bot->ent )
				{
					needs->wish[NAV_AGI_FLAG_TEAM2] = 20;
				}
				else if( needs->bot->aggression < 6 )
				{
					needs->wish[NAV_AGI_FLAG_TEAM1] = 2;
					needs->wish[NAV_AGI_FLAG_TEAM2] = 20;
				}
				else
				{
					needs->wish[NAV_AGI_FLAG_TEAM1] = 20;
					needs->wish[NAV_AGI_FLAG_TEAM2] = 2;
				}
			}
		}
		else
		{
			while((pent = find_entitybyclassname( pent, "carried_flag_team2" ) ) )
			{
				if( pent->v.owner == needs->bot->ent ) 
				{
					needs->wish[NAV_AGI_FLAG_TEAM1] = 20;
				}
				else if( needs->bot->aggression < 6 )
				{
					needs->wish[NAV_AGI_FLAG_TEAM2] = 2;
					needs->wish[NAV_AGI_FLAG_TEAM1] = 20;
				}
				else
				{
					needs->wish[NAV_AGI_FLAG_TEAM2] = 20;
					needs->wish[NAV_AGI_FLAG_TEAM1] = 2;
				}
			}
		}
	}

	needs->maxwish = 0;
	for (i=0; i<MAX_NAV_TYPES; i++) 
		if ( mapgraph_itemavailable(i) && (needs->wish[i] > needs->maxwish) )
			needs->maxwish = needs->wish[i];
	needs->weaponwish = 0;
	for (i=NAV_W_CROSSBOW; i<=NAV_A_GLOCKCLIP; i++) 
		if ( mapgraph_itemavailable(i) ) needs->weaponwish += needs->wish[i];
}

void
needs_getwishlist(NEEDS *needs)
{
	switch (mod_id) {
		case AG_DLL:		needs_agwishlist(needs);		break;
		case HUNGER_DLL:	needs_hungerwishlist(needs);	break;
		case VALVE_DLL:		needs_valvewishlist(needs);	break;
		case HOLYWARS_DLL:	needs_hwwishlist(needs);		break;
		case DMC_DLL:		needs_dmcwishlist(needs);		break;
		case GEARBOX_DLL:	needs_gearboxwishlist(needs);	break;
	}
}

void
needs_updatewishlist(NEEDS *needs)
{
	if (worldtime() > needs->wishupdate) {
		memset( &needs->wish, 0, sizeof needs->wish );
		needs_getwishlist(needs);
		needs->wishupdate = worldtime() + 1.0;
	}
}
