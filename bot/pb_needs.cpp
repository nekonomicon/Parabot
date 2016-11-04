#include "pb_needs.h"
#include "pb_global.h"
#include "parabot.h"
#include "pb_mapgraph.h"


extern int mod_id;
extern PB_MapGraph mapGraph;
extern bool haloOnBase;
extern bool headToBunker;
extern float nextAirstrikeTime;




void PB_Needs::init( CParabot *botClass )
{
	airstrikeKnown = false;
	bot = botClass;
	haloKnownOnBase = false;
	maxWish = 0;
	newItemPriorities = false;
	weaponWish = 0;
	for (int i=0; i<MAX_NAV_TYPES; i++) wish[i] = 0;
	wishUpdate = -100;
}


float PB_Needs::needForHealth() 
// returns a value between 0 and 10 indicating the need for health
{ 
	#define MIN_HEALTH 20.0	// need=10 if health is below this value
	
	float need = (100.0 - bot->ent->v.health) * (10.0/(100.0-MIN_HEALTH));
	if (need < 0) need = 0;
	else if (need > 10) need = 10;
	//debugMsg( "VHealth=%.1f   ", ent->v.health );
	//debugMsg( "Health=%.1f\n", need );
	return need;
};



float PB_Needs::needForArmor() 
// returns a value between 0 and 10 indicating the need for armor
{ 
	#define	MAX_ARMOR_WISH 4.0	// need for armor when armorvalue==0

	float need = (100.0 - bot->ent->v.armorvalue) * (0.01*MAX_ARMOR_WISH);
	if (need < 0) need = 0;
	//debugMsg( "Armor=%.1f\n", need );
	return need;
};


float PB_Needs::wishForCombat()
// returns a value between 0 and 10 indicating the wish for enemy encounter
{
	// health
	float health = (bot->ent->v.health-10) / (70-10);
	if (health<0) health = 0;
	else if (health>1.2) health = 1.2;
	// armor
	float armor = (bot->ent->v.armorvalue / 100) + 0.9;
	if (armor>1.4) armor = 1.4;
	// weapon
	float weapon = 0.1;

	switch (mod_id) {
	case VALVE_DLL:
			 if ( bot->combat.hasWeapon( VALVE_WEAPON_EGON      ) || 
				  bot->combat.hasWeapon( VALVE_WEAPON_GAUSS     )    )  weapon = 1;
		else if ( bot->combat.hasWeapon( VALVE_WEAPON_MP5       ) || 
				  bot->combat.hasWeapon( VALVE_WEAPON_RPG       )    )  weapon = 0.8;
		else if ( bot->combat.hasWeapon( VALVE_WEAPON_SHOTGUN   )    )  weapon = 0.6;
		else if ( bot->combat.hasWeapon( VALVE_WEAPON_HORNETGUN ) || 
				  bot->combat.hasWeapon( VALVE_WEAPON_PYTHON    ) ||
				  bot->combat.hasWeapon( VALVE_WEAPON_CROSSBOW  )    )  weapon = 0.4;
		break;
	case HOLYWARS_DLL:
			 if ( bot->combat.hasWeapon( HW_WEAPON_MACHINEGUN     ) || 
				  bot->combat.hasWeapon( HW_WEAPON_ROCKETLAUNCHER )    )  weapon = 1;
		else if ( bot->combat.hasWeapon( HW_WEAPON_RAILGUN        )    )  weapon = 0.8;
		else if ( bot->combat.hasWeapon( HW_WEAPON_DOUBLESHOTGUN  )    )  weapon = 0.7;
		break;
	case DMC_DLL:
			 if ( bot->combat.hasWeapon( DMC_WEAPON_LIGHTNING	  ) || 
				  bot->combat.hasWeapon( DMC_WEAPON_SUPERNAILGUN   )    )  weapon = 1;
		else if ( bot->combat.hasWeapon( DMC_WEAPON_NAILGUN        ) || 
				  bot->combat.hasWeapon( DMC_WEAPON_ROCKETLAUNCHER )    )  weapon = 0.8;
		else if ( bot->combat.hasWeapon( DMC_WEAPON_SUPERSHOTGUN   ) || 
				  bot->combat.hasWeapon( DMC_WEAPON_GRENLAUNCHER   )    )  weapon = 0.6;
		break;
	case GEARBOX_DLL:
		weapon = 0.8;
		break;
	}
	// wish
	float wish = health*armor*weapon*10 + 0.5*bot->aggression;
	if (wish > 10.0) wish = 10;
	return wish;
}


