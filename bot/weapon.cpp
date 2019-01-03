#include "parabot.h"
#include "weapon.h"
#include "pb_global.h"
#include "bot.h"
#include "bot_weapons.h"


extern bot_t bots[32];
extern int mod_id;
extern bot_weapon_t weapon_defs[MAX_WEAPONS];


///////////////////////////////////////////////////////////////////////////////////
//
//  WEAPON LISTS	( all orders must match pb_weapon.h! )
//
//        highAimProb:  norm=0.5  fast=0.4  expl=0.3  closeCombat=0.2  autoAim=0.1
//
///////////////////////////////////////////////////////////////////////////////////

// weapon volumes (from SDK)
#define WV_NONE		  0.0
#define	WV_SWING	128.0
#define WV_QUIET	200.0
#define WV_NORMAL	600.0
#define WV_LOUD	   1000.0
#define WV_FLASH	450.0
#define WV_LOAD		256.0

// weapon flashes (from SDK)
#define WF_NONE		  0.0
#define WF_DIM		128.0
#define WF_NORMAL	256.0
#define WF_BRIGHT	512.0


// Valve Weaponlist 
static tWeaponRec valveWeapon[MAX_VALVE_WEAPONS] = { 
	// name				bestDist	cone	highAimProb	secAmmo	volAttack1	volAttack2	visAttack1	visAttack2	fireDelay	shortName
	{ "",					 0,		0.1,	0.0,		false,	WV_NONE,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	""				},	//   doesn't exist!
	{ "weapon_crowbar",		10,		0.866,	0.0,		false,	WV_SWING,	WV_SWING,	WF_NONE,	WF_NONE,	0.0,	"crowbar"		},
	{ "weapon_9mmhandgun", 200,		0.052,	0.5,		false,	WV_NORMAL,	WV_NORMAL,	WF_NORMAL,	WF_NORMAL,	0.0,	"glock"			},	// = glock
	{ "weapon_357",		   600,		0.017,	0.5,		false,	WV_LOUD,	WV_NONE,	WF_NORMAL,	WF_NONE,	0.0,	"python"		},	// = python
	{ "weapon_9mmAR",	   200,		0.104,	0.4,		true,	WV_NORMAL,	WV_NORMAL,	WF_NORMAL,	WF_BRIGHT,	0.0,	"mp5"			},	// = mp5
	{ "",					 0,		0.1,	0.0,		false,	WV_NONE,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	""				},	//   doesn't exist!
	{ "weapon_crossbow",   600,		0.017,	0.5,		false,	WV_QUIET,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	"crossbow"		},
	{ "weapon_shotgun",		50,		0.173,	0.5,		false,	WV_LOUD,	WV_LOUD,	WF_NORMAL,	WF_NORMAL,	0.0,	"shotgun"		},
	{ "weapon_rpg",		   400,		0.173,	0.1,		false,	WV_LOUD,	WV_NONE,	WF_BRIGHT,	WF_NONE,	0.0,	"rocketlauncher"},
	{ "weapon_gauss",	   300,		0.017,	0.5,		false,	WV_FLASH,	WV_LOAD,	WF_NORMAL,	WF_NONE,	0.0,	"gauss"			},
	{ "weapon_egon",	   300,		0.017,	0.5,		false,	WV_FLASH,	WV_FLASH,	WF_BRIGHT,	WF_BRIGHT,	0.0,	"egon"			},
	{ "weapon_hornetgun",  200,		0.173,	0.1,		false,	WV_QUIET,	WV_NORMAL,	WF_NORMAL,	WF_NORMAL,	0.0,	"hornetgun"		},
	{ "weapon_handgrenade",400,		0.342,	0.3,		false,	WV_NONE,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	"handgrenade"	},
	{ "weapon_tripmine",	 0,		0.1,	0.0,		false,	WV_QUIET,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	"tripmine"		},
	{ "weapon_satchel",    400,		0.342,	0.3,		false,	WV_QUIET,	WV_QUIET,	WF_NONE,	WF_NONE,	0.0,	"satchel"		},
	{ "weapon_snark",	   400,		0.173,	0.1,		false,	WV_QUIET,	WV_QUIET,	WF_NONE,	WF_NONE,	0.0,	"snark"			} 
};


// Holywars Weaponlist
static tWeaponRec holywarsWeapon[MAX_HW_WEAPONS]/* = { 
	{ "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 }, { "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 },
	{ "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 }, { "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 },
	{ "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 }, { "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 },
	{ "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 }, { "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 },
	{ "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 }, { "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 },
	{ "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 }, { "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 },
	{ "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 }, { "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 },
	{ "",0,0.0,false,WV_NONE,WV_NONE,WF_NONE,WF_NONE,0.0 },
	// name					bestDist	cone	highAimProb	secAmmo	volAttack1	volAttack2	visAttack1	visAttack2	fireDelay	shortName
	{ "weapon_jackhammer",		10,		0.866,	0.2,		false,	WV_NORMAL,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	"jackhammer"	},
	{ "weapon_doubleshotgun",   50,		0.173,	0.5,		false,	WV_LOUD,	WV_NONE,	WF_NORMAL,	WF_NONE,	0.0,	"doubleshotgun" },	
	{ "weapon_machinegun",	   200,		0.342,	0.5,		false,	WV_NORMAL,	WV_NONE,	WF_NORMAL,	WF_NONE,	0.0,	"machinegun"	},		
	{ "weapon_rocketlauncher", 400,		0.342,	0.4,		false,	WV_LOUD,	WV_NONE,	WF_BRIGHT,	WF_NONE,	0.0,	"rocketlauncher"},
	{ "",						 0,		0.0,	0.0,		false,	WV_NONE,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	""				},	//   doesn't exist!
	{ "weapon_railgun",		   600,		0.017,	0.5,		false,	WV_FLASH,	WV_NONE,	WF_NORMAL,	WF_NONE,	0.0,	"railgun"		},
}*/;


// DMC Weaponlist
static tWeaponRec dmcWeapon[MAX_DMC_WEAPONS] = { 
	// name					bestDist	cone	highAimProb	secAmmo	volAttack1	volAttack2	visAttack1	visAttack2	fireDelay	shortName
	{ "weapon_axe",				10,		0.866,	0.2,		false,	WV_SWING,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	"crowbar"		},
	{ "weapon_shotgun",		   200,		0.173,	0.5,		false,	WV_LOUD,	WV_NONE,	WF_NORMAL,	WF_NONE,	0.0,	"shotgun"		},	
	{ "weapon_doubleshotgun",   50,		0.173,	0.5,		false,	WV_LOUD,	WV_NONE,	WF_NORMAL,	WF_NONE,	0.0,	"doubleshotgun" },	
	{ "weapon_nailgun",		   200,		0.104,	0.5,		false,	WV_NORMAL,	WV_NONE,	WF_NORMAL,	WF_NONE,	0.0,	"nailgun"		},	
	{ "weapon_supernailgun",   200,		0.104,	0.5,		false,	WV_NORMAL,	WV_NONE,	WF_NORMAL,	WF_NONE,	0.0,	"supernailgun"	},	
	{ "weapon_grenadelauncher",250,		0.342,	0.4,		false,	WV_LOUD,	WV_NONE,	WF_BRIGHT,	WF_NONE,	0.0,	"grenadelauncher" },
	{ "weapon_rocketlauncher", 250,		0.342,	0.4,		false,	WV_LOUD,	WV_NONE,	WF_BRIGHT,	WF_NONE,	0.0,	"rocketlauncher"},
	{ "weapon_lightning",	   400,		0.017,	0.5,		false,	WV_FLASH,	WV_NONE,	WF_BRIGHT,	WF_NONE,	0.0,	"lightning-gun" },
};


