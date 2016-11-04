#ifndef PB_PERCEPTION_H
#define PB_PERCEPTION_H

#pragma warning( disable : 4786 )	// disable stl warnings
#pragma warning( disable : 4800 )	// disable int->bool warnings


#include "pb_global.h"
#include <list>


// perception-item classification
#define PI_PLAYER		1	// unidentified player in team-game
#define PI_FRIEND		2	// teammate in team-game
#define PI_FOE			3	// enemy
#define PI_HOSTAGE		4	// CS
#define PI_BOMB			5	// CS
#define PI_WEAPONBOX	6	// weaponbox
#define PI_EXPLOSIVE	7	// grenade, satchel
#define PI_LASERDOT		8	// RPG laserdot
#define PI_TRIPMINE		9	// Beam-Tripmine
#define PI_HALO			10	// HW
#define PI_DAMAGE		11	// damage perception
#define PI_SNARK		12	// snark
#define PI_NEWAREA		13	// a big area that bot hasn't seen before


#define MAX_PERCEPTION	13

// perception-item state
#define PI_VISIBLE		(1<<0)	// is seen
#define PI_AUDIBLE		(1<<1)	// is heard but can't be tracked
#define PI_TRACKABLE	(1<<2)	// is heard and trackable
#define PI_TACTILE		(1<<3)	// is attacking
#define PI_PREDICTED	(1<<4)	// has been seen and is predicted now
#define PI_ORIG_KNOWN	(1<<5)	// for damage: origin/inflictor is known

#define PI_UNKNOWN		-1	// unknown model index

// flags
#define PI_BEST_ARMED		(1<<0)	// best weapon has been armed for that enemy
#define PI_ALERT			(1<<1)	// enemy knows that bot has seen it
#define PI_HIGH_PRIORITY	(1<<2)	// e.g. the Saint in HW
#define PI_FOCUS1			(1<<3)	// bot is attacking this frame
#define PI_FOCUS2			(1<<4)	// bot is attacking last frame
#define PI_UNREACHABLE		(1<<5)	// bot can't reach this item by roaming
#define PI_DISAPPEARED		(1<<6)	// item has just become not visible
#define PI_PREEMPTIVE		(1<<7)	// bot uses preemptive fire for that enemy

// distances
#define MAX_DIST_VP		3000	// max. visible player detection distance
#define MAX_DIST_VPR	 800	// max. visible player recognition distance
#define MAX_DIST_VI		1200	// max. visible item detection distance

#define UNKNOWN_POS		Vector(0,0,0)



class PB_Percept 
{
public:
	
	int		id;
	float	botSensitivity;		// sensitivity of bot that perceived item
	edict_t *entity;			// pointer to edict
	short	pClass;				// classification of item
	short	pState;				// kind of perception
	int		model;				// model index of item
	float	update;				// worldtime when goal-queue should be updated
	float	firstDetection;		// worldTime first detection was registered
	float	lastDetection;		// worldTime last detection was registered
	float	lastSeenTime;		// worldTime last visible contact was registered
	Vector	lastPos;			// last observed position (seen or heard)
	Vector	lastSeenPos;		// last seen position
	Vector	lastSeenVelocity;	// last seen velocity

	// for enemy:
	float	rating;				// weapon-rating
	float	orientation;		// dotproduct if item is facing the bot
	float	distance;			// distance to bot
	int		flags;
	Vector	predTarget;			// position the item is supposed to reach
	float	lastCalcTarget;		// last worldtime target was predicted
	Vector	predAppearance;		// position where the item is supposed to show up
	float	lastCalcAppearance;	// last worldtime appearance was predicted

	//PB_Percept() {}
	PB_Percept( float botSens, edict_t *ent, short state, short realClass, float dist );

	float getReactionTime( edict_t *ent, short state, short realClass, float dist );
	// returns the time needed to react to the item (time to enter goal-queue)

	Vector predictedPosition( Vector &botPos );
	// returns a predicted position for an enemy that is not perceived anymore

	Vector predictedAppearance( Vector &botPos );
	// returns the position where a not visible enemy is most likely to get into line-of-sight

	bool hasBeenVisible()		{ return (pState & PI_PREDICTED); }
	bool isVisible()			{ return (pState & PI_VISIBLE); }
	bool isTrackable()			{ return (pState & (PI_TRACKABLE | PI_VISIBLE | PI_PREDICTED)); }
	bool inflictorKnown()		{ return (pState & PI_ORIG_KNOWN); }
	bool hasBeenSpotted()		{ return (pState & (PI_VISIBLE | PI_PREDICTED)); }
	bool isMoving()				{ return (entity->v.velocity.Length() > 5.0); }
	bool isFacingBot()			{ return (orientation > 0.7); }
	bool isAimingAtBot();
	bool isHurtingBot()			{ return (pState & PI_TACTILE); }
	bool isAlert()				{ return (flags & PI_ALERT); }
	bool canBeAttackedBest()	{ return (flags & PI_BEST_ARMED); }
	bool hasHighPriority()		{ return (flags & PI_HIGH_PRIORITY); }
	bool hasFocus()				{ return (flags & (PI_FOCUS1|PI_FOCUS2)); }
	bool isReachable()			{ return (!(flags & PI_UNREACHABLE)); }
	bool hasJustDisappeared()	{ return (flags & PI_DISAPPEARED); }
	bool isUnderPreemptiveFire(){ return (flags & PI_PREEMPTIVE); }
	float targetAccuracy();
	
	bool operator==(const PB_Percept& O) const  {  return (lastPos.x == O.lastPos.x); }
	bool operator!=(const PB_Percept& O) const  {  return (lastPos.x != O.lastPos.x); }
	bool operator<(const PB_Percept& O) const   {  return (lastPos.x >  O.lastPos.x); }
	bool operator>(const PB_Percept& O) const   {  return (lastPos.x <  O.lastPos.x); }
};


typedef std::list<PB_Percept> tPerceptionList;


class PB_Perception
{

public:

	PB_Perception();

	~PB_Perception();

	void init( edict_t *ent );
	// initializes all necessary variables

	void setSensitivity( int skill );

	void addAttack( edict_t *att, int dmg  );
	// adds a damage perception from entity att (maybe 0) with amount dmg

	void addNewArea( Vector viewDir );
	// adds a new area perception in the direction viewDir

	bool isNewPerception( tPerceptionList &pList, PB_Percept &perc );

	bool isNewTactilePerception( tPerceptionList &pList, PB_Percept &perc );

	void collectData();

	void resetPlayerClassifications();
	// enforces new classifications for all players

	edict_t* getNearestTripmine();
	// returns nearest tripmine or 0 if no tripmine seen

	bool underFire();
	// returns true is bot is currently being attacked

	tPerceptionList* perceptionList() { return &(detections[cdet]); }


	int numEnemies, numUnidentified;	// just for fast queries


private:
		
	edict_t *botEnt;
	float sensitivity;
	int cdet, odet;
	tPerceptionList detections[2];	// two lists to be able to compare old and actual
	tPerceptionList tactileDetections;
	float maxSpeed;
	
	bool classify( PB_Percept &perc );
	// identifies a PI_PLAYER perception as friend or foe if possible

	bool addIfVisible( edict_t *ent, int pClass );
	bool addIfVisible( Vector pos, edict_t *ent, int pClass );
	
	void checkDamageFor( PB_Percept &player );
	void checkInflictorFor( PB_Percept &dmg );

};

#endif