#ifndef PB_OBSERVER_H
#define PB_OBSERVER_H


#include "pb_global.h"
#include "marker.h"
#include <queue>


// max. observed players
#define MAX_OBS		32
// max. bots
#define MAX_BOTS	32
// max. stored waypoints for following
#define MAX_WPTS	128


class PB_Observer
{


public:

	PB_Observer();
	// calls init()

	~PB_Observer();

	void init();
	// initializes the observer before mapstart
	
	void registerClients();
	// registers all human clients currently connected and valid

	void observeAll();
	// observes all human clients currently registered

	int playerId( edict_t *player );
	// returns the observer id of player

	void reportPartner( int botId, int observerId );
	// links botId with observerId

	void addWaypoint( int observerId, Vector pos, int action = 0, int col = 1 );
	// adds a waypoint for followers

	PB_Path_Waypoint getNextWaypoint( int botId );
	// returns the next waypoint to follow player for bot with id

	void reportWaypointReached( int botId );

	bool shouldFollow( int botId, edict_t *bot );
	// returns true if bot should move towards partner

	bool canNotFollow( int botId );
	// if bot isn't succesful in following frees waypoints and returns true

	bool partnerInCombat( int botId );
	// returns true if bots registered partner is involved in a combat

	bool partnerValid( int botId );


private:

	typedef struct {
		EHANDLE		player;				// pointer to observed players
		bool		active;				// true if observation runs
		int			leadWaypoint;		// latest waypoint and plat of observed
		int			lastPlatId;			//   player, indices in waypoint-table
		edict_t		*platform;
		PB_Navpoint *lastReachedNav;
		Vector		lastWpPos;			// position where last waypoint was stored
		float		lastWpTime;
		float		lastWpYaw;
		Vector		lastFramePos;		// position (for teleporters)
		Vector		lastFrameVel;		// velocity (for teleporters)
		float		health;
		int			frags;
		bool		jumpPressed;
		bool		usePressed;
		bool		inCombat;
		float		fallTime;
		short		currentCell;
		short		lastCell;
		float		lastCellTime;
	} tObserverData;

	tObserverData	obs[MAX_OBS];
	
	PB_Path_Waypoint waypoint[MAX_OBS][MAX_WPTS];	// waypoint lists to follow players
	PB_Path_Platform platInfo[MAX_OBS][MAX_WPTS];
	
	
	int			partner[MAX_BOTS];			// stores botpartner
	int			currentWaypoint[MAX_BOTS];	// index in waypoint-table

	CMarker		trail[MAX_OBS];
	std::queue<int>	markerId[MAX_OBS];


	int registerPlayer( edict_t *player );	
	// registers the player for observation, observerId is returned

	void clear( int oId );
	// clears all lists for observer oId

	void startObservation( int oId );

	int checkGround( int oId, edict_t **plat );
	// returns WP_ON_LADDER and WP_ON_PLATFORM if necessary

	bool shouldObservePlayer( int oId );

	void checkForJump( int oId, Vector &pos );

	void checkForUse( int oId, Vector &pos );

	void checkForMove( int oId, Vector &pos );

	void checkForCamping( int oId, Vector &pos );

	void checkForTripmines( int oId, Vector &pos );

	void checkForButtonShot( int oId, Vector &pos );

	void checkPlayerHealth( int oId );

	int getStartIndex( int oId, PB_Navpoint *endNav );

	void newNavpointReached( int oId, Vector &pos, PB_Navpoint *endNav );

	void updateCellInfo( int i );
	
};

#endif