// TFC Weaponlist
static tWeaponRec tfcWeapon[MAX_TFC_WEAPONS];/* = { 
	{ "", 0, 0.0, false, 0.0 }, { "", 0, 0.0, false, 0.0 }, { "", 0, 0.0, false, 0.0 },
	// name					bestDist	highAimProb	secAmmo	fireDelay
	{ "tf_weapon_medikit",		 10,	0.3,		false,	0.0 },
	{ "tf_weapon_spanner",		 10,	0.3,		false,	0.0 },
	{ "tf_weapon_axe",			 10,	0.3,		false,	0.0 },
	{ "tf_weapon_sniperrifle",	600,	0.6,		false,	0.0 },
	{ "tf_weapon_autorifle",	300,	0.6,		false,	0.0 },
	{ "tf_weapon_shotgun",		 50,	0.6,		false,	0.0 },
	{ "tf_weapon_supershotgun",  50,	0.6,		false,	0.0 },
	{ "tf_weapon_ng",			300,	0.6,		false,	0.0 },
	{ "tf_weapon_superng",		300,	0.6,		false,	0.0 },
	{ "tf_weapon_gl",			300,	0.6,		false,	0.0 },
	{ "tf_weapon_flamethrower", 100,	0.5,		false,	0.0 },
	{ "tf_weapon_rpg",			400,	0.4,		false,	0.0 },
	{ "tf_weapon_ic",			300,	0.6,		false,	0.0 },
	{ "",						  0,	0.0,		false,	0.0 },
	{ "tf_weapon_ac",			300,	0.6,		false,	0.0 },
	{ "",						  0,	0.0,		false,	0.0 },
	{ "",						  0,	0.0,		false,	0.0 },
	{ "tf_weapon_tranq",		 10,	0.3,		false,	0.0 },
	{ "tf_weapon_railgun",		300,	0.6,		false,	0.0 },
	{ "tf_weapon_pl",			300,	0.6,		false,	0.0 },
	{ "tf_weapon_knife",		 10,	0.3,		false,	0.0 },
};*/


// Gearbox Weaponlist 
static tWeaponRec gearboxWeapon[MAX_GEARBOX_WEAPONS] = { 
	// name				bestDist	cone	highAimProb	secAmmo	volAttack1	volAttack2	visAttack1	visAttack2	fireDelay	shortName
	{ "",					 0,		0.0,	0.0,		false,	WV_NONE,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	""				},	//   doesn't exist!
	{ "weapon_crowbar",		10,		0.866,	0.2,		false,	WV_SWING,	WV_SWING,	WF_NONE,	WF_NONE,	0.0,	"crowbar"		},
	{ "weapon_9mmhandgun", 200,		0.052,	0.5,		false,	WV_NORMAL,	WV_NORMAL,	WF_NORMAL,	WF_NORMAL,	0.0,	"glock"			},	// = glock
	{ "weapon_357",		   600,		0.017,	0.5,		false,	WV_LOUD,	WV_NONE,	WF_BRIGHT,	WF_NONE,	0.0,	"python"		},	// = python
	{ "weapon_9mmAR",	   200,		0.104,	0.5,		true,	WV_NORMAL,	WV_NORMAL,	WF_NORMAL,	WF_BRIGHT,	0.0,	"mp5"			},	// = mp5
	{ "",					 0,		0.0,	0.0,		false,	WV_NONE,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	""				},	//   doesn't exist!
	{ "weapon_crossbow",   600,		0.017,	0.5,		false,	WV_QUIET,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	"crossbow"		},
	{ "weapon_shotgun",		50,		0.173,	0.5,		false,	WV_LOUD,	WV_LOUD,	WF_NORMAL,	WF_NORMAL,	0.0,	"shotgun"		},
	{ "weapon_rpg",		   400,		0.342,	0.4,		false,	WV_LOUD,	WV_NONE,	WF_BRIGHT,	WF_NONE,	0.0,	"rocketlauncher"},
	{ "weapon_gauss",	   300,		0.017,	0.5,		false,	WV_FLASH,	WV_LOAD,	WF_NORMAL,	WF_NONE,	0.0,	"gauss"			},
	{ "weapon_egon",	   300,		0.017,	0.5,		false,	WV_FLASH,	WV_FLASH,	WF_BRIGHT,	WF_BRIGHT,	0.0,	"egon"			},
	{ "weapon_hornetgun",  200,		0.342,	0.3,		false,	WV_QUIET,	WV_NORMAL,	WF_DIM,		WF_DIM,		0.0,	"hornetgun"		},
	{ "weapon_handgrenade",400,		0.342,	0.4,		false,	WV_NONE,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	"handgrenade"	},
	{ "weapon_tripmine",	 0,		0.0,	0.0,		false,	WV_QUIET,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	"tripmine"		},
	{ "weapon_satchel",    400,		0.342,	0.4,		false,	WV_QUIET,	WV_QUIET,	WF_NONE,	WF_NONE,	0.0,	"satchel"		},
	{ "weapon_snark",	   400,		0.342,	0.3,		false,	WV_QUIET,	WV_QUIET,	WF_NONE,	WF_NONE,	0.0,	"snark"			},
	{ "weapon_grapple",	   200,		0.342,	0.5,		false,	WV_QUIET,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	"grapple"		},
	{ "weapon_eagle",	   600,		0.017,	0.5,		false,	WV_LOUD,	WV_NONE,	WF_BRIGHT,	WF_NONE,	0.0,	"eagle"			},
	{ "weapon_pipewrench",	10,		0.866,	0.2,		false,	WV_SWING,	WV_SWING,	WF_NONE,	WF_NONE,	0.0,	"pipewrench"	},
	{ "weapon_m249",	   200,		0.104,	0.5,		false,	WV_LOUD,	WV_NONE,	WF_NORMAL,	WF_NONE,	0.0,	"m249"			},
	{ "weapon_displacer",	10,		0.342,	0.2,		false,	WV_QUIET,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	"displacer"		},
	{ "",					 0,		0.0,	0.0,		false,	WV_NONE,	WV_NONE,	WF_NONE,	WF_NONE,	0.0,	""				},	//   doesn't exist!
	{ "weapon_shockrifle", 300,		0.052,	0.5,		false,	WV_FLASH,	WV_NONE,	WF_BRIGHT,	WF_NONE,	0.0,	"shockrifle"	},
	{ "weapon_sporelauncher",250,	0.342,	0.4,		false,	WV_LOUD,	WV_NONE,	WF_BRIGHT,	WF_NONE,	0.0,	"sporelauncher" },
	{ "weapon_sniperrifle",600,		0.017,	0.5,		false,	WV_LOUD,	WV_NONE,	WF_DIM,		WF_NONE,	0.0,	"sniperrifle"	},
	{ "weapon_knife",		10,		0.866,	0.2,		false,	WV_SWING,	WV_SWING,	WF_NONE,	WF_NONE,	0.0,	"knife"			},
	{ "weapon_penguin",	   400,		0.342,	0.3,		false,	WV_QUIET,	WV_QUIET,	WF_NONE,	WF_NONE,	0.0,	"penguin"		}
};

