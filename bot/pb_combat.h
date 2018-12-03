#ifndef PB_COMBAT_H
#define PB_COMBAT_H


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

	void init( int slot, EDICT *ent, ACTION *act, ROAMING *pFinder );
	// initializes all necessary variables

	float getRating( PB_Percept &perceipt );
	// rates importance of enemy

	bool shootAtEnemy( Vec3D *enemyOrigin, float accuracy );
	// picks best place to shoot at and fires

	bool shootAtEnemy( EDICT *enemy, float accuracy );
	// picks best place to shoot at and fires

	void closeCombatMovement( PB_Percept &perceipt );

	void evade(PB_Percept &perceipt, Vec3D *right);

	void retreat( EDICT *enemy );
	// flees from the enemy
	
	void idleActions();
	// manages weapon actions when no enemy is around

	bool hasWeapon( int wId ) { return weaponhandling_available(&weapon, wId); }

	
	float nextWeaponCheck;	// worldtime next armBestWeapon should be called
	WEAPONHANDLING	weapon;


private:
	EDICT	 *botEnt;
	ACTION	*action;
	ROAMING *pathfinder;
	float		enemyContact;		// worldtime enemy has been registered
	
	int strafeState;			
	float nextStrafe, nextJump;

	bool closeUp, gainDistance;

	// void classifyDistance( float dist );

};

#endif
