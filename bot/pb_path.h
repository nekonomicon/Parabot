#if !defined( PB_PATH_H )
#define PB_PATH_H


#include <list>
#include <vector>
//using namespace std;
#include "pb_navpoint.h"


//using namespace std;


#define MAX_PATH_WEIGHT	1000000


// path flags (specials)
#define PATH_NORMAL          0
#define PATH_ALL			 7
#define PATH_NEED_LONGJUMP   (1<<  0)
#define PATH_NEED_GAUSSJUMP  (1<<  1)
#define PATH_CAUSES_DAMAGE   (1<<  2)


// waypoint flags (higher 16 bits of act)
#define WP_DMG_OCURRED			(1<< 18)	// used to determine pathmode
#define WP_ON_PLATFORM			(1<< 19)
#define WP_ON_LADDER			(1<< 20)
#define WP_NOT_REACHABLE		(1<< 21)	// reached() always returns false
#define WP_IS_NAVPOINT			(1<< 22)	// marks a navpoint in the followers-trail
#define WP_NOT_INITIALIZED		(1<< 23)	// waypoint contains no valid data
#define WP_TELEPORTER_HINT		(1<< 24)	// marks a hint for a teleporter
#define WP_PLAT_NEEDS_TRIGGER	(1<< 25)	// only together with WP_ON_PLATFORM
#define WP_AT_PLATFORM_START	(1<< 26)	// only together with WP_ON_PLATFORM
#define WP_AT_PLATFORM_END		(1<< 27)	// only together with WP_ON_PLATFORM


// journey modes (influence on PB_Path::weight())
#define JOURNEY_FAST		0
#define JOURNEY_RELIABLE	1
#define JOURNEY_LONELY		2
#define JOURNEY_CROWDED		3

void setJourneyMode( int mode );


// extern method to get Navpoint data:
extern PB_Navpoint& getNavpoint( int index );	


class PB_Path_Waypoint 
{
public:
	typedef struct {
		Vector	pos;		// position
		int		act;		// action + flags
		float	arrival;	// time this waypoint has been reached (since start)
	} TSaveData;

	TSaveData data;
	
	PB_Path_Waypoint();
	PB_Path_Waypoint( Vector pos, int action=0, float arrival=0 );
	void reset() { data.act = WP_NOT_INITIALIZED; }
	bool valid() { return !(data.act & WP_NOT_INITIALIZED); }
	Vector pos() { return data.pos; }
	Vector pos( edict_t *ent );
	int action() { return (data.act & 0x0000FFFF); }
	bool isOnLadder() { if ((data.act & WP_ON_LADDER)>0) return true; return false;}
	bool isOnPlatform() { if ((data.act & WP_ON_PLATFORM)>0) return true; return false;}
	bool isAtPlatformStart() { if ((data.act & WP_AT_PLATFORM_START)>0) return true; return false;}
	bool isNavpoint()   { if ((data.act & WP_IS_NAVPOINT)>0) return true; return false;}
	bool needsTriggerForPlat()  { if ((data.act & WP_PLAT_NEEDS_TRIGGER)>0) return true; return false;}
	bool causedDamage() { if ((data.act & WP_DMG_OCURRED)>0) return true; return false;}

	bool reached( edict_t *ent );
	// returns true if waypoint is reached by ent

	bool operator==(const PB_Path_Waypoint& O) const  {  return data.arrival == O.data.arrival; }
	bool operator!=(const PB_Path_Waypoint& O) const  {  return data.arrival != O.data.arrival; }
	bool operator<(const PB_Path_Waypoint& O) const   {  return data.arrival <  O.data.arrival; }
	bool operator>(const PB_Path_Waypoint& O) const   {  return data.arrival >  O.data.arrival; }
};


class PB_Path_Attack 
{
public:
	typedef struct {
		Vector	pos;		// position of attacker
		float	time;		// time the attack ocurred (since start)
	} TSaveData;

	TSaveData data;

	bool operator==(const PB_Path_Attack& O) const  {  return data.time == O.data.time; }
	bool operator<(const PB_Path_Attack& O) const   {  return data.time <  O.data.time; }
};


class PB_Path_Platform
{
public:
	typedef struct {
		int		navId;		// ID of Platform 
		Vector	pos;		// position (abs_min) of navpoint entity
	} TSaveData;

	TSaveData data;

	PB_Path_Platform() {};
	PB_Path_Platform( int navId, Vector &pos ) { data.navId = navId;  data.pos = pos; }

	bool operator==(const PB_Path_Platform& O) const  {  return data.navId == O.data.navId; }
	bool operator<(const PB_Path_Platform& O) const   {  return data.navId <  O.data.navId; }
};


typedef std::list<PB_Path_Waypoint> WaypointList;
typedef std::vector<PB_Path_Attack> AttackList;	
typedef std::vector<PB_Path_Platform> PlatformList;



class PB_Path 
{

public:

	//--------------------------------------------------------------------
	//  INITIALIZATION METHODS
	
	PB_Path();
	
	~PB_Path();

	void initReturnOf( PB_Path &path );
	// initialize with swapped start and end point of path

	//--------------------------------------------------------------------
	//  OBSERVER METHODS

	void startRecord( int startPnt, float worldTime );
	// start recording a path from given point at given time

