#include "parabot.h"
#include "marker.h"
#include "bot.h"
#include <stdio.h>

#define BIGGAP_SCANDIST   32.0f
#define MAX_ZREACH	45
#define NO_DIR 0
#define LEFT   1
#define RIGHT  2

extern int mod_id;

void
roaming_init(ROAMING *pathfinder, EDICT *botEnt, ACTION *act)
{
	pathfinder->pev = botEnt;
	pathfinder->action = act;
	pathfinder->debugtrace = false;
	pathfinder->debugway = false;
	roaming_reset(pathfinder, &zerovector);
	//markerId = marker.newMarker( pathfinder->jumptarget, 1 );
}

void
roaming_reset(ROAMING *pathfinder, const Vec3D *newTarget)
{
	Vec3D tDir, targetAngle;

	pathfinder->checkifpassage = false;
	pathfinder->followleft = false;
	pathfinder->followright = false;
	pathfinder->strafeleftcount = 0;
	pathfinder->straferightcount = 0;
	pathfinder->passededge = 0;
	vsub(newTarget, &pathfinder->pev->v.origin, &tDir);
	vectoangles(&tDir, &targetAngle);
	fixangle(&targetAngle);
	action_setmoveangleyaw(pathfinder->action, targetAngle.y);
	pathfinder->lastxydist = 10000;
}

static bool
roaming_biggapat(ROAMING *pathfinder, Vec3D *pos)
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

