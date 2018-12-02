#pragma once
#if !defined(PB_WEAPON_H)
#define PB_WEAPON_H

// weapon structure
typedef struct {
	const char	*name;
	float	bestDist;
	float	cone;
	float	highAimProb;
	bool	secAmmo;
	float	volAttack1;
	float	volAttack2;
	float	visAttack1;
	float	visAttack2;
	float	fireDelay;
	const char	*shortName;
} tWeaponRec;

#define MAX_WEAPONS	32	// have to fit in 32-bit-flag

typedef struct weapon {
	EDICT		*botent;
	ACTION		*botaction;			// pointer to action-instace the bot is using
	tWeaponRec	*modweapon;			// pointer to the correct mod-weapons
	int		 currentweapon;			// holds the Weapon ID that all functions are working with
	int		 armedweapon;			// weapon the player has armed

	int		 botslot;			// slot the bot is using

	float		 nextattacktime;		// worldtime next attack can occur
	float		 lastattacktime;		// worldtime last attack was executed

	int		 bestmode[MAX_WEAPONS];		// if weapons have several modes, the best get
							// stored here in getWeaponScore()

	int		 minmodweapon, maxmodweapon;

	float		 grenadelaunchtime;		// when to throw/bomb
	Vec3D		 grenadetarget;			// for handgrenades to remember where to throw if forced to
	int		 grenadewid;			// holds wid if grenadePrepared

	float		 loadstarttime;
	bool		 reloading;			// true if current weapon is being reloaded
	bool		 grenadeprepared;		// for handgrenades and satchels
	bool		 loadinggauss;
} WEAPON;

// weapon ID values for Valve's Half-Life Deathmatch
#define MIN_VALVE_WEAPONS		   1

#define VALVE_WEAPON_CROWBAR       1
#define VALVE_WEAPON_GLOCK         2
#define VALVE_WEAPON_PYTHON        3
#define VALVE_WEAPON_MP5           4
#define VALVE_WEAPON_CHAINGUN      5
#define VALVE_WEAPON_CROSSBOW      6
#define VALVE_WEAPON_SHOTGUN       7
#define VALVE_WEAPON_RPG           8
#define VALVE_WEAPON_GAUSS         9
#define VALVE_WEAPON_EGON         10
#define VALVE_WEAPON_HORNETGUN    11
#define VALVE_WEAPON_HANDGRENADE  12
#define VALVE_WEAPON_TRIPMINE     13
#define VALVE_WEAPON_SATCHEL      14
#define VALVE_WEAPON_SNARK        15

#define MAX_VALVE_WEAPONS		  16


// weapon ID values for Valve's Team Fortress Classic & 1.5
#define MIN_TFC_WEAPONS			  1

#define TFC_WEAPON_UNKNOWN1       1
#define TFC_WEAPON_UNKNOWN2       2
#define TFC_WEAPON_MEDIKIT        3
#define TFC_WEAPON_SPANNER        4
#define TFC_WEAPON_AXE            5
#define TFC_WEAPON_SNIPERRIFLE    6
#define TFC_WEAPON_AUTORIFLE      7
#define TFC_WEAPON_SHOTGUN        8
#define TFC_WEAPON_SUPERSHOTGUN   9
#define TFC_WEAPON_NAILGUN       10
#define TFC_WEAPON_SUPERNAILGUN  11
#define TFC_WEAPON_GL            12
#define TFC_WEAPON_FLAMETHROWER  13
#define TFC_WEAPON_RPG           14
#define TFC_WEAPON_IC            15
#define TFC_WEAPON_UNKNOWN16     16
#define TFC_WEAPON_AC            17
#define TFC_WEAPON_UNKNOWN18     18
#define TFC_WEAPON_UNKNOWN19     19
#define TFC_WEAPON_TRANQ         20
#define TFC_WEAPON_RAILGUN       21
#define TFC_WEAPON_PL            22
#define TFC_WEAPON_KNIFE         23

#define MAX_TFC_WEAPONS			 24


// weapon ID values for HolyWars
#define MIN_HW_WEAPONS				15