	void addWaypoint( PB_Path_Waypoint &wp );
	// adds the waypoint to the path

	void addWaypoint( Vector &pos, int action, float arrival );
	// adds the waypoint to the path

	void addPlatformInfo( int navId, Vector &pos );
	// adds platform info

	void stopRecord( int endPnt, float worldTime, int mode );
	// stops recording

	//--------------------------------------------------------------------
	//  BOT NAVIGATION METHODS

	void startAttempt( float worldTime );
	// start the path at given time

	void cancelAttempt();
	// cancels the pass, no reportTarget needs to be called

	void addAttack( Vector &origin );
	// stores the position of attack (inclusive actual position in path)

	void reportEnemySpotted() { data.enemyEncounters++; }
	// enemies spotted before they could attack the bot

	Vector getViewPos( edict_t *traveller, int &prior );
	// returns, depending on the position in the path, a place to look at for possible attackers
	// prior returns priotity to look at this place

	int getNextAction();
	// returns the action to perform at the next waypoint

	bool waitForPlatform();
	// return true if bot has to wait for a platform before continuing to the next waypoint

	Vector nextPlatformPos();
	// returns the next waypoint on a platform or (0,0,0) if no platform on path

	PB_Path_Waypoint getNextWaypoint();
	// returns the next waypoint

	Vector getLastWaypointPos( edict_t *playerEnt );
	// returns the position of the last waypoint reached

	void reportWaypointReached();
	// confirms the waypoint as reached, internally choose next waypoint

	void reportWaypointFailed();
	// message that the waypoint has not been reached in time, roll back possible changes

	void reportTargetReached( edict_t *traveller, float worldTime );
	// confirms path as finished, internally set new success-variables

	void reportTargetFailed();
	// confirms path as failed, internally set new success-variables

	bool finished( edict_t *traveller );
	// returns true if all waypoints have been confirmed and end-navpoint is reached by traveller
	
	bool timeOut( float worldTime );
	// returns true if the next waypoint could not be reached in time

	bool cannotBeContinued( edict_t *ent );
	// returns true if the path has to be canceled

	//--------------------------------------------------------------------
	//  UTILITY METHODS

	void clear();
	// deletes all path-data, cancels recording

	void markForDelete() { readyToDelete = true; }
	// path isn't saved with this flag set

	void makeIndependant() { data.dataId = data.privateId; }
	// data will be saved with this path

	void makeDependantOf( int id) { data.dataId = id; }
	// data will be saved with other path

	void load( FILE *fp );
	// init path from file

	void save( FILE *fp );
	// save path to file if not readyToDelete

	void print();
	// print path info using debugMsg()

	void mark();
	// mark all waypoints

	//--------------------------------------------------------------------
	//  DATA ACCESS METHODS

	int startId() { return data.startId; }
	// returns the start navpoint index

	int endId() { return data.endId; }
	// returns the end(target) navpoint index

	PB_Navpoint& startNav();
	// returns the start navpoint

	PB_Navpoint& endNav();
	// returns the end(target) navpoint

	void setId( int id ) { data.privateId = id; }

	int id() { return data.privateId; }

	int dataId() { return data.dataId; }

	int getSpecialId() { return specialId; }

	void setSpecialId( int sId ) { specialId = sId; }

	int mode() { return data.specials; }

	bool deleted() { return readyToDelete; }

	bool hasData() { return (data.dataId==data.privateId); }

	bool isRecording() { return (data.endId==-1); }

	float weight();
	// used by graph algo, returns weight of path depending of journeyMode

	bool valid( int mode ); // returns true if passable in mode

	// operators for stl algorithms
	bool operator==(const PB_Path& O) const  {  return data.privateId == O.data.privateId; }
	bool operator<(const PB_Path& O) const   {  return data.privateId <  O.data.privateId; }
	

// public because must be reachable from backward path

	WaypointList *waypoint;		// waypoint list
	AttackList	 *hiddenAttack;	// hidden attack list
	PlatformList *platformPos;	// position platforms have to take


//private:

	typedef struct {
		int		privateId;		// path ID
		int		dataId;			// path ID where waypoint- and hiddenAttack-data is stored
								// different from private Id when backward path
		int		startId, endId;	// start- and end-navpoint index
		int		specials;		// flags for special skills needed to pass
		float	passTime;		// average time needed to pass
		float	scheduledTime;	// time of player, needed for backwards calculation
		int		attempts;		// attempts of passing
		int		lastAttempt;	// last attempt of passing (value of mapGraph::passCount)
		int		successful;		// certainty factor = succesful passes / attempts
		float	lostHealth;		// sum of all health lost on this pass
		int		enemyEncounters;// number of enemy encounters on this path
	} TSaveData;

	TSaveData data;

	bool	readyToDelete;	// if true, path is not saved
	bool	forwardPass;	// 
	float	startTime;		// world-time of start
	int		specialId;		// navpoint index of e.g. platform this path leads over
							// temporary store
	WaypointList::iterator currentWaypoint, lastReachedWaypoint;
	PlatformList::iterator currentPlat, lastReachedPlat;

	edict_t *lastWaitingPlat;
	float	waitPlatEndTime;
	edict_t *ignoredPlat;
	
};

#endif
