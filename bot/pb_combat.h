#ifndef PB_COMBAT_H
#define PB_COMBAT_H


#include "pb_global.h"
#include "pb_roaming.h"
#include "pb_weaponhandling.h"
#include "pb_perception.h"


// combat reports
#define CR_NO_ENEMY			1
#define CR_ENGAGING			2
#define CR_RETREATING		3
#define CR_ENEMY_TOO_STRONG	4


#define CHECK_WEAPON_COMBAT		0.5	// intervall in which to check for best weapon
#define CHECK_WEAPON_IDLE		3	// the same for idle


class PB_Combat
{

public:

	void init( int slot, edict_t *ent, PB_Action *act, PB_Roaming *pFinder );
	// initializes all necessary variables

	float getRating( PB_Percept &perceipt );
	// rates importance of enemy

	bool shootAtEnemy( Vector enemyOrigin, float accuracy );
	// picks best place to shoot at and fires

	bool shootAtEnemy( edict_t *enemy, float accuracy );
	// picks best place to shoot at and fires

	void closeCombatMovement( PB_Percept &perceipt );

	Vector evade( PB_Percept &perceipt );

	void retreat( edict_t *enemy );
	// flees from the enemy
	
	void idleActions();
	// manages weapon actions when no enemy is around

	bool hasWeapon( int wId ) { return weapon.available( wId ); }

	
	float nextWeaponCheck;	// worldTime next armBestWeapon should be called
	PB_WeaponHandling	weapon;


private:
		
	edict_t	 *botEnt;
	PB_Action *action;
	PB_Roaming *pathfinder;
	float		enemyContact;		// worldtime enemy has been registered
	
	int strafeState;			
	float nextStrafe, nextJump;

	bool closeUp, gainDistance;

	void classifyDistance( float dist );

};

#endif