float PB_Needs::wishForSniping( bool weaponCheck )
// returns a value between 0 and 10 indicating the wish for sniping
{
	// health
	float health = (bot->ent->v.health-10) / (80-10);
	if (health<0) health = 0;
	else if (health>1) health = 1;
	// weapon
	float weapon = 0;
	if (weaponCheck) {
		switch (mod_id) {
			case VALVE_DLL:		if ( bot->combat.hasWeapon( VALVE_WEAPON_CROSSBOW ) ) weapon = 1;
								if ( bot->combat.hasWeapon( VALVE_WEAPON_PYTHON   ) ) weapon = 0.5; 
								break;
			case HOLYWARS_DLL:	if ( bot->combat.hasWeapon( HW_WEAPON_RAILGUN ) ) weapon = 1;
								break;
			case DMC_DLL:		if ( bot->combat.hasWeapon( DMC_WEAPON_LIGHTNING ) ) weapon = 1;
								break;
			case GEARBOX_DLL:	if ( bot->combat.hasWeapon( VALVE_WEAPON_CROSSBOW ) ) weapon = 1;
								break;
		}
	}
	else weapon = 1.5;	// turrets
			  
	// wish
	float wish = health*weapon*5 + (5-bot->aggression);
	
	const float outTime = 40;// time after which camping gives 0 points
	float x = outTime + (worldTime() - bot->lastCamp) - bot->campTime;
	// while camping x=outTime-campTime, else x increasing
	if (x>outTime) {
		if (bot->aggression < 2.5) x = outTime + (x-outTime)/(10*bot->aggression);
		else				  x = outTime;
	}
	float timeFactor = x/outTime;
//	if (timeFactor > 0) timeFactor = 1;	// not linear!

	return (timeFactor*wish);
}



