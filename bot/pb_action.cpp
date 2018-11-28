#include "parabot.h"
#include "pb_action.h"

extern int mod_id;
extern bool pb_pause;
float globalFrameTime = 0;	// to access msec independant of bots

void PB_Action::init( EDICT *botEnt )
{
	ent = botEnt;
	msecStart = 1000; 
	msecCount = 0;
	inJump = false;
	nextJumpTime = 0;
	useCounter = 0;
	nextUseTime = 0;
	duckEndTime = 0;
	stopEndTime = 0;
	nextViewUpdate = 0;
	deltaView = zerovector;
	turnCount = 1;
	maxTurn = 0;
	fineJump = false;
	longJumpState = 0;
	maxSpeed = servermaxspeed();
	viewAngle = zerovector;
	currentView = zerovector;
	targetPos = zerovector;
	hitProb = 0;
	currentMSec = 0;
	targetDist = 0;
	lastMove = worldtime();
	lastMoveCheck = worldtime();
	vupdTime = 0.1;
	weaponCone = 0.1;	// 5°
	memset( &targetDiff, 0, sizeof targetDiff );
}

void PB_Action::reset() 
{ 
	action = 0;
	speed = 0;
	strafe = 0;
	viewAngle.z = 0;
	moveAngle.z = 0;
	currentView.z = 0;
	viewPrior = 0;
	movePrior = 0;
	notStucking = false;
	targetVel = zerovector;
}


void PB_Action::add( int code, Vec3D *exactPos )
{
	switch (code) {

	case BOT_JUMP:
		action |= ACTION_JUMP;
		if (!is_underwater( ent )) {	// no jumps underwater
			inJump = true;
			jumpPos = ent->v.origin.z;
		}
		break;

	case BOT_DELAYED_JUMP:
		if (exactPos) {
			fineJump = true;
			vcopy(exactPos, &fineJumpPos);
		}
		nextJumpTime = worldtime() + 1.0;
		break;

	case BOT_LONGJUMP:
		longJumpState = 2;
		break;

	case BOT_USE:
		if (exactPos) {
			nextUseTime = worldtime() + 0.5;
			nextUsePos = *exactPos;
		}
		else 
			useCounter = 6;
		break;

	case BOT_DUCK:
		action |= ACTION_CROUCH;
		break;

	case BOT_DUCK_LONG:
		duckEndTime = worldtime() + 1.5;
		break;

	case BOT_STOP_RUNNING:
		stopEndTime = worldtime() + 0.5;
		break;

	case BOT_FIRE_PRIM:
		action |= ACTION_ATTACK1;	
		break;

	case BOT_FIRE_SEC:
		action |= ACTION_ATTACK2;
		break;

	case BOT_RELEASE_SEC:
		action &= ~ACTION_ATTACK2;
		break;

	case BOT_RELOAD:
		action |= ACTION_RELOAD;
		break;

	case BOT_STRAFE_LEFT:
		// DEBUG_MSG( "Strafing left\n" );
		if (exactPos) strafe = -exactPos->x;
		else strafe = -400;	// strong influence
		break;

	case BOT_STRAFE_RIGHT:
		// DEBUG_MSG( "Strafing right\n" );
		if (exactPos) strafe = exactPos->x;
		else strafe = 400;	// strong influence
		break;

	}
}


void PB_Action::setMaxSpeed()
{
	setSpeed( maxSpeed );
}


void PB_Action::setMoveAngle( Vec3D *angle ) 
{ 
	fixangle(angle);
	vcopy(angle, &moveAngle); 
	//DEBUG_MSG( "setMove  " );
}


void PB_Action::setMoveAngleYaw( float angle )
{
	Vec3D ma = {0, angle, 0};
	fixangle( &ma );
	moveAngle = ma;
	//DEBUG_MSG( "setMoveYaw  " );
}

