#pragma once
#if !defined(PB_WEAPON_HANDLING_H)
#define PB_WEAPON_HANDLING_H


#define CHANGE_WEAPON_DELAY		1.0	// time needed to switch between weapons

typedef struct weaponhandling {
	EDICT	*botent;
	ACTION	*botaction;			// pointer to action-instace the bot is using

	WEAPON	 weapon;
	int	 botslot;			// slot the bot is using
	int	 defaultweapon;
	int	 armedweapon;
	int	 preferredweapon;
	int	 preferredmode;
	float	 preferredweapontimeout;
	float	 lastmodeswitch;		// worldtime last change (zoom in/out) had ocurred
	
	int	 minmodweapon, maxmodweapon;
	bool	 weaponusable;
} WEAPONHANDLING;

void	weaponhandling_init(WEAPONHANDLING *wh, int slot, EDICT *ent, ACTION *action);
// has to be called with the botSlot before all other methods

void	weaponhandling_initcurrentweapon(WEAPONHANDLING *wh);
// call every frame

int	weaponhandling_currentweapon(WEAPONHANDLING *wh);
// returns the WeaponID of the currently handled weapon

bool	weaponhandling_available(WEAPONHANDLING *wh, int wId);
// returns true if bot can use weapon wId

bool	weaponhandling_attack(WEAPONHANDLING *wh, Vec3D *target, float accuracy, Vec3D *relVel);
// attacks in best mode at best time the given position when accuracy is reached

void	weaponhandling_checkforforcedattack(WEAPONHANDLING *wh);
int	weaponhandling_getbestweapon(WEAPONHANDLING *wh, float distance, float hitProb, int flags );
// returns the weapon-ID of best weapon for given situation

bool	weaponhandling_bestweaponusable(WEAPONHANDLING *wh);
// returns true if getBestWeapon could deliver a score > 0

bool	weaponhandling_armbestweapon(WEAPONHANDLING *wh, float distance, float hitProb, int flags);
// arms the best weapon for given situation, returns true if already armed before
	
void	weaponhandling_setpreferredweapon(WEAPONHANDLING *wh, int wId, int mode);
// sets the weapon that will be armed by armBestWeapon during the next 0.5 seconds

float	weaponhandling_getweaponscore(WEAPONHANDLING *wh, int wId, float distance, float hitProb, int flags, bool checkAmmo);
float	weaponhandling_bestdistance(WEAPONHANDLING *wh, int wId);
float	weaponhandling_currenthighaimprob(WEAPONHANDLING *wh);
#endif
