#include "pb_roaming.h"
#include "pb_global.h"
#include "marker.h"
#include "bot.h"


#define MAX_ZREACH	45
#define NO_DIR 0
#define LEFT   1
#define RIGHT  2


extern int mod_id;
//CMarker marker;



void fixAngle( Vector &angle );


void PB_Roaming::init( edict_t *botEnt, PB_Action *act )
{
	pev = botEnt;
	action = act;
	debugTrace = false;
	debugWay = false;
	reset( Vector(0,0,0) );
	//markerId = marker.newMarker( jumpTarget, 1 );
}


void PB_Roaming::reset( Vector newTarget )
{
	checkIfPassage = false;
	followLeft = false;
	followRight = false;
	strafeLeftCount = 0;
	strafeRightCount = 0;
	passedEdge = 0;
	Vector tDir = newTarget - pev->v.origin; 
	Vector targetAngle = UTIL_VecToAngles( tDir ); 
	fixAngle( targetAngle );
	action->setMoveAngleYaw( targetAngle.y );
	lastXyDist = 10000;
}


bool PB_Roaming::bigGapAt( Vector pos )
// returns true if the gap is big enough to fall  through, else false
{
	TraceResult tr;
	
	UTIL_TraceLine (pos, pos+Vector(32,0,0), ignore_monsters, 0, &tr);
	float distXp = tr.flFraction * 32;
	UTIL_TraceLine (pos, pos+Vector(-32,0,0), ignore_monsters, 0, &tr);
	float distXn = tr.flFraction * 32;
	UTIL_TraceLine (pos, pos+Vector(0,32,0), ignore_monsters, 0, &tr);
	float distYp = tr.flFraction * 32;
	UTIL_TraceLine (pos, pos+Vector(0,-32,0), ignore_monsters, 0, &tr);
	float distYn = tr.flFraction * 32;
	if ( ((distXp+distXn) > 32) && ((distYp+distYn) > 32) ) return true;
	else return false;
}


