#include "parabot.h"
#include "pb_roaming.h"
#include "pb_global.h"
#include "marker.h"
#include "bot.h"
#include <stdio.h>

#define BIGGAP_SCANDIST   32.0f
#define MAX_ZREACH	45
#define NO_DIR 0
#define LEFT   1
#define RIGHT  2

extern int mod_id;

void PB_Roaming::init( EDICT *botEnt, ACTION *act )
{
	pev = botEnt;
	action = act;
	debugTrace = false;
	debugWay = false;
	reset(&zerovector);
	//markerId = marker.newMarker( jumpTarget, 1 );
}

void PB_Roaming::reset(const Vec3D *newTarget)
{
	Vec3D tDir, targetAngle;

	checkIfPassage = false;
	followLeft = false;
	followRight = false;
	strafeLeftCount = 0;
	strafeRightCount = 0;
	passedEdge = 0;
	vsub(newTarget, &pev->v.origin, &tDir);
	vectoangles(&tDir, &targetAngle);
	fixangle(&targetAngle);
	action_setmoveangleyaw(action, targetAngle.y);
	lastXyDist = 10000;
}


bool PB_Roaming::bigGapAt(Vec3D *pos)
// returns true if the gap is big enough to fall  through, else false
{
	TRACERESULT tr;
	Vec3D dir, shift = {BIGGAP_SCANDIST, 0.0f, 0.0f};
	Vec2D distp, distn;

	vadd(pos, &shift, &dir);
	trace_line(pos, &dir, true, false, 0, &tr);
	distp.x = tr.fraction * BIGGAP_SCANDIST;
	shift.x = -shift.x;

	vadd(pos, &shift, &dir);
	trace_line(pos, &dir, true, false, 0, &tr);
	distn.x = tr.fraction * BIGGAP_SCANDIST;
	shift.x = 0.0f;
	shift.y = BIGGAP_SCANDIST;

	vadd(pos, &shift, &dir);
	trace_line(pos, &dir, true, false, 0, &tr);
	distp.y = tr.fraction * BIGGAP_SCANDIST;
	shift.y = -shift.y;

	vadd(pos, &shift, &dir);
	trace_line(pos, &dir, true, false, 0, &tr);
	distn.y = tr.fraction * BIGGAP_SCANDIST;

	if (((distp.x + distn.x) > BIGGAP_SCANDIST) && ((distp.y + distn.y) > BIGGAP_SCANDIST))
		return true;

	return false;
}

