#include "parabot.h"
#include "action.h"

extern int mod_id;
extern bool pb_pause;

void
action_init(ACTION *action, EDICT *bot)
{
	action->e = bot;
	action->injump = false;
	action->nextjumptime = 0;
	action->usecounter = 0;
	action->nextusetime = 0;
	action->duckendtime = 0;
	action->stopendtime = 0;
	action->nextviewupdate = 0;
	vcopy(&zerovector, &action->deltaview);
	action->turncount = 1;
	action->maxturn = 0;
	action->finejump = false;
	action->longjumpstate = 0;
	action->maxspeed = servermaxspeed();
	vcopy(&zerovector, &action->viewangle);
	vcopy(&zerovector, &action->currentview);
	vcopy(&zerovector, &action->targetpos);
	action->hitprob = 0;
	action->targetdist = 0;
	action->lastmove = worldtime();
	action->lastmovecheck = worldtime();
	action->vupdtime = 0.1;
	action->weaponcone = 0.1;	// 5°
	memset(&action->targetdiff, 0, sizeof action->targetdiff);
}

void
action_reset(ACTION *action)
{ 
	action->action = 0;
	action->speed = 0;
	action->strafe = 0;
	action->viewangle.z = 0;
	action->moveangle.z = 0;
	action->currentview.z = 0;
	action->viewprior = 0;
	action->moveprior = 0;
	action->notstucking = false;
	vcopy(&zerovector, &action->targetvel);
}

void
action_add(ACTION *action, int code, Vec3D *exactPos)
{
	switch (code) {
	case BOT_JUMP:
		action->action |= ACTION_JUMP;
		if (!is_underwater(action->e)) {	// no jumps underwater
			action->injump = true;
			action->jumppos = action->e->v.origin.z;
		}
		break;
	case BOT_DELAYED_JUMP:
		if (exactPos) {
			action->finejump = true;
			vcopy(exactPos, &action->finejumppos);
		}
		action->nextjumptime = worldtime() + 1.0;
		break;
	case BOT_LONGJUMP:
		action->longjumpstate = 2;
		break;
	case BOT_USE:
		if (exactPos) {
			action->nextusetime = worldtime() + 0.5;
			action->nextusepos = *exactPos;
		} else
			action->usecounter = 6;
		break;
	case BOT_DUCK:
		action->action |= ACTION_CROUCH;
		break;
	case BOT_DUCK_LONG:
		action->duckendtime = worldtime() + 1.5;
		break;
	case BOT_STOP_RUNNING:
		action->stopendtime = worldtime() + 0.5;
		break;
	case BOT_FIRE_PRIM:
		action->action |= ACTION_ATTACK1;
		break;
	case BOT_FIRE_SEC:
		action->action |= ACTION_ATTACK2;
		break;
	case BOT_RELEASE_SEC:
		action->action &= ~ACTION_ATTACK2;
		break;
	case BOT_RELOAD:
		action->action |= ACTION_RELOAD;
		break;
	case BOT_STRAFE_LEFT:
		// DEBUG_MSG( "Strafing left\n" );
		if (exactPos)
			action->strafe = -exactPos->x;
		else
			action->strafe = -400;	// strong influence
		break;
	case BOT_STRAFE_RIGHT:
		// DEBUG_MSG( "Strafing right\n" );
		if (exactPos)
			action->strafe = exactPos->x;
		else
			action->strafe = 400;	// strong influence
		break;
	}
}

void
action_setmaxspeed(ACTION *action)
{
	action_setspeed(action, action->maxspeed);
}

float
action_getmaxspeed(ACTION *action)
{
	return action->maxspeed;
}

void
action_setmoveangle(ACTION *action, Vec3D *angle) 
{ 
	fixangle(angle);
	vcopy(angle, &action->moveangle); 
	//DEBUG_MSG("setMove  ");
}

void
action_setmoveangleyaw(ACTION *action, float angle)
{
	Vec3D ma = {0, angle, 0};
	fixangle(&ma);
	vcopy(&ma, &action->moveangle);
	//DEBUG_MSG("setMoveYaw  ");
}