void PB_Roaming::checkJump(vec3_t origin, vec3_t dir, checkWayRes *res )
// returns in *res: 
// 1) no gap:			gap=false
// 2) crossable gap:	gap=true	blocked=false	jumpPos		landPos			shouldJump
// 3) uncrossable gap:	gap=true	blocked=true	wallAngle	wallDistance	tooFar	tooClose
{
	#define MAX_JUMP_DIST 200
	#define GAP_SCANDIST   40	

	TraceResult tr;
	Vector vFrom, vDown, floor, jumpPos, jumpEnd, landPos, planeAngle;

	dir = GAP_SCANDIST*dir.Normalize();
	vFrom = origin + dir;
	/////////// just security....
	UTIL_TraceLine (origin, vFrom, ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction<1.0) {
		res->gap = false;
		res->blocked = true;
		debugMsg("Fraction Error in checkJump(), fraction=%.f\n", tr.flFraction );
		return;
	}
	//////////
	vDown = Vector(0,0,-512);
	UTIL_TraceLine (vFrom, vFrom+vDown, ignore_monsters, ENT(pev), &tr);
	floor = vFrom + tr.flFraction*vDown;
	int material = UTIL_PointContents( floor + Vector(0,0,1) );
	// We have: floor and material in front of us

	if ( ( ((floor.z+36+MAX_ZREACH)<target.z) && ((floor.z+36+16)<origin.z) ||		// gap goes deeper than target
			tr.flFraction==1.0 ||													// gap deeper than 512 units
		    material==CONTENTS_LAVA	|| material==CONTENTS_SLIME   			  ) &&	// dangerous
			bigGapAt( vFrom+Vector(0,0,-36-4) )										)
	{	
		if (debugWay) {
			if (material==CONTENTS_LAVA) {
				debugMsg( "Running into lava\n" );
			}
			else debugMsg( "Approaching gap, " );
		}
		res->gap = true;
		vFrom.z += (-36 -4);
		UTIL_TraceLine (vFrom, vFrom-dir, ignore_monsters, ENT(pev), &tr);
		jumpPos = vFrom - tr.flFraction*dir;	// edge position
		planeAngle = UTIL_VecToAngles (-tr.vecPlaneNormal);	// treat gap like wall
		// We have: edge(jump-)position and angle
		Vector jumpVec = dir * (MAX_JUMP_DIST/GAP_SCANDIST);
		Vector h (0,0,50);
		UTIL_TraceLine (jumpPos+h, jumpPos+h+jumpVec, ignore_monsters, ENT(pev), &tr);
		jumpEnd = jumpPos + h + jumpVec * (tr.flFraction-0.1);  // 20 vor Trace-Ende
		UTIL_TraceLine (jumpEnd, jumpEnd+vDown, ignore_monsters, ENT(pev), &tr);
		landPos = jumpEnd + tr.flFraction*vDown; // floor on other side
		material = UTIL_PointContents( landPos + Vector(0,0,1) );
		// We have: floor and material at our landing pos
		Vector dist = jumpPos - pev->v.origin;
		dist.z=0;
		float d=dist.Length();
		Vector dirNorm = dir.Normalize();
		float dot = DotProduct( pev->v.velocity, dirNorm );
		float minJumpSpeed = 0.9*action->getMaxSpeed();
		
		if ((landPos.z+36+MAX_ZREACH)<target.z ||
			tr.flFraction==1.0 ||	
			//followLeft || followRight ||				// probably not moving in jump direction!
			dot < minJumpSpeed ||			// too slow
			material==CONTENTS_LAVA	|| material==CONTENTS_SLIME	)	// can't cross
		{	
			if (debugWay) {
				if (material==CONTENTS_LAVA) {
					debugMsg( "would jump into lava\n" );
				}
				else debugMsg( "landPos too low: %.f \n", (landPos.z+36) );
			}
			res->blocked = true;
			res->shouldJump=0;
			res->wallAngle = planeAngle;
			res->wallDistance = d;
			if (d>50) res->tooFar   = true;
			if (d<25) res->tooClose = true;
		}
		else {
			res->blocked = false;
			res->jumpPos = jumpPos;
			res->landPos = landPos;
			if (d<25) {
				if (debugWay) debugMsg( "jump!\n" );
				res->shouldJump=2;	// jump fast
			}
			else if (debugWay) debugMsg( "dist=%.f\n",d );
		}
	}
	else { // no gap 
		res->gap = false;
	}
}


