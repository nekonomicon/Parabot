#pragma once
#if !defined(PB_ACTION_H)
#define PB_ACTION_H

#include "sdk_common.h"

// movement constants:
#define	MAX_YAW			 20

#define MAX_VDELAY 11


class PB_Action
{

public:

	void init( EDICT *botEnt );
	void reset();
	void setAimSkill( int skill );
	void setMoveAngle( Vec3D *angle );
	void setMoveAngleYaw( float angle );
	void setMoveDir( Vec3D *vec, int prior = 0 );
	void setViewAngle( Vec3D *angle, int prior = 0 );
	void setViewDir( Vec3D *vec, int prior = 0 );
	void setViewLikeMove();
	void setAimDir( Vec3D *currentPos, Vec3D *relVelocity );
	void setSpeed( float value )	  { speed = value;     }
	void setMaxSpeed();
	float getMaxSpeed() { return maxSpeed; }
	void add( int code, Vec3D *exactPos );
	bool gotStuck();
	void resetStuck();
	void dontGetStuck() { notStucking = true; }
	void perform();

	float moveAngleYaw() { return moveAngle.y; }
	bool jumping() { return inJump; }
	bool pausing() { return notStucking; }
	float targetAccuracy() { return hitProb; }
	// returns the accurcy for beforehand passed target
	Vec3D *getMoveDir();
	Vec3D *getViewAngle() { return &viewAngle; }
	float getFrameTime() { return (currentMSec/1000); }
	int getAimSkill() { return aimSkill; }
	float estimateHitProb();
	void setWeaponCone (float cone )	{ weaponCone = cone; }


private:

	Vec3D		moveAngle, viewAngle;		// actual move angle and view(shoot!)angle
	float		 speed;						// speed to move at
	float		 strafe;					// strafe speed (>0 = right, <0 = left )
	int			 action;					// instant action e.g. fire, jump, duck
	float		 msecStart, msecCount;
	float		 currentMSec;				// frameTime * 1000
		
	EDICT		*ent;			// bot entity
	float		jumpPos;		// zpos where jump started
	bool		inJump;			// true while ducked in jump
	bool		fineJump;
	Vec3D		fineJumpPos;
	int			longJumpState;
	float		nextJumpTime;	// for delayed jump
	int			viewPrior, movePrior;		// priorities
	float		maxSpeed;

	int			aimSkill;
	
	Vec3D		currentView;	// delayed view
	Vec3D		deltaView;
	float		targetDiff[MAX_VDELAY];		// storing the difference to target for the last frames
	float		hitProb;
	float		weaponCone;
	Vec3D		targetPos, targetVel;
	float		targetDist;

	bool		notStucking;

	int			useCounter;
	float		nextUseTime;
	Vec3D		nextUsePos;

	float		duckEndTime;
	float		stopEndTime;

	float		lastMove, lastMoveCheck, nextViewUpdate, vupdTime, aimAccuracy;
	float		maxTurn;
	int			turnCount;

	Vec3D *calcViewAngle();
	float msec();
	// returns an adequate msec-value to pass to the engine

};

#endif