#define HW_WEAPON_JACKHAMMER		15
#define HW_WEAPON_DOUBLESHOTGUN		16
#define HW_WEAPON_MACHINEGUN		17
#define HW_WEAPON_ROCKETLAUNCHER	18
#define HW_WEAPON_RAILGUN			20

#define MAX_HW_WEAPONS				21



// weapon ID values for DMC
#define MIN_DMC_WEAPONS				 0

#define DMC_WEAPON_CROWBAR			 0
#define DMC_WEAPON_QUAKEGUN			 1
#define DMC_WEAPON_SUPERSHOTGUN		 2
#define DMC_WEAPON_NAILGUN			 3
#define DMC_WEAPON_SUPERNAILGUN		 4
#define DMC_WEAPON_GRENLAUNCHER		 5
#define DMC_WEAPON_ROCKETLAUNCHER	 6
#define DMC_WEAPON_LIGHTNING	     7

#define MAX_DMC_WEAPONS			     8


// weapon ID values for Counter-Strike
#define MIN_CS_WEAPONS		     1

#define CS_WEAPON_P228           1
#define CS_WEAPON_SHIELDGUN       2
#define CS_WEAPON_SCOUT          3
#define CS_WEAPON_HEGRENADE      4
#define CS_WEAPON_XM1014         5
#define CS_WEAPON_C4             6
#define CS_WEAPON_MAC10          7
#define CS_WEAPON_AUG            8
#define CS_WEAPON_SMOKEGRENADE   9
#define CS_WEAPON_ELITE         10
#define CS_WEAPON_FIVESEVEN     11
#define CS_WEAPON_UMP45         12
#define CS_WEAPON_SG550         13
#define CS_WEAPON_GALIL      14
#define CS_WEAPON_FAMAS      15
#define CS_WEAPON_USP           16
#define CS_WEAPON_GLOCK18       17
#define CS_WEAPON_AWP           18
#define CS_WEAPON_MP5NAVY       19
#define CS_WEAPON_M249          20
#define CS_WEAPON_M3            21
#define CS_WEAPON_M4A1          22
#define CS_WEAPON_TMP           23
#define CS_WEAPON_G3SG1         24
#define CS_WEAPON_FLASHBANG     25
#define CS_WEAPON_DEAGLE        26
#define CS_WEAPON_SG552         27
#define CS_WEAPON_AK47          28
#define CS_WEAPON_KNIFE         29
#define CS_WEAPON_P90           30

#define MAX_CS_WEAPONS		    31


// weapon ID values for Gearbox's Opposing Force
#define MIN_GEARBOX_WEAPONS		   1
/*
#define VALVE_WEAPON_CROWBAR       1
#define VALVE_WEAPON_GLOCK         2
#define VALVE_WEAPON_PYTHON        3
#define VALVE_WEAPON_MP5           4
#define VALVE_WEAPON_CHAINGUN      5
#define VALVE_WEAPON_CROSSBOW      6
#define VALVE_WEAPON_SHOTGUN       7
#define VALVE_WEAPON_RPG           8
#define VALVE_WEAPON_GAUSS         9
#define VALVE_WEAPON_EGON         10
#define VALVE_WEAPON_HORNETGUN    11
#define VALVE_WEAPON_HANDGRENADE  12
#define VALVE_WEAPON_TRIPMINE     13
#define VALVE_WEAPON_SATCHEL      14
#define VALVE_WEAPON_SNARK        15
*/
#define GEARBOX_WEAPON_GRAPPLE			16
#define GEARBOX_WEAPON_EAGLE			17
#define GEARBOX_WEAPON_PIPEWRENCH		18
#define GEARBOX_WEAPON_M249				19
#define GEARBOX_WEAPON_DISPLACER		20
#define GEARBOX_WEAPON_SHOCKRIFLE		22
#define GEARBOX_WEAPON_SPORELAUNCHER	23
#define GEARBOX_WEAPON_SNIPERRIFLE		24
#define GEARBOX_WEAPON_KNIFE			25
#define GEARBOX_WEAPON_PENGUIN			26

#define MAX_GEARBOX_WEAPONS				27