void PB_Roaming::checkFront (float sideOfs, Vector tAngle, checkWayRes *res)
// checks way forward, sideOfs > 0 : to the right 
{
#define H_NEED_JUMP    20	// for DMC! -> 16 for HLDM?
#define H_JUMP_BLOCKED (MAX_ZREACH+1)
#define H_NEED_DUCK	   72
#define H_DUCK_BLOCKED 34
float   CHECK_FORW_DISTANCE = 40;


	TraceResult tr, trStairs;
	Vector      vFrom, vDir, vSide, planeAngle;
	
	bool needJump=false, jumpBlocked=false, 
		 needDuck=false, duckBlocked=false;

	res->blocked	= false;
	res->shouldDuck = false;
	res->shouldJump = 0;
	
	// init vDir:
	vec3_t aDir;
	aDir.x = 0;  // use only yaw angle
	aDir.y = action->moveAngleYaw();	// tAngle.y;
	aDir.z = 0; 
	UTIL_MakeVectors (aDir);
	vDir = gpGlobals->v_forward * CHECK_FORW_DISTANCE;
	vSide = gpGlobals->v_right * sideOfs;
	
	// check needJump			
	vFrom = pev->v.origin + vSide;		
	vFrom.z += (-36 + H_NEED_JUMP);
	UTIL_TraceLine (vFrom, vFrom+vDir, dont_ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction < 1.0) { 
		planeAngle = UTIL_VecToAngles (tr.vecPlaneNormal);
		if (planeAngle.x<40) {
			if (planeAngle.x==0) {
				vFrom.z -= H_NEED_JUMP/2;
				UTIL_TraceLine (vFrom, vFrom+vDir, dont_ignore_monsters, ENT(pev), &trStairs);
				if ((1.001*trStairs.flFraction)<tr.flFraction) {
					//if (debugTrace) debugMsg( "Climbing stairs...\n" );
				}
				else {
					needJump=1;		// jump slow if no stairs
					if (debugTrace) debugMsg( "Need jump!\n" );
				}
			}
			else {
				needJump=1;		// jump slow if angle > 50 deg
				if (debugTrace) debugMsg( "Need jump!\n" );
			}
		}
		else if (debugTrace) debugMsg( "No jump, angle ok: %.f!\n",planeAngle.x );
	}	
	
	// check jumpBlocked
	vFrom = pev->v.origin + vSide;	
	vFrom.z += (-36 + H_JUMP_BLOCKED);
	UTIL_TraceLine (vFrom, vFrom+vDir, dont_ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction < 1.0) { 
		if (debugTrace) debugMsg( "Jump blocked!\n" );
		jumpBlocked=true; 
	}	

	// check needDuck
	vFrom = pev->v.origin + vSide;	
	vFrom.z += (-36 + H_NEED_DUCK);
	UTIL_TraceLine (vFrom, vFrom+vDir, dont_ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction < 1.0) { 
		if (debugTrace) debugMsg( "Need duck!\n" );
		needDuck=true; 
	}	
	
	// check duckBlocked
	vFrom = pev->v.origin + vSide;	
	vFrom.z += (-36 + H_DUCK_BLOCKED);
	UTIL_TraceLine (vFrom, vFrom+vDir, dont_ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction < 1.0) { 
		if (debugTrace) debugMsg( "Duck blocked!\n" );
		duckBlocked=true; 
	}	
	
	// set return vars
	if (needJump && !jumpBlocked) {
		res->shouldJump = 1;
		res->landPos = pev->v.origin + vDir;
	}
	if (needDuck && !duckBlocked) {
		if (mod_id != DMC_DLL)
			res->shouldDuck = true;
		else {
			res->blocked = true;
			if (!needJump) planeAngle = UTIL_VecToAngles (tr.vecPlaneNormal); // not calculated yet...
			res->wallAngle = planeAngle;
			return;		// no gap check if blocked
		}
	}
	if (jumpBlocked && duckBlocked) {
		res->blocked = true;
		if (!needJump) planeAngle = UTIL_VecToAngles (tr.vecPlaneNormal); // not calculated yet...
		res->wallAngle = planeAngle;
		return;		// no gap check if blocked
	}
		// needjump ||
	if (needDuck||jumpBlocked||duckBlocked) return; // not free

	if (!action->jumping()) checkJump (pev->v.origin+vSide, vDir, res);
	return;
}


