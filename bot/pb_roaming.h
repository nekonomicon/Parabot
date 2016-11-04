#ifndef PB_ROAMING_H
#define PB_ROAMING_H


#include "extdll.h"
#include "pb_action.h"
#include "pb_navpoint.h"

class PB_Roaming
{

typedef struct {
	int		shouldJump;  // 0=no, 1=slow, 2=fast
	bool	shouldDuck;
	bool	blocked;
	bool	gap;
	vec3_t	wallAngle;
	float   wallDistance;
	bool	onTouch;
	bool	tooClose, tooFar;
	vec3_t  jumpPos, landPos;	// for transmitting possible jumps
} checkWayRes;


public:

	void init( edict_t *botEnt, PB_Action *act );
	void reset( Vector newTarget );
	void checkWay( Vector &targetPos );
	bool targetNotReachable();


//private:

	int strafeRightCount, strafeLeftCount;
		
	bool followLeft, followRight, checkIfPassage;
	float passedEdge;
	bool debugWay, debugTrace;
	vec3_t passageOrigin;
	float passageDistance;
	int passageTries;
	edict_t *pev;
	PB_Action *action;
	Vector target, jumpTarget;
	float lastXyDist;	// last XY-Dist to target
	int markerId;



void checkJump(vec3_t origin, vec3_t dir, checkWayRes *res );
void checkFront (float sideOfs, Vector tAngle, checkWayRes *res);
void checkSide (int side, float frontOfs, checkWayRes *res);
int searchExit (Vector wallAngle);
bool bigGapAt( Vector pos );



};

#endif