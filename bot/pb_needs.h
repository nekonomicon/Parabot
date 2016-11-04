#ifndef PB_NEEDS_H
#define PB_NEEDS_H



#include "pb_navpoint.h"
class CParabot;


class PB_Needs
{	

public:
	
	void init( CParabot *botClass );
	void updateWishList();
	float desireFor( int navId ) { return wish[navId]; }
	float needForHealth();
	float needForArmor();
	float needForItems()	{ return maxWish; }
	float needForWeapons()	{ return weaponWish; }
	bool newPriorities()	{ return newItemPriorities; }
	void affirmPriorities() { newItemPriorities = false; }

	float wishForCombat();
	// returns a value between 0 and 10 indicating the wish for enemy encounter

	float wishForSniping( bool weaponCheck=true );
	


protected:

	

	void valveWishList();
	void hwWishList();
	void dmcWishList();
	void gearboxWishList();
	void getWishList();

	float	wish[MAX_NAV_TYPES];// wish-values for different items
	float	maxWish;			// max. wish-value
	float	weaponWish;			// added wishes of all available weapons and ammo
	float	wishUpdate;			// worldTime wishes are calculated again
	bool	newItemPriorities;	// to be able to cancel current journey for other targets
	bool	haloKnownOnBase;	// used to set newItemPriorities
	bool	airstrikeKnown;

	CParabot *bot;				// pointer to bot class

};

#endif