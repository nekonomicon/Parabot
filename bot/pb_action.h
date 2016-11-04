#ifndef PB_ACTION_H
#define PB_ACTION_H


#include "extdll.h"


// movement constants:
#define	MAX_YAW			 20

#define MAX_VDELAY 11


class PB_Action
{

public:

	void init( edict_t *botEnt );
	void reset();
	void setAimSkill( int skill );
	void setMoveAngle( Vector angle );
	void setMoveAngleYaw( float angle );
	void setMoveDir( Vector vec, int prior = 0 );
	void setViewAngle( Vector angle, int prior = 0 );
	void setViewDir( Vector vec, int prior = 0 );
	void setViewLikeMove();
	void setAimDir( Vector currentPos, Vector relVelocity = Vector(0,0,0) );
	void setSpeed( float value )	  { speed = value;     }
	void setMaxSpeed();
	float getMaxSpeed() { return maxSpeed; }
	void add( int code, Vector *exactPos = 0 );
	bool gotStuck();
	void resetStuck();
	void dontGetStuck() { notStucking = true; }
	void perform();

	float moveAngleYaw() { return moveAngle.y; }
	bool jumping() { return inJump; }
	bool pausing() { return notStucking; }
	float targetAccuracy() { return hitProb; }
	// returns the accurcy for beforehand passed target
	Vector getMoveDir();
	Vector getViewAngle() { return viewAngle; }
	float getFrameTime() { return (currentMSec/1000); }
	int getAimSkill() { return aimSkill; }
	float estimateHitProb();
	void setWeaponCone (float cone )	{ weaponCone = cone; }


private:

	Vector		 moveAngle, viewAngle;		// actual move angle and view(shoot!)angle
	float		 speed;						// speed to move at
	float		 strafe;					// strafe speed (>0 = right, <0 = left )
	int			 action;					// instant action e.g. fire, jump, duck
	float		 msecStart, msecCount;
	float		 currentMSec;				// frameTime * 1000
		
	edict_t		*ent;			// bot entity
	float		jumpPos;		// zpos where jump started
	bool		inJump;			// true while ducked in jump
	bool		fineJump;
	Vector		fineJumpPos;
	int			longJumpState;
	float		nextJumpTime;	// for delayed jump
	int			viewPrior, movePrior;		// priorities
	float		maxSpeed;

	int			aimSkill;
	
	Vector		currentView;	// delayed view
	Vector		deltaView;
	float		targetDiff[MAX_VDELAY];		// storing the difference to target for the last frames
	float		hitProb;
	float		weaponCone;
	Vector		targetPos, targetVel;
	float		targetDist;

	bool		notStucking;

	int			useCounter;
	float		nextUseTime;
	Vector		nextUsePos;

	float		duckEndTime;
	float		stopEndTime;

	float		lastMove, lastMoveCheck, nextViewUpdate, vupdTime, aimAccuracy;
	float		maxTurn;
	int			turnCount;

	Vector calcViewAngle();
	float msec();
	// returns an adequate msec-value to pass to the engine

};

#endif