void PB_Roaming::checkJump(Vec3D *origin, Vec3D *_dir, checkWayRes *res )
// returns in *res: 
// 1) no gap:			gap=false
// 2) crossable gap:	gap=true	blocked=false	jumpPos		landPos			shouldJump
// 3) uncrossable gap:	gap=true	blocked=true	wallAngle	wallDistance	tooFar	tooClose
{
	#define MAX_JUMP_DIST 200
	#define GAP_SCANDIST   40	

	TRACERESULT tr;
	Vec3D vFrom, vDown = {0.0f, 0.0f, -512.0f}, floor, jumpPos, jumpEnd, landpos, planeAngle, dir, dist;

	normalize(_dir);
	vscale(_dir, GAP_SCANDIST, _dir);
	vadd(origin, _dir, &vFrom);

	/////////// just security....
	trace_line(origin, &vFrom, true, false, pev, &tr);
	if (tr.fraction < 1.0f) {
		res->gap = false;
		res->blocked = true;
		DEBUG_MSG("Fraction Error in checkJump(), fraction=%.f\n", tr.fraction );
		return;
	}
	//////////

	vadd(&vFrom, &vDown, &dir);
	trace_line(&vFrom, &dir, true, false, pev, &tr);
	vma(&vFrom, tr.fraction, &vDown, &floor);
	floor.z += 1.0f;
	int material = pointcontents(&floor);
	floor.z -= 1.0f;
	vFrom.z += (-36.0f -4.0f);

	// We have: floor and material in front of us
	if(((((floor.z + 36 + MAX_ZREACH) < target.z)
	    && ((floor.z + 36 + 16) < origin->z))	// gap goes deeper than target
	    || tr.fraction == 1.0f			// gap deeper than 512 units
	    || material == CONTENTS_LAVA
	    || material == CONTENTS_SLIME)		// dangerous
	    && bigGapAt(&vFrom)) {
		if (debugWay) {
			if (material == CONTENTS_LAVA) {
				DEBUG_MSG("Running into lava\n");
			} else {
				DEBUG_MSG("Approaching gap, ");
			}
		}
		res->gap = true;
		vsub(&vFrom, _dir, &dir);
		trace_line(&vFrom, &dir, true, false, (pev), &tr);
		vma(&vFrom, -tr.fraction, _dir, &jumpPos);
		vinv(&tr.planenormal);
		vectoangles(&tr.planenormal, &planeAngle);
		// We have: edge(jump-)position and angle
		Vec3D jumpVec, jumpdir;
		vscale(_dir, MAX_JUMP_DIST / GAP_SCANDIST, &jumpVec);
		Vec3D h = {0.0f, 0.0f, 50.0f};
		vadd(&jumpPos, &h, &jumpEnd);
		vadd(&jumpEnd, &jumpVec, &jumpdir);
		trace_line(&jumpEnd, &jumpdir, true, false, pev, &tr);
		vma(&jumpEnd, tr.fraction - 0.1f, &jumpVec, &jumpEnd); // 20 vor Trace-Ende
		vadd(&jumpEnd, &vDown, &jumpdir);
		trace_line(&jumpEnd, &jumpdir, true, false, pev, &tr);
		vma(&jumpEnd, tr.fraction, &vDown, &landpos); // floor on other side
		landpos.z += 1.0f;
		material = pointcontents(&landpos);
		landpos.z -= 1.0f;

		// We have: floor and material at our landing pos
		vsub(&jumpPos, &pev->v.origin, &dist);
		float d = vlen2d((Vec2D *)&dist);
		Vec3D dirNorm;
		vcopy(&dir, &dirNorm);
		normalize(&dirNorm);
		float dot = dotproduct(&pev->v.velocity, &dirNorm);
		float minJumpSpeed = 0.9f * action_getmaxspeed(action);

		if ((landpos.z + 36.0f + MAX_ZREACH) < target.z
		    || tr.fraction == 1.0f
//		    || followLeft || followRight			// probably not moving in jump direction!
		    || dot < minJumpSpeed			// too slow
		    || material == CONTENTS_LAVA
		    || material == CONTENTS_SLIME) {	// can't cross
			if (debugWay) {
				if (material==CONTENTS_LAVA) {
					DEBUG_MSG( "would jump into lava\n" );
				} else
					DEBUG_MSG( "landpos too low: %.f \n", (landpos.z + 36.0f) );
			}
			res->blocked = true;
			res->shouldJump = 0;
			res->wallAngle = planeAngle;
			res->wallDistance = d;
			if (d > 50) res->tooFar   = true;
			if (d < 25) res->tooClose = true;
		} else {
			res->blocked = false;
			res->jumpPos = jumpPos;
			res->landPos = landpos;
			if (d < 25) {
				if (debugWay)
					DEBUG_MSG("jump!\n");
				res->shouldJump = 2;	// jump fast
			} else if (debugWay)
				DEBUG_MSG( "dist=%.f\n",d );
		}
	} else { // no gap 
		res->gap = false;
	}
}