void
action_setmovedir(ACTION *action, Vec3D *vec, int prior)
{
	if (prior >= action->moveprior) {
		Vec3D angle, dir;

		vsub(vec, &action->e->v.origin, &dir);
		vectoangles(&dir, &angle);
		fixangle(&angle);
		angle.x = -angle.x;
		action_setmoveangle(action, &angle);
		action->moveprior = prior;
		// DEBUG_MSG("setMoveDir  ");
	}
}

float
action_moveangleyaw(ACTION *action)
{
	return action->moveangle.y;
}

bool
action_jumping(ACTION *action)
{
	return action->injump;
}

bool
action_pausing(ACTION *action)
{
	return action->notstucking;
}

float
action_targetaccuracy(ACTION *action)
{
	return action->hitprob;
}

void
action_getmovedir(ACTION *action, Vec3D *movedir)
{
	fixangle(&action->moveangle);
	makevectors(&action->moveangle);
	vcopy(&com.globals->fwd, movedir);
}

void
action_setviewangle(ACTION *action, Vec3D *angle, int prior)
{ 
	if (prior >= action->viewprior) {
		vcopy(angle, &action->viewangle);
		action->viewprior = prior;
		vcopy(&zerovector, &action->targetvel);	// must be set != 0 afterwards!
	}
}

void
action_setviewdir(ACTION *action, Vec3D *vec, int prior)
{
	if (prior >= action->viewprior) {
		Vec3D angle, dir;

		eyepos(action->e, &dir);
		vsub(vec, &dir, &dir);
		vectoangles(&dir, &angle);
		fixangle(&angle);
		angle.x = -angle.x;
		action_setviewangle(action, &angle, prior);
		action->viewprior = prior;
	}
}

void
action_setviewlikemove(ACTION *action)
{ 
	if (action->viewprior == 0)
		action->viewangle = action->moveangle; 
}

float
action_estimatehitprob(ACTION *action)
{
	Vec3D currentDiff;
	float dirErrorForSkill[10] = { 20, 15, 12, 10, 8, 6, 4, 2, 1, 0 };
	float dirError, cdl, center, hsize, prob;

	dirError = 0.5f * dirErrorForSkill[action->aimskill - 1];
	vsub(&action->viewangle, &action->currentview, &currentDiff);
	fixangle(&currentDiff);

	cdl = vlen(&currentDiff) + randomfloat(-dirError, dirError) + randomfloat(-dirError, dirError);

	if (cdl > 45.0f)
		return 0;

	center = 0.0001894f * action->targetdist * cdl / action->weaponcone;
	hsize = 32.0f / (action->weaponcone * action->targetdist);
	prob = getprob(center + hsize) - getprob(center - hsize);

	return prob;
}

void
action_setaimdir(ACTION *action, Vec3D *currentPos, Vec3D *relVelocity)
{
	if (action->viewprior <= 2) {
		Vec3D tVec, angle;

		vcopy(currentPos, &action->targetpos);
		eyepos(action->e, &tVec);
		vsub(&action->targetpos, &tVec, &tVec);
		action->targetdist = vlen(&tVec);
		vectoangles(&tVec, &angle);
		fixangle(&angle);
		angle.x = -angle.x;
		action_setviewangle(action, &angle, 2);
		vcopy(relVelocity, &action->targetvel);
		action->hitprob = action_estimatehitprob(action);
	} else {
		DEBUG_MSG("Aiming priority too low!\n");
	}
}

void
action_setweaponcone(ACTION *action, float cone)
{
	action->weaponcone = cone;
}

void
action_setspeed(ACTION *action, float value)
{
	action->speed = value;
}