// Hunger Weaponlist
static tWeaponRec hungerWeapon[MAX_HUNGER_WEAPONS] = {
        // name                         bestDist        cone    highAimProb     secAmmo volAttack1      volAttack2      visAttack1      visAttack2      fireDelay       shortName
        { "",                                    0,             0.1,    0.0,            false,  WV_NONE,        WV_NONE,        WF_NONE,        WF_NONE,        0.0,    "" },
        { "weapon_crowbar",             10,             0.866,  0.0,            false,  WV_SWING,       WV_SWING,       WF_NONE,        WF_NONE,        0.0,    "umbrella"               },
        { "weapon_9mmhandgun", 200,             0.052,  0.5,            false,  WV_NORMAL,      WV_NORMAL,      WF_NORMAL,      WF_NORMAL,      0.0,    "beretta"                 },      // = glock
        { "weapon_357",            600,         0.017,  0.5,            false,  WV_LOUD,        WV_NONE,        WF_NORMAL,      WF_NONE,        0.0,    "python"                },      // = python
        { "weapon_9mmAR",          200,         0.104,  0.4,            true,   WV_NORMAL,      WV_NORMAL,      WF_NORMAL,      WF_BRIGHT,      0.0,    "Thompson"                   },      // = mp5
	{ "",			0,             0.1,  0.0,            false,  WV_NONE,      WV_NONE,      WF_NONE,      WF_NONE,      0.0,    ""                 },
        { "weapon_crossbow",   600,             0.017,  0.5,            false,  WV_QUIET,       WV_NONE,        WF_NONE,        WF_NONE,        0.0,    "crossbow"              },
        { "weapon_shotgun",             50,             0.173,  0.5,            false,  WV_LOUD,        WV_LOUD,        WF_NORMAL,      WF_NORMAL,      0.0,    "shotgun"               },
        { "weapon_rpg",            400,         0.173,  0.1,            false,  WV_LOUD,        WV_NONE,        WF_BRIGHT,      WF_NONE,        0.0,    "rocketlauncher"	},
        { "weapon_gauss",          300,         0.017,  0.5,            false,  WV_FLASH,       WV_LOAD,        WF_NORMAL,      WF_NONE,        0.0,    "gauss"                 },
        { "weapon_egon",           200,         0.017,  0.5,            false,  WV_FLASH,       WV_FLASH,       WF_BRIGHT,      WF_BRIGHT,      0.0,    "flamethrower"                  },
        { "",  					0,             0.1,  0.0,            false,  WV_NONE,       WV_NONE,      WF_NONE,      WF_NONE,      0.0,    ""             },
        { "weapon_handgrenade",400,             0.342,  0.3,            false,  WV_NONE,        WV_NONE,        WF_NONE,        WF_NONE,        0.0,    "TNT"   },
        { "weapon_tripmine",     0,             0.1,    0.0,            false,  WV_QUIET,       WV_NONE,        WF_NONE,        WF_NONE,        0.0,    "tripmine"              },
        { "weapon_satchel",    400,             0.342,  0.3,            false,  WV_QUIET,       WV_QUIET,       WF_NONE,        WF_NONE,        0.0,    "satchel"               },
        { "weapon_snark",          400,         0.173,  0.1,            false,  WV_QUIET,       WV_QUIET,       WF_NONE,        WF_NONE,        0.0,    "snark"                 },
	{ "weapon_th_spanner",            10,             0.866,  0.0,            false,  WV_SWING,       WV_SWING,       WF_NONE,        WF_NONE,        0.0,    "spanner"               },
	{ "weapon_th_ap9",      200,             0.104,  0.5,            false,  WV_NORMAL,      WV_NORMAL,      WF_NORMAL,      WF_NORMAL,      0.0,    "AP9"               },
	{ "weapon_th_shovel",            10,             0.866,  0.0,            false,  WV_SWING,       WV_SWING,       WF_NONE,        WF_NONE,        0.0,    "shovel"               },
	{ "weapon_th_sniper",   600,             0.017,  0.5,            false,  WV_QUIET,       WV_NONE,        WF_NONE,        WF_NONE,        0.0,    "H&K G36"              },
	{ "weapon_einar1",   600,             0.017,  0.5,            false,  WV_QUIET,       WV_NONE,        WF_NONE,        WF_NONE,        0.0,    "H&K G36"              },
	{ "weapon_th_taurus", 200,             0.052,  0.5,            false,  WV_NORMAL,      WV_NORMAL,      WF_NORMAL,      WF_NORMAL,      0.0,    "taurus"                 },
	{ "weapon_th_chaingun", 200,             0.104,    0.5,            false,  WV_LOUD,       WV_LOUD,        WF_NORMAL,        WF_NORMAL,        0.0,    "chaingun" },
	{ "weapon_th_medkit", 0,             0.1,    0.0,            false,  WV_NONE,       WV_NONE,        WF_NONE,        WF_NONE,        0.0,    "medkit" },
};

const char *
weapon_getweaponname(int wId)
{
	switch (mod_id)	{
	case AG_DLL:
	case VALVE_DLL:		if (wId >= MIN_VALVE_WEAPONS && wId < MAX_VALVE_WEAPONS) return valveWeapon[wId].shortName;
						else return "weapon";						
	case HOLYWARS_DLL:	if (wId >= MIN_HW_WEAPONS && wId < MAX_HW_WEAPONS) return holywarsWeapon[wId].shortName;
						else return "weapon";
	case TFC_DLL:		if (wId >= MIN_TFC_WEAPONS && wId < MAX_TFC_WEAPONS) return tfcWeapon[wId].shortName;
						else return "weapon";
	case DMC_DLL:		if (wId >= MIN_DMC_WEAPONS && wId < MAX_DMC_WEAPONS) return dmcWeapon[wId].shortName;
						else return "weapon";
	case HUNGER_DLL:	if (wId >= MIN_HUNGER_WEAPONS && wId < MAX_HUNGER_WEAPONS) return hungerWeapon[wId].shortName;
						else return "weapon";
	case GEARBOX_DLL:	if (wId >= MIN_GEARBOX_WEAPONS && wId < MAX_GEARBOX_WEAPONS) return gearboxWeapon[wId].shortName;
						else return "weapon";
	}
	return "shitty unknown MOD weapon";
}



///////////////////////////////////////////////////////////////////////////////////
//
//  INIT ROUTINES
//
///////////////////////////////////////////////////////////////////////////////////

static void
weapon_initmod(WEAPON *weapon)
{
	weapon->modweapon = 0;
	switch (mod_id)	{
	case AG_DLL:
	case VALVE_DLL:		weapon->modweapon = &valveWeapon[0];
						weapon->minmodweapon = MIN_VALVE_WEAPONS;
						weapon->maxmodweapon = MAX_VALVE_WEAPONS;
						break;
	case HOLYWARS_DLL:	weapon->modweapon = &holywarsWeapon[0];
						weapon->minmodweapon = MIN_HW_WEAPONS;
						weapon->maxmodweapon = MAX_HW_WEAPONS;
						break;
	case TFC_DLL:		weapon->modweapon = &tfcWeapon[0];
						weapon->minmodweapon = MIN_TFC_WEAPONS;
						weapon->maxmodweapon = MAX_TFC_WEAPONS;
						break;
	case DMC_DLL:		weapon->modweapon = &dmcWeapon[0];
						weapon->minmodweapon = MIN_DMC_WEAPONS;
						weapon->maxmodweapon = MAX_DMC_WEAPONS;
						break;
	case HUNGER_DLL:	weapon->modweapon = &hungerWeapon[0];
						weapon->minmodweapon = MIN_HUNGER_WEAPONS;
						weapon->maxmodweapon = MAX_HUNGER_WEAPONS;
						break;
	case GEARBOX_DLL:	weapon->modweapon = &gearboxWeapon[0];
						weapon->minmodweapon = MIN_GEARBOX_WEAPONS;
						weapon->maxmodweapon = MAX_GEARBOX_WEAPONS;
						break;
	}
	assert( modweapon != 0 );
}

void
weapon_construct(WEAPON *weapon)
{
	weapon_initmod(weapon);
	weapon->currentweapon = weapon->minmodweapon;
}

void
weapon_construct2(WEAPON *weapon, int wId)
{
	weapon_initmod(weapon);
	if (wId >= weapon->minmodweapon && wId < weapon->maxmodweapon)
		weapon->currentweapon = wId;
	else
		weapon->currentweapon = weapon->minmodweapon;
}

void
weapon_init(WEAPON *weapon, int slot, EDICT *ent, ACTION *action)
{
	weapon->botslot = slot;
	weapon->botent = ent;
	weapon->botaction = action;
	for (int i = 0; i < MAX_WEAPONS; i++) {
		weapon->bestmode[i] = 1;	// primary attack
	}
	weapon->nextattacktime = 0;
	weapon->lastattacktime = 0;
	weapon->reloading = false;
	weapon->grenadeprepared = false;
	weapon->loadinggauss = false;
	weapon->armedweapon = weapon->minmodweapon;
	weapon->currentweapon = weapon->minmodweapon;
}

void
weapon_setcurrentweapon(WEAPON *weapon, int wId)
{ 
	if (wId >= weapon->minmodweapon && wId < weapon->maxmodweapon)
		weapon->currentweapon = wId;
}

void
weapon_registerarmedweapon(WEAPON *weapon, int wId)
{ 
	if (wId >= weapon->minmodweapon && wId < weapon->maxmodweapon)
		weapon->armedweapon = wId;
}