void PB_Roaming::checkFront (float sideOfs, Vec3D *angle, checkWayRes *res)
// checks way forward, sideOfs > 0 : to the right 
{
#define H_NEED_JUMP    20	// for DMC! -> 16 for HLDM?
#define H_JUMP_BLOCKED (MAX_ZREACH+1)
#define H_NEED_DUCK	   72
#define H_DUCK_BLOCKED 34

	float   CHECK_FORW_DISTANCE = 40;
	TRACERESULT tr, trStairs;
	Vec3D      vFrom, vDir, vSide, planeAngle, dir;

	bool needJump = false, jumpBlocked = false, 
	needDuck = false, duckBlocked = false;

	res->blocked	= false;
	res->shouldDuck = false;
	res->shouldJump = 0;

	// init vDir:
	Vec3D aDir = {0.0f, action_moveangleyaw(action), 0.0f}; // use only yaw angle tAngle.y;
	makevectors(&aDir);
	vscale(&com.globals->fwd, CHECK_FORW_DISTANCE, &vDir);
	vscale(&com.globals->right, sideOfs, &vSide);

	// check needJump		
	vadd(&pev->v.origin, &vSide, &vFrom);
	vFrom.z += (-36 + H_NEED_JUMP);
	vadd(&vFrom, &vDir, &dir);
	trace_line(&vFrom, &dir, false, false, pev, &tr);
	if (tr.fraction < 1.0f) { 
		vectoangles(&tr.planenormal, &planeAngle);
		if (planeAngle.x < 40) {
			if (planeAngle.x == 0) {
				vFrom.z -= H_NEED_JUMP * 0.5f;
				vadd(&vFrom, &vDir, &dir);
				trace_line(&vFrom, &dir, false, false, pev, &trStairs);
				if ((1.001f * trStairs.fraction) < tr.fraction) {
					//if (debugTrace) DEBUG_MSG( "Climbing stairs...\n" );
				} else {
					needJump = 1;	// jump slow if no stairs
					if (debugTrace) DEBUG_MSG( "Need jump!\n" );
				}
			} else {
				needJump = 1;		// jump slow if angle > 50 deg
				if (debugTrace) DEBUG_MSG( "Need jump!\n" );
			}
		} else if (debugTrace) DEBUG_MSG( "No jump, angle ok: %.f!\n", planeAngle.x );
	}

	// check jumpBlocked
	vadd(&pev->v.origin, &vSide, &vFrom);
	vFrom.z += (-36 + H_JUMP_BLOCKED);
	vadd(&vFrom, &vDir, &dir);
	trace_line(&vFrom, &dir, false, false, pev, &tr);
	if (tr.fraction < 1.0) { 
		if (debugTrace) DEBUG_MSG( "Jump blocked!\n" );
		jumpBlocked = true;
	}	

	// check needDuck
	vadd(&pev->v.origin, &vSide, &vFrom);
	vFrom.z += (-36 + H_NEED_DUCK);
	vadd(&vFrom, &vDir, &dir);
	trace_line(&vFrom, &dir, false, false, pev, &tr);
	if (tr.fraction < 1.0) { 
		if (debugTrace) DEBUG_MSG( "Need duck!\n" );
		needDuck = true;
	}	
	
	// check duckBlocked
	vadd(&pev->v.origin, &vSide, &vFrom);
	vFrom.z += (-36 + H_DUCK_BLOCKED);
	vadd(&vFrom, &vDir, &dir);
	trace_line(&vFrom, &dir, false, false, pev, &tr);
	if (tr.fraction < 1.0) { 
		if (debugTrace) DEBUG_MSG( "Duck blocked!\n" );
		duckBlocked = true; 
	}
	
	// set return vars
	if (needJump && !jumpBlocked) {
		res->shouldJump = 1;
		vadd(&pev->v.origin, &vDir, &res->landPos);
	}
	if (needDuck && !duckBlocked) {
		if (mod_id != DMC_DLL) {
			res->shouldDuck = true;
		} else {
			res->blocked = true;
			if (!needJump)
				vectoangles(&tr.planenormal, &planeAngle); // not calculated yet...
			vcopy(&planeAngle, &res->wallAngle);
			return;		// no gap check if blocked
		}
	}
	if (jumpBlocked && duckBlocked) {
		res->blocked = true;
		if (!needJump)
			vectoangles(&tr.planenormal, &planeAngle); // not calculated yet...
		vcopy(&planeAngle, &res->wallAngle);
		return;		// no gap check if blocked
	}
		// needjump ||
	if (needDuck || jumpBlocked || duckBlocked) return; // not free

	if (!action_jumping(action)) {
		vadd(&pev->v.origin, &vSide, &vSide);
		checkJump(&vSide, &vDir, res);
	}
	return;
}