void PB_Needs::valveWishList()
{

	for (int i=0; i<MAX_NAV_TYPES; i++) wish[i] = 0;

	if (headToBunker) {
		wish[NAV_S_AIRSTRIKE_COVER] = 20;
		maxWish = wish[NAV_S_AIRSTRIKE_COVER];
		if (!airstrikeKnown) {
			newItemPriorities = true;
			airstrikeKnown = true;
		}
		if (mapGraph.getNearestNavpoint( Vector(0,0,0), NAV_S_AIRSTRIKE_COVER ))
			return;	// only head for bunker if cover exists!
	}
	else if (airstrikeKnown) {
		newItemPriorities = true;
		airstrikeKnown = false;
	}

	if (worldTime() > nextAirstrikeTime) wish[NAV_S_AIRSTRIKE_BUTTON] = 2;

	wish[NAV_I_HEALTHKIT]     = needForHealth();
	wish[NAV_F_HEALTHCHARGER] = needForHealth();
	if (bot->senses.numEnemies > 0) wish[NAV_F_HEALTHCHARGER] = 0;
	wish[NAV_I_BATTERY]		  = needForArmor();
	wish[NAV_F_RECHARGE]	  = needForArmor();
	if (bot->senses.numEnemies > 0) wish[NAV_F_RECHARGE] = 0;

	wish[NAV_S_CAMPING] = wishForSniping()-0.5;	
	wish[NAV_F_TANKCONTROLS] = wishForSniping(false)-0.5;	
	
	if (bot->combat.hasWeapon( VALVE_WEAPON_TRIPMINE )) wish[NAV_S_USE_TRIPMINE] = 2;
	
	if (!bot->hasLongJump()) wish[NAV_I_LONGJUMP] = 5;
	
	if ( !( bot->combat.hasWeapon( VALVE_WEAPON_MP5 )		|| 
			bot->combat.hasWeapon( VALVE_WEAPON_SHOTGUN )	|| 
			bot->combat.hasWeapon( VALVE_WEAPON_GAUSS )		|| 
			bot->combat.hasWeapon( VALVE_WEAPON_EGON )			))
	{	// no big gun at hand...
		wish[NAV_W_MP5]		= 9;
		wish[NAV_W_SHOTGUN] = 9;
		wish[NAV_W_GAUSS]	= 9;
		wish[NAV_W_EGON]	= 9;
	}
	else
	{	// have one but want more :-)
		if (!bot->combat.hasWeapon( VALVE_WEAPON_EGON		)) wish[NAV_W_EGON]			= 5;
		if (!bot->combat.hasWeapon( VALVE_WEAPON_GAUSS		)) wish[NAV_W_GAUSS]		= 5;
		if (!bot->combat.hasWeapon( VALVE_WEAPON_MP5			)) wish[NAV_W_MP5]			= 4;
		if (!bot->combat.hasWeapon( VALVE_WEAPON_SHOTGUN		)) wish[NAV_W_SHOTGUN]		= 3;
	}
	// rest of armatory...
	if (!bot->combat.hasWeapon( VALVE_WEAPON_CROSSBOW	)) wish[NAV_W_CROSSBOW]		= 3;
	if (!bot->combat.hasWeapon( VALVE_WEAPON_HORNETGUN	)) wish[NAV_W_HORNETGUN]	= 1.5;
	if (!bot->combat.hasWeapon( VALVE_WEAPON_PYTHON		)) wish[NAV_W_PYTHON]		= 2;
	if (!bot->combat.hasWeapon( VALVE_WEAPON_RPG			)) wish[NAV_W_RPG]			= 4.5;
	if (!bot->combat.hasWeapon( VALVE_WEAPON_TRIPMINE	)) wish[NAV_W_TRIPMINE]		= 2;

	// copy identical ids
	wish[NAV_W_9MMAR]	= wish[NAV_W_MP5];
	wish[NAV_W_357]		= wish[NAV_W_PYTHON];

	// grenades
	wish[NAV_W_HANDGRENADE] = 1;
	wish[NAV_W_SATCHEL] = 1.5;
	wish[NAV_W_SNARK] = 2;

	// ammo
	wish[NAV_A_MP5GRENADES] = 1.5;
	wish[NAV_A_MP5CLIP] = 0.8;
	wish[NAV_A_EGONCLIP] = 0.8;
	wish[NAV_A_RPGCLIP] = 0.8;
	wish[NAV_A_RPG_ROCKET] = 0.4;
	wish[NAV_A_CROSSBOW_BOLT] = 0.4;
	wish[NAV_A_BUCKSHOT] = 0.4;	
	wish[NAV_A_357] = 0.4;

	// copy identical ids
	wish[NAV_A_ARGRENADES]	= wish[NAV_A_MP5GRENADES];
	wish[NAV_A_9MMCLIP]		= wish[NAV_A_MP5CLIP];
	wish[NAV_A_9MMAR]		= wish[NAV_A_MP5CLIP];
	wish[NAV_A_GAUSSCLIP]	= wish[NAV_A_EGONCLIP];

	maxWish = 0;
	for (i=0; i<MAX_NAV_TYPES; i++) 
		if ( mapGraph.itemAvailable(i) && (wish[i]>maxWish) )
			maxWish = wish[i];
	weaponWish = 0;
	for (i=NAV_W_CROSSBOW; i<=NAV_A_GLOCKCLIP; i++) 
		if ( mapGraph.itemAvailable(i) ) weaponWish += wish[i];
}