static void
roaming_checkjump(ROAMING *pathfinder, Vec3D *origin, Vec3D *_dir, checkWayRes *res)
// returns in *res: 
// 1) no gap:			gap=false
// 2) crossable gap:	gap=true	blocked=false	jumppos		landpos			shouldjump
// 3) uncrossable gap:	gap=true	blocked=true	wallangle	walldistance	toofar	tooclose
{
	#define MAX_JUMP_DIST 200
	#define GAP_SCANDIST   40	

	TRACERESULT tr;
	Vec3D vFrom, vDown = {0.0f, 0.0f, -512.0f}, floor, jumppos, jumpEnd, landpos, planeAngle, dir, dist;

	normalize(_dir);
	vscale(_dir, GAP_SCANDIST, _dir);
	vadd(origin, _dir, &vFrom);

	/////////// just security....
	trace_line(origin, &vFrom, true, false, pathfinder->pev, &tr);
	if (tr.fraction < 1.0f) {
		res->gap = false;
		res->blocked = true;
		DEBUG_MSG("Frpathfinder->action Error in checkJump(), fraction=%.f\n", tr.fraction );
		return;
	}
	//////////

	vadd(&vFrom, &vDown, &dir);
	trace_line(&vFrom, &dir, true, false, pathfinder->pev, &tr);
	vma(&vFrom, tr.fraction, &vDown, &floor);
	floor.z += 1.0f;
	int material = pointcontents(&floor);
	floor.z -= 1.0f;
	vFrom.z += (-36.0f -4.0f);

	// We have: floor and material in front of us
	if(((((floor.z + 36 + MAX_ZREACH) < pathfinder->target.z)
	    && ((floor.z + 36 + 16) < origin->z))	// gap goes deeper than target
	    || tr.fraction == 1.0f			// gap deeper than 512 units
	    || material == CONTENTS_LAVA
	    || material == CONTENTS_SLIME)		// dangerous
	    && roaming_biggapat(pathfinder, &vFrom)) {
		if (pathfinder->debugway) {
			if (material == CONTENTS_LAVA) {
				DEBUG_MSG("Running into lava\n");
			} else {
				DEBUG_MSG("Approaching gap, ");
			}
		}
		res->gap = true;
		vsub(&vFrom, _dir, &dir);
		trace_line(&vFrom, &dir, true, false, (pathfinder->pev), &tr);
		vma(&vFrom, -tr.fraction, _dir, &jumppos);
		vinv(&tr.planenormal);
		vectoangles(&tr.planenormal, &planeAngle);
		// We have: edge(jump-)position and angle
		Vec3D jumpVec, jumpdir;
		vscale(_dir, MAX_JUMP_DIST / GAP_SCANDIST, &jumpVec);
		Vec3D h = {0.0f, 0.0f, 50.0f};
		vadd(&jumppos, &h, &jumpEnd);
		vadd(&jumpEnd, &jumpVec, &jumpdir);
		trace_line(&jumpEnd, &jumpdir, true, false, pathfinder->pev, &tr);
		vma(&jumpEnd, tr.fraction - 0.1f, &jumpVec, &jumpEnd); // 20 vor Trace-Ende
		vadd(&jumpEnd, &vDown, &jumpdir);
		trace_line(&jumpEnd, &jumpdir, true, false, pathfinder->pev, &tr);
		vma(&jumpEnd, tr.fraction, &vDown, &landpos); // floor on other side
		landpos.z += 1.0f;
		material = pointcontents(&landpos);
		landpos.z -= 1.0f;

		// We have: floor and material at our landing pos
		vsub(&jumppos, &pathfinder->pev->v.origin, &dist);
		float d = vlen2d((Vec2D *)&dist);
		Vec3D dirNorm;
		vcopy(&dir, &dirNorm);
		normalize(&dirNorm);
		float dot = dotproduct(&pathfinder->pev->v.velocity, &dirNorm);
		float minJumpSpeed = 0.9f * action_getmaxspeed(pathfinder->action);

		if ((landpos.z + 36.0f + MAX_ZREACH) < pathfinder->target.z
		    || tr.fraction == 1.0f
//		    || pathfinder->followleft || pathfinder->followright			// probably not moving in jump direction!
		    || dot < minJumpSpeed			// too slow
		    || material == CONTENTS_LAVA
		    || material == CONTENTS_SLIME) {	// can't cross
			if (pathfinder->debugway) {
				if (material==CONTENTS_LAVA) {
					DEBUG_MSG( "would jump into lava\n" );
				} else
					DEBUG_MSG( "landpos too low: %.f \n", (landpos.z + 36.0f) );
			}
			res->blocked = true;
			res->shouldjump = 0;
			res->wallangle = planeAngle;
			res->walldistance = d;
			if (d > 50) res->toofar   = true;
			if (d < 25) res->tooclose = true;
		} else {
			res->blocked = false;
			res->jumppos = jumppos;
			res->landpos = landpos;
			if (d < 25) {
				if (pathfinder->debugway)
					DEBUG_MSG("jump!\n");
				res->shouldjump = 2;	// jump fast
			} else if (pathfinder->debugway)
				DEBUG_MSG( "dist=%.f\n",d );
		}
	} else { // no gap 
		res->gap = false;
	}
}