// weapon ID values for They Hunger Trilogy
#define MIN_HUNGER_WEAPONS                1
/*
#define VALVE_WEAPON_CROWBAR       1
#define VALVE_WEAPON_GLOCK         2
#define VALVE_WEAPON_PYTHON        3
#define VALVE_WEAPON_MP5           4
#define VALVE_WEAPON_CHAINGUN      5
#define VALVE_WEAPON_CROSSBOW      6
#define VALVE_WEAPON_SHOTGUN       7
#define VALVE_WEAPON_RPG           8
#define VALVE_WEAPON_GAUSS         9
#define VALVE_WEAPON_EGON         10
#define VALVE_WEAPON_HORNETGUN    11
#define VALVE_WEAPON_HANDGRENADE  12
#define VALVE_WEAPON_TRIPMINE     13
#define VALVE_WEAPON_SATCHEL      14
#define VALVE_WEAPON_SNARK	15
*/
#define HUNGER_WEAPON_SPANNER	16
#define HUNGER_WEAPON_AP9	17
#define HUNGER_WEAPON_SHOVEL	18
#define HUNGER_WEAPON_SNIPER    19
#define HUNGER_WEAPON_TFCSNIPER	20
#define HUNGER_WEAPON_TAURUS	21
#define HUNGER_WEAPON_CHAINGUN	22
#define HUNGER_WEAPON_MEDKIT 	23

#define MAX_HUNGER_WEAPONS	24

// weapon flags
#define WF_UNDERWATER			(1<<0)
#define WF_FAST_ATTACK			(1<<1)
#define WF_IMMEDIATE_ATTACK		(1<<2)
#define WF_HIDE_ORIGIN			(1<<3)
#define WF_NEED_GRENADE			(1<<4)
#define WF_SINGLE_SHOT_KILL	    (1<<5)
#define WF_ENEMY_ABOVE			(1<<6)
#define WF_ENEMY_BELOW			(1<<7)

// define weapon advice flags
#define WA_EXACT_AIM			(1<<0)
#define WA_FIRE					(1<<1)
#define WA_CLOSE_UP				(1<<2)
#define WA_GAIN_DISTANCE		(1<<3)


#define CHANGE_WEAPON_DELAY		1.0	// time needed to switch between weapons

void		 weapon_construct(WEAPON *weapon);
void		 weapon_construct2(WEAPON *weapon, int wId);
// directly initializing currentWeapon

const char	*weapon_getweaponname(int wId);
// returns the short weapon name of weapon wId

void		 weapon_init(WEAPON *weapon, int slot, EDICT *ent, ACTION *action);
// has to be called with the botSlot before all other methods

float		 weapon_getscore(WEAPON *weapon, float distance, float hitProb, int flags, bool checkAmmo);
// returns the weapon-score for given situation

float		 weapon_getaudibledistance(WEAPON *weapon, int attackFlags);
// returns the audible distance

bool		 weapon_attack(WEAPON *weapon, Vec3D *target, float accuracy, Vec3D *relVel);
// attacks in best mode at best time the given position when accuracy is reached

bool		 weapon_hastofinishattack(WEAPON *weapon);
// true if bot has to finish e.g. a grenade attack

int		 weapon_armedgrenade(WEAPON *weapon);
// returns the weapon id that has to finish attack

void		 weapon_finishattack(WEAPON *weapon);
// throws grenade to avoid blasting up 

bool		 weapon_needreload(WEAPON *weapon);
// returns true if the current weapon needs to be reloaded

void		 weapon_reload(WEAPON *weapon);
// reloads weapon

float		 weapon_bestdistance(WEAPON *weapon);
// returns best distance for currentWeapon

int		 weapon_ammo1(WEAPON *weapon);
int		 weapon_ammo2(WEAPON *weapon);
float		 weapon_highaimprob(WEAPON *weapon);
// returns necessary aimProb for a high precision attack for current weapon

void		 weapon_setcurrentweapon(WEAPON *weapon, int wId);
void		 weapon_registerarmedweapon(WEAPON *weapon, int wId);
const char	*weapon_name(WEAPON *weapon);
// returns the weapon-name

float		 weapon_cone(WEAPON *weapon);
void		 weapon_setnextattacktime(WEAPON *weapon, float time);
int		 weapon_bestattackmode(WEAPON *weapon);
void		 weapon_setattackmode(WEAPON *weapon, int mode);

#endif