void PB_Action::setMoveDir( Vec3D *vec, int prior )
{
	if (prior >= movePrior) {
		Vec3D angle, dir;

		vsub(vec, &ent->v.origin, &dir);
		vectoangles(&dir, &angle);
		fixangle(&angle);
		angle.x = -angle.x;
		setMoveAngle(&angle);
		movePrior = prior;
		// DEBUG_MSG("setMoveDir  ");
	}
}

Vec3D *PB_Action::getMoveDir()
{
	fixangle(&moveAngle);
	makevectors(&moveAngle);
	return &com.globals->fwd;
}

void PB_Action::setViewAngle( Vec3D *angle, int prior )
{ 
	if (prior >= viewPrior) {
		vcopy(angle, &viewAngle);
		viewPrior = prior;
		vcopy(&zerovector, &targetVel);	// must be set != 0 afterwards!
	}
}

void PB_Action::setViewDir( Vec3D *vec, int prior )
{
	if (prior >= viewPrior) {
		Vec3D angle, dir;

		eyepos(ent, &dir);
		vsub(vec, &dir, &dir);
		vectoangles(&dir, &angle);
		fixangle(&angle);
		angle.x = -angle.x;
		setViewAngle(&angle, prior);
		viewPrior = prior;
	}
}

void PB_Action::setViewLikeMove() 
{ 
	if (viewPrior == 0) viewAngle = moveAngle; 
}

float PB_Action::estimateHitProb()
{
	Vec3D currentDiff;
	float dirErrorForSkill[10] = { 20, 15, 12, 10, 8, 6, 4, 2, 1, 0 };
	float dirError, cdl, center, hsize, prob;

	dirError = 0.5f * dirErrorForSkill[aimSkill - 1];
	vsub(&viewAngle, &currentView, &currentDiff);
	fixangle(&currentDiff);

	cdl = vlen(&currentDiff) + randomfloat(-dirError, dirError) + randomfloat(-dirError, dirError);

	if (cdl > 45.0f)
		return 0;

	center = 0.0001894f * targetDist * cdl / weaponCone;
	hsize = 32.0f / (weaponCone * targetDist);
	prob = getprob(center + hsize) - getprob(center - hsize);

	return prob;
}


void PB_Action::setAimDir( Vec3D *currentPos, Vec3D *relVelocity )
{
	if (viewPrior<=2) {
		Vec3D tVec, angle;

		vcopy(currentPos, &targetPos);
		eyepos(ent, &tVec);
		vsub(&targetPos, &tVec, &tVec);
		targetDist = vlen(&tVec);
		vectoangles(&tVec, &angle);
		fixangle(&angle);
		angle.x = -angle.x;
		setViewAngle(&angle, 2);
		vcopy(relVelocity, &targetVel);
		hitProb = estimateHitProb();
	} else {
		DEBUG_MSG("Aiming priority too low!\n");
	}
}

