#pragma once
#if !defined(PB_COMBAT_H)
#define PB_COMBAT_H

// combat reports
enum {
	CR_NO_ENEMY = 1,
	CR_ENGAGING,
	CR_RETREATING,
	CR_ENEMY_TOO_STRONG
};

#define CHECK_WEAPON_COMBAT		0.5	// intervall in which to check for best weapon
#define CHECK_WEAPON_IDLE		3	// the same for idle

typedef struct combat {
	EDICT		*botent;
	ACTION		*action;
	ROAMING		*pathfinder;
	WEAPONHANDLING	 weapon;
	float		 nextweaponcheck;	// worldtime next armBestWeapon should be called
	float		 enemycontact;		// worldtime enemy has been registered
	int		 strafestate;
	float		 nextstrafe, nextjump;
	bool		 closeup, gaindistance;
} COMBAT;

void	combat_init(COMBAT *combat, int slot, EDICT *ent, ACTION *act, ROAMING *pFinder);
// initializes all necessary variables

float	combat_getrating(COMBAT *combat, PB_Percept &perceipt);
// rates importance of enemy

bool	combat_shootatenemy(COMBAT *combat, Vec3D *enemyOrigin, float accuracy);
// picks best place to shoot at and fires

bool	combat_shootatenemy2(COMBAT *combat, EDICT *enemy, float accuracy);
// picks best place to shoot at and fires

// void	combat_closecombatmovement(COMBAT *combat, PB_Percept &perceipt);

// void	combat_evade(COMBAT *combat, PB_Percept &perceipt, Vec3D *right);

// void	combat_retreat(COMBAT *combat, EDICT *enemy);
// flees from the enemy

// void	combat_idleactions(COMBAT *combat);
// manages weapon actions when no enemy is around

bool	combat_hasweapon(COMBAT *combat, int wId);

#endif
