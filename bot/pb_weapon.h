#if !defined( PB_WEAPON_H )
#define PB_WEAPON_H


#include "pb_action.h"


// weapon structure
typedef struct {
	char	name[64];
	float	bestDist;
	float	cone;
	float	highAimProb;
	bool	secAmmo;
	float	volAttack1;
	float	volAttack2;
	float	visAttack1;
	float	visAttack2;
	float	fireDelay;
	char	shortName[32];
} tWeaponRec;


#define MAX_WEAPONS 32	// have to fit in 32-bit-flag

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
#define CS_WEAPON_UNKNOWN1       2
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
#define CS_WEAPON_UNKNOWN6      14
#define CS_WEAPON_UNKNOWN7      15
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



const char* getWeaponName( int wId );
// returns the short weapon name of weapon wId


class PB_Weapon
{
	
public:
	
	PB_Weapon();

	PB_Weapon( int wId );
	// directly initializing currentWeapon
	
	void init( int slot, edict_t *ent, PB_Action *action );
	// has to be called with the botSlot before all other methods

	float getScore( float distance, float hitProb, int flags, bool checkAmmo );
	// returns the weapon-score for given situation

	float getAudibleDistance( int attackFlags );
	// returns the audible distance

	bool attack( Vector target, float accuracy, Vector relVel = Vector(0,0,0) );
	// attacks in best mode at best time the given position when accuracy is reached

	bool hasToFinishAttack() { return grenadePrepared; }
	// true if bot has to finish e.g. a grenade attack

	int armedGrenade() { return grenadeWId; }
	// returns the weapon id that has to finish attack

	void finishAttack();
	// throws grenade to avoid blasting up 

	bool needReload();
	// returns true if the current weapon needs to be reloaded

	void reload();
	// reloads weapon

	float bestDistance();
	// returns best distance for currentWeapon

	int ammo1();

	int ammo2();

	float highAimProb();
	// returns necessary aimProb for a high precision attack for current weapon

	void setCurrentWeapon( int wId );
	
	void registerArmedWeapon( int wId );

	char* name() {  return ( (char*)&modWeapon[currentWeapon].name );  }
	// returns the weapon-name

	float cone()	{ return modWeapon[currentWeapon].cone; }
	void setNextAttackTime( float time ) {	nextAttackTime = time; }

	int bestAttackMode() {  return bestMode[currentWeapon];  }

	void setAttackMode( int mode )  {  bestMode[currentWeapon] = mode;  }



private:

	int currentWeapon;			// holds the Weapon ID that all functions are working with
	int armedWeapon;			// weapon the player has armed

	int			botSlot;		// slot the bot is using
	edict_t		*botEnt;
	PB_Action	*botAction;		// pointer to action-instace the bot is using

	float	nextAttackTime;		// worldTime next attack can occur
	float	lastAttackTime;		// worldTime last attack was executed
	bool	reloading;			// true if current weapon is being reloaded

	int bestMode[MAX_WEAPONS];	// if weapons have several modes, the best get
								// stored here in getWeaponScore()
	
	tWeaponRec *modWeapon;		// pointer to the correct mod-weapons
	int minModWeapon, maxModWeapon;
	

	bool grenadePrepared;		// for handgrenades and satchels
	float grenadeLaunchTime;	// when to throw/bomb
	Vector grenadeTarget;		// for handgrenades to remember where to throw if forced to
	int grenadeWId;				// holds wid if grenadePrepared

	bool loadingGauss;
	float loadStartTime;

	// private methods:

	void initMOD();
	float valveWeaponScore( float distance, float hitProb, int flags, bool checkAmmo );
	float hwWeaponScore( float distance, float hitProb, int flags, bool checkAmmo );
	float dmcWeaponScore( float distance, float hitProb, int flags, bool checkAmmo );
	float csWeaponScore( float distance, float hitProb, int flags, bool checkAmmo );
	float tfcWeaponScore( float distance, float hitProb, int flags, bool checkAmmo );
	float gearboxWeaponScore( float distance, float hitProb, int flags, bool checkAmmo );
	
	bool attackValveHandgrenade( Vector &target );
	bool attackValveSatchel( Vector &target );
};

#endif