static void
roaming_checkfront(ROAMING *pathfinder, float sideOfs, Vec3D *angle, checkWayRes *res)
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
	res->shouldduck = false;
	res->shouldjump = 0;

	// init vDir:
	Vec3D aDir = {0.0f, action_moveangleyaw(pathfinder->action), 0.0f}; // use only yaw angle tAngle.y;
	makevectors(&aDir);
	vscale(&com.globals->fwd, CHECK_FORW_DISTANCE, &vDir);
	vscale(&com.globals->right, sideOfs, &vSide);

	// check needJump		
	vadd(&pathfinder->pev->v.origin, &vSide, &vFrom);
	vFrom.z += (-36 + H_NEED_JUMP);
	vadd(&vFrom, &vDir, &dir);
	trace_line(&vFrom, &dir, false, false, pathfinder->pev, &tr);
	if (tr.fraction < 1.0f) { 
		vectoangles(&tr.planenormal, &planeAngle);
		if (planeAngle.x < 40) {
			if (planeAngle.x == 0) {
				vFrom.z -= H_NEED_JUMP * 0.5f;
				vadd(&vFrom, &vDir, &dir);
				trace_line(&vFrom, &dir, false, false, pathfinder->pev, &trStairs);
				if ((1.001f * trStairs.fraction) < tr.fraction) {
					//if (pathfinder->debugtrace) DEBUG_MSG( "Climbing stairs...\n" );
				} else {
					needJump = 1;	// jump slow if no stairs
					if (pathfinder->debugtrace) DEBUG_MSG( "Need jump!\n" );
				}
			} else {
				needJump = 1;		// jump slow if angle > 50 deg
				if (pathfinder->debugtrace) DEBUG_MSG( "Need jump!\n" );
			}
		} else if (pathfinder->debugtrace) DEBUG_MSG( "No jump, angle ok: %.f!\n", planeAngle.x );
	}

	// check jumpBlocked
	vadd(&pathfinder->pev->v.origin, &vSide, &vFrom);
	vFrom.z += (-36 + H_JUMP_BLOCKED);
	vadd(&vFrom, &vDir, &dir);
	trace_line(&vFrom, &dir, false, false, pathfinder->pev, &tr);
	if (tr.fraction < 1.0) { 
		if (pathfinder->debugtrace) DEBUG_MSG( "Jump blocked!\n" );
		jumpBlocked = true;
	}	

	// check needDuck
	vadd(&pathfinder->pev->v.origin, &vSide, &vFrom);
	vFrom.z += (-36 + H_NEED_DUCK);
	vadd(&vFrom, &vDir, &dir);
	trace_line(&vFrom, &dir, false, false, pathfinder->pev, &tr);
	if (tr.fraction < 1.0) { 
		if (pathfinder->debugtrace) DEBUG_MSG( "Need duck!\n" );
		needDuck = true;
	}	
	
	// check duckBlocked
	vadd(&pathfinder->pev->v.origin, &vSide, &vFrom);
	vFrom.z += (-36 + H_DUCK_BLOCKED);
	vadd(&vFrom, &vDir, &dir);
	trace_line(&vFrom, &dir, false, false, pathfinder->pev, &tr);
	if (tr.fraction < 1.0) { 
		if (pathfinder->debugtrace) DEBUG_MSG( "Duck blocked!\n" );
		duckBlocked = true; 
	}
	
	// set return vars
	if (needJump && !jumpBlocked) {
		res->shouldjump = 1;
		vadd(&pathfinder->pev->v.origin, &vDir, &res->landpos);
	}
	if (needDuck && !duckBlocked) {
		if (mod_id != DMC_DLL) {
			res->shouldduck = true;
		} else {
			res->blocked = true;
			if (!needJump)
				vectoangles(&tr.planenormal, &planeAngle); // not calculated yet...
			vcopy(&planeAngle, &res->wallangle);
			return;		// no gap check if blocked
		}
	}
	if (jumpBlocked && duckBlocked) {
		res->blocked = true;
		if (!needJump)
			vectoangles(&tr.planenormal, &planeAngle); // not calculated yet...
		vcopy(&planeAngle, &res->wallangle);
		return;		// no gap check if blocked
	}
		// needjump ||
	if (needDuck || jumpBlocked || duckBlocked) return; // not free

	if (!action_jumping(pathfinder->action)) {
		vadd(&pathfinder->pev->v.origin, &vSide, &vSide);
		roaming_checkjump(pathfinder, &vSide, &vDir, res);
	}
	return;
}

