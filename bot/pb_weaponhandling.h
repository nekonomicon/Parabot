#if !defined( PB_WEAPON_HANDLING_H )
#define PB_WEAPON_HANDLING_H


#include "pb_weapon.h"


#define CHANGE_WEAPON_DELAY		1.0	// time needed to switch between weapons



class PB_WeaponHandling
{
	
public:
	
	PB_WeaponHandling();
	
	void init( int slot, edict_t *ent, PB_Action *action );
	// has to be called with the botSlot before all other methods

	void initCurrentWeapon();
	// call every frame

	int currentWeapon();
	// returns the WeaponID of the currently handled weapon

	bool available( int wId );
	// returns true if bot can use weapon wId

	bool attack( Vector target, float accuracy, Vector relVel = Vector(0,0,0) );
	// attacks in best mode at best time the given position when accuracy is reached

	void checkForForcedAttack();

	int getBestWeapon( float distance, float hitProb, int flags );
	// returns the weapon-ID of best weapon for given situation

	bool bestWeaponUsable() { return weaponUsable; }
	// returns true if getBestWeapon could deliver a score > 0

	bool armBestWeapon( float distance, float hitProb, int flags );
	// arms the best weapon for given situation, returns true if already armed before
	
	void setPreferredWeapon( int wId, int mode=1 );
	// sets the weapon that will be armed by armBestWeapon during the next 0.5 seconds

	float getWeaponScore( int wId, float distance, float hitProb, int flags, bool checkAmmo );

	float bestDistance( int wId );

	float currentHighAimProb();



private:

	int			botSlot;				// slot the bot is using
	edict_t		*botEnt;
	PB_Action	*botAction;				// pointer to action-instace the bot is using

	PB_Weapon	weapon;
	int			defaultWeapon;
	int			armedWeapon;
	int			preferredWeapon;
	int			preferredMode;
	float		preferredWeaponTimeOut;
	float		lastModeSwitch;			// worldTime last change (zoom in/out) had ocurred
	
	bool		weaponUsable;

	int			minModWeapon, maxModWeapon;
	


	void switchToWeapon( int wId );

};

#endif