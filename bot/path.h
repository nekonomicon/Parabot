#pragma once
#if !defined( PB_PATH_H )
#define PB_PATH_H
#include <list>
#include <vector>
#include "navpoint.h"

#define MAX_PATH_WEIGHT	1000000

// path flags (specials)
enum {
	PATH_NORMAL = 0,
	PATH_NEED_LONGJUMP =	BIT(0),
	PATH_NEED_GAUSSJUMP =	BIT(1),
	PATH_CAUSES_DAMAGE =	BIT(2),
	PATH_ALL = (PATH_NEED_LONGJUMP|PATH_NEED_GAUSSJUMP|PATH_CAUSES_DAMAGE)
};

// waypoint flags (higher 16 bits of act)
enum {
	WP_DMG_OCURRED =	BIT(18),	// used to determine pathmode
	WP_ON_PLATFORM =	BIT(19),
	WP_ON_LADDER =		BIT(20),
	WP_NOT_REACHABLE =	BIT(21),	// reached() always returns false
	WP_IS_NAVPOINT =	BIT(22),	// marks a navpoint in the followers-trail
	WP_NOT_INITIALIZED =	BIT(23),	// waypoint contains no valid data
	WP_TELEPORTER_HINT =	BIT(24),	// marks a hint for a teleporter
	WP_PLAT_NEEDS_TRIGGER =	BIT(25),	// only together with WP_ON_PLATFORM
	WP_AT_PLATFORM_START =	BIT(26),	// only together with WP_ON_PLATFORM
	WP_AT_PLATFORM_END =	BIT(27)		// only together with WP_ON_PLATFORM
};

// journey modes (influence on PB_Path::weight())
enum {
	JOURNEY_FAST,
	JOURNEY_RELIABLE,
	JOURNEY_LONELY,
	JOURNEY_CROWDED
};

void setJourneyMode( int mode );


// extern method to get Navpoint data:
extern NAVPOINT* getNavpoint( int index );	


typedef struct path_waypoint {
	Vec3D	pos;		// position
	int	act;		// action + flags
	float	arrival;	// time this waypoint has been reached (since start)
} PATH_WAYPOINT;

void	 path_waypoint_reset(PATH_WAYPOINT *wp);
bool	 path_waypoint_valid(PATH_WAYPOINT *wp);
Vec3D	*path_waypoint_pos(PATH_WAYPOINT *wp);
Vec3D	*path_waypoint_pos(PATH_WAYPOINT *wp, EDICT *ent);
int	 path_waypoint_action(PATH_WAYPOINT *wp);
bool	 path_waypoint_isonladder(PATH_WAYPOINT *wp);
bool	 path_waypoint_isonplatform(PATH_WAYPOINT *wp);
bool	 path_waypoint_isatplatformstart(PATH_WAYPOINT *wp);
bool	 path_waypoint_isnavpoint(PATH_WAYPOINT *wp);
bool	 path_waypoint_needstriggerforplat(PATH_WAYPOINT *wp);
bool	 path_waypoint_causeddamage(PATH_WAYPOINT *wp);
bool	 path_waypoint_reached(PATH_WAYPOINT *wp, EDICT *ent);
// returns true if waypoint is reached by ent

typedef struct path_attack {
	Vec3D	pos;		// position of attacker
	float	time;		// time the attack ocurred (since start)
} PATH_ATTACK;

typedef struct path_platform {
	int	navid;		// ID of Platform 
	Vec3D	pos;		// position (abs_min) of navpoint entity
} PATH_PLATFORM;

typedef std::list<PATH_WAYPOINT> WaypointList;
typedef std::vector<PATH_ATTACK> AttackList;	
typedef std::vector<PATH_PLATFORM> PlatformList;

typedef struct path_savedata {
	int	privateid;	// path ID
	int	dataid;		// path ID where waypoint- and hiddenAttack-data is stored
				// different from private Id when backward path
	int	startid, endid;	// start- and end-navpoint index
	int	specials;	// flags for special skills needed to pass
	float	passtime;	// average time needed to pass
	float	scheduledtime;	// time of player, needed for backwards calculation
	int	attempts;	// attempts of passing
	int	lastattempt;	// last attempt of passing (value of mapGraph::passCount)
	int	successful;	// certainty factor = succesful passes / attempts
	float	losthealth;	// sum of all health lost on this pass
	int	enemyencounters;// number of enemy encounters on this path
} PATH_SAVEDATA;