void PB_Roaming::checkSide (int side, float frontOfs, checkWayRes *res) 
// viewing vector supposed to be in com.globals, call directly after checkFront!
{
#define MAX_WALL_DISTANCE   65
#define MAX_TOUCH		   0.77	//(50 / MAX_WALL_DISTANCE)
#define MIN_TOUCH	       0.38	//(25 / MAX_WALL_DISTANCE)

	TRACERESULT tr;
	Vec3D vDir, newdir, start;
	
	vma(&pev->v.origin, frontOfs, &com.globals->fwd, &start);
	res->tooClose = false;
	res->tooFar = false;

	if (side == LEFT)
		vscale(&com.globals->right, -MAX_WALL_DISTANCE, &vDir);
	else if (side == RIGHT)
		vscale(&com.globals->right, MAX_WALL_DISTANCE, &vDir);
	else
		return;

	vadd(&start, &vDir, &newdir);
	trace_line(&start, &newdir, true, false, pev, &tr);

	if (tr.fraction < 1.0f) {
		res->onTouch = true;
		res->wallDistance = tr.fraction * MAX_WALL_DISTANCE;
		vectoangles(&tr.planenormal, &res->wallAngle);
		if (tr.fraction > MAX_TOUCH) res->tooFar   = true;
		if (tr.fraction < MIN_TOUCH) res->tooClose = true;
	//	DEBUG_MSG( "Edict: %i\n", tr.hit->serialnumber);
	} else {
		newdir.z -= 40.0f;
		if (pointcontents(&newdir) == CONTENTS_EMPTY) {
			// following gap instead of wall!
			start.z -= 40.0f;
			trace_line(&newdir, &start, true, false, pev, &tr);
			//if (tr.fraction < 1.0f) {
				tr.fraction = 1.0f - tr.fraction;
				res->onTouch = true;
				res->wallDistance = tr.fraction * MAX_WALL_DISTANCE;
				if (tr.fraction < 1.0f) {
					vcopy(&tr.planenormal, &res->wallAngle);
				} else {
					vcopy(&vDir, &res->wallAngle);
				}
				vinv(&res->wallAngle);
				vectoangles(&res->wallAngle, &res->wallAngle);
				if (tr.fraction > MAX_TOUCH) res->tooFar   = true;
				if (tr.fraction < MIN_TOUCH) res->tooClose = true;
			/*} else { // this should never happen (bot has to stand on something!)
				DEBUG_MSG( "UNEXPECTED RESULT in PB_Roaming::checkSide!\n" );
				res->onTouch = false; 
				res->gap = false;
				res->shouldJump = false;
			}*/
		} else {
			res->onTouch = false; 
			res->gap = false;
			res->shouldJump = false;
			res->wallDistance = 0;
		}
	}
	return;
}


int PB_Roaming::searchExit (Vec3D *wallangle)
// bot is supposed to stand in front of wall 
{
#define TRACE_STEP  72
#define MAX_SPEED	270
float   TRACE_DEPTH = 20 + MAX_SPEED/5;

	TRACERESULT tr;
	bool		traceLeft = true, traceRight = true, foundLeftExit = false, foundRightExit = false;
	Vec3D      vLeftFrom, vRightFrom, vForw, vWallDir, dir;
	float		dLeft = 0, dRight = 0;  // distance to left/right

	wallangle->x = 0;
	wallangle->z = 0;
	makevectors(wallangle);

	vcopy(&com.globals->right, &vWallDir);	// vDir = Vector to wall-right
	vinv(&vWallDir);
	vcopy(&com.globals->fwd, &vForw);	// vForw = Vector towards wall
        vinv(&vForw);

	vcopy(&pev->v.origin, &vLeftFrom);
	vcopy(&pev->v.origin, &vRightFrom);
	int dbgCnt = 0;
	do {
		if (traceLeft) {
			dLeft += TRACE_STEP;
			vma(&vLeftFrom, -TRACE_STEP, &vWallDir, &dir);
			trace_line(&vLeftFrom, &dir, true, false, NULL, &tr);
			if (tr.fraction == 1.0f) { // didn't hit anything
				vcopy(&tr.endpos, &vLeftFrom);
				vma(&vLeftFrom, TRACE_DEPTH, &vForw, &dir);
				trace_line(&vLeftFrom, &dir, true, false, NULL, &tr);
				if (tr.fraction == 1.0f) {
					traceLeft = false;  
					traceRight = false;
					foundLeftExit = true;
				}
			} else
				traceLeft=false; // encountered wall 
		}
		if (traceRight) {
			dRight += TRACE_STEP;
			vma(&vRightFrom, TRACE_STEP, &vWallDir, &dir);
			trace_line(&vRightFrom, &dir, true, false, NULL, &tr);
			if (tr.fraction == 1.0f) { // didn't hit anything
				vcopy(&tr.endpos, &vRightFrom);
				vma(&vRightFrom, TRACE_DEPTH, &vForw, &dir);
				trace_line(&vRightFrom, &dir, true, false, NULL, &tr);
				if (tr.fraction == 1.0f) {
					traceLeft = false;  
					traceRight = false; 
					foundRightExit = true;
				}
			} else
				traceRight = false; // encountered wall 
		}
	} while ((traceRight || traceLeft) && ++dbgCnt < 1000);
	if (dbgCnt == 1000) {
		FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
		fprintf( dfp, ">1000 recursions in PB_Roaming::searchExit!\n" ); 
		fclose( dfp );
	}

	if (foundLeftExit)
		return LEFT;
	else if (foundRightExit)
		return RIGHT;

	return NO_DIR;
}