///////////////////////////////////////////////////////////////////////////////////
//
//  WEAPON SCORES
//
///////////////////////////////////////////////////////////////////////////////////

static float
weapon_valveweaponscore(WEAPON *weapon, float distance, float hitProb, int flags, bool checkAmmo)
// returns max. 10 pts
{
	float score = 0;
	float score2 = 0;
	bool notSuitable = false;

	switch (weapon->currentweapon) {
	case VALVE_WEAPON_CROWBAR:
		if ( flags & WF_NEED_GRENADE ) break;
		
		if (bm_cbar) {
                        if( bm_cbar->value ) {
				if( distance < 55 ) {
					weapon->bestmode[weapon->currentweapon] = 1;
					score = 9;
				} else if( distance < 300 ) {
					weapon->bestmode[weapon->currentweapon] = 2;
					score = 5;
				}
			} else if( distance < 55 )
				score = 9;
			else if( distance < 100 )
				score = ( 100 - distance ) / 5;
		} else if( distance < 55 )
			score = 9;
		else if( distance < 100 )
			score = ( 100 - distance ) / 5;

		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_GLOCK:
		if ( flags & WF_NEED_GRENADE ) break;
					
		score = 1.5 - (2*distance/6400);
		if (score<0.1) score = 0.1;

		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 5) score /= 2;
		}
		if (hitProb > 0.3) weapon->bestmode[weapon->currentweapon] = 2;
		else			   weapon->bestmode[weapon->currentweapon] = 1;	// don't waste bullets

		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_PYTHON:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
		
		score = 4 - (4*distance/6400);
		if (score<0.3) score = 0.3;

		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 3) score /= 2;
		}
		if ( hitProb < 0.5) score *= (hitProb + 0.5);	// punish slow reload
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_MP5:
		if ( flags & WF_UNDERWATER ) break;

		if (checkAmmo && (weapon_ammo2(weapon)>0) && (distance>250)) {	// if not sure assume that
			  if (distance<400) score2 = (distance-250) / 15;	// player doesn't have grens
			  else if (distance<550) score2 = (550-distance) / 15;
			  weapon->bestmode[weapon->currentweapon] = 2;
		}
		if ( flags & WF_NEED_GRENADE ) break;
				
		score = 7 - (7*distance/1600);
		if (score<0.2) score = 0.2;
		
		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if ( flags & WF_SINGLE_SHOT_KILL ) {
			notSuitable = true;
			if  ((score/10) > score2) weapon->bestmode[weapon->currentweapon] = 1;
		}
		else if (score > score2) weapon->bestmode[weapon->currentweapon] = 1;
		break;

	case VALVE_WEAPON_CROSSBOW:
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;

		if (distance<600) {
			score = (distance-200) / 100;
			if (score<2) score = 2;		// in any case better than glock!
		}
		else score = 4;

		if (hitProb>0.5) weapon->bestmode[weapon->currentweapon] = 2;	// use sniper mode when possible
		else {
			weapon->bestmode[weapon->currentweapon] = 1;
			if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		}
		break;

	case VALVE_WEAPON_SHOTGUN:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
				
		score = 8 - (8*distance/1600);
		if (score<0.2) score = 0.2;

		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 4) score /= 2;
		}
		if (hitProb < 0.5) score *= (hitProb + 0.5);	// punish slow reload
		
		if (hitProb>0.5 && distance<200 && weapon_ammo1(weapon)>1) {
			weapon->bestmode[weapon->currentweapon] = 2;	// use double mode when possible
			score = 10;			// to win against crowbar
		}
		else {
			weapon->bestmode[weapon->currentweapon] = 1;
			if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		}
		break;

	case VALVE_WEAPON_RPG:
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;

		if (distance < 400) {
			weapon->bestmode[weapon->currentweapon] = 1;
			score = (distance-250) / 18.75;
			if (score<1.5) score = 1.5;	// don't use glock!
		}
		else {
			weapon->bestmode[weapon->currentweapon] = 2;
			score = 8 - 8*(distance-400)/1600; 
			if ( flags & WF_ENEMY_ABOVE ) score -= 1;
			if (score<1) score = 1;
		}
		break;

	case VALVE_WEAPON_GAUSS:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
				
		score = 6;
		
		if (checkAmmo) {
			if (weapon_ammo1(weapon) <= 1) score = 0;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if(hitProb>0.5)
			weapon->bestmode[weapon->currentweapon] = 2;
		else
			weapon->bestmode[weapon->currentweapon] = 1;
		break;

	case VALVE_WEAPON_EGON:
		if((bm_gluon && bm_gluon->value)
			|| (com.gamedll_flags & GAMEDLL_SEVS))
		{
				return 0;
		}
		if (flags & (WF_UNDERWATER | WF_NEED_GRENADE)) break;
				
		if (distance < 250) {
			score = (distance - 150.0f) / 12.5f;
			if (score < 0.5f) score = 0.5f;
		}
		else {
			score = 8 - 8*(distance-250)/1600;
			if (score<4) score = 4;		// not that bad...
		}

		if (checkAmmo) {
			if (weapon_ammo1(weapon) <= 1) break;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_HORNETGUN:
		
		score = 2;
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_HANDGRENADE:
		if ( flags & WF_UNDERWATER ) break;
		// explosion at 450
		if (300<=distance && distance<=600) score = 1;
		break;

	case VALVE_WEAPON_TRIPMINE:
		if( bm_trip && bm_trip->value )
		{
			weapon->bestmode[weapon->currentweapon] = randomfloat( 1.5, 2.0 );
		}
		return 0;		// never arm for combat

	case VALVE_WEAPON_SATCHEL:
		if ( flags & (WF_UNDERWATER | WF_ENEMY_ABOVE)) break;
		if (250<=distance && distance<=400) 
			score = 3;
		break;

	case VALVE_WEAPON_SNARK:
		if ( flags & (WF_UNDERWATER | WF_ENEMY_ABOVE)) break;
		if (200<=distance && distance<=1000) {
			score = 0.75;	// must be less than glock (to use glock against snarks)
			if ( flags & WF_ENEMY_BELOW ) score += 2;
		}
		break;

	default:
		DEBUG_MSG( "ValveWeaponScore: Unknown ID %i !\n", currentweapon );
	
	}

	// switch-time handicaps
	if ( weapon->armedweapon != weapon->currentweapon ) {
		if (flags & WF_IMMEDIATE_ATTACK) {
			if ( score > 4.1 ) score -= 4.0;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
		else if (flags & WF_FAST_ATTACK) {
			if ( score > 2.1 ) score -= 2.0;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
		else {
			if ( score > 0.6 ) score -= 0.5;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
	}
	
	if (notSuitable) score /= 10;

	// DEBUG_MSG( "wscore=%.2f\n", score );
	return score;
}

static float
weapon_hwweaponscore(WEAPON *weapon, float distance, float hitProb, int flags, bool checkAmmo)
// returns max. 10 pts
{
	float score = 0;
	bool notSuitable = false;

	switch (weapon->currentweapon) {

	case HW_WEAPON_JACKHAMMER:
		if ( flags & WF_NEED_GRENADE ) break;
		
		if (distance < 55) score = 9;
		else if (distance < 100) score = (100-distance) / 5;

		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case HW_WEAPON_DOUBLESHOTGUN:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
				
		score = 8 - (8*distance/1600);
		if (score<0.2) score = 0.2;

		if (checkAmmo && weapon_ammo1(weapon) == 0) score = 0;
		if (hitProb < 0.5) score *= (hitProb + 0.5);	// punish slow reload
		
		if (hitProb>0.5 && distance<200) score = 10;	
		else if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case HW_WEAPON_MACHINEGUN:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
				
		score = 7 - (7*distance/1600);
		if (score<0.2) score = 0.2;
		
		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;			
		break;

	case HW_WEAPON_ROCKETLAUNCHER:
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;
		//score = 10; break;

		if (distance < 400) {
			score = (distance-250) / 18.75;
			if (score<1.5) score = 1.5;	// don't use jackhammer!
		}
		else {
			score = 8 - 8*(distance-400)/1600; 
			if (score<1) score = 1;
		}
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;
	
	case HW_WEAPON_RAILGUN:
		if ( flags & WF_NEED_GRENADE ) break;
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;

		score = hitProb * 15;
		if (score<2) score = 2;		// in any case better than jackhammer

		break;

	default:
		DEBUG_MSG( "HolyWarsWeaponScore: Unknown ID %i !\n", currentweapon );
	
	}

	// switch-time handicaps
	if ( weapon->armedweapon != weapon->currentweapon ) {
		if (flags & WF_IMMEDIATE_ATTACK) {
			if ( score > 4.1 ) score -= 4.0;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
		else if (flags & WF_FAST_ATTACK) {
			if ( score > 2.1 ) score -= 2.0;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
		else {
			if ( score > 0.6 ) score -= 0.5;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
	}
	
	if (notSuitable) score /= 10;

	// DEBUG_MSG( "wscore=%.2f\n", score );
	return score;

}

static float
weapon_dmcweaponscore(WEAPON *weapon, float distance, float hitProb, int flags, bool checkAmmo )
// returns max. 10 pts
{
	float score = 0;
	bool notSuitable = false;

	switch (weapon->currentweapon) {
	case DMC_WEAPON_CROWBAR:
		if ( flags & WF_NEED_GRENADE ) break;
		
		if (distance < 55) score = 9;
		else if (distance < 100) score = (100-distance) / 5;

		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case DMC_WEAPON_QUAKEGUN:
		if ( flags & WF_NEED_GRENADE ) break;
				
		score = 3 - (3*distance/1800);
		if (score<0.2) score = 0.2;

		if (checkAmmo && weapon_ammo1(weapon) == 0) score = 0;
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case DMC_WEAPON_SUPERSHOTGUN:
		if ( flags & WF_NEED_GRENADE ) break;
				
		score = 6 - (6*(distance-100)/200);
		if (score>6) score = 6;
		else if (score<0.1) score = 0.1;

		if (checkAmmo && weapon_ammo1(weapon) == 0) score = 0;
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case DMC_WEAPON_NAILGUN:
		if ( flags & WF_NEED_GRENADE ) break;
				
		score = 4 - (4*distance/1600);
		if (hitProb<0.3) score += 1;		// fast-fire bonus
		if (score<0.2) score = 0.2;
		
		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;			
		break;

	case DMC_WEAPON_SUPERNAILGUN:
		if ( flags & WF_NEED_GRENADE ) break;
				
		score = 6 - (6*distance/1600);
		if (hitProb<0.3) score += 1;		// fast-fire bonus
		if (score<0.2) score = 0.2;
		
		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;			
		break;

	case DMC_WEAPON_GRENLAUNCHER:
		if ( flags & WF_ENEMY_ABOVE ) break;	// can't use this towards higher pos
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;
		
		if ( flags & WF_ENEMY_BELOW ) {		// this is going to be fun ;-)
			if (distance < 500) {
				float minDist = 150;
				if (has_quaddamage(weapon->botent )) minDist = 300;
				if (distance>minDist) score = 5;
			}
			else {
				score = 5 - 5*(distance-500)/700;
				if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
			}
			if (hitProb<0.2) score += 1;	// explosive bonus
		}
		else {
			if (distance < 250) {
				if (!has_quaddamage( weapon->botent )) {		// that would be very unwise
					score = (distance-150) / 16.67;
					if (score<2.5) score = 2.5;	// don't switch weapon!
				}
			}
			else {
				score = 6 - 6*(distance-250)/500; 
				if (score<1) score = 1;
				if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
			}
		}
		break;
		
	case DMC_WEAPON_ROCKETLAUNCHER:
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;
		
		if (distance < 250) {
			if (!has_quaddamage( weapon->botent )) {		// that would be very unwise
				score = (distance-150) / 12.5;
				if (score<2.5) score = 2.5;	// don't switch weapon!
			}
		}
		else {
			score = 8 - 8*(distance-250)/1350; 
			if ( flags & WF_ENEMY_ABOVE ) score -= 2;
			if (score<1) score = 1;
		}
		break;
	
	case DMC_WEAPON_LIGHTNING:
		if ( flags & WF_NEED_GRENADE ) break;
		if ( flags & WF_UNDERWATER ) {
			// suppose others are underwater as well ;-)
			if (is_invulnerable( weapon->botent )) score = 10;
			break;
		}
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;

		score = hitProb * 20;
		if (score<3) score = 3;		

		break;

	default:
		DEBUG_MSG( "DMCWeaponScore: Unknown ID %i !\n", currentweapon );
	
	}

	// switch-time handicaps
	if ( weapon->armedweapon != weapon->currentweapon ) {
		if (flags & WF_IMMEDIATE_ATTACK) {
			if ( score > 4.1 ) score -= 4.0;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
		else if (flags & WF_FAST_ATTACK) {
			if ( score > 2.1 ) score -= 2.0;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
		else {
			if ( score > 0.6 ) score -= 0.5;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
	}
	
	if (notSuitable) score /= 10;
	if (has_quaddamage( weapon->botent )) score *= 3;

	//DEBUG_MSG( "wscore=%.2f\n", score );
	return score;

}

static float
weapon_csweaponscore(WEAPON *weapon, float distance, float hitProb, int flags, bool checkAmmo )
// returns max. 10 pts
{
	float score = 0;

	if ( weapon->armedweapon==weapon->currentweapon ) score = 10;

	return score;
}

static float
weapon_tfcweaponscore(WEAPON *weapon, float distance, float hitProb, int flags, bool checkAmmo )
// returns max. 10 pts
{
	float score = 0;

	if ( weapon->armedweapon==weapon->currentweapon ) score = 10;
	
	return score;
}

static float
weapon_gearboxweaponscore(WEAPON *weapon, float distance, float hitProb, int flags, bool checkAmmo )
// returns max. 10 pts
{
	float score = 0;
	float score2 = 0;
	bool notSuitable = false;

	switch (weapon->currentweapon) {

	case VALVE_WEAPON_CROWBAR:
	case GEARBOX_WEAPON_PIPEWRENCH:
	case GEARBOX_WEAPON_KNIFE:
		if ( flags & WF_NEED_GRENADE ) break;
		
		if (distance < 55) score = 9;
		else if (distance < 100) score = (100-distance) / 5;

		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_GLOCK:
		if ( flags & WF_NEED_GRENADE ) break;
					
		score = 1.5 - (2*distance/6400);
		if (score<0.1) score = 0.1;

		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 5) score /= 2;
		}
		if (hitProb > 0.3) weapon->bestmode[weapon->currentweapon] = 2;
		else			   weapon->bestmode[weapon->currentweapon] = 1;	// don't waste bullets

		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_PYTHON:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
		
		score = 4 - (4*distance/6400);
		if (score<0.3) score = 0.3;

		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 3) score /= 2;
		}
		if ( hitProb < 0.5) score *= (hitProb + 0.5);	// punish slow reload
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_MP5:
		if ( flags & WF_UNDERWATER ) break;

		if (checkAmmo && (weapon_ammo2(weapon)>0) && (distance>250)) {	// if not sure assume that
			  if (distance<400) score2 = (distance-250) / 15;	// player doesn't have grens
			  else if (distance<550) score2 = (550-distance) / 15;
			  weapon->bestmode[weapon->currentweapon] = 2;
		}
		if ( flags & WF_NEED_GRENADE ) break;
				
		score = 7 - (7*distance/1600);
		if (score<0.2) score = 0.2;
		
		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if ( flags & WF_SINGLE_SHOT_KILL ) {
			notSuitable = true;
			if  ((score/10) > score2) weapon->bestmode[weapon->currentweapon] = 1;
		}
		else if (score > score2) weapon->bestmode[weapon->currentweapon] = 1;
		break;

	case VALVE_WEAPON_CROSSBOW:
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;

		if (distance<600) {
			score = (distance-200) / 100;
			if (score<2) score = 2;		// in any case better than glock!
		}
		else score = 4;

		if (hitProb>0.5) weapon->bestmode[weapon->currentweapon] = 2;	// use sniper mode when possible
		else {
			weapon->bestmode[weapon->currentweapon] = 1;
			if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		}
		break;

	case VALVE_WEAPON_SHOTGUN:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
				
		score = 8 - (8*distance/1600);
		if (score<0.2) score = 0.2;

		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 4) score /= 2;
		}
		if ( hitProb < 0.5) score *= (hitProb + 0.5);	// punish slow reload
		
		if (hitProb>0.5 && distance<200 && weapon_ammo1(weapon)>1) {
			weapon->bestmode[weapon->currentweapon] = 2;	// use double mode when possible
			score = 10;			// to win against crowbar
		}
		else {
			weapon->bestmode[weapon->currentweapon] = 1;
			if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		}
		break;

	case VALVE_WEAPON_RPG:
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;

		if (distance < 400) {
			weapon->bestmode[weapon->currentweapon] = 1;
			score = (distance-250) / 18.75;
			if (score<1.5) score = 1.5;	// don't use glock!
		}
		else {
			weapon->bestmode[weapon->currentweapon] = 2;
			score = 8 - 8*(distance-400)/1600; 
			if ( flags & WF_ENEMY_ABOVE ) score -= 1;
			if (score<1) score = 1;
		}
		break;

	case VALVE_WEAPON_GAUSS:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
				
		score = 6;
		
		if (checkAmmo) {
			if (weapon_ammo1(weapon) <= 1) score = 0;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if(hitProb>0.5)
			weapon->bestmode[weapon->currentweapon] = 2;
		else
			weapon->bestmode[weapon->currentweapon] = 1;
		break;

	case VALVE_WEAPON_EGON:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
				
		if (distance < 250) {
			score = (distance-150) / 12.5;
			if (score<0.5) score = 0.5;
		}
		else {
			score = 8 - 8*(distance-250)/1600;
			if (score<4) score = 4;		// not that bad...
		}

		if (checkAmmo) {
			if (weapon_ammo1(weapon) <= 1) break;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_HORNETGUN:
		
		score = 2;
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_HANDGRENADE:
		if ( flags & WF_UNDERWATER ) break;
		// explosion at 450
		if (300<=distance && distance<=600) score = 1;
		break;

	case VALVE_WEAPON_TRIPMINE:
		return 0;		// never arm for combat

	case VALVE_WEAPON_SATCHEL:
		if ( flags & (WF_UNDERWATER | WF_ENEMY_ABOVE)) break;
		if (250<=distance && distance<=400) 
			score = 3;
		break;

	case VALVE_WEAPON_SNARK:
	case GEARBOX_WEAPON_PENGUIN:
		if ( flags & (WF_UNDERWATER | WF_ENEMY_ABOVE)) break;
		if (200<=distance && distance<=1000) {
			score = 0.75;	// must be less than glock (to use glock against snarks)
			if ( flags & WF_ENEMY_BELOW ) score += 2;
		}
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case GEARBOX_WEAPON_GRAPPLE:
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;

		score = 1.5 - (distance/800);
		if (score<0.1) score = 0.1;

		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case GEARBOX_WEAPON_EAGLE:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
		
		score = 4 - (4*distance/6400);
		if (score<0.3) score = 0.3;

		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 3) score /= 2;
		}
		break;

	case GEARBOX_WEAPON_M249:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;

		score = 8 - (8*distance/3200);
		if (score<0.3) score = 0.3;
		
		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case GEARBOX_WEAPON_DISPLACER:
		return 0;		// never arm for combat

	case GEARBOX_WEAPON_SHOCKRIFLE:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
				
		score = 7 - 7*(distance-250)/1600;
		if (score<3) score = 3;		// not that bad...
		
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case GEARBOX_WEAPON_SPORELAUNCHER:
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;
		
		if (distance < 250) {
			score = (distance-150) / 16.67;
			if (score<2.5) score = 2.5;	// don't switch weapon!
		}
		else {
			score = 6 - 6*(distance-250)/1350; 
			if ( flags & WF_ENEMY_ABOVE ) score -= 2;
			if (score<1) score = 1;
		}
		break;

	case GEARBOX_WEAPON_SNIPERRIFLE:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;

		if (distance<600) {
			score = (distance-100) / 100;
			if (score<3) score = 3;		// in any case better than glock!
		}
		else score = 5;
		break;
	

	default:
		DEBUG_MSG( "GearboxWeaponScore: Unknown ID %i !\n", currentweapon );
	
	}

	// switch-time handicaps
	if ( weapon->armedweapon != weapon->currentweapon ) {
		if (flags & WF_IMMEDIATE_ATTACK) {
			if ( score > 4.1 ) score -= 4.0;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
		else if (flags & WF_FAST_ATTACK) {
			if ( score > 2.1 ) score -= 2.0;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
		else {
			if ( score > 0.6 ) score -= 0.5;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
	}
	
	if (notSuitable) score /= 10;

	// DEBUG_MSG( "wscore=%.2f\n", score );
	return score;
}

static float
weapon_hungerweaponscore(WEAPON *weapon, float distance, float hitProb, int flags, bool checkAmmo )
// returns max. 10 pts
{
	float score = 0;
	float score2 = 0;
	bool notSuitable = false;

	switch (weapon->currentweapon) {

	case VALVE_WEAPON_CROWBAR:
	case HUNGER_WEAPON_SPANNER:
	case HUNGER_WEAPON_SHOVEL:
		if ( flags & WF_NEED_GRENADE ) break;
		
		if (distance < 55) score = 9;
		else if (distance < 100) score = (100-distance) / 5;

		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_GLOCK:
	case HUNGER_WEAPON_TAURUS:
		if ( flags & WF_NEED_GRENADE ) break;
					
		score = 1.5 - (2*distance/6400);
		if (score<0.1) score = 0.1;

		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 5) score /= 2;
		}

		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_PYTHON:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
		
		score = 4 - (4*distance/6400);
		if (score<0.3) score = 0.3;

		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 3) score /= 2;
		}
		if ( hitProb < 0.5) score *= (hitProb + 0.5);	// punish slow reload
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_MP5:
		if ( flags & WF_UNDERWATER ) break;

		if (checkAmmo && (weapon_ammo2(weapon)>0) && (distance>250)) {	// if not sure assume that
			  if (distance<400) score2 = (distance-250) / 15;	// player doesn't have grens
			  else if (distance<550) score2 = (550-distance) / 15;
			  weapon->bestmode[weapon->currentweapon] = 2;
		}
		if ( flags & WF_NEED_GRENADE ) break;
				
		score = 7 - (7*distance/1600);
		if (score<0.2) score = 0.2;
		
		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if ( flags & WF_SINGLE_SHOT_KILL ) {
			notSuitable = true;
			if  ((score/10) > score2) weapon->bestmode[weapon->currentweapon] = 1;
		}
		else if (score > score2) weapon->bestmode[weapon->currentweapon] = 1;
		break;

	case VALVE_WEAPON_CROSSBOW:
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;

		if (distance<600) {
			score = (distance-200) / 100;
			if (score<2) score = 2;		// in any case better than glock!
		}
		else score = 4;

		if (hitProb>0.5) weapon->bestmode[weapon->currentweapon] = 2;	// use sniper mode when possible
		else {
			weapon->bestmode[weapon->currentweapon] = 1;
			if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		}
		break;

	case HUNGER_WEAPON_CHAINGUN:
	case VALVE_WEAPON_EGON:
                if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
                
		if (distance < 250) {
			score = (distance-150) / 12.5;
			if (score<0.5) score = 0.5;
		}
		else {
			score = 8 - 8*(distance-250)/1600;
			if (score<4) score = 4;         // not that bad...
		}

                if (checkAmmo) {
                        if (weapon_ammo1(weapon) == 0) score = 0;
                        else if (weapon_ammo1(weapon) <= 10) score /= 2;
                }
		if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		break;

	case VALVE_WEAPON_SHOTGUN:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
				
		score = 8 - (8*distance/1600);
		if (score<0.2) score = 0.2;

		if (checkAmmo) {
			if (weapon_ammo1(weapon) == 0) score = 0;
			else if (weapon_ammo1(weapon) <= 4) score /= 2;
		}
		if ( hitProb < 0.5) score *= (hitProb + 0.5);	// punish slow reload
		
		if (hitProb>0.5 && distance<200 && weapon_ammo1(weapon)>1) {
			weapon->bestmode[weapon->currentweapon] = 2;	// use double mode when possible
			score = 10;			// to win against crowbar
		}
		else {
			weapon->bestmode[weapon->currentweapon] = 1;
			if ( flags & WF_SINGLE_SHOT_KILL ) notSuitable = true;
		}
		break;

	case VALVE_WEAPON_RPG:
		if (checkAmmo && weapon_ammo1(weapon) == 0) break;

		if (distance < 400) {
			weapon->bestmode[weapon->currentweapon] = 1;
			score = (distance-250) / 18.75;
			if (score<1.5) score = 1.5;	// don't use glock!
		}
		else {
			weapon->bestmode[weapon->currentweapon] = 2;
			score = 8 - 8*(distance-400)/1600; 
			if ( flags & WF_ENEMY_ABOVE ) score -= 1;
			if (score<1) score = 1;
		}
		break;

	case VALVE_WEAPON_GAUSS:
		if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
				
		score = 6;
		
		if (checkAmmo) {
			if (weapon_ammo1(weapon) <= 1) score = 0;
			else if (weapon_ammo1(weapon) <= 10) score /= 2;
		}
		if(hitProb>0.5)
			weapon->bestmode[weapon->currentweapon] = 2;
		else
			weapon->bestmode[weapon->currentweapon] = 1;
		break;

	case VALVE_WEAPON_HANDGRENADE:
		if ( flags & WF_UNDERWATER ) break;
		// explosion at 450
		if (300<=distance && distance<=600) score = 1;
		break;

	case VALVE_WEAPON_TRIPMINE:
	case HUNGER_WEAPON_MEDKIT:
		return 0;		// never arm for combat

	case VALVE_WEAPON_SATCHEL:
		if ( flags & (WF_UNDERWATER | WF_ENEMY_ABOVE)) break;
		if (250<=distance && distance<=400) 
			score = 3;
		break;

	case VALVE_WEAPON_SNARK:
		if ( flags & (WF_UNDERWATER | WF_ENEMY_ABOVE)) break;
		if (200<=distance && distance<=1000) {
			score = 0.75;	// must be less than glock (to use glock against snarks)
			if ( flags & WF_ENEMY_BELOW ) score += 2;
		}
		break;

	case HUNGER_WEAPON_SNIPER:
	case HUNGER_WEAPON_TFCSNIPER:
                if ( flags & (WF_UNDERWATER | WF_NEED_GRENADE) ) break;
                if (checkAmmo && weapon_ammo1(weapon) == 0) break;

                if (distance<600) {
                        score = (distance-100) / 100;
                        if (score<3) score = 3;         // in any case better than glock!
                }
                else score = 5;
                break;
	case HUNGER_WEAPON_AP9:
                if ( flags & WF_NEED_GRENADE ) break;

                score = 7 - (7*distance/1600);
                if (score<0.2) score = 0.2;

                if (checkAmmo) {
                        if (weapon_ammo1(weapon) == 0) score = 0;
                        else if (weapon_ammo1(weapon) <= 10) score /= 2;
                }

                if ( flags & WF_SINGLE_SHOT_KILL ) {
                        notSuitable = true;
                }

		if (hitProb > 0.3) weapon->bestmode[weapon->currentweapon] = 2;
                else weapon->bestmode[weapon->currentweapon] = 1;

                break;
	default:
		DEBUG_MSG( "HungerWeaponScore: Unknown ID %i !\n", currentweapon );
	
	}

	// switch-time handicaps
	if ( weapon->armedweapon != weapon->currentweapon ) {
		if (flags & WF_IMMEDIATE_ATTACK) {
			if ( score > 4.1 ) score -= 4.0;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		} else if (flags & WF_FAST_ATTACK) {
			if ( score > 2.1 ) score -= 2.0;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		} else {
			if ( score > 0.6 ) score -= 0.5;
			else if (score > 0) score = 0.1;	// it has to remain better than 0
		}
	}
	
	if (notSuitable) score /= 10;

	// DEBUG_MSG( "wscore=%.2f\n", score );
	return score;
}

float
weapon_getscore(WEAPON *weapon, float distance, float hitProb, int flags, bool checkAmmo)
// returns max. 10 pts
{
	switch (mod_id)	{
	case AG_DLL:
	case VALVE_DLL:		return weapon_valveweaponscore(weapon, distance, hitProb, flags, checkAmmo );
	case HOLYWARS_DLL:	return weapon_hwweaponscore(weapon, distance, hitProb, flags, checkAmmo );
	case DMC_DLL:		return weapon_dmcweaponscore(weapon, distance, hitProb, flags, checkAmmo );
	case CSTRIKE_DLL:	return weapon_csweaponscore(weapon, distance, hitProb, flags, checkAmmo );
	case TFC_DLL:		return weapon_tfcweaponscore(weapon, distance, hitProb, flags, checkAmmo );
	case HUNGER_DLL:	return weapon_hungerweaponscore(weapon, distance, hitProb, flags, checkAmmo );
	case GEARBOX_DLL:	return weapon_gearboxweaponscore(weapon, distance, hitProb, flags, checkAmmo );	
	}

	ERROR_MSG( "FATAL ERROR in getWeaponScore(): Unknown MOD-ID!\n" );
	return 0;
}




///////////////////////////////////////////////////////////////////////////////////
//
//  ATTACK ROUTINES
//
///////////////////////////////////////////////////////////////////////////////////

static bool
weapon_attackvalvehandgrenade(WEAPON *weapon, Vec3D *target)
{
	if (worldtime() < weapon->nextattacktime) return false;

	bool grenadeThrown = false;
	if (!weapon->grenadeprepared) {
		// DEBUG_MSG( "Arming HG!\n" );
		weapon->grenadeprepared = true;
		weapon->grenadelaunchtime = worldtime() + 2.0;
		vcopy(target, &weapon->grenadetarget);
		weapon->grenadewid = VALVE_WEAPON_HANDGRENADE;
	}
	if (worldtime() < weapon->grenadelaunchtime) {
		// DEBUG_MSG( "Holding HG, counter=%.1f\n", (weapon->grenadelaunchtime - worldtime()) );
		action_add(weapon->botaction, BOT_FIRE_PRIM, NULL);
	} else {
		// DEBUG_MSG( "Throwing HG!\n" );
		grenadeThrown = true;
		weapon->grenadeprepared = false;
		weapon->nextattacktime = worldtime() + 0.5;	// give time to switch weapons
	}

	return grenadeThrown;
}

static bool
weapon_attackvalvesatchel(WEAPON *weapon, Vec3D *target )
{
	if (worldtime() < weapon->nextattacktime) return false;

	bool grenadeThrown = false;
	if (!weapon->grenadeprepared) {
		// DEBUG_MSG( "Throwing satchel!\n" );
		action_add(weapon->botaction, BOT_FIRE_PRIM, NULL);
		if ( Q_STREQ( STRING(weapon->botent->v.viewmodel), "models/v_satchel_radio.mdl" ) ) {
			weapon->grenadeprepared=true;
			weapon->grenadelaunchtime = worldtime() + 1.5;
			weapon->grenadewid = VALVE_WEAPON_SATCHEL;
		}
	}
	else if ( worldtime() > weapon->grenadelaunchtime ) {
		// DEBUG_MSG( "Blowing up satchel!\n" );
		action_add(weapon->botaction, BOT_FIRE_PRIM, NULL);
		if ( worldtime() > (weapon->grenadelaunchtime + 0.1f)) {
			grenadeThrown = true;
			weapon->grenadeprepared = false;
			weapon->nextattacktime = worldtime() + 0.5;	// give time to switch weapons
		}
	}
	//else DEBUG_MSG( "Waiting for satchel, counter=%.1f\n", (weapon->grenadelaunchtime - worldtime()) );

	return grenadeThrown;
}

bool
weapon_attack(WEAPON *weapon, Vec3D *target, float accuracy, Vec3D *relVel )
// aims at target, if accuracy is reached fires and returns true, else returns false
// reloads when necessary
{
	bool fired = false;

	action_setaimdir(weapon->botaction, target, relVel );

	// check for reload:
	if (weapon_needreload(weapon))
		weapon_reload(weapon);
	else {
		// load special weapons:
		if ( mod_id == VALVE_DLL || mod_id == AG_DLL || mod_id == HUNGER_DLL || mod_id == GEARBOX_DLL ) {	
			if ( weapon->currentweapon == VALVE_WEAPON_GAUSS && weapon->bestmode[weapon->currentweapon] == 2 ) {
				if (!weapon->loadinggauss) {
					weapon->loadinggauss = true;
					weapon->loadstarttime = worldtime();
					weapon->nextattacktime = worldtime() + 1.0;	// at least load 1 sec.
				}
				// check if we have to stop loading to not explode...
				if ((worldtime() - weapon->loadstarttime) < 5)
					action_add(weapon->botaction, BOT_FIRE_SEC, NULL);
				else {
					weapon->loadinggauss = false;
					fired = true;
				}
			} else if (weapon->currentweapon==VALVE_WEAPON_HANDGRENADE) {
				fired = weapon_attackvalvehandgrenade(weapon, target );// pull trigger before aiming perfectly
			} else if (weapon->currentweapon==VALVE_WEAPON_SATCHEL && weapon->grenadeprepared) {
				fired = weapon_attackvalvesatchel(weapon, target );	// no need for aiming when satchel thrown
			}
		}
		if (fired) return true;	// no need to go on

		// fire if delay and accuracy are ok:
		if (( worldtime() > weapon->nextattacktime ) && 
			(action_targetaccuracy(weapon->botaction) > accuracy || weapon->lastattacktime > (worldtime() - 0.5f) )) {
			weapon->lastattacktime = worldtime();
			if ( weapon->bestmode[weapon->currentweapon] == 1 ) {	
				// primary fire:
				if (mod_id==VALVE_DLL || mod_id == AG_DLL || mod_id == HUNGER_DLL || mod_id == GEARBOX_DLL) {
					if (weapon->currentweapon == VALVE_WEAPON_HANDGRENADE)
						fired = weapon_attackvalvehandgrenade(weapon, target);
					else if (weapon->currentweapon == VALVE_WEAPON_SATCHEL)
						fired = weapon_attackvalvesatchel(weapon, target);
					else {  action_add(weapon->botaction, BOT_FIRE_PRIM, NULL);  fired = true;  }
				} else {  action_add(weapon->botaction, BOT_FIRE_PRIM, NULL );  fired = true;  }
			} else {
				// secondary fire:
				if (mod_id==VALVE_DLL || mod_id == AG_DLL || mod_id == HUNGER_DLL || mod_id==GEARBOX_DLL) {
					if (weapon->currentweapon==VALVE_WEAPON_GAUSS) {
						action_add(weapon->botaction, BOT_RELEASE_SEC, NULL);
						weapon->loadinggauss = false;
					} else if ( weapon->currentweapon == VALVE_WEAPON_CROSSBOW || 
						      weapon->currentweapon == VALVE_WEAPON_PYTHON   || 
							  weapon->currentweapon == VALVE_WEAPON_RPG) 
					{	// these weapons had their correct mode set in armBestWeapon
						action_add(weapon->botaction, BOT_FIRE_PRIM, NULL);
					} else {
						action_add(weapon->botaction, BOT_FIRE_SEC, NULL);
					}
				} else
					action_add(weapon->botaction, BOT_FIRE_SEC, NULL);
				fired = true;
			}
			weapon->nextattacktime = worldtime() + weapon->modweapon[weapon->currentweapon].fireDelay;
			if ( weapon->modweapon[weapon->currentweapon].fireDelay > 0 ) {		// has to release button
				float rDelay = randomfloat( 0.0f, 0.3f );	// add random delay
				weapon->nextattacktime += rDelay;
			}
		}
	}
	return fired;
}

bool
weapon_hastofinishattack(WEAPON *weapon)
{
	return weapon->grenadeprepared;
}

int
weapon_armedgrenade(WEAPON *weapon)
{
	return weapon->grenadewid;
}

void
weapon_finishattack(WEAPON *weapon)
{
	// DEBUG_MSG( "Forced to finish attack!\n" );
	if (mod_id == VALVE_DLL || mod_id == AG_DLL || mod_id == HUNGER_DLL || mod_id == GEARBOX_DLL) {
		if (weapon->currentweapon==VALVE_WEAPON_HANDGRENADE) {
			action_setviewdir(weapon->botaction, &weapon->grenadetarget, 5 );
			weapon_attackvalvehandgrenade(weapon, &weapon->grenadetarget );
		} else if (weapon->currentweapon==VALVE_WEAPON_SATCHEL) {
			weapon_attackvalvesatchel(weapon, &weapon->grenadetarget );
		} else {
			DEBUG_MSG( "ERROR: Illegal wID!\n" );
		}
	}
}




///////////////////////////////////////////////////////////////////////////////////
//
//  STANDARD QUERIES
//
///////////////////////////////////////////////////////////////////////////////////


int
weapon_ammo1(WEAPON *weapon)
{
	return (bots[weapon->botslot].m_rgAmmo[weapon_defs[weapon->currentweapon].iAmmo1]);
}

int
weapon_ammo2(WEAPON *weapon)
{
	return (bots[weapon->botslot].m_rgAmmo[weapon_defs[weapon->currentweapon].iAmmo2]);
}

float
weapon_highaimprob(WEAPON *weapon)
{
	assert( weapon->modweapon != 0 );
	return weapon->modweapon[weapon->currentweapon].highAimProb;
}

float
weapon_getaudibledistance(WEAPON *weapon, int attackFlags)
{
	assert( weapon->modweapon != 0 );

	if (attackFlags & ACTION_ATTACK2)
		return (2 * weapon->modweapon[weapon->currentweapon].volAttack2);
	return (2 * weapon->modweapon[weapon->currentweapon].volAttack1);
}

bool
weapon_needreload(WEAPON *weapon)
// returns true if the current weapon needs to be reloaded
{
	if (mod_id == DMC_DLL)
		return false;	// no reload in DMC!

	// ammoclip values only available for armed weapons!
	if (weapon->currentweapon != weapon->armedweapon)
		return false;

	bool shouldReload = false;

	if (weapon->modweapon[weapon->currentweapon].secAmmo && weapon->bestmode[weapon->currentweapon] == 2) {	
		// attack with secondary ammo
		if (bots[weapon->botslot].current_weapon.iAmmo2 == 0) {
			weapon->bestmode[weapon->currentweapon] = 1;
			if (bots[weapon->botslot].current_weapon.iClip == 0)
				shouldReload = true;
		}
	} else {
		// attack with primary ammo
		if ( bots[weapon->botslot].current_weapon.iClip == 0 ) shouldReload = true;
	}

	// check if reloading has to be set to false
	if (!shouldReload) weapon->reloading = false;

	return shouldReload;
}

void
weapon_reload(WEAPON *weapon)
{
	// only execute if not reloading yet and weapon armed
	if (!weapon->reloading && weapon->currentweapon == weapon->armedweapon) {
		weapon->reloading = true;
		action_add(weapon->botaction, BOT_RELOAD, NULL);
		// DEBUG_MSG("Reload!\n");
	}
}

float
weapon_bestdistance(WEAPON *weapon)
// returns best distance for currentweapon
{
	assert(weapon->modweapon != 0);
	return weapon->modweapon[weapon->currentweapon].bestDist;
}

const char *
weapon_name(WEAPON *weapon)
{
	return weapon->modweapon[weapon->currentweapon].name;
}

float
weapon_cone(WEAPON *weapon)
{
	return weapon->modweapon[weapon->currentweapon].cone;
}

void
weapon_setnextattacktime(WEAPON *weapon, float time)
{
	weapon->nextattacktime = time;
}

int
weapon_bestattackmode(WEAPON *weapon)
{
	return weapon->bestmode[weapon->currentweapon];
}

void
weapon_setattackmode(WEAPON *weapon, int mode)
{
	weapon->bestmode[weapon->currentweapon] = mode;
}