void PB_Action::setAimSkill( int skill )
{
	float updateTimes[10] = { 0.1, 0.1, 0.2, 0.2, 0.3, 0.3, 0.4, 0.4, 0.5, 0.6 }; 
	float accuracies[10] = { 0.0, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
		
	aimSkill = skill;
	vupdTime = updateTimes[10 - skill];
	aimAccuracy = accuracies[10 - skill];
//	INFO_MSG( "Set Aim to %i\n", skill );
}

Vec3D *PB_Action::calcViewAngle()
{
	Vec3D currentDiff, toAdd;

	viewAngle.z = 0;
	fixangle(&viewAngle);		// optimal viewAngle
	currentView.z = 0;
	vsub(&viewAngle, &currentView, &currentDiff);
	fixangle(&currentDiff);	// current angle difference to target

	// try to estimate hit probability
	for (int i = (MAX_VDELAY - 1); i > 0; i--)
		targetDiff[i] = targetDiff[i - 1];

	targetDiff[0] = vlen(&currentDiff) * targetDist;
	hitProb = estimateHitProb();

	if (worldtime_reached( nextViewUpdate )) {
		nextViewUpdate = worldtime() + vupdTime;

		if (!vcomp(&targetVel, &zerovector)) {	
			// predict position for moving target:
			Vec3D predPos, tVec, angle;

			vma(&targetPos, vupdTime, &targetVel, &predPos);
			eyepos(ent, &tVec);
			vsub(&predPos, &tVec, &tVec);
			vectoangles(&tVec, &angle);
			fixangle(&angle);
			angle.x = -angle.x;
			vsub(&angle, &currentView, &deltaView);
		} else {
			vcopy(&currentDiff, &deltaView);
		}

		// take accuracy into account:
		float distConst = 5.0f / ( 2.0f + vlen(&deltaView) );
		float bl = 1.0f / ( 1.0f + distConst ) - 0.4f;
		float bh = 1.5f + distConst;
		bl = aimAccuracy * (1.0f - bl);
		bh = aimAccuracy * (bh - 1.0f);	// rnd( 1-a*bl, 1+a*bh)

		float acc = randomfloat(1.0f - bl, 1.0f + bh);
		vscale(&deltaView, acc, &deltaView);
		fixangle(&deltaView);
		maxTurn = vlen(&deltaView) / (float)turnCount;
		turnCount = 0;
	}

	vcopy(&deltaView, &toAdd);
	if (toAdd.x > maxTurn) toAdd.x = maxTurn;
	else if (toAdd.x < -maxTurn) toAdd.x = -maxTurn;
	if (toAdd.y > maxTurn) toAdd.y = maxTurn;
	else if (toAdd.y < -maxTurn) toAdd.y = -maxTurn;

	vsub(&deltaView, &toAdd, &deltaView);
	vadd(&currentView, &toAdd, &currentView);
	turnCount++;

	fixangle(&currentView);
	return &currentView;
}

bool PB_Action::gotStuck()
{
	if ( (lastMoveCheck - lastMove) > 1.0f ) {
		DEBUG_MSG( "BOT IS STUCK!\n" );
		return true;
	}
	return false;
}

void PB_Action::resetStuck()
{
	lastMove = lastMoveCheck;
}

void PB_Action::perform()
{
	if (inJump && !(action & ACTION_JUMP)) {	// jumping with jump button released
		action |= ACTION_CROUCH;
		if ((ent->v.flags & FL_ONGROUND) || ent->v.origin.z < (jumpPos - 100)) {
			action &= ~ACTION_CROUCH;
			inJump = false;
		}
	}
	// check for long duck
	if (duckEndTime > 0) {
		if ( worldtime() < duckEndTime ) add(BOT_DUCK, NULL);
		else duckEndTime = 0;
	}
	// check for use
	if (useCounter > 0) {
		useCounter--;
		if (useCounter & 1) action |= ACTION_USE;
	}
	if (nextUseTime > 0) {
		setAimDir(&nextUsePos, NULL);	// to be able to use accuracy
		setMoveDir(&nextUsePos, 3);
		setMaxSpeed();
		// don't get stuck...
		dontGetStuck();
		if ( targetAccuracy() > 0.8 ) {
			action |= ACTION_USE;
			nextUseTime = 0;
		}
	}
	// check for longjump
	if (longJumpState > 0) {
		Vec3D difAngle;

		vsub(&ent->v.angles, &moveAngle, &difAngle);
		fixangle(&difAngle);
		float difX = fabsf(difAngle.x);
		if (difX > 90) difX = 180 - difX;
		//float vel = vlen(&ent->v.velocity);
		//if ( (vel * difX)  > 2000 ) {
		if (difX > 5) {
			DEBUG_MSG( "Waiting for longjump, x=%.f\n", difX );
			setSpeed( 0 );
		} else {
			DEBUG_MSG( "Executing longjump, x=%.f\n", difX );
			add(BOT_DUCK, NULL);
			if (longJumpState == 1) {
				setMaxSpeed();
				add(BOT_JUMP, NULL);
			}
			longJumpState--;
		}
	}
	// check for delayed jump
	if ( (nextJumpTime > 0) && (worldtime() > nextJumpTime) ) {
		add(BOT_JUMP, NULL);
		nextJumpTime = 0;
		fineJump = false;
	}
	// while fineJump is true, ignore all given moveAngles and speeds...
	if (fineJump) {
		Vec3D dir;
		//DEBUG_MSG(" Action:FineJump in %.f\n", (nextJumpTime-worldtime()) );
		setMoveDir(&fineJumpPos);
		vsub(&fineJumpPos, &ent->v.origin, &dir);
		float bestSpeed = 5 * vlen(&dir);
		if (bestSpeed > maxSpeed) bestSpeed = maxSpeed;
		dontGetStuck();
		setSpeed( bestSpeed );
	}
	// check for stop
	if (stopEndTime > 0) {
		dontGetStuck();
		float vel = vlen(&ent->v.velocity);
		if (vel > 50) {
			Vec3D dir;

			vadd(&ent->v.origin, &ent->v.velocity, &dir);
			setMoveDir(&dir, 5);
			setSpeed(-vel);
			DEBUG_MSG( "Stopping\n" );
		}
		else stopEndTime = 0;
	}
	
	// stuck: velocity test
/*	if (speed>0 && !notStucking) {
		action |= ACTION_MOVEFORWARD;
		Vec3D currentVel = ent->v.velocity;
		if ( jumping() ) currentVel.z = 0;	// jumping doesn't count as movement
		float reachedSpeed = vlen(currentVel) / speed;
		if ( reachedSpeed > 0.5 ||		// move ok
			 nextUseTime > 0		)	// waiting to push button
			lastMove = worldtime();
	} else {	// bot is waiting, everything ok
		lastMove = worldtime();
	}*/
	if (speed>0) action |= ACTION_MOVEFORWARD;
	else if (speed<0) action |= ACTION_MOVEBACK;
	else notStucking = true;		// this is intentional!

	if (notStucking) {	// bot is waiting, everything ok
		lastMove = worldtime();
	} else {
		Vec3D currentVel;
		vcopy(&ent->v.velocity, &currentVel);
		if (jumping()) currentVel.z = 0;	// jumping doesn't count as movement
		if (vlen(&currentVel) > (maxSpeed * 0.25f) ) lastMove = worldtime();
	}
	lastMoveCheck = worldtime();

	Vec3D view;

	vcopy(calcViewAngle(), &view);
	view.z = 0;
	ent->v.v_angle = view;
	ent->v.angles.y = view.y;
	ent->v.angles.x = -view.x / 3.0f;
	ent->v.angles.z = 0;

	// prevent crash?
	fixangle(&moveAngle);
	if (speed > 10000 || speed < -10000) {
		speed = 0;		
		DEBUG_MSG( "FATAL SPEED\n" );
	}
	if (strafe > 10000 || strafe < -10000) {
		strafe = 0;
		DEBUG_MSG( "FATAL STRAFE\n" );
	}
	runplayermove(ent, &moveAngle, speed, strafe, 0, action, 0, (byte)msec());
}

float PB_Action::msec()
{
	if ((msecStart + msecCount / 1000) < (worldtime() - 0.5f)) {	// after pause 
		msecStart = worldtime() - 0.05f;
		msecCount = 0;
		DEBUG_MSG( "MSEC: SUPPOSED PAUSE\n" );
	}

	if (msecStart > worldtime()) {					// after map changes...
		msecStart = worldtime() - 0.05f;
		msecCount = 0;
		DEBUG_MSG( "MSEC: SUPPOSED MAPCHANGE\n" );
	}
	float opt = (worldtime() - msecStart) * 1000;		// optimal msec value since start of 1 sec period
	currentMSec = opt - msecCount;					// value ve have to add to reach optimum
	globalFrameTime = currentMSec / 1000;				// duration of last frame in sec
	msecCount = opt;
	if (msecCount > 1000) {							// do we have to start a new 1 sec period?
		msecStart += msecCount / 1000;
		msecCount = 0;
	}
	// check from THE FATAL:
	if (currentMSec < 5) currentMSec = 5;
	else if (currentMSec > 255) currentMSec = 255;
	return currentMSec;
}