void
action_setaimskill(ACTION *action, int skill)
{
	float updateTimes[10] = { 0.1, 0.1, 0.2, 0.2, 0.3, 0.3, 0.4, 0.4, 0.5, 0.6 }; 
	float accuracies[10] = { 0.0, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
		
	action->aimskill = skill;
	action->vupdtime = updateTimes[10 - skill];
	action->aimaccuracy = accuracies[10 - skill];
//	INFO_MSG( "Set Aim to %i\n", skill );
}

void
action_calcviewangle(ACTION *action, Vec3D *v_angle)
{
	Vec3D currentDiff, toAdd;

	action->viewangle.z = 0;
	fixangle(&action->viewangle);		// optimal viewAngle
	action->currentview.z = 0;
	vsub(&action->viewangle, &action->currentview, &currentDiff);
	fixangle(&currentDiff);	// current angle difference to target

	// try to estimate hit probability
	for (int i = (MAX_VDELAY - 1); i > 0; i--)
		action->targetdiff[i] = action->targetdiff[i - 1];

	action->targetdiff[0] = vlen(&currentDiff) * action->targetdist;
	action->hitprob = action_estimatehitprob(action);

	if (worldtime_reached(action->nextviewupdate)) {
		action->nextviewupdate = worldtime() + action->vupdtime;

		if (!vcomp(&action->targetvel, &zerovector)) {
			// predict position for moving target:
			Vec3D predpos, tVec, angle;

			vma(&action->targetpos, action->vupdtime, &action->targetvel, &predpos);
			eyepos(action->e, &tVec);
			vsub(&predpos, &tVec, &tVec);
			vectoangles(&tVec, &angle);
			fixangle(&angle);
			angle.x = -angle.x;
			vsub(&angle, &action->currentview, &action->deltaview);
		} else {
			vcopy(&currentDiff, &action->deltaview);
		}

		// take accuracy into account:
		float distConst = 5.0f / ( 2.0f + vlen(&action->deltaview) );
		float bl = 1.0f / ( 1.0f + distConst ) - 0.4f;
		float bh = 1.5f + distConst;
		bl = action->aimaccuracy * (1.0f - bl);
		bh = action->aimaccuracy * (bh - 1.0f);	// rnd( 1-a*bl, 1+a*bh)

		float acc = randomfloat(1.0f - bl, 1.0f + bh);
		vscale(&action->deltaview, acc, &action->deltaview);
		fixangle(&action->deltaview);
		action->maxturn = vlen(&action->deltaview) / (float)action->turncount;
		action->turncount = 0;
	}

	vcopy(&action->deltaview, &toAdd);
	if (toAdd.x > action->maxturn)
		toAdd.x = action->maxturn;
	else if (toAdd.x < -action->maxturn)
		toAdd.x = -action->maxturn;

	if (toAdd.y > action->maxturn)
		toAdd.y = action->maxturn;
	else if (toAdd.y < -action->maxturn)
		toAdd.y = -action->maxturn;

	vsub(&action->deltaview, &toAdd, &action->deltaview);
	vadd(&action->currentview, &toAdd, &action->currentview);
	++action->turncount;

	fixangle(&action->currentview);
	vcopy(&action->currentview, v_angle);
}

bool
action_gotstuck(ACTION *action)
{
	if ((action->lastmovecheck - action->lastmove) > 1.0f) {
		DEBUG_MSG("BOT IS STUCK!\n");
		return true;
	}
	return false;
}

void
action_resetstuck(ACTION *action)
{
	action->lastmove = action->lastmovecheck;
}

void
action_dontgetstuck(ACTION *action)
{
	action->notstucking = true;
}

void
action_getviewangle(ACTION *action, Vec3D *viewangle)
{
	vcopy(&action->viewangle, viewangle);
}

int
action_getaimskill(ACTION *action)
{
	return action->aimskill;
}

void
action_perform(ACTION *action)
{
	if (action->injump && !(action->action & ACTION_JUMP)) {	// jumping with jump button released
		action->action |= ACTION_CROUCH;
		if ((action->e->v.flags & FL_ONGROUND) || action->e->v.origin.z < (action->jumppos - 100)) {
			action->action &= ~ACTION_CROUCH;
			action->injump = false;
		}
	}
	// check for long duck
	if (action->duckendtime > 0) {
		if (worldtime() < action->duckendtime)
			action_add(action, BOT_DUCK, NULL);
		else
			action->duckendtime = 0;
	}
	// check for use
	if (action->usecounter > 0) {
		--action->usecounter;
		if (action->usecounter & 1)
			action->action |= ACTION_USE;
	}
	if (action->nextusetime > 0) {
		action_setaimdir(action, &action->nextusepos, NULL);	// to be able to use accuracy
		action_setmovedir(action, &action->nextusepos, 3);
		action_setmaxspeed(action);
		// don't get stuck...
		action_dontgetstuck(action);
		if (action_targetaccuracy(action) > 0.8f) {
			action->action |= ACTION_USE;
			action->nextusetime = 0;
		}
	}
	// check for longjump
	if (action->longjumpstate > 0) {
		Vec3D difAngle;

		vsub(&action->e->v.angles, &action->moveangle, &difAngle);
		fixangle(&difAngle);
		float difX = fabsf(difAngle.x);
		if (difX > 90) difX = 180 - difX;
		//float vel = vlen(&e->v.velocity);
		//if ( (vel * difX)  > 2000 ) {
		if (difX > 5) {
			DEBUG_MSG( "Waiting for longjump, x=%.f\n", difX );
			action_setspeed(action, 0);
		} else {
			DEBUG_MSG( "Executing longjump, x=%.f\n", difX );
			action_add(action, BOT_DUCK, NULL);
			if (action->longjumpstate == 1) {
				action_setmaxspeed(action);
				action_add(action, BOT_JUMP, NULL);
			}
			--action->longjumpstate;
		}
	}
	// check for delayed jump
	if ((action->nextjumptime > 0) && (worldtime() > action->nextjumptime)) {
		action_add(action, BOT_JUMP, NULL);
		action->nextjumptime = 0;
		action->finejump = false;
	}
	// while fineJump is true, ignore all given moveAngles and speeds...
	if (action->finejump) {
		Vec3D dir;
		//DEBUG_MSG(" Action:FineJump in %.f\n", (action->nextjumptime - worldtime()));
		action_setmovedir(action, &action->finejumppos, 0);
		vsub(&action->finejumppos, &action->e->v.origin, &dir);
		float bestspeed = 5 * vlen(&dir);
		if (bestspeed > action->maxspeed)
			bestspeed = action->maxspeed;
		action_dontgetstuck(action);
		action_setspeed(action, bestspeed);
	}
	// check for stop
	if (action->stopendtime > 0) {
		action_dontgetstuck(action);
		float vel = vlen(&action->e->v.velocity);
		if (vel > 50) {
			Vec3D dir;

			vadd(&action->e->v.origin, &action->e->v.velocity, &dir);
			action_setmovedir(action, &dir, 5);
			action_setspeed(action, -vel);
			DEBUG_MSG( "Stopping\n" );
		} else
			action->stopendtime = 0;
	}
	
	// stuck: velocity test
/*	if (action->speed > 0 && !action->notStucking) {
		action->action |= ACTION_MOVEFORWARD;
		Vec3D currentVel = action->e->v.velocity;
		if (action_jumping(action))
			currentVel.z = 0;	// jumping doesn't count as movement
		float reachedSpeed = vlen(currentVel) / speed;
		if (reachedSpeed > 0.5			// move ok
		    || nextUseTime > 0)			// waiting to push button
			action->lastMove = worldtime();
	} else {	// bot is waiting, everything ok
		action->lastmove = worldtime();
	}*/
	if (action->speed > 0)
		action->action |= ACTION_MOVEFORWARD;
	else if (action->speed < 0)
		action->action |= ACTION_MOVEBACK;
	else
		action->notstucking = true;		// this is intentional!

	if (action->notstucking) {	// bot is waiting, everything ok
		action->lastmove = worldtime();
	} else {
		Vec3D currentVel;
		vcopy(&action->e->v.velocity, &currentVel);
		if (action_jumping(action))
			currentVel.z = 0;	// jumping doesn't count as movement
		if (vlen(&currentVel) > (action->maxspeed * 0.25f))
			action->lastmove = worldtime();
	}
	action->lastmovecheck = worldtime();

	Vec3D view;

	action_calcviewangle(action, &view);
	view.z = 0;
	action->e->v.v_angle = view;
	action->e->v.angles.y = view.y;
	action->e->v.angles.x = -view.x / 3.0f;
	action->e->v.angles.z = 0;

	// prevent crash?
	fixangle(&action->moveangle);
	if (action->speed > 10000 || action->speed < -10000) {
		action->speed = 0;		
		DEBUG_MSG( "FATAL SPEED\n" );
	}
	if (action->strafe > 10000 || action->strafe < -10000) {
		action->strafe = 0;
		DEBUG_MSG( "FATAL STRAFE\n" );
	}
	runplayermove(action->e, &action->moveangle, action->speed, action->strafe, 0, action->action, 0, getframerateinterval());
}