void PB_Needs::hwWishList()
{

	for (int i=0; i<MAX_NAV_TYPES; i++) wish[i] = 0;

	if ( haloOnBase ) {
		wish[NAV_HW_HALOBASE] = 20;
		maxWish = wish[NAV_HW_HALOBASE];
		if (!haloKnownOnBase) {
			bot->senses.resetPlayerClassifications();
			newItemPriorities = true;
			haloKnownOnBase = true;
		}
		return;	// no other targets...
	}
	else if (haloKnownOnBase) {
		bot->senses.resetPlayerClassifications();
		newItemPriorities = true;
		haloKnownOnBase = false;
	}

	wish[NAV_I_HEALTHKIT]     = needForHealth();
	wish[NAV_F_HEALTHCHARGER] = needForHealth();
	if (bot->senses.numEnemies > 0) wish[NAV_F_HEALTHCHARGER] = 0;
	wish[NAV_I_BATTERY]		  = needForArmor();
	
	wish[NAV_S_CAMPING] = wishForSniping()-0.5;	

	if (!bot->combat.hasWeapon( HW_WEAPON_DOUBLESHOTGUN	)) wish[NAV_HWW_DOUBLESHOTGUN]	= 3;
	if (!bot->combat.hasWeapon( HW_WEAPON_MACHINEGUN		)) wish[NAV_HWW_MACHINEGUN]		= 5;
	if (!bot->combat.hasWeapon( HW_WEAPON_ROCKETLAUNCHER	)) wish[NAV_HWW_ROCKETLAUNCHER]	= 5;
	if (!bot->combat.hasWeapon( HW_WEAPON_RAILGUN		)) wish[NAV_HWW_RAILGUN]		= 4;
	
	// ammo
	wish[NAV_HWA_DOUBLESHOTGUN] = 0.4;
	wish[NAV_HWA_MACHINEGUN] = 0.8;
	wish[NAV_HWA_ROCKETLAUNCHER] = 0.4;
	wish[NAV_HWA_RAILGUN] = 0.4;

	maxWish = 0;
	for (i=0; i<MAX_NAV_TYPES; i++) 
		if ( mapGraph.itemAvailable(i) && (wish[i]>maxWish) ) maxWish = wish[i];
	weaponWish = 0;
	for (i=NAV_HWW_DOUBLESHOTGUN; i<=NAV_HWA_ROCKETLAUNCHER; i++) 
		if ( mapGraph.itemAvailable(i) ) weaponWish += wish[i];
}


void PB_Needs::dmcWishList()
{
	float need;
		
	for (int i=0; i<MAX_NAV_TYPES; i++) wish[i] = 0;

	wish[NAV_DMCI_HEALTH_NORM]  = needForHealth();
	wish[NAV_DMCI_HEALTH_SMALL] = wish[NAV_DMCI_HEALTH_NORM]*0.6;
	wish[NAV_DMCI_HEALTH_LARGE] = 3 + needForHealth();
		need = (100 - bot->ent->v.armorvalue)/40; if (need<0) need=0;
	wish[NAV_DMCI_ARMOR1] = need;
		need = (150 - bot->ent->v.armorvalue)/40; if (need<0) need=0;
	wish[NAV_DMCI_ARMOR2]	  = need;
	wish[NAV_DMCI_ARMOR3]	  = 20;
		need = (200 - bot->ent->v.armorvalue)/40; if (need<0) need=0;
	wish[NAV_DMCI_ARMOR_INV]  = need;
	wish[NAV_DMCI_ENVIROSUIT]  = 20;
	wish[NAV_DMCI_INVISIBILITY]  = 3;
	wish[NAV_DMCI_INVULNERABILITY]  = 3;
	wish[NAV_DMCI_SUPERDAMAGE]  = 3;
	
	wish[NAV_S_CAMPING] = wishForSniping()-0.5;	

	if ( !( bot->combat.hasWeapon( DMC_WEAPON_NAILGUN )			|| 
			bot->combat.hasWeapon( DMC_WEAPON_SUPERNAILGUN )	|| 
			bot->combat.hasWeapon( DMC_WEAPON_ROCKETLAUNCHER )	|| 
			bot->combat.hasWeapon( DMC_WEAPON_LIGHTNING )			))
	{	// no big gun at hand...
		wish[DMC_WEAPON_SUPERNAILGUN]	= 9;
		wish[DMC_WEAPON_ROCKETLAUNCHER]	= 9;
		wish[DMC_WEAPON_LIGHTNING]		= 9;
	}
	else {		
		if (!bot->combat.hasWeapon( DMC_WEAPON_SUPERNAILGUN	)) wish[NAV_DMCW_SUPERNAILGUN]	= 3.5;
		if (!bot->combat.hasWeapon( DMC_WEAPON_ROCKETLAUNCHER)) wish[NAV_DMCW_ROCKETLAUNCHER]= 5;
		if (!bot->combat.hasWeapon( DMC_WEAPON_LIGHTNING		)) wish[NAV_DMCW_LIGHTNING]		= 5;
	}
	if (!bot->combat.hasWeapon( DMC_WEAPON_NAILGUN		)) wish[NAV_DMCW_NAILGUN]		= 2.5;
	if (!bot->combat.hasWeapon( DMC_WEAPON_SUPERSHOTGUN	)) wish[NAV_DMCW_SUPERSHOTGUN]	= 2;
	if (!bot->combat.hasWeapon( DMC_WEAPON_GRENLAUNCHER	)) wish[NAV_DMCW_GRENLAUNCHER]	= 3.5;

	// ammo
	wish[NAV_DMCI_SHELLS] = 0.4;
	wish[NAV_DMCI_SPIKES] = 0.4;
	wish[NAV_DMCI_ROCKETS] = 0.8;
	wish[NAV_DMCI_CELLS] = 0.8;

	maxWish = 0;
	for (i=0; i<MAX_NAV_TYPES; i++) 
		if ( mapGraph.itemAvailable(i) && (wish[i]>maxWish) ) maxWish = wish[i];
	weaponWish = 0;
	for (i=NAV_DMCW_QUAKEGUN; i<=NAV_DMCW_LIGHTNING; i++) 
		if ( mapGraph.itemAvailable(i) ) weaponWish += wish[i];
}


