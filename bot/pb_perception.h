#pragma once
#ifndef PB_PERCEPTION_H
#define PB_PERCEPTION_H

#include "pb_global.h"
#include <list>

enum { // perception-item classification
	PI_PLAYER = 1,	// unidentified player in team-game
	PI_FRIEND,	// teammate in team-game
	PI_FOE,		// enemy
	PI_HOSTAGE,	// CS
	PI_BOMB,	// CS
	PI_WEAPONBOX,	// weaponbox
	PI_EXPLOSIVE,	// grenade, satchel
	PI_LASERDOT,	// RPG laserdot
	PI_TRIPMINE,	// Beam-Tripmine
	PI_HALO,	// HW
	PI_DAMAGE,	// damage perception
	PI_SNARK,	// snark
	PI_NEWAREA	// a big area that bot hasn't seen before
};

#define MAX_PERCEPTION PI_NEWAREA

enum { // perception-item state
	PI_VISIBLE =	BIT(0),	// is seen
	PI_AUDIBLE =	BIT(1),	// is heard but can't be tracked
	PI_TRACKABLE =	BIT(2),	// is heard and trackable
	PI_TACTILE =	BIT(3),	// is attacking
	PI_PREDICTED =	BIT(4),	// has been seen and is predicted now
	PI_ORIG_KNOWN =	BIT(5),	// for damage: origin/inflictor is known

	PI_UNKNOWN =	BIT(31)	// unknown model index
};

enum { // flags
	PI_BEST_ARMED =		BIT(0),	// best weapon has been armed for that enemy
	PI_ALERT =		BIT(1),	// enemy knows that bot has seen it
	PI_HIGH_PRIORITY =	BIT(2),	// e.g. the Saint in HW
	PI_FOCUS1 =		BIT(3),	// bot is attacking this frame
	PI_FOCUS2 =		BIT(4),	// bot is attacking last frame
	PI_UNREACHABLE =	BIT(5),	// bot can't reach this item by roaming
	PI_DISAPPEARED =	BIT(6),	// item has just become not visible
	PI_PREEMPTIVE =		BIT(7)	// bot uses preemptive fire for that enemy
};

// distances
#define MAX_DIST_VP		3000	// max. visible player detection distance
#define MAX_DIST_VPR	 800	// max. visible player recognition distance
#define MAX_DIST_VI		1200	// max. visible item detection distance

#define UNKNOWN_POS		&zerovector



class PB_Percept 
{
public:
	
	int		id;
	float	botSensitivity;		// sensitivity of bot that perceived item
	EDICT *entity;			// pointer to edict
	short	pClass;				// classification of item
	short	pState;				// kind of perception
	int		model;				// model index of item
	float	update;				// worldtime when goal-queue should be updated
	float	firstDetection;		// worldtime first detection was registered
	float	lastDetection;		// worldtime last detection was registered
	float	lastSeenTime;		// worldtime last visible contact was registered
	Vec3D	lastPos;			// last observed position (seen or heard)
	Vec3D	lastSeenPos;		// last seen position
	Vec3D	lastSeenVelocity;	// last seen velocity

	// for enemy:
	float	rating;				// weapon-rating
	float	orientation;		// dotproduct if item is facing the bot
	float	distance;			// distance to bot
	int		flags;
	Vec3D	predTarget;			// position the item is supposed to reach
	float	lastCalcTarget;		// last worldtime target was predicted
	Vec3D	predAppearance;		// position where the item is supposed to show up
	float	lastCalcAppearance;	// last worldtime appearance was predicted

	//PB_Percept() {}
	PB_Percept( float botSens, EDICT *ent, short state, short realClass, float dist );

	float getReactionTime( EDICT *ent, short state, short realClass, float dist );
	// returns the time needed to react to the item (time to enter goal-queue)

	void predictedPosition( const Vec3D *botPos, Vec3D *lastPos );
	// returns a predicted position for an enemy that is not perceived anymore

	Vec3D *predictedAppearance( const Vec3D *botPos );
	// returns the position where a not visible enemy is most likely to get into line-of-sight

	bool hasBeenVisible()		{ return (pState & PI_PREDICTED); }
	bool isVisible()			{ return (pState & PI_VISIBLE); }
	bool isTrackable()			{ return (pState & (PI_TRACKABLE | PI_VISIBLE | PI_PREDICTED)); }
	bool inflictorKnown()		{ return (pState & PI_ORIG_KNOWN); }
	bool hasBeenSpotted()		{ return (pState & (PI_VISIBLE | PI_PREDICTED)); }
	bool isMoving()				{ return (vlen(&entity->v.velocity) > 5.0); }
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

	void init( EDICT *ent );
	// initializes all necessary variables

	void setSensitivity( int skill );

	void addAttack( EDICT *att, int dmg  );
	// adds a damage perception from entity att (maybe 0) with amount dmg

	void addNewArea( Vec3D *viewDir );
	// adds a new area perception in the direction viewDir

	bool isNewPerception( tPerceptionList &pList, PB_Percept &perc );

	bool isNewTactilePerception( tPerceptionList &pList, PB_Percept &perc );

	void collectData();

	void resetPlayerClassifications();
	// enforces new classifications for all players

	EDICT* getNearestTripmine();
	// returns nearest tripmine or 0 if no tripmine seen

	bool underFire();
	// returns true is bot is currently being attacked

	tPerceptionList* perceptionList() { return &(detections[cdet]); }


	int numEnemies, numUnidentified;	// just for fast queries


private:
		
	EDICT *botEnt;
	float sensitivity;
	int cdet, odet;
	tPerceptionList detections[2];	// two lists to be able to compare old and actual
	tPerceptionList tactileDetections;
	float maxSpeed;
	
	bool classify( PB_Percept &perc );
	// identifies a PI_PLAYER perception as friend or foe if possible

	bool addIfVisible( EDICT *ent, int pClass );
	bool addIfVisible( Vec3D *pos, EDICT *ent, int pClass );
	
	void checkDamageFor( PB_Percept &player );
	void checkInflictorFor( PB_Percept &dmg );
};

#endif