static void
roaming_checkside(ROAMING *pathfinder, int side, float frontOfs, checkWayRes *res) 
// viewing vector supposed to be in com.globals, call directly after roaming_checkfront!
{
#define MAX_WALL_DISTANCE   65
#define MAX_TOUCH		   0.77	//(50 / MAX_WALL_DISTANCE)
#define MIN_TOUCH	       0.38	//(25 / MAX_WALL_DISTANCE)

	TRACERESULT tr;
	Vec3D vDir, newdir, start;
	
	vma(&pathfinder->pev->v.origin, frontOfs, &com.globals->fwd, &start);
	res->tooclose = false;
	res->toofar = false;

	if (side == LEFT)
		vscale(&com.globals->right, -MAX_WALL_DISTANCE, &vDir);
	else if (side == RIGHT)
		vscale(&com.globals->right, MAX_WALL_DISTANCE, &vDir);
	else
		return;

	vadd(&start, &vDir, &newdir);
	trace_line(&start, &newdir, true, false, pathfinder->pev, &tr);

	if (tr.fraction < 1.0f) {
		res->ontouch = true;
		res->walldistance = tr.fraction * MAX_WALL_DISTANCE;
		vectoangles(&tr.planenormal, &res->wallangle);
		if (tr.fraction > MAX_TOUCH) res->toofar   = true;
		if (tr.fraction < MIN_TOUCH) res->tooclose = true;
	//	DEBUG_MSG( "Edict: %i\n", tr.hit->serialnumber);
	} else {
		newdir.z -= 40.0f;
		if (pointcontents(&newdir) == CONTENTS_EMPTY) {
			// following gap instead of wall!
			start.z -= 40.0f;
			trace_line(&newdir, &start, true, false, pathfinder->pev, &tr);
			//if (tr.fraction < 1.0f) {
				tr.fraction = 1.0f - tr.fraction;
				res->ontouch = true;
				res->walldistance = tr.fraction * MAX_WALL_DISTANCE;
				if (tr.fraction < 1.0f) {
					vcopy(&tr.planenormal, &res->wallangle);
				} else {
					vcopy(&vDir, &res->wallangle);
				}
				vinv(&res->wallangle);
				vectoangles(&res->wallangle, &res->wallangle);
				if (tr.fraction > MAX_TOUCH) res->toofar   = true;
				if (tr.fraction < MIN_TOUCH) res->tooclose = true;
			/*} else { // this should never happen (bot has to stand on something!)
				DEBUG_MSG( "UNEXPECTED RESULT in PB_Roaming::roaming_checkside!\n" );
				res->ontouch = false; 
				res->gap = false;
				res->shouldjump = false;
			}*/
		} else {
			res->ontouch = false; 
			res->gap = false;
			res->shouldjump = false;
			res->walldistance = 0;
		}
	}
	return;
}

static int
roaming_searchexit(ROAMING *pathfinder, Vec3D *wallangle)
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

	vcopy(&pathfinder->pev->v.origin, &vLeftFrom);
	vcopy(&pathfinder->pev->v.origin, &vRightFrom);
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
		fprintf( dfp, ">1000 recursions in PB_Roaming::roaming_searchexit!\n" ); 
		fclose( dfp );
	}

	if (foundLeftExit)
		return LEFT;
	else if (foundRightExit)
		return RIGHT;

	return NO_DIR;
}

