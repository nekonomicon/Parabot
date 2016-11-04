#ifndef PARABOT_H
#define PARABOT_H

#include "pb_global.h"
#include "pb_mapgraph.h"
#include "pb_action.h"
#include "pb_roaming.h"
#include "pb_combat.h"
#include "pb_perception.h"
#include "pb_goalfinder.h"
#include "pb_needs.h"




#define PB_NO_TASK				0
#define PB_ROAMING				1
#define PB_LOADING				2
#define PB_CAMPING				3
#define PB_ON_TOUR				4
#define PB_IN_COMBAT			5
#define PB_FOLLOW_AND_ASSIST	6

#define PB_REACH_DISTANCE 40	// 40 units near target = reached
#define PB_ROAMING_COUNT  100	// get new target every 5 seconds



class CParabot
{


//private:
public:

	float lastThink;
	float campTime, lastCamp;
	float makeRoomTime;
	bool preemptiveFire;
	int preemptiveWeapon, preemptiveMode;
	float lastRespawn;

	char		goalMove[80], goalView[80], goalAct[80];	// set when goals are executed

	// used in ON_TOUR
	PB_Journey	actualJourney;		// journey bot is traveling
	PB_Path		*actualPath;		// path bot is currently on
	PB_Path_Waypoint waypoint;		// waypoint bot is approaching
	PB_Navpoint	*actualNavpoint;	// navpoint bot is on, 0 if not on any navpoint
	PB_Needs	needs;
	bool		stoppedForPlat;
	bool		mustShootObject;
	Vector		shootObjectPos;
	edict_t		*huntingFor;		// points to edict of enemy bot is hunting for or 0
	edict_t		*fleeingFrom;		// points to edict of enemy bot is fleeing from or 0

	// used in ROAMING
	PB_Roaming	pathfinder;			// old bot pathfinding
	PB_Navpoint	*roamingTarget;		// navpoint bot tries to reach in roaming-mode
	int			roamingCount;		// time-out variable for updating roamingTarget
	short		roamingRoute[128];	// contains cell indices to target
	int			roamingIndex;		// current target index in route
	int			roamingBreak;		// after reaching this index a new route is calculated
	Vector		lastJumpPos;
	float		cellTimeOut;
	short		cellToReach;

	// used in FOLLOW_AND_ASSIST
	EHANDLE		partner;			// player the bot is following
	
	// used in IN_COMBAT
	PB_Combat	combat;
	
	// Perception module
	PB_Perception senses;

	// GoalFinder module
	PB_GoalFinder goalFinder;

	// character variables
	float	aggression;
	int		chatRate;

	// state variables
	int			botState;			// actual state (Wandering, In_Combat, ...)
	
	int			slot;				// slot in bots[] this bot is using
	int			team;				// team the bot is in
	edict_t		*ent;				// pointer to bot entity

	// MOD variables
	
	
	void executeGoals();

	bool positionReached( Vector pos );
	// returns true if bot has reached pos within a distance of PB_REACH_DISTANCE
	void pathFinished();
	void pathFailed();
	void pathCheckWay();
	// do 2 traces and modify action if jump/duck-nessecity

	void checkForTripmines();
	
	bool hasLongJump();
	bool getJourneyTarget();
	void getRoamingTarget();
	void setRoamingIndex( int x );


public:

	PB_Action	 action;

	CParabot( edict_t *botEnt, int botSlot );
	~CParabot();
	void setAggression( int agr ) { aggression = ((float)agr)/2; }
	void setCommunication( int comm ) { chatRate = comm; }
	void initAfterRespawn();
	// initializes bot after respawn
	void botThink();
	Vector botPos() { return ent->v.origin; }
	

	
	void followActualPath();
	void approachRoamingTarget();
	void followActualRoute();

	void registerDamage( int amount, Vector origin, int type );
	void registerDeath( edict_t *killer, const char *wpnName );
	void registerKill( edict_t *victim, const char *wpnName );
	void reportEnemySpotted();

	void setGoalViewDescr( char *descr );
	void setGoalMoveDescr( char *descr );
	void setGoalActDescr( char *descr );

};


int UTIL_GetTeam(edict_t *pEntity);


#endif // PARABOT_H