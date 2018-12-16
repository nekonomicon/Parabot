#pragma once
#if !defined(PB_NEEDS_H)
#define PB_NEEDS_H

typedef struct needs {
	CParabot *bot;				// pointer to bot class
	float	wish[MAX_NAV_TYPES];// wish-values for different items
	float	maxwish;			// max. wish-value
	float	weaponwish;			// added wishes of all available weapons and ammo
	float	wishupdate;			// worldtime wishes are calculated again
	bool	newitempriorities;	// to be able to cancel current journey for other targets
	bool	haloknownonbase;	// used to set newItemPriorities
	bool	airstrikeknown;
} NEEDS;

void	needs_init(NEEDS *needs, CParabot *botClass);
void	needs_updatewishlist(NEEDS *needs);
float	needs_desirefor(NEEDS *needs, int navId);
float	needs_wishforhealth(NEEDS *needs);
float	needs_wishforarmor(NEEDS *needs);
float	needs_wishforitems(NEEDS *needs);
float	needs_wishforweapons(NEEDS *needs);
bool	needs_newpriorities(NEEDS *needs);
void	needs_affirmpriorities(NEEDS *needs);

float	needs_wishforcombat(NEEDS *needs);
// returns a value between 0 and 10 indicating the wish for enemy encounter

float	needs_wishforsniping(NEEDS *needs, bool weaponCheck);
/*	void valveWishList();
        void hwWishList();
        void dmcWishList();
        void gearboxWishList();
        void hungerWishList();
        void agWishList();
        void getWishList();
*/
#endif