void
roaming_checkway(ROAMING *pathfinder, const Vec3D *targetPos)
{
	Vec3D tDir, targetAngle;
	checkWayRes left, right, side;

	assert( pathfinder->action != 0 );
	assert( pathfinder->pev != 0 );

	if (action_jumping(pathfinder->action)) {
		vcopy(&pathfinder->jumptarget, &pathfinder->target);
		vsub(&pathfinder->target, &pathfinder->pev->v.origin, &tDir);
		vectoangles(&tDir, &targetAngle);
		fixangle(&targetAngle);
		float jumpSpeed = vlen2d((Vec2D *)&tDir);
		action_setmoveangleyaw(pathfinder->action, targetAngle.y);
		//pathfinder->action->dontGetStuck();
		action_setspeed(pathfinder->action, 5.0f * jumpSpeed);
		// DEBUG_MSG( "AC! " );
		//marker.drawMarkers();
		return;	// air control!
	}

	vcopy(targetPos, &pathfinder->target);
	vsub(&pathfinder->target, &pathfinder->pev->v.origin, &tDir);
	vectoangles(&tDir, &targetAngle);
	fixangle(&targetAngle);
	float targetDistance = vlen(&tDir);
	
	action_setmaxspeed(pathfinder->action);

	roaming_checkfront(pathfinder, -16, &targetAngle, &left);
	roaming_checkfront(pathfinder, 16, &targetAngle, &right);	// first check forward...
	bool sideValid = false;		// if side is initialized

	if (pathfinder->followleft) {
		roaming_checkside(pathfinder, LEFT, 0, &side);	//-20
		if (side.ontouch)
			sideValid = true;

		if (!pathfinder->checkifpassage) {  // following the wall
			if (!side.ontouch && (pathfinder->passededge < com.globals->time)) {	// it has disappeared...
				right.blocked = false;	// direction change -> cancel everything
				left.shouldjump = false;
				right.shouldjump = false;
				if ((fabsf(anglediff(action_moveangleyaw(pathfinder->action), targetAngle.y)) <= 90) &&
					(fabsf(anglediff(action_moveangleyaw(pathfinder->action) + 90, targetAngle.y)) <= 90) ) {
					pathfinder->followleft = false;
					if (pathfinder->debugway)
						DEBUG_MSG( "Freed from left wall.\n");
				} else {
					action_setmoveangleyaw(pathfinder->action, action_moveangleyaw(pathfinder->action) + 90);  // turn left
					pathfinder->checkifpassage = true;
					vcopy(&pathfinder->pev->v.origin, &pathfinder->passageorigin);
					pathfinder->passagetries = 0;
					if (pathfinder->debugway)
						DEBUG_MSG( "Searching passage at the left... " );
				}
			} else {  // everything fine
				pathfinder->passagedistance = side.walldistance + 24;  // if wall disappears next frame
				if (side.tooclose || left.blocked) action_add(pathfinder->action, BOT_STRAFE_RIGHT, NULL);
				else if (side.toofar) action_add(pathfinder->action, BOT_STRAFE_LEFT, NULL);
			}
		} else { // in search of passage
			pathfinder->passagetries++;
			if (!left.blocked)
				action_add(pathfinder->action, BOT_STRAFE_LEFT, NULL );
			Vec3D moved;
			vsub(&pathfinder->pev->v.origin, &pathfinder->passageorigin, &moved);
			if (vlen(&moved) >= pathfinder->passagedistance) {
				if (side.ontouch) {	// that's the wall we were looking for 
					action_setmoveangleyaw(pathfinder->action, side.wallangle.y + 90);
					if (pathfinder->debugway) DEBUG_MSG( "Alligned to new wall.\n" );
				} else {  // very short wall!
					pathfinder->followleft = false;		// let him free
					if (pathfinder->debugway) DEBUG_MSG( "Could not find wall!\n" );
				}
				pathfinder->checkifpassage=false;
			} else { // not yet reached the passage
				if (pathfinder->passagetries > 8) {
					action_add(pathfinder->action, BOT_STRAFE_RIGHT, NULL);	// follow old direction
					if (pathfinder->passagetries > 20) {
						pathfinder->checkifpassage = false;
						pathfinder->followleft = false;		// let him free
						if (pathfinder->debugway) DEBUG_MSG( "Could not reach it! dist=%.f, gone=%.f.\n", pathfinder->passagedistance, vlen(moved));
					}
				}
			}
		}

		if (right.blocked) {	// in any case check for wall in front
			action_setmoveangleyaw(pathfinder->action, right.wallangle.y + 90);  // follow wall to Right
			if (pathfinder->debugway) DEBUG_MSG( " Blocked, turning right.\n");
		}
	} else if (pathfinder->followright) {
		roaming_checkside(pathfinder, RIGHT, 0, &side);	// -20
		if (side.ontouch) sideValid = true;
		
		if (!pathfinder->checkifpassage) {  // following the wall
			if (!side.ontouch && (pathfinder->passededge < com.globals->time)) {  // it has disappeared...
				left.blocked = false;	// direction change -> cancel everything
				left.shouldjump = false;
				right.shouldjump = false;
				if ((fabsf(anglediff(action_moveangleyaw(pathfinder->action), targetAngle.y)) <= 90) &&
					(fabsf(anglediff(action_moveangleyaw(pathfinder->action) - 90, targetAngle.y)) <= 90)) {
					pathfinder->followright = false;
					if (pathfinder->debugway) DEBUG_MSG( "Freed from right wall.\n");
				} else {
					action_setmoveangleyaw(pathfinder->action, action_moveangleyaw(pathfinder->action) - 90);  // turn right
					pathfinder->checkifpassage = true;
					pathfinder->passageorigin = pathfinder->pev->v.origin;
					pathfinder->passagetries = 0;
					if (pathfinder->debugway) DEBUG_MSG( "Searching passage at the right... ");
				}
			} else {  // everything fine
				pathfinder->passagedistance = side.walldistance + 24;  // if wall disappears next frame
				if (side.tooclose || right.blocked) action_add(pathfinder->action, BOT_STRAFE_LEFT, NULL);
				else if (side.toofar) action_add(pathfinder->action, BOT_STRAFE_RIGHT, NULL);
			}
		} else { // in search of passage
			pathfinder->passagetries++;
			if (!right.blocked) action_add(pathfinder->action, BOT_STRAFE_RIGHT, NULL);
			Vec3D moved;
			vsub(&pathfinder->pev->v.origin, &pathfinder->passageorigin, &moved);
			if (vlen(&moved)>=pathfinder->passagedistance) {
				if (side.ontouch) {	// that's the wall we were looking for 
					action_setmoveangleyaw(pathfinder->action, side.wallangle.y - 90);
					if (pathfinder->debugway) DEBUG_MSG("Alligned to new wall.\n");
				} else {  // very short wall!
					pathfinder->followright = false;		// let him free
					if (pathfinder->debugway) DEBUG_MSG( "Could not find wall!\n");
				}
				pathfinder->checkifpassage = false;
			} else { // not yet reached the passage
				if (pathfinder->passagetries > 8) {
					action_add(pathfinder->action, BOT_STRAFE_LEFT, NULL);	// follow old direction
					if (pathfinder->passagetries > 20) {
						pathfinder->checkifpassage = false;
						pathfinder->followright = false;		// let him free
						if (pathfinder->debugway) DEBUG_MSG( "Could not reach it! dist=%.f, gone=%.f.\n",pathfinder->passagedistance, vlen(moved));
					}
				}
			}
		}

		if (left.blocked) {	// in any case check for wall in front
			action_setmoveangleyaw(pathfinder->action, left.wallangle.y - 90);  // follow wall to Left
			if (pathfinder->debugway) DEBUG_MSG(" Blocked, turning left.\n");
		}
	} else { // not following any walls
		if (left.blocked && right.blocked) {	// suddenly a wall
			int turn;
			if (left.wallangle.y == right.wallangle.y) { // in front of plain wall
				float adtw = anglediff(left.wallangle.y, targetAngle.y);
				if (fabsf(adtw) > 135) { // target straight behind
					turn = roaming_searchexit(pathfinder, &left.wallangle);
					if (turn == NO_DIR) {
						if (pathfinder->debugway) DEBUG_MSG( "Blocked, no exit found: ");
						turn = randomint(LEFT, RIGHT);
					} else if (pathfinder->debugway)
						DEBUG_MSG( "Blocked, found exit: ");
				} else { // target more in one direction of wall, head to that direction
					if (adtw > 0)
						turn = LEFT;
					else
						turn = RIGHT;
					if (pathfinder->debugway) DEBUG_MSG( "Blocked, heading for target: ");
				}
			} else { // in corner
				turn = randomint(LEFT, RIGHT);
				pathfinder->passededge = com.globals->time + 0.5;
			}

			if (turn == LEFT) {
				if (pathfinder->debugway) DEBUG_MSG("Turning left, following wall.\n");
				action_setmoveangleyaw(pathfinder->action, left.wallangle.y - 90);	// follow wall to left
				pathfinder->followright = true;		// wall should be at the right
			} else if (turn == RIGHT) {
				if (pathfinder->debugway) DEBUG_MSG("Turning right, following wall.\n");
				action_setmoveangleyaw(pathfinder->action, right.wallangle.y + 90);	// follow wall to right
				pathfinder->followleft = true;	// wall should be at the left
			}
		} else {
			action_setmoveangleyaw(pathfinder->action, targetAngle.y);

			if (left.blocked) {
				action_add(pathfinder->action, BOT_STRAFE_RIGHT, NULL);
				pathfinder->straferightcount++;
				if (pathfinder->straferightcount > 3) {
					if (pathfinder->debugway) DEBUG_MSG( "Strafing too long, mode set to pathfinder->followleft\n");
					pathfinder->followleft = true;
					pathfinder->straferightcount = 0;
				}
			} else
				pathfinder->straferightcount = 0;
			if (right.blocked) {
				action_add(pathfinder->action, BOT_STRAFE_LEFT, NULL);
				pathfinder->strafeleftcount++;
				if (pathfinder->strafeleftcount > 3) {
					if (pathfinder->debugway) DEBUG_MSG( "Strafing too long, mode set to pathfinder->followright\n");
					pathfinder->followright = true;
					pathfinder->strafeleftcount = 0;
				}
			} else
				pathfinder->strafeleftcount = 0;
		}
	}
	
	if (!action_jumping(pathfinder->action)) {	// jump...
		if (left.shouldjump > 0 && !right.blocked) {
			pathfinder->jumptarget = left.landpos;
			//marker.setPos( markerId, pathfinder->jumptarget );
			action_add(pathfinder->action, BOT_JUMP, NULL);
		} else if (right.shouldjump > 0 && !left.blocked) {
			pathfinder->jumptarget = right.landpos;
			//marker.setPos( markerId, pathfinder->jumptarget );
			action_add(pathfinder->action, BOT_JUMP, NULL);
		}
	}

	if ((left.shouldduck || right.shouldduck)) action_add(pathfinder->action, BOT_DUCK, NULL); // ...or duck if necessary

	TRACERESULT tr;
	bool targetVisible;
	Vec3D eyePos;
	eyepos(pathfinder->pev, &eyePos);
	trace_line(&pathfinder->target, &eyePos, false, false, 0, &tr);	
	if (tr.fraction != 1.0f && tr.hit != pathfinder->pev) targetVisible = false;
	else targetVisible = true;

	if (targetVisible) { // special behaviour if target is visible:
		if ((targetDistance < 70) ||	     // - too close to target, ignore walls
			 ((pathfinder->followleft || pathfinder->followright) && sideValid && (fabsf(anglediff(side.wallangle.y, targetAngle.y)) <= 90)) ) {	
			//if (pathfinder->debugway)
			// DEBUG_MSG( "Close to target!\n");
			action_setmoveangleyaw(pathfinder->action, targetAngle.y);
			action_setmaxspeed(pathfinder->action);
			pathfinder->followright = false;
			pathfinder->followleft = false;
			pathfinder->checkifpassage = false;
		}
	}

	// underwater behaviour:
	if (is_underwater( pathfinder->pev )) {
		if (needsair( pathfinder->pev )) {
			action_add(pathfinder->action, BOT_JUMP, NULL);
		} else {
			Vec3D angle = {-60.0f, action_moveangleyaw(pathfinder->action), 0.0f};
			if (targetPos->z > (pathfinder->pev->v.origin.z + 32.0f))
				action_setmoveangle(pathfinder->action, &angle);
			else if (targetPos->z < (pathfinder->pev->v.origin.z - 32.0f)) {
				angle.x = -angle.x;
				action_setmoveangle(pathfinder->action, &angle);
			}
		}
	}
}

bool
roaming_targetnotreachable(ROAMING *pathfinder)
{
	Vec3D dir;
	
	vsub(&pathfinder->target, &pathfinder->pev->v.origin, &dir);
	float dist2d = vlen2d((Vec2D *)&dir);
	if (dist2d < 40.0f && dist2d > pathfinder->lastxydist)
		return true;
	pathfinder->lastxydist = dist2d;

	return false;
}