typedef struct path {
	WaypointList	*waypoint;		// waypoint list
	AttackList	*hiddenattack;	// hidden attack list
	PlatformList	*platformpos;	// position platforms have to take
	EDICT		*lastwaitingplat;
	EDICT		*ignoredplat;
	PATH_SAVEDATA	 data;

	float		 starttime;		// world-time of start
	int		 specialid;		// navpoint index of e.g. platform this path leads over
							// temporary store
	WaypointList::iterator	currentwaypoint, lastreachedwaypoint;
	PlatformList::iterator	currentplat, lastreachedplat;

	float		 waitplatendtime;
	bool		 readytodelete;	// if true, path is not saved
	bool		 forwardpass;	//
} PATH;

//--------------------------------------------------------------------
//  INITIALIZATION METHODS

void	path_initreturnof(PATH *newpath, PATH *oldpath);
// initialize with swapped start and end point of path

//--------------------------------------------------------------------
//  OBSERVER METHODS

void	path_startrecord(PATH *path, int startPnt, float worldtime);
// start recording a path from given point at given time

void	path_addwaypoint(PATH *path, PATH_WAYPOINT *wp);
// adds the waypoint to the path

void	path_addwaypoint(PATH *path, Vec3D *pos, int action, float arrival);
// adds the waypoint to the path

void	path_addplatforminfo(PATH *path, int navId, Vec3D *pos);
// adds platform info

void	path_stoprecord(PATH *path, int endPnt, float worldtime, int mode);
// stops recording

//--------------------------------------------------------------------
//  BOT NAVIGATION METHODS

void	path_startattempt(PATH *path, float worldtime);
// start the path at given time

void	path_cancelattempt(PATH *path);
// cancels the pass, no reportTarget needs to be called

void	path_addattack(PATH *path, Vec3D *origin);
// stores the position of attack (inclusive actual position in path)

void	path_reportenemyspotted(PATH *path);
// enemies spotted before they could attack the bot

void	path_getviewpos(PATH *path, EDICT *traveller, int *prior, Vec3D *pos);
// returns, depending on the position in the path, a place to look at for possible attackers
// prior returns priotity to look at this place

int	path_getnextaction(PATH *path);
// returns the action to perform at the next waypoint

bool	path_waitforplatform(PATH *path);
// return true if bot has to wait for a platform before continuing to the next waypoint

void	path_nextplatformpos(PATH *path, Vec3D *pos);
// returns the next waypoint on a platform or null vector if no platform on path

void	path_getnextwaypoint(PATH *path, PATH_WAYPOINT *wp);
// returns the next waypoint

void	path_getlastwaypointpos(PATH *path, EDICT *playerEnt, Vec3D *pos);
// returns the position of the last waypoint reached

void	path_reportwaypointreached(PATH *path);
// confirms the waypoint as reached, internally choose next waypoint

// void	path_reportwaypointfailed(PATH *path);
// message that the waypoint has not been reached in time, roll back possible changes

void	path_reporttargetreached(PATH *path, EDICT *traveller, float worldtime);
// confirms path as finished, internally set new success-variables

void	path_reporttargetfailed(PATH *path);
// confirms path as failed, internally set new success-variables

bool	path_finished(PATH *path, EDICT *traveller);
// returns true if all waypoints have been confirmed and end-navpoint is reached by traveller

bool	path_timeout(PATH *path, float worldtime);
// returns true if the next waypoint could not be reached in time

bool	path_cannotbecontinued(PATH *path, EDICT *ent);
// returns true if the path has to be canceled

//--------------------------------------------------------------------
//  UTILITY METHODS

void	path_clear(PATH *path);
// deletes all path-data, cancels recording

void	path_markfordelete(PATH *path);
// path isn't saved with this flag set

void	path_makeindependant(PATH *path);
// data will be saved with this path

void	path_makedependantof(PATH *path, int id);
// data will be saved with other path

void	path_load(PATH *path, FILE *fp);
// init path from file

void	path_save(PATH *path, FILE *fp);
// save path to file if not readyToDelete

#if _DEBUG
void	path_print(PATH *path);
// print path info using DEBUG_MSG()

void	path_mark(PATH *path);
// mark all waypoints
#endif
//--------------------------------------------------------------------
//  DATA ACCESS METHODS

int	path_startid(PATH *path);
// returns the start navpoint index

int	path_endid(PATH *path);
// returns the end(target) navpoint index

NAVPOINT	*path_startnav(PATH *path);
// returns the start navpoint

NAVPOINT	*path_endnav(PATH *path);
// returns the end(target) navpoint

void	path_setid(PATH *path, int id);

int	path_id(PATH *path);

int	path_dataid(PATH *path);

int	path_getspecialid(PATH *path);

void	path_setspecialid(PATH *path, int sId);

int	path_mode(PATH *path);

bool	path_deleted(PATH *path);

bool	path_hasdata(PATH *path);

bool	path_isrecording(PATH *path);

float	path_weight(PATH *path);
// used by graph algo, returns weight of path depending of journeyMode

bool	path_valid(PATH *path, int mode); // returns true if passable in mode

#endif
