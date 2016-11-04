#include "pb_action.h"
#include "pb_global.h"

extern int mod_id;
extern bool pb_pause;
float globalFrameTime = 0;	// to access msec independant of bots


void fixAngle( Vector &angle )
{
	if (angle.x > 10000 || angle.x < -10000) {
		angle.x = 0;
	}
	if (angle.y > 10000 || angle.y < -10000) {
		angle.y = 0;
	}
	
	while (angle.x >  180) angle.x -= 360;
	while (angle.x < -180) angle.x += 360;
	while (angle.y >  180) angle.y -= 360;
	while (angle.y < -180) angle.y += 360;

	angle.z = 0;
}


void PB_Action::init( edict_t *botEnt )
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
	deltaView = Vector(0,0,0);
	turnCount = 1;
	maxTurn = 0;
	fineJump = false;
	longJumpState = 0;
	maxSpeed = CVAR_GET_FLOAT("sv_maxspeed");
	viewAngle = Vector(0,0,0);
	currentView = Vector(0,0,0);
	targetPos = Vector(0,0,0);
	hitProb = 0;
	currentMSec = 0;
	targetDist = 0;
	lastMove = worldTime();
	lastMoveCheck = worldTime();
	vupdTime = 0.1;
	weaponCone = 0.1;	// 5°
	int i;
	for (i=0; i<MAX_VDELAY; i++) targetDiff[i] = 0;
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
	targetVel = Vector(0,0,0);
}


void PB_Action::add( int code, Vector *exactPos )
{
	switch (code) {

	case BOT_JUMP:
		action |= IN_JUMP;
		if (!isUnderwater( ent )) {	// no jumps underwater
			inJump = true;
			jumpPos = ent->v.origin.z;
		}
		break;

	case BOT_DELAYED_JUMP:
		if (exactPos) {
			fineJump = true;
			memcpy( &fineJumpPos, exactPos, sizeof(Vector) );
		}
		nextJumpTime = worldTime() + 1.0;
		break;

	case BOT_LONGJUMP:
		longJumpState = 2;
		break;

	case BOT_USE:
		if (exactPos) {
			nextUseTime = worldTime() + 0.5;
			nextUsePos = *exactPos;
		}
		else 
			useCounter = 6;
		break;

	case BOT_DUCK:
		action |= IN_DUCK;
		break;

	case BOT_DUCK_LONG:
		duckEndTime = worldTime() + 1.5;
		break;

	case BOT_STOP_RUNNING:
		stopEndTime = worldTime() + 0.5;
		break;

	case BOT_FIRE_PRIM:
		action |= IN_ATTACK;	
		break;

	case BOT_FIRE_SEC:
		action |= IN_ATTACK2;
		break;

	case BOT_RELEASE_SEC:
		action &= ~IN_ATTACK2;
		break;

	case BOT_RELOAD:
		action |= IN_RELOAD;
		break;

	case BOT_STRAFE_LEFT:
		//debugMsg( "Strafing left\n" );
		if (exactPos) strafe = -exactPos->x;
		else strafe = -400;	// strong influence
		break;

	case BOT_STRAFE_RIGHT:
		//debugMsg( "Strafing right\n" );
		if (exactPos) strafe = exactPos->x;
		else strafe = 400;	// strong influence
		break;

	}
}


void PB_Action::setMaxSpeed()
{
	setSpeed( maxSpeed );
}


void PB_Action::setMoveAngle( Vector angle ) 
{ 
	fixAngle( angle );
	moveAngle = angle; 
	//debugMsg( "setMove  " );
}


void PB_Action::setMoveAngleYaw( float angle )
{
	Vector ma = Vector( 0, angle, 0 );
	fixAngle( ma );
	moveAngle = ma;
	//debugMsg( "setMoveYaw  " );
}


void PB_Action::setMoveDir( Vector vec, int prior )
{
	if (prior>=movePrior) {
		Vector angle = UTIL_VecToAngles( vec - ent->v.origin );
		fixAngle( angle );
		angle.x = -angle.x;
		setMoveAngle( angle );
		movePrior = prior;
		//debugMsg( "setMoveDir  " );
	}
}


Vector PB_Action::getMoveDir()
{
	fixAngle( moveAngle );
	UTIL_MakeVectors( moveAngle );
	return gpGlobals->v_forward;
}


void PB_Action::setViewAngle( Vector angle, int prior )
{ 
	if (prior>=viewPrior) {
		viewAngle = angle;
		viewPrior = prior;
		targetVel = Vector(0,0,0);		// must be set != 0 afterwards!
	}
}


void PB_Action::setViewDir( Vector vec, int prior )
{
	if (prior>=viewPrior) {
		Vector angle = UTIL_VecToAngles( vec - (ent->v.origin+ent->v.view_ofs) );
		fixAngle( angle );
		angle.x = -angle.x;
		setViewAngle( angle, prior );
		viewPrior = prior;
	}
}


void PB_Action::setViewLikeMove() 
{ 
	if (viewPrior == 0) viewAngle = moveAngle; 
}


