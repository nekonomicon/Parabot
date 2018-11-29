#pragma once
#ifndef PB_ROAMING_H
#define PB_ROAMING_H

#include "pb_navpoint.h"

class PB_Roaming
{

typedef struct {
	int		shouldJump;  // 0=no, 1=slow, 2=fast
	bool	shouldDuck;
	bool	blocked;
	bool	gap;
	Vec3D	wallAngle;
	float   wallDistance;
	bool	onTouch;
	bool	tooClose, tooFar;
	Vec3D  jumpPos, landPos;	// for transmitting possible jumps
} checkWayRes;


public:

	void init( EDICT *botEnt, ACTION *act );
	void reset( const Vec3D *newTarget );
	void checkWay( const Vec3D *targetPos );
	bool targetNotReachable();


//private:

	int strafeRightCount, strafeLeftCount;
		
	bool followLeft, followRight, checkIfPassage;
	float passedEdge;
	bool debugWay, debugTrace;
	Vec3D passageOrigin;
	float passageDistance;
	int passageTries;
	EDICT *pev;
	ACTION *action;
	Vec3D target, jumpTarget;
	float lastXyDist;	// last XY-Dist to target
#if _DEBUG
	int markerId;
#endif //_DEBUG


void checkJump (Vec3D *origin, Vec3D *dir, checkWayRes *res );
void checkFront (float sideOfs, Vec3D *tAngle, checkWayRes *res);
void checkSide (int side, float frontOfs, checkWayRes *res);
int searchExit (Vec3D *wallAngle);
bool bigGapAt (Vec3D *pos);
};

#endif