void PB_Roaming::checkSide (int side, float frontOfs, checkWayRes *res) 
// viewing vector supposed to be in gpGlobals, call directly after checkFront!
{
#define MAX_WALL_DISTANCE   65
#define MAX_TOUCH		   0.77	//(50 / MAX_WALL_DISTANCE)
#define MIN_TOUCH	       0.38	//(25 / MAX_WALL_DISTANCE)

	TraceResult tr;
	Vector vDir, start;
	
	start = pev->v.origin + frontOfs*gpGlobals->v_forward;
	res->tooClose = false;
	res->tooFar = false;

	if (side==LEFT)		  vDir = -MAX_WALL_DISTANCE*gpGlobals->v_right;
	else if (side==RIGHT) vDir =  MAX_WALL_DISTANCE*gpGlobals->v_right;
	else return;
	UTIL_TraceLine (start, start+vDir, ignore_monsters, ENT(pev), &tr);
	
	if (tr.flFraction<1.0) {
		res->onTouch = true;
		res->wallDistance = tr.flFraction*MAX_WALL_DISTANCE;
		res->wallAngle = UTIL_VecToAngles (tr.vecPlaneNormal);
		if (tr.flFraction>MAX_TOUCH) res->tooFar   = true;
		if (tr.flFraction<MIN_TOUCH) res->tooClose = true;
	//	debugMsg( "Edict: %i\n", tr.pHit->serialnumber);
	}
	else {
		if ( UTIL_PointContents( start+vDir+Vector(0,0,-40) ) == CONTENTS_EMPTY ) {
			// following gap instead of wall!
			UTIL_TraceLine (start+vDir+Vector(0,0,-40), start+Vector(0,0,-40), ignore_monsters, ENT(pev), &tr);
			//if (tr.flFraction<1.0) {
				tr.flFraction = 1.0 - tr.flFraction;
				res->onTouch = true;
				res->wallDistance = tr.flFraction*MAX_WALL_DISTANCE;
				if (tr.flFraction<1.0) 
					res->wallAngle = UTIL_VecToAngles (-tr.vecPlaneNormal);
				else
					res->wallAngle = UTIL_VecToAngles (-vDir);
				if (tr.flFraction>MAX_TOUCH) res->tooFar   = true;
				if (tr.flFraction<MIN_TOUCH) res->tooClose = true;
			/*}
			else { // this should never happen (bot has to stand on something!)
				debugMsg( "UNEXPECTED RESULT in PB_Roaming::checkSide!\n" );
				res->onTouch=false; 
				res->gap=false;
				res->shouldJump=false;
			}*/
		}
		else {
			res->onTouch=false; 
			res->gap=false;
			res->shouldJump=false;
		}
	}
	return;
}


int PB_Roaming::searchExit (Vector wallAngle)
// bot is supposed to stand in front of wall 
{
#define TRACE_STEP  72
#define MAX_SPEED	270
float   TRACE_DEPTH = 20 + MAX_SPEED/5;

	TraceResult tr;
	bool		traceLeft=true, traceRight=true, foundLeftExit=false, foundRightExit=false;
	Vector      vLeftFrom, vRightFrom, vForw, vWallDir;
	float		dLeft=0, dRight=0;  // distance to left/right

	
	wallAngle.x = 0;
	wallAngle.z = 0;
	UTIL_MakeVectors (wallAngle);
	vWallDir = - gpGlobals->v_right;	// vDir = Vector to wall-right
	vForw = - gpGlobals->v_forward;		// vForw = Vector towards wall
	
	vLeftFrom = pev->v.origin;
	vRightFrom = pev->v.origin;
	int dbgCnt = 0;
	do {
		if (traceLeft) {
			dLeft += TRACE_STEP;
			UTIL_TraceLine (vLeftFrom, vLeftFrom-TRACE_STEP*vWallDir, ignore_monsters, NULL, &tr);
			if (tr.flFraction==1.0) { // didn't hit anything
				vLeftFrom = tr.vecEndPos;
				UTIL_TraceLine (vLeftFrom, vLeftFrom+TRACE_DEPTH*vForw, ignore_monsters, NULL, &tr);
				if (tr.flFraction==1.0) {
					traceLeft=false;  
					traceRight=false;
					foundLeftExit = true;
				}
			}
			else traceLeft=false; // encountered wall 
		}
		if (traceRight) {
			dRight += TRACE_STEP;
			UTIL_TraceLine (vRightFrom, vRightFrom+TRACE_STEP*vWallDir, ignore_monsters, NULL, &tr);
			if (tr.flFraction==1.0) { // didn't hit anything
				vRightFrom = tr.vecEndPos;
				UTIL_TraceLine (vRightFrom, vRightFrom+TRACE_DEPTH*vForw, ignore_monsters, NULL, &tr);
				if (tr.flFraction==1.0) {
					traceLeft=false;  
					traceRight=false; 
					foundRightExit = true;
				}
			}	
			else traceRight=false; // encountered wall 
		}
	} while ((traceRight || traceLeft) && ++dbgCnt < 1000);
	if (dbgCnt==1000) {
		FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
		fprintf( dfp, ">1000 recursions in PB_Roaming::searchExit!\n" ); 
		fclose( dfp );
	}
	
	if (foundLeftExit) return LEFT;
	else if (foundRightExit) return RIGHT;
	return NO_DIR;
}