void PB_Needs::gearboxWishList()
{
	for (int i=0; i<MAX_NAV_TYPES; i++) wish[i] = 0;

	wish[NAV_I_HEALTHKIT]     = needForHealth();
	wish[NAV_F_HEALTHCHARGER] = needForHealth();
	if (bot->senses.numEnemies > 0) wish[NAV_F_HEALTHCHARGER] = 0;
	wish[NAV_I_BATTERY]		  = needForArmor();
	wish[NAV_F_RECHARGE]	  = needForArmor();
	if (bot->senses.numEnemies > 0) wish[NAV_F_RECHARGE] = 0;

	wish[NAV_S_CAMPING] = wishForSniping()-0.5;	
	wish[NAV_F_TANKCONTROLS] = wishForSniping(false)-0.5;	
	
	if (bot->combat.hasWeapon( VALVE_WEAPON_TRIPMINE )) wish[NAV_S_USE_TRIPMINE] = 2;
	
	if (!bot->hasLongJump()) wish[NAV_I_LONGJUMP] = 5;
	
	if ( !( bot->combat.hasWeapon( VALVE_WEAPON_MP5 )		|| 
			bot->combat.hasWeapon( VALVE_WEAPON_SHOTGUN )	|| 
			bot->combat.hasWeapon( VALVE_WEAPON_GAUSS )		|| 
			bot->combat.hasWeapon( VALVE_WEAPON_EGON ) 		|| 
			bot->combat.hasWeapon( GEARBOX_WEAPON_M249 )	|| 
			bot->combat.hasWeapon( GEARBOX_WEAPON_EAGLE )	|| 
			bot->combat.hasWeapon( GEARBOX_WEAPON_SHOCKRIFLE ) ))
	{	// no big gun at hand...
		wish[NAV_W_MP5]		= 9;
		wish[NAV_W_SHOTGUN] = 9;
		wish[NAV_W_GAUSS]	= 9;
		wish[NAV_W_EGON]	= 9;
		wish[NAV_OFW_M249]	= 9;
		wish[NAV_OFW_EAGLE]	= 9;
		wish[NAV_OFW_SHOCKRIFLE]= 9;
	}
	else
	{	// have one but want more :-)
		if (!bot->combat.hasWeapon( VALVE_WEAPON_EGON		)) wish[NAV_W_EGON]			= 5;
		if (!bot->combat.hasWeapon( VALVE_WEAPON_GAUSS		)) wish[NAV_W_GAUSS]		= 5;
		if (!bot->combat.hasWeapon( VALVE_WEAPON_MP5			)) wish[NAV_W_MP5]			= 4;
		if (!bot->combat.hasWeapon( VALVE_WEAPON_SHOTGUN		)) wish[NAV_W_SHOTGUN]		= 3;
		if (!bot->combat.hasWeapon( GEARBOX_WEAPON_M249		)) wish[NAV_OFW_M249]		= 4;
		if (!bot->combat.hasWeapon( GEARBOX_WEAPON_EAGLE		)) wish[NAV_OFW_EAGLE]		= 3;
		if (!bot->combat.hasWeapon( GEARBOX_WEAPON_SHOCKRIFLE)) wish[NAV_OFW_SHOCKRIFLE]	= 4;
	}
	// rest of armatory...
	if (!bot->combat.hasWeapon( VALVE_WEAPON_CROSSBOW		)) wish[NAV_W_CROSSBOW]		= 3;
	if (!bot->combat.hasWeapon( VALVE_WEAPON_HORNETGUN		)) wish[NAV_W_HORNETGUN]	= 1.5;
	if (!bot->combat.hasWeapon( VALVE_WEAPON_PYTHON			)) wish[NAV_W_PYTHON]		= 2;
	if (!bot->combat.hasWeapon( VALVE_WEAPON_RPG				)) wish[NAV_W_RPG]			= 4.5;
	if (!bot->combat.hasWeapon( VALVE_WEAPON_TRIPMINE		)) wish[NAV_W_TRIPMINE]		= 2;
	if (!bot->combat.hasWeapon( GEARBOX_WEAPON_GRAPPLE		)) wish[NAV_OFW_GRAPPLE]	= 4;
	if (!bot->combat.hasWeapon( GEARBOX_WEAPON_SPORELAUNCHER	)) wish[NAV_OFW_SPORELAUNCHER]= 3;
	if (!bot->combat.hasWeapon( GEARBOX_WEAPON_SNIPERRIFLE	)) wish[NAV_OFW_SNIPERRIFLE]= 4;
	if (!bot->combat.hasWeapon( GEARBOX_WEAPON_KNIFE			)) wish[NAV_OFW_KNIFE]		= 0.5;
	
	// copy identical ids
	wish[NAV_W_9MMAR]	= wish[NAV_W_MP5];
	wish[NAV_W_357]		= wish[NAV_W_PYTHON];

	// grenades
	wish[NAV_W_HANDGRENADE] = 1;
	wish[NAV_W_SATCHEL] = 1.5;
	wish[NAV_W_SNARK] = 2;
	wish[NAV_OFW_PENGUIN] = 2;

	// ammo
	wish[NAV_A_MP5GRENADES] = 1.5;
	wish[NAV_A_MP5CLIP] = 0.8;
	wish[NAV_A_EGONCLIP] = 0.8;
	wish[NAV_A_RPGCLIP] = 0.8;
	wish[NAV_A_RPG_ROCKET] = 0.4;
	wish[NAV_A_CROSSBOW_BOLT] = 0.4;
	wish[NAV_A_BUCKSHOT] = 0.4;	
	wish[NAV_A_357] = 0.4;
	wish[NAV_OFA_556] = 0.8;
	wish[NAV_OFA_762] = 0.8;
	wish[NAV_OFA_EAGLECLIP] = 0.4;
	wish[NAV_OFA_SPORE] = 0.4;

	// copy identical ids
	wish[NAV_A_ARGRENADES]	= wish[NAV_A_MP5GRENADES];
	wish[NAV_A_9MMCLIP]		= wish[NAV_A_MP5CLIP];
	wish[NAV_A_9MMAR]		= wish[NAV_A_MP5CLIP];
	wish[NAV_A_GAUSSCLIP]	= wish[NAV_A_EGONCLIP];

	maxWish = 0;
	for (i=0; i<MAX_NAV_TYPES; i++) 
		if ( mapGraph.itemAvailable(i) && (wish[i]>maxWish) )
			maxWish = wish[i];
	weaponWish = 0;
	for (i=NAV_W_CROSSBOW; i<=NAV_A_GLOCKCLIP; i++) 
		if ( mapGraph.itemAvailable(i) ) weaponWish += wish[i];
	for (i=NAV_OFW_GRAPPLE; i<=NAV_OFA_SPORE; i++) 
		if ( mapGraph.itemAvailable(i) ) weaponWish += wish[i];

}


void PB_Needs::getWishList()
{
	switch (mod_id) {
		case VALVE_DLL:		valveWishList();	break;
		case HOLYWARS_DLL:	hwWishList();		break;
		case DMC_DLL:		dmcWishList();		break;
		case GEARBOX_DLL:	gearboxWishList();	break;
	}
}


void PB_Needs::updateWishList()
{
	if (worldTime() > wishUpdate) {
		getWishList();
		wishUpdate = worldTime() + 1.0;
	}
}