void PB_Roaming::checkWay( const Vec3D *targetPos ) 
{
	Vec3D tDir, targetAngle;
	checkWayRes left, right, side;

	assert( action != 0 );
	assert( pev != 0 );

	if (action_jumping(action)) {
		vcopy(&jumpTarget, &target);
		vsub(&target, &pev->v.origin, &tDir);
		vectoangles(&tDir, &targetAngle);
		fixangle(&targetAngle);
		float jumpSpeed = vlen2d((Vec2D *)&tDir);
		action_setmoveangleyaw(action, targetAngle.y);
		//action->dontGetStuck();
		action_setspeed(action, 5.0f * jumpSpeed);
		// DEBUG_MSG( "AC! " );
		//marker.drawMarkers();
		return;	// air control!
	}

	vcopy(targetPos, &target);
	vsub(&target, &pev->v.origin, &tDir);
	vectoangles(&tDir, &targetAngle);
	fixangle(&targetAngle);
	float targetDistance = vlen(&tDir);
	
	action_setmaxspeed(action);

	checkFront(-16, &targetAngle, &left);
	checkFront( 16, &targetAngle, &right);	// first check forward...
	bool sideValid = false;		// if side is initialized

	if (followLeft) {
		checkSide(LEFT, 0, &side);	//-20
		if (side.onTouch)
			sideValid = true;

		if (!checkIfPassage) {  // following the wall
			if (!side.onTouch && (passedEdge < com.globals->time)) {	// it has disappeared...
				right.blocked = false;	// direction change -> cancel everything
				left.shouldJump = false;
				right.shouldJump = false;
				if ((fabsf(anglediff(action_moveangleyaw(action), targetAngle.y)) <= 90) &&
					(fabsf(anglediff(action_moveangleyaw(action) + 90, targetAngle.y)) <= 90) ) {
					followLeft = false;
					if (debugWay)
						DEBUG_MSG( "Freed from left wall.\n");
				} else {
					action_setmoveangleyaw(action, action_moveangleyaw(action) + 90);  // turn left
					checkIfPassage = true;
					vcopy(&pev->v.origin, &passageOrigin);
					passageTries = 0;
					if (debugWay)
						DEBUG_MSG( "Searching passage at the left... " );
				}
			} else {  // everything fine
				passageDistance = side.wallDistance + 24;  // if wall disappears next frame
				if (side.tooClose || left.blocked) action_add(action, BOT_STRAFE_RIGHT, NULL);
				else if (side.tooFar) action_add(action, BOT_STRAFE_LEFT, NULL);
			}
		} else { // in search of passage
			passageTries++;
			if (!left.blocked)
				action_add(action, BOT_STRAFE_LEFT, NULL );
			Vec3D moved;
			vsub(&pev->v.origin, &passageOrigin, &moved);
			if (vlen(&moved) >= passageDistance) {
				if (side.onTouch) {	// that's the wall we were looking for 
					action_setmoveangleyaw(action, side.wallAngle.y + 90);
					if (debugWay) DEBUG_MSG( "Alligned to new wall.\n" );
				} else {  // very short wall!
					followLeft = false;		// let him free
					if (debugWay) DEBUG_MSG( "Could not find wall!\n" );
				}
				checkIfPassage=false;
			} else { // not yet reached the passage
				if (passageTries > 8) {
					action_add(action, BOT_STRAFE_RIGHT, NULL);	// follow old direction
					if (passageTries > 20) {
						checkIfPassage = false;
						followLeft = false;		// let him free
						if (debugWay) DEBUG_MSG( "Could not reach it! dist=%.f, gone=%.f.\n", passageDistance, vlen(moved));
					}
				}
			}
		}

		if (right.blocked) {	// in any case check for wall in front
			action_setmoveangleyaw(action, right.wallAngle.y + 90);  // follow wall to Right
			if (debugWay) DEBUG_MSG( " Blocked, turning right.\n");
		}
	}

	else if (followRight) {
		checkSide (RIGHT, 0, &side);	// -20
		if (side.onTouch) sideValid = true;
		
		if (!checkIfPassage) {  // following the wall
			if (!side.onTouch && (passedEdge < com.globals->time)) {  // it has disappeared...
				left.blocked = false;	// direction change -> cancel everything
				left.shouldJump = false;
				right.shouldJump = false;
				if ((fabsf(anglediff(action_moveangleyaw(action), targetAngle.y)) <= 90) &&
					(fabsf(anglediff(action_moveangleyaw(action) - 90, targetAngle.y)) <= 90)) {
					followRight = false;
					if (debugWay) DEBUG_MSG( "Freed from right wall.\n");
				} else {
					action_setmoveangleyaw(action, action_moveangleyaw(action) - 90);  // turn right
					checkIfPassage = true;
					passageOrigin = pev->v.origin;
					passageTries = 0;
					if (debugWay) DEBUG_MSG( "Searching passage at the right... ");
				}
			} else {  // everything fine
				passageDistance = side.wallDistance + 24;  // if wall disappears next frame
				if (side.tooClose || right.blocked) action_add(action, BOT_STRAFE_LEFT, NULL);
				else if (side.tooFar) action_add(action, BOT_STRAFE_RIGHT, NULL);
			}
		} else { // in search of passage
			passageTries++;
			if (!right.blocked) action_add(action, BOT_STRAFE_RIGHT, NULL);
			Vec3D moved;
			vsub(&pev->v.origin, &passageOrigin, &moved);
			if (vlen(&moved)>=passageDistance) {
				if (side.onTouch) {	// that's the wall we were looking for 
					action_setmoveangleyaw(action, side.wallAngle.y - 90);
					if (debugWay) DEBUG_MSG("Alligned to new wall.\n");
				} else {  // very short wall!
					followRight = false;		// let him free
					if (debugWay) DEBUG_MSG( "Could not find wall!\n");
				}
				checkIfPassage = false;
			} else { // not yet reached the passage
				if (passageTries > 8) {
					action_add(action, BOT_STRAFE_LEFT, NULL);	// follow old direction
					if (passageTries > 20) {
						checkIfPassage = false;
						followRight = false;		// let him free
						if (debugWay) DEBUG_MSG( "Could not reach it! dist=%.f, gone=%.f.\n",passageDistance, vlen(moved));
					}
				}
			}
		}

		if (left.blocked) {	// in any case check for wall in front
			action_setmoveangleyaw(action, left.wallAngle.y - 90);  // follow wall to Left
			if (debugWay) DEBUG_MSG(" Blocked, turning left.\n");
		}
	} else { // not following any walls
		if (left.blocked && right.blocked) {	// suddenly a wall
			int turn;
			if (left.wallAngle.y == right.wallAngle.y) { // in front of plain wall
				float adtw = anglediff(left.wallAngle.y, targetAngle.y);
				if (fabsf(adtw) > 135) { // target straight behind
					turn = searchExit(&left.wallAngle);
					if (turn == NO_DIR) {
						if (debugWay) DEBUG_MSG( "Blocked, no exit found: ");
						turn = randomint(LEFT, RIGHT);
					} else if (debugWay)
						DEBUG_MSG( "Blocked, found exit: ");
				} else { // target more in one direction of wall, head to that direction
					if (adtw > 0)
						turn = LEFT;
					else
						turn = RIGHT;
					if (debugWay) DEBUG_MSG( "Blocked, heading for target: ");
				}
			} else { // in corner
				turn = randomint(LEFT, RIGHT);
				passedEdge = com.globals->time + 0.5;
			}

			if (turn == LEFT) {
				if (debugWay) DEBUG_MSG("Turning left, following wall.\n");
				action_setmoveangleyaw(action, left.wallAngle.y - 90);	// follow wall to left
				followRight = true;		// wall should be at the right
			} else if (turn == RIGHT) {
				if (debugWay) DEBUG_MSG("Turning right, following wall.\n");
				action_setmoveangleyaw(action, right.wallAngle.y + 90);	// follow wall to right
				followLeft = true;	// wall should be at the left
			}
		} else {
			action_setmoveangleyaw(action, targetAngle.y);

			if (left.blocked) {
				action_add(action, BOT_STRAFE_RIGHT, NULL);
				strafeRightCount++;
				if (strafeRightCount > 3) {
					if (debugWay) DEBUG_MSG( "Strafing too long, mode set to followLeft\n");
					followLeft = true;
					strafeRightCount = 0;
				}
			} else
				strafeRightCount = 0;
			if (right.blocked) {
				action_add(action, BOT_STRAFE_LEFT, NULL);
				strafeLeftCount++;
				if (strafeLeftCount > 3) {
					if (debugWay) DEBUG_MSG( "Strafing too long, mode set to followRight\n");
					followRight = true;
					strafeLeftCount = 0;
				}
			} else
				strafeLeftCount = 0;
		}
	}
	
	if (!action_jumping(action)) {	// jump...
		if (left.shouldJump > 0 && !right.blocked) {
			jumpTarget = left.landPos;
			//marker.setPos( markerId, jumpTarget );
			action_add(action, BOT_JUMP, NULL);
		} else if (right.shouldJump > 0 && !left.blocked) {
			jumpTarget = right.landPos;
			//marker.setPos( markerId, jumpTarget );
			action_add(action, BOT_JUMP, NULL);
		}
	}

	if ((left.shouldDuck || right.shouldDuck)) action_add(action, BOT_DUCK, NULL); // ...or duck if necessary

	TRACERESULT tr;
	bool targetVisible;
	Vec3D eyePos;
	eyepos(pev, &eyePos);
	trace_line(&target, &eyePos, false, false, 0, &tr);	
	if (tr.fraction != 1.0f && tr.hit != pev) targetVisible = false;
	else targetVisible = true;

	if (targetVisible) { // special behaviour if target is visible:
		if ((targetDistance < 70) ||	     // - too close to target, ignore walls
			 ((followLeft || followRight) && sideValid && (fabsf(anglediff(side.wallAngle.y, targetAngle.y)) <= 90)) ) {	
			//if (debugWay)
			// DEBUG_MSG( "Close to target!\n");
			action_setmoveangleyaw(action, targetAngle.y);
			action_setmaxspeed(action);
			followRight = false;
			followLeft = false;
			checkIfPassage = false;
		}
	}

	// underwater behaviour:
	if (is_underwater( pev )) {
		if (needsair( pev )) {
			action_add(action, BOT_JUMP, NULL);
		} else {
			Vec3D angle = {-60.0f, action_moveangleyaw(action), 0.0f};
			if (targetPos->z > (pev->v.origin.z + 32.0f))
				action_setmoveangle(action, &angle);
			else if (targetPos->z < (pev->v.origin.z - 32.0f)) {
				angle.x = -angle.x;
				action_setmoveangle(action, &angle);
			}
		}
	}
}

bool PB_Roaming::targetNotReachable()
{
	Vec3D dir;
	
	vsub(&target, &pev->v.origin, &dir);
	float dist2d = vlen2d((Vec2D *)&dir);
	if (dist2d < 40 && dist2d > lastXyDist)
		return true;
	lastXyDist = dist2d;

	return false;
}