float getProb( float x )
{
	if (x < -1) return 0;
	if (x <  0) return 0.5*(x+1)*(x+1);
	if (x <  1) return 1 - 0.5*(1-x)*(1-x);
	return 1;
}


float PB_Action::estimateHitProb()
{
	float dirErrorForSkill[10] = { 20, 15, 12, 10, 8, 6, 4, 2, 1, 0 };
	float dirError = 0.5 * dirErrorForSkill[aimSkill-1];
	Vector currentDiff = viewAngle - currentView;
	fixAngle( currentDiff );
	float cdl = currentDiff.Length() + RANDOM_FLOAT( -dirError, dirError ) + RANDOM_FLOAT( -dirError, dirError );
	if (cdl > 45) return 0;
	float center = 0.0001894 * targetDist * cdl / weaponCone;
	float hsize = 32.0 / (weaponCone * targetDist);
	float prob = getProb( center + hsize ) - getProb( center - hsize );
	return prob;
}


void PB_Action::setAimDir( Vector currentPos, Vector relVelocity )
{
	if (viewPrior<=2) {
		targetPos = currentPos;
		Vector tVec = targetPos - (ent->v.origin + ent->v.view_ofs);
		targetDist = tVec.Length();
		Vector angle = UTIL_VecToAngles( tVec );
		fixAngle( angle );
		angle.x = -angle.x;
		setViewAngle( angle, 2 );
		targetVel = relVelocity;
		hitProb = estimateHitProb();
	}
	else {
		debugMsg( "Aiming priority too low!\n" );
	}
}