void PB_Roaming::checkWay ( Vector &targetPos ) 
{
	checkWayRes left, right, side;

	assert( action!=0 );
	assert( pev!=0 );
	
	if (action->jumping()) {
		target = jumpTarget;
		Vector tDir = target - pev->v.origin; 
		Vector targetAngle = UTIL_VecToAngles( tDir ); 
		fixAngle( targetAngle );
		tDir.z = 0;
		float jumpSpeed = tDir.Length();
		action->setMoveAngleYaw( targetAngle.y );
		//action->dontGetStuck();
		action->setSpeed( 5*jumpSpeed );
		//debugMsg( "AC! " );
		//marker.drawMarkers();
		return;	// air control!
	}

	target = targetPos;
	Vector tDir = targetPos - pev->v.origin; 
	Vector targetAngle = UTIL_VecToAngles( tDir ); 
	fixAngle( targetAngle );
	float targetDistance = tDir.Length();
	
	action->setMaxSpeed();

	checkFront (-16, targetAngle, &left);
	checkFront ( 16, targetAngle, &right);	// first check forward...
	bool sideValid = false;		// if side is initialized

	if (followLeft) {
		checkSide (LEFT, 0, &side);	//-20
		if (side.onTouch) sideValid = true;
		
		if (!checkIfPassage) {  // following the wall
			if (!side.onTouch && (passedEdge<gpGlobals->time)) {  // it has disappeared...
				right.blocked = false;	// direction change -> cancel everything
				left.shouldJump = false;
				right.shouldJump = false;
				if ( (abs(UTIL_AngleDiff(action->moveAngleYaw(),targetAngle.y))<=90) &&
					(abs(UTIL_AngleDiff(action->moveAngleYaw()+90,targetAngle.y))<=90) ) {
					followLeft=false;
					if (debugWay) debugMsg( "Freed from left wall.\n");
				}
				else {
					action->setMoveAngleYaw( action->moveAngleYaw() + 90 );  // turn left
					checkIfPassage = true;
					passageOrigin = pev->v.origin;
					passageTries = 0;
					if (debugWay) debugMsg( "Searching passage at the left... " );
				}
			}
			else {  // everything fine
				passageDistance = side.wallDistance + 24;  // if wall disappears next frame
				if (side.tooClose || left.blocked) action->add( BOT_STRAFE_RIGHT );
				else if (side.tooFar) action->add( BOT_STRAFE_LEFT );
			}
		}

		else { // in search of passage
			passageTries++;
			if (!left.blocked) action->add( BOT_STRAFE_LEFT );
			Vector moved = (pev->v.origin - passageOrigin);
			if (moved.Length()>=passageDistance) {
				if (side.onTouch) {	// that's the wall we were looking for 
					action->setMoveAngleYaw (side.wallAngle.y+90);
					if (debugWay) debugMsg( "Alligned to new wall.\n" );
				}
				else {  // very short wall!
					followLeft = false;		// let him free
					if (debugWay) debugMsg( "Could not find wall!\n" );
				}
				checkIfPassage=false;
			}
			else { // not yet reached the passage
				if (passageTries>8) {
					action->add( BOT_STRAFE_RIGHT );	// follow old direction
					if (passageTries>20) {
						checkIfPassage=false;
						followLeft = false;		// let him free
						if (debugWay) debugMsg( "Could not reach it! dist=%.f, gone=%.f.\n",passageDistance,moved.Length());
					}
				}
			}
		}

		if (right.blocked) {	// in any case check for wall in front
			action->setMoveAngleYaw (right.wallAngle.y+90);  // follow wall to Right
			if (debugWay) debugMsg( " Blocked, turning right.\n");
		}
	}

	else if (followRight) {
		checkSide (RIGHT, 0, &side);	// -20
		if (side.onTouch) sideValid = true;
		
		if (!checkIfPassage) {  // following the wall
			if (!side.onTouch && (passedEdge<gpGlobals->time)) {  // it has disappeared...
				left.blocked = false;	// direction change -> cancel everything
				left.shouldJump = false;
				right.shouldJump = false;
				if ( (abs(UTIL_AngleDiff(action->moveAngleYaw(),targetAngle.y))<=90) &&
					(abs(UTIL_AngleDiff(action->moveAngleYaw()-90,targetAngle.y))<=90) ) {
					followRight=false;
					if (debugWay) debugMsg( "Freed from right wall.\n");
				}
				else {
					action->setMoveAngleYaw( action->moveAngleYaw()-90 );  // turn right
					checkIfPassage = true;
					passageOrigin = pev->v.origin;
					passageTries = 0;
					if (debugWay) debugMsg( "Searching passage at the right... ");
				}
			}
			else {  // everything fine
				passageDistance = side.wallDistance + 24;  // if wall disappears next frame
				if (side.tooClose || right.blocked) action->add( BOT_STRAFE_LEFT );
				else if (side.tooFar) action->add( BOT_STRAFE_RIGHT );
			}
		}

		else { // in search of passage
			passageTries++;
			if (!right.blocked) action->add( BOT_STRAFE_RIGHT );
			Vector moved = (pev->v.origin - passageOrigin);
			if (moved.Length()>=passageDistance) {
				if (side.onTouch) {	// that's the wall we were looking for 
					action->setMoveAngleYaw( side.wallAngle.y-90 );
					if (debugWay) debugMsg( "Alligned to new wall.\n");
				}
				else {  // very short wall!
					followRight = false;		// let him free
					if (debugWay) debugMsg( "Could not find wall!\n");
				}
				checkIfPassage=false;
			}
			else { // not yet reached the passage
				if (passageTries>8) {
					action->add( BOT_STRAFE_LEFT );	// follow old direction
					if (passageTries>20) {
						checkIfPassage=false;
						followRight = false;		// let him free
						if (debugWay) debugMsg( "Could not reach it! dist=%.f, gone=%.f.\n",passageDistance,moved.Length());
					}
				}
			}
		}

		if (left.blocked) {	// in any case check for wall in front
			action->setMoveAngleYaw( left.wallAngle.y-90 );  // follow wall to Left
			if (debugWay) debugMsg( " Blocked, turning left.\n");
		}
	}

	else { // not following any walls
		if (left.blocked && right.blocked) {	// suddenly a wall
			int turn;
			if (left.wallAngle.y==right.wallAngle.y) { // in front of plain wall
				float adtw = UTIL_AngleDiff(left.wallAngle.y, targetAngle.y);
				if (abs(adtw)>135) { // target straight behind
					turn = searchExit (left.wallAngle);
					if (turn==NO_DIR) {
						if (debugWay) debugMsg( "Blocked, no exit found: ");
						turn = RANDOM_LONG (LEFT,RIGHT);
					}
					else if (debugWay) debugMsg( "Blocked, found exit: ");
				}
				else { // target more in one direction of wall, head to that direction
					if (adtw>0) turn=LEFT;
					else		turn=RIGHT;
					if (debugWay) debugMsg( "Blocked, heading for target: ");
				}
			}
			else { // in corner
				turn = RANDOM_LONG (LEFT,RIGHT);
				passedEdge = gpGlobals->time + 0.5;
			}

			if (turn==LEFT) {
				if (debugWay) debugMsg( "Turning left, following wall.\n");
				action->setMoveAngleYaw (left.wallAngle.y-90);		// follow wall to left
				followRight = true;		// wall should be at the right
			}
			else if (turn==RIGHT){
				if (debugWay) debugMsg( "Turning right, following wall.\n");
				action->setMoveAngleYaw (right.wallAngle.y+90);	// follow wall to right
				followLeft = true;		// wall should be at the left
			}
		}
		else {
			action->setMoveAngleYaw (targetAngle.y);

			if (left.blocked) {
				action->add( BOT_STRAFE_RIGHT );
				strafeRightCount++;
				if (strafeRightCount>3) {
					if (debugWay) debugMsg( "Strafing too long, mode set to followLeft\n");
					followLeft = true;
					strafeRightCount = 0;
				}
			}
			else strafeRightCount = 0;
			if (right.blocked) {
				action->add( BOT_STRAFE_LEFT );
				strafeLeftCount++;
				if (strafeLeftCount>3) {
					if (debugWay) debugMsg( "Strafing too long, mode set to followRight\n");
					followRight = true;
					strafeLeftCount = 0;
				}
			}
			else strafeLeftCount = 0;
		}
	}
	
	if (!action->jumping()) {	// jump...
		if (left.shouldJump>0 && !right.blocked) {
			jumpTarget = left.landPos;
			//marker.setPos( markerId, jumpTarget );
			action->add( BOT_JUMP );
		}
		else if (right.shouldJump>0 && !left.blocked) {
			jumpTarget = right.landPos;
			//marker.setPos( markerId, jumpTarget );
			action->add( BOT_JUMP );
		}
	}
			
	if ((left.shouldDuck || right.shouldDuck)) action->add( BOT_DUCK ); // ...or duck if necessary

	TraceResult tr;
	bool targetVisible;
	Vector eyePos = pev->v.origin + pev->v.view_ofs;
	UTIL_TraceLine( target, eyePos, dont_ignore_monsters, 0, &tr);	
	if (tr.flFraction != 1.0 && tr.pHit != pev) targetVisible = false;
	else targetVisible = true;

	if (targetVisible) { // special behaviour if target is visible:
		if ( (targetDistance<70) ||	     // - too close to target, ignore walls
			 ((followLeft||followRight) && sideValid && (abs(UTIL_AngleDiff(side.wallAngle.y,targetAngle.y))<=90)) ) {	
			//if (debugWay) 
			//debugMsg( "Close to target!\n");
			action->setMoveAngleYaw (targetAngle.y);
			action->setMaxSpeed();
			followRight=false;
			followLeft=false;
			checkIfPassage=false;
		}
	}

	// underwater behaviour:
	if (isUnderwater( pev )) {
		if (needsAir( pev )) {
			action->add( BOT_JUMP );
		}
		else {
			if (targetPos.z > (pev->v.origin.z+32)) 
				action->setMoveAngle( Vector( -60, action->moveAngleYaw(), 0 ));
			else if (targetPos.z < (pev->v.origin.z-32)) 
				action->setMoveAngle( Vector( +60, action->moveAngleYaw(), 0 ));
		}
	}
}



bool PB_Roaming::targetNotReachable()
{
	Vector xyDir = target - pev->v.origin;
	xyDir.z = 0;
	float xyDist = xyDir.Length();
	if (xyDist < 40 && xyDist > lastXyDist) return true;
	lastXyDist = xyDist;
	return false;
}


float UTIL_AngleDiff( float destAngle, float srcAngle )
{
	if (destAngle < -360) {
		debugMsg( "FATAL ERROR at UTIL_AngleDiff: destAngle = %.f\n", destAngle );
		destAngle = srcAngle;
	}
	else if (destAngle > 360) {
		debugMsg( "FATAL ERROR at UTIL_AngleDiff: destAngle = %.f\n", destAngle );
		destAngle = srcAngle;
	}
	while (destAngle < srcAngle) destAngle += 360;
	while (destAngle > srcAngle) destAngle -= 360;
	float diff = srcAngle - destAngle;
	if (diff>180) diff = 360 - diff;
	return diff;
}
