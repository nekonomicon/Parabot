#pragma once
#if !defined(PB_OBSERVER_H)
#define PB_OBSERVER_H

#include "pb_global.h"
#include "marker.h"
#include <queue>

// max. observed players
#define MAX_OBS		32 // TODO: To remove stupid limits.
// max. bots
#define MAX_BOTS	32
// max. stored waypoints for following
#define MAX_WPTS	128

typedef struct observer {
	PB_Path_Waypoint waypoint[MAX_WPTS];
	PB_Path_Platform platinfo[MAX_WPTS];
	EDICT		*player;				// pointer to observed players
	int		 leadwaypoint;		// latest waypoint and plat of observed
	int		 lastplatid;			//   player, indices in waypoint-table
	EDICT		*platform;
	PB_Navpoint	*lastreachednav;
	Vec3D		 lastwppos;			// position where last waypoint was stored
	float		 lastwptime;
	float		 lastwpyaw;
	Vec3D		 lastframepos;		// position (for teleporters)
	Vec3D		 lastframevel;		// velocity (for teleporters)
	float		 health;
	int		 frags;
	float		 falltime;
	short		 currentcell;
	short		 lastcell;
	float		 lastcelltime;
	int		 partner;
	int		 currentwaypoint;
#if _DEBUG
	CMarker		 trail;
	std::queue<int>	 markerId;
#endif //_DEBUG
	bool		 active;		// true if observation runs
	bool		 jumppressed;
        bool		 usepressed;
        bool		 incombat;
} OBSERVER;

void			observer_init();
// initializes the observer before mapstart

void			observer_registerclients();
// registers all human clients currently connected and valid

void			observer_observeall();
// observes all human clients currently registered

int			observer_playerid(EDICT *player);
// returns the observer id of player
void			observer_reportpartner(int botId, int observerId);
// links botId with observerId

void			observer_addwaypoint(int observerId, Vec3D *pos, int action = 0, int col = 1);
// adds a waypoint for followers

PB_Path_Waypoint	observer_getnextwaypoint(int botId);
// returns the next waypoint to follow player for bot with id

void			observer_reportwaypointreached(int botId);

bool			observer_shouldfollow(int botId, EDICT *bot);
// returns true if bot should move towards partner

bool			observer_cannotfollow(int botId);
// if bot isn't succesful in following frees waypoints and returns true

bool			observer_partnerincombat(int botId);
// returns true if bots registered partner is involved in a combat

bool			observer_partnervalid(int botId);

#endif