void PB_Action::setAimSkill( int skill )
{
	float updateTimes[10] = { 0.1, 0.1, 0.2, 0.2, 0.3, 0.3, 0.4, 0.4, 0.5, 0.6 }; 
	float accuracies[10] = { 0.0, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
		
	aimSkill = skill;
	vupdTime = updateTimes[10-skill];
	aimAccuracy = accuracies[10-skill];
//	char buf[80];
//	sprintf( buf, "Set Aim to %i\n", skill );
//	infoMsg( buf );
}


bool worldTimeReached( float time )
{
	static float lastTime = 0;

	// take care of level changes:
	if (worldTime() < lastTime) return true;	
	lastTime = worldTime();

	if (worldTime() > time) return true;
	else return false;
	
}


Vector PB_Action::calcViewAngle()
{
	viewAngle.z = 0;
	fixAngle( viewAngle );		// optimal viewAngle
	currentView.z = 0;
	Vector currentDiff = viewAngle - currentView;
	fixAngle( currentDiff );	// current angle difference to target
	
	// try to estimate hit probability
	for (int i=(MAX_VDELAY-1); i>0; i--) targetDiff[i] = targetDiff[i-1];
	targetDiff[0] = currentDiff.Length() * targetDist;
	hitProb = estimateHitProb();


	if (worldTimeReached( nextViewUpdate )) {
		nextViewUpdate = worldTime() + vupdTime;

		if (targetVel != Vector(0,0,0)) {	
			// predict position for moving target:
			Vector predPos = targetPos + vupdTime * targetVel;
			Vector tVec = predPos - (ent->v.origin + ent->v.view_ofs);
			Vector angle = UTIL_VecToAngles( tVec );
			fixAngle( angle );
			angle.x = -angle.x;
			deltaView = angle - currentView;
		}
		else {
			deltaView = currentDiff;
		}
		// take accuracy into account:
		float distConst = 5.0 / ( 2.0 + deltaView.Length() );
		float bl = 1.0 / ( 1.0 + distConst ) - 0.4;
		float bh = 1.5 + distConst;
		bl = aimAccuracy * (1 - bl);
		bh = aimAccuracy * (bh - 1);	// rnd( 1-a*bl, 1+a*bh)

		float acc = RANDOM_FLOAT( 1.0-bl, 1.0+bh );
		deltaView = deltaView * acc;
		fixAngle( deltaView );
		maxTurn = deltaView.Length() / (float)turnCount;
		turnCount = 0;
	}
	
	Vector toAdd = deltaView;
	if (toAdd.x>maxTurn) toAdd.x = maxTurn;
	else if (toAdd.x<-maxTurn) toAdd.x = -maxTurn;
	if (toAdd.y>maxTurn) toAdd.y = maxTurn;
	else if (toAdd.y<-maxTurn) toAdd.y = -maxTurn;
	
	deltaView = deltaView - toAdd;
	currentView = currentView + toAdd;
	turnCount++;

	fixAngle( currentView );
	return currentView;
}



bool PB_Action::gotStuck()
{
	if ( (lastMoveCheck-lastMove) > 1.0 ) {
		debugMsg( "BOT IS STUCK!\n" );
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
	if (inJump && !(action & IN_JUMP)) {	// jumping with jump button released
		action |= IN_DUCK;
		if (FBitSet(ent->v.flags, FL_ONGROUND) || ent->v.origin.z<(jumpPos-100)) {
			action &= ~IN_DUCK;
			inJump=false;
		}
	}
	// check for long duck
	if (duckEndTime > 0) {
		if ( worldTime() < duckEndTime ) add( BOT_DUCK );
		else duckEndTime = 0;
	}
	// check for use
	if (useCounter > 0) {
		useCounter--;
		if (useCounter & 1) action |= IN_USE;
	}
	if (nextUseTime > 0) {
		setAimDir( nextUsePos );	// to be able to use accuracy
		setMoveDir( nextUsePos, 3 );
		setMaxSpeed();
		// don't get stuck...
		dontGetStuck();
		if ( targetAccuracy() > 0.8 ) {
			action |= IN_USE;
			nextUseTime = 0;
		}
	}
	// check for longjump
	if (longJumpState > 0) {
		Vector difAngle = ent->v.angles - moveAngle;
		fixAngle( difAngle );
		float difX = abs(difAngle.x);
		if (difX > 90) difX = 180 - difX;
		//float vel = ((Vector)ent->v.velocity).Length();
		//if ( (vel * difX)  > 2000 ) {
		if ( difX > 5 ) {
			debugMsg( "Waiting for longjump, x=%.f\n", difX );
			setSpeed( 0 );
		}
		else {
			debugMsg( "Executing longjump, x=%.f\n", difX );
			add( BOT_DUCK );
			if (longJumpState == 1) {
				setMaxSpeed();
				add( BOT_JUMP );
			}
			longJumpState--;
		}
	}
	// check for delayed jump
	if ( (nextJumpTime>0) && (worldTime()>nextJumpTime) ) {
		add( BOT_JUMP );
		nextJumpTime = 0;
		fineJump = false;
	}
	// while fineJump is true, ignore all given moveAngles and speeds...
	if (fineJump) {
		//debugMsgF(" Action:FineJump in %.f\n", (nextJumpTime-worldTime()) );
		setMoveDir( fineJumpPos );
		float bestSpeed = 5 * (fineJumpPos - ent->v.origin).Length();
		if (bestSpeed > maxSpeed) bestSpeed = maxSpeed;
		dontGetStuck();
		setSpeed( bestSpeed );
	}
	// check for stop
	if (stopEndTime > 0) {
		dontGetStuck();
		float vel = ((Vector)ent->v.velocity).Length();
		if (vel > 50) {
			setMoveDir( ent->v.origin + ent->v.velocity, 5 );
			setSpeed( -vel );
			debugMsg( "Stopping\n" );
		}
		else stopEndTime = 0;
	}
	
	// stuck: velocity test
/*	if (speed>0 && !notStucking) {
		action |= IN_FORWARD;
		Vector currentVel = ent->v.velocity;
		if ( jumping() ) currentVel.z = 0;	// jumping doesn't count as movement
		float reachedSpeed = currentVel.Length() / speed;
		if ( reachedSpeed > 0.5 ||		// move ok
			 nextUseTime > 0		)	// waiting to push button
			lastMove = worldTime();
	}
	else {	// bot is waiting, everything ok
		lastMove = worldTime();
	}*/
	if (speed>0) action |= IN_FORWARD;
	else if (speed<0) action |= IN_BACK;
	else notStucking = true;		// this is intentional!

	if (notStucking) {	// bot is waiting, everything ok
		lastMove = worldTime();
	}
	else {	
		Vector currentVel = ent->v.velocity;
		if ( jumping() ) currentVel.z = 0;	// jumping doesn't count as movement
		if ( currentVel.Length() > (maxSpeed/4) ) lastMove = worldTime();
	}
	lastMoveCheck = worldTime();
	

	Vector view = calcViewAngle();

	view.z = 0;
	ent->v.v_angle = view;
	ent->v.angles.y = view.y;
	ent->v.angles.x = -view.x / 3;
	ent->v.angles.z = 0;

	// prevent crash?
	fixAngle( moveAngle );
	if (speed > 10000 || speed < -10000) {
		speed = 0;		
		debugMsg( "FATAL SPEED\n" );
	}
	if (strafe > 10000 || strafe < -10000) {
		strafe = 0;
		debugMsg( "FATAL STRAFE\n" );
	}
	g_engfuncs.pfnRunPlayerMove( ent, moveAngle, 
		speed, strafe, 0, action, 0, (byte)msec() );
}


float PB_Action::msec()
{
	if ((msecStart+msecCount/1000) < (worldTime()-0.5)) {	// after pause 
		msecStart = worldTime() - 0.05;
		msecCount = 0;
		debugMsg( "MSEC: SUPPOSED PAUSE\n" );
	}

	if (msecStart > worldTime()) {					// after map changes...
		msecStart = worldTime() - 0.05;
		msecCount = 0;
		debugMsg( "MSEC: SUPPOSED MAPCHANGE\n" );
	}
	float opt = (worldTime()-msecStart) * 1000;		// optimal msec value since start of 1 sec period
	currentMSec = opt - msecCount;					// value ve have to add to reach optimum
	globalFrameTime = currentMSec/1000;				// duration of last frame in sec
	msecCount = opt;
	if (msecCount > 1000) {							// do we have to start a new 1 sec period?
		msecStart += msecCount/1000;
		msecCount = 0;
	}
	// check from THE FATAL:
	if (currentMSec < 5) currentMSec = 5;
    else if (currentMSec > 255) currentMSec = 255;
	return currentMSec;
}