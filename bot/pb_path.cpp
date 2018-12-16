#include "parabot.h"
#include "pb_path.h"
#include "pb_global.h"
#include <stdio.h>

int journeyMode = JOURNEY_FAST;

void setJourneyMode(int mode)
// sets global journey mode
{
	journeyMode = mode;
}



PB_Path_Waypoint::PB_Path_Waypoint()
{
	data.act = WP_NOT_INITIALIZED;
}


PB_Path_Waypoint::PB_Path_Waypoint( Vec3D *pos, int action, float arrival )
{
	vcopy(pos, &data.pos);
	data.act = action;
	data.arrival = arrival;
}


bool PB_Path_Waypoint::reached( EDICT *ent )
// returns true if waypoint is reached from pos
{
	if (data.act & WP_NOT_REACHABLE) return false;

	Vec3D wpDir, vdir;
	vsub(pos(ent), &ent->v.origin, &wpDir);
	float dist = vlen(&wpDir);

	if (dist < 16.0)
		return true;

	if (dist > 40.0)
		return false;

	vcopy(&ent->v.velocity, &vdir);
	vscale(&wpDir, 1.0f / dist, &wpDir);
	normalize(&vdir);
	float dres = dotproduct(&vdir, &wpDir);
	if (dres < 0)
		return true;	// waypoint has been passed

	return false;

/*	float requiredDist = 40;
	// approach jumps and platforms up to 30 units
	if ( action() == BOT_JUMP || isOnPlatform() ) requiredDist = 30;
	// approach ladder waypoints up to 20 units
	if (data.act & WP_ON_LADDER) requiredDist = 20;
	if (dist < requiredDist) return true;
	else return false;*/
}


Vec3D *PB_Path_Waypoint::pos( EDICT *ent ) 
// adjusts positions if ent is on ladder
{
	assert( ent != 0 );
	if (!is_onladder(ent))
		return &data.pos;

	data.pos.z += 20.0f;

	return &data.pos;
}

PB_Path::PB_Path()
{
	data.startId = 0;
	data.endId = 0;
	waypoint = 0;
	hiddenAttack = 0;
	platformPos = 0;
	readyToDelete = false;
}

PB_Path::~PB_Path()
{
/*	don't delete anything because:
 *  1) if path is member of mapgrap the clear function is called there
 *  2) if path is temporal it might contain waypoint/hiddenAttack-pointers to
 *     paths in mapgraph which must not be deleted
 */
}


NAVPOINT *PB_Path::startNav()
// returns the start navpoint
{
	return getNavpoint( data.startId );
}


NAVPOINT *PB_Path::endNav()
// returns the end navpoint
{
	return getNavpoint( data.endId );
}


void PB_Path::startRecord( int startPnt, float worldtime )
// start recording a path from given point at given time
{
	data.privateId = -1;
	data.dataId = -1;			// needs to be initialized in PB_MapGraph::addPath
	data.startId = startPnt;
	data.endId = -1;			// = recording
	startTime = worldtime;
	data.attempts = 0;
	data.lastAttempt = getTotalAttempts();	// init as new
	data.successful = 0;
	data.lostHealth = 0;
	data.enemyEncounters = 0;
	readyToDelete = false;
	specialId = -1;
	waypoint = new WaypointList;	
	hiddenAttack = new AttackList;
	platformPos = new PlatformList;
}


void PB_Path::addWaypoint( PB_Path_Waypoint &wp )
// adds the waypoint to the path
{
	assert( waypoint != 0 );
	waypoint->push_back( wp );
}


void PB_Path::addWaypoint( Vec3D *pos, int action, float arrival )
// adds the waypoint to the path
{
	PB_Path_Waypoint wp(pos, action, arrival - startTime );
	//wp.data.pos = pos;
	//wp.data.act = action;
	//wp.data.arrival = arrival - startTime;
	assert( waypoint != 0 );
	waypoint->push_back( wp );
}


void PB_Path::addPlatformInfo( int navId, Vec3D *pos )
// adds platform info
{
	PB_Path_Platform pf( navId, pos );
	assert(platformPos != 0);
	platformPos->push_back(pf);
}


void PB_Path::stopRecord( int endPnt, float worldtime, int mode )
// stops recording
{
	data.endId = endPnt;
	data.scheduledTime = worldtime - startTime;
	data.passTime = data.scheduledTime;
	data.specials = mode;
}


void PB_Path::clear()
// deletes all path-data, cancels recording
{
	if (waypoint) {
		waypoint->clear();
		delete waypoint;
		waypoint = 0;
	}
	if (hiddenAttack) {
		hiddenAttack->clear();
		delete hiddenAttack;
		hiddenAttack = 0;
	}
	if (platformPos) {
		platformPos->clear();
		delete platformPos;
		platformPos = 0;
	}
	data.endId = 0;			// not recording anything
	readyToDelete = true;	// just in case...
}

#if _DEBUG
void PB_Path::print()
{
	DEBUG_MSG( "path from " );
	startNav().print();
	DEBUG_MSG( " to " );
	endNav().print();
}
#endif

#include "marker.h"
#include <queue>

#if _DEBUG
void PB_Path::mark()
// mark all waypoints
{
	WaypointList::iterator wpi = waypoint->begin();
	PB_Path_Waypoint wp;
	while (wpi != waypoint->end()) {
		glMarker.newMarker( wpi->pos(), 1 );
		wpi++;
	}
	if (currentWaypoint != waypoint->end()) {
		glMarker.newMarker( Vector(0,0,10)+currentWaypoint->pos(), 1 );
	} else
		DEBUG_MSG( "Current wp = end\n" );
	if (lastReachedWaypoint != waypoint->end()) {
		glMarker.newMarker( Vector(0,0,10)+lastReachedWaypoint->pos(), 2 );
	} else
		DEBUG_MSG( "Last reached wp = end\n" );
}
#endif //_DEBUG

void PB_Path::load( FILE *fp )
// init path from file
{
	readyToDelete = false;

	// read data
	int i;
	fread( &data, sizeof(TSaveData), 1, fp );

	if (data.dataId == data.privateId) {	// read lists if stored here
		// read waypoints
		waypoint = new WaypointList;
		int numWpts;
		fread( &numWpts, sizeof(int), 1, fp );
		PB_Path_Waypoint wp;
		assert( waypoint != 0 );
		for (i = 0; i < numWpts; i++) {
			//printf( "w" );
			fread( &wp.data, sizeof(PB_Path_Waypoint::TSaveData), 1, fp );
			waypoint->push_back( wp );
		}

		// read attacks
		hiddenAttack = new AttackList;
		int numAtts;
		fread( &numAtts, sizeof(int), 1, fp );
		PB_Path_Attack att;
		assert( hiddenAttack != 0 );
		for (i = 0; i < numAtts; i++) {
			//printf( "a" );
			fread( &att.data, sizeof(PB_Path_Attack::TSaveData), 1, fp );
			hiddenAttack->push_back( att );
		}

		// read platform info
		platformPos = new PlatformList;
		int numPlats;
		fread( &numPlats, sizeof(int), 1, fp );
		PB_Path_Platform pf;
		assert( platformPos != 0 );
		for (i = 0; i < numPlats; i++) {
			fread(&(pf.data.navId), sizeof(int), 1, fp);
			fread(&pf.data.pos, 3 * sizeof(float), 1, fp);
			if(pf.data.navId < 0) {
				readyToDelete = true;
				DEBUG_MSG( "Deleted 1 path because of incorrect platform info\n" );
			}
			platformPos->push_back( pf );
		}
	} else {	// must be initialized later on when all paths are loaded...
		waypoint = 0;
		hiddenAttack = 0;
		platformPos = 0;
	}
}


void PB_Path::save( FILE *fp )
// save path to file if not readyToDelete
{
	if (readyToDelete) return;
	
	// save data
	//printf( "P" );
	int i;
	fwrite( &data, sizeof(TSaveData), 1, fp );
	
	if (data.dataId == data.privateId) {	// save lists if stored here
		// save waypoints
		assert( waypoint != 0 );
		int numWpts = waypoint->size();
		fwrite( &numWpts, sizeof(int), 1, fp );
		WaypointList::iterator wpi = waypoint->begin();
		PB_Path_Waypoint wp;
		for (i = 0; i < numWpts; i++) {
			//printf( "W" );
			fwrite( &(wpi->data), sizeof(PB_Path_Waypoint::TSaveData), 1, fp );
			wpi++;
		}
		
		// save attacks
		assert( hiddenAttack != 0 );
		int numAtts = hiddenAttack->size();
		fwrite( &numAtts, sizeof(int), 1, fp );
		AttackList::iterator atti = hiddenAttack->begin();
		//PB_Path_Attack att;
		for (i = 0; i < numAtts; i++) {
			//printf( "A" );
			fwrite( &(atti->data), sizeof(PB_Path_Attack::TSaveData), 1, fp );
			atti++;
		}

		// save platform pos
		assert( platformPos != 0 );
		int numPlats = platformPos->size();
		fwrite( &numPlats, sizeof(int), 1, fp );
		PlatformList::iterator pfi = platformPos->begin();
		for (i = 0; i < numPlats; i++) {
			assert( pfi->data.navId < 1000 );
			fwrite( &(pfi->data.navId), sizeof(int), 1, fp );
			fwrite( &pfi->data.pos, 3 * sizeof(float), 1, fp );
			pfi++;
		}
	}
}


void PB_Path::initReturnOf( PB_Path &path )
{
	data.privateId = -1;
	data.dataId = path.data.privateId;
	data.startId = path.endId();
	data.endId = path.startId();
	data.scheduledTime = path.data.scheduledTime;
	data.passTime = data.scheduledTime;
	data.specials = 0;
	data.attempts = 0;
	data.lastAttempt = 0;
	data.successful = 0;
	data.lostHealth = 0;
	data.enemyEncounters = 0;
	readyToDelete = false;
	specialId = path.specialId;
	waypoint = path.waypoint;			// use same data 
	hiddenAttack = path.hiddenAttack;
	platformPos = path.platformPos;
}


float PB_Path::weight()
// used by graph algo, returns weight of path depending of journeyMode
{ 
	float certainty = 0.5;		// base certainty for new paths
	float encounterProb = 0.1;	

	switch( journeyMode ) {

	case JOURNEY_FAST:			// return fastest journey
		return data.passTime; 

	case JOURNEY_RELIABLE:		// return most reliable journey
		if (data.attempts > 0)
			certainty = ((float)data.successful) / ((float)data.attempts);
		return (1.0001f - certainty);

	case JOURNEY_CROWDED:		// return journey with most enemy encounters
		if (data.attempts > 0) {
			encounterProb = ((float)data.enemyEncounters) / ((float)data.attempts);
			if (encounterProb > 9.9) encounterProb = 9.9;
		}
		return (10.0f - encounterProb);	// encounterProb may be >1

	case JOURNEY_LONELY:
		if (data.attempts > 0) 
			encounterProb = ((float)data.enemyEncounters) / ((float)data.attempts);
		return encounterProb;
	}
	DEBUG_MSG( "Unknown journeymode!\n" );
	return 0;
}	

bool PB_Path::valid( int mode )
{
	if (readyToDelete) return false;	// don't accept deleted paths

	int dif = mode ^ data.specials;	// differences between searchmode and path-specials
	int neg = mode ^ (-1);		// specials not allowed in searchmode
	return !(dif & neg);		// only valid if AND = 0
}


void PB_Path::startAttempt( float worldtime )
// start the path at given time
{
	Vec3D dir;

	assert(waypoint != 0);
	assert(platformPos != 0);

	if (waypoint->size() > 1) {
		WaypointList::iterator wpForw = waypoint->begin();
		vsub(wpForw->pos(), navpoint_pos(startNav()), &dir);
		float distForw = vlen(&dir);
		WaypointList::iterator wpBackw = waypoint->end();
		wpBackw--;
		vsub(wpBackw->pos(), navpoint_pos(startNav()), &dir);
		float distBackw = vlen(&dir);
		// path in forward direction ?
		if (distForw <= distBackw) {
			forwardPass = true;
			currentWaypoint = wpForw;
			currentPlat = platformPos->begin();
		} else {
			forwardPass = false;
			currentWaypoint = wpBackw;
			currentPlat = platformPos->end();	
			if (platformPos->size() > 1) currentPlat--;
		}
	} else {	// no waypoints stored: simulate forward
		forwardPass = true;
		currentWaypoint = waypoint->begin();
		currentPlat = platformPos->begin();
	}

	lastReachedWaypoint = waypoint->end();	// nothing reached yet
	lastReachedPlat = platformPos->end();
	startTime = worldtime;
	data.attempts++;
	incTotalAttempts();
	data.lastAttempt = getTotalAttempts();
	lastWaitingPlat = 0;
	ignoredPlat = 0;
}


void PB_Path::cancelAttempt()
// cancels the pass, no reportTarget needs to be called
{
	if (data.attempts > 0) data.attempts--;
}


void PB_Path::reportTargetReached( EDICT *traveller, float worldtime )
// confirms path as finished, internally set new success-variables
{
	float allTime   = (data.passTime  * ((float)data.successful)) 
					+ (worldtime - startTime);
	
	data.successful++;
	if (data.successful == 0) {
		ERROR_MSG( "reportTargetReached!" );
		return;
	}
	data.passTime = allTime / ((float)data.successful);		// set new passTime
	navpoint_reportvisit(endNav(), traveller, worldtime ); //now in observer
	if (ignoredPlat) {	// we ignored a plat but arrived nevertheless?
		WaypointList::iterator wpi = waypoint->begin();
		PlatformList::iterator pfi = platformPos->begin();
		while (wpi!=waypoint->end()) {
			if (wpi->isOnPlatform()) {
				EDICT *plat = navpoint_entity(getNavpoint(pfi->data.navId));
				// delete the platform flag for these waypoints
				if (plat==ignoredPlat) wpi->data.act &= (~WP_ON_PLATFORM);
				pfi++;
			}
			wpi++;
		}
	}
}


extern bool pb_pause;

void PB_Path::reportTargetFailed()
// confirms path as failed, internally set new success-variables
{
	if (data.attempts == 0) return;	// must be...

	float certainty = ((float)data.successful) / ((float)data.attempts);	
	// check new certainty
	if (certainty < 0.5f) {
		markForDelete();
#if _DEBUG
		DEBUG_MSG( "Auto-deleted " );  print();  DEBUG_MSG( "\n" );
		//DEBUG_MSG( "%i attempts, ", data.attempts );  
		//DEBUG_MSG( "%i succesful\n", data.successful );
		//mark();
#endif
		//pb_pause = true;
	}
	DEBUG_MSG( "%i attempts, %i succesful\n", data.attempts, data.successful );  
	//mark();
	//pb_pause = true;
}


void PB_Path::addAttack( Vec3D *origin )
// stores the position of attack (inclusive actual position in path)
{
	assert( hiddenAttack != 0 );

	PB_Path_Attack att;
	vcopy(origin, &att.data.pos);
	att.data.time = currentWaypoint->data.arrival;

	hiddenAttack->push_back( att );
}


void PB_Path::getViewPos( EDICT *traveller, int *prior, Vec3D *pos) 
// Priority values return 0 (nextWaypoint) or 2 (button)
{
	
	assert( waypoint != 0 );
	if ( lastReachedWaypoint != waypoint->end() ) {	// watch at buttons for pressing them
		if (lastReachedWaypoint->action() == BOT_USE) {
			assert( traveller != 0 );
			// Vec3D t = traveller->v.origin;
			vcopy(navpoint_pos(startNav()), pos); //lastReachedWaypoint->pos();
			*prior = 2;	// UNDONE: ButtonUse uses AimDir, not ViewDir!!!
			return;
		}
	}
	*prior = 0;

	vadd(getNextWaypoint().pos(traveller), &traveller->v.view_ofs, pos);
}


bool PB_Path::timeOut( float worldtime )
// returns true if the next waypoint could not be reached in time
{	
	float plan = data.scheduledTime;			// default = approaching path end

	assert( waypoint != 0 );
	if (currentWaypoint != waypoint->end()) {	// approaching another waypoint
		if (forwardPass) {						// path in forward direction ?
			plan = currentWaypoint->data.arrival;
		} else {	// backward passing
			plan = data.scheduledTime - currentWaypoint->data.arrival;
		}
	}

	if (worldtime > (startTime + plan + 3)) return true;
	else return false;
}


bool PB_Path::cannotBeContinued( EDICT *ent )
// returns true if the path has to be canceled
{
	Vec3D dir;

	PB_Path_Waypoint nextWP = getNextWaypoint();
	vsub(nextWP.pos(ent), &ent->v.origin, &dir);
	float dist = vlen(&dir);
	if (dist > 100) {
		// suppose bot has fallen...
		if (nextWP.isOnPlatform()) return true;	 
	}
	return false;
}

bool PB_Path::finished( EDICT *traveller )
// returns true if all waypoints have been confirmed and end-navpoint is reached by traveller
{
	// UNDONE for teleporter hintpoints!
//	if (currentWaypoint == waypoint->end()) {	// no more waypoints to go...
	if (navpoint_reached(endNav(), traveller)) return true;
//	}
	return false;
}

int PB_Path::getNextAction()
// returns the action to perform at the next waypoint
{
	if ( !hasData() ) return 0;					// don't do anything on return path

	assert( waypoint != 0 );
	if (currentWaypoint != waypoint->end()) {	// approaching another waypoint
		return currentWaypoint->action();
	}
	else return 0;
}

bool PB_Path::waitForPlatform()
// return true if bot has to wait for a platform before continuing to the next waypoint
{
	#define MAX_PLAT_WAIT 2.0	// maximal time to wait if platform is not moving

	bool wait = false;
	assert( platformPos != 0 );
	if (platformPos->size() == 0) return false;	// no platform on path

	if ((currentWaypoint != waypoint->end()) && currentWaypoint->isOnPlatform()) {
		if (currentPlat != platformPos->end()) {
			EDICT *plat = navpoint_entity(getNavpoint(currentPlat->data.navId));
			assert (plat != 0);

			if (plat != lastWaitingPlat) {
				lastWaitingPlat = plat;
				waitPlatEndTime = worldtime() + MAX_PLAT_WAIT;
			}

			Vec3D tDir, vDir;
			vsub(&currentPlat->data.pos, &plat->v.absmin, &tDir);
			float tLen = vlen(&tDir);
			vcopy(&plat->v.velocity, &vDir);
			float vLen = vlen(&vDir);
			if (vLen > 0) waitPlatEndTime = worldtime() + MAX_PLAT_WAIT;

			if (tLen > 16) wait = true;

			if (worldtime() > waitPlatEndTime) {
				ignoredPlat = plat;
				wait = false;
			}

	/*		if (getNavpoint( currentPlat->data.navId ).type()==NAV_F_PLAT) {
				// for platforms only wait if they are moving (touch lift problem!)
				if (tLen > 16 && vLen > 0) wait = true;
			} else {
				// always wait for trains and doors
				if (tLen > 16) wait = true;
			}
			*/

			if (!wait && vLen > 0) {
				vscale(&vDir, 1.0f / vLen, &vDir);
				vscale(&tDir, 1.0f / tLen, &tDir);
				float dres = dotproduct(&vDir, &tDir);
				if (dres > 0) wait = true;	// platform still moving in dir
			}
		}
	}
	return wait;
}

void PB_Path::nextPlatformPos(Vec3D *pos)
// returns the next waypoint on a platform or null vector if no platform on path
{
	Vec3D wpPlatPos;
	assert( platformPos != 0 );
	if (platformPos->size() == 0) {	// no platform on path
		vcopy(&zerovector, pos);
		return;
	}
	if ((currentWaypoint != waypoint->end()) && currentWaypoint->isOnPlatform()) {
		vcopy(currentWaypoint->pos(), &wpPlatPos);
	} else {
		WaypointList::iterator storeWP = currentWaypoint;
		PlatformList::iterator storePF = currentPlat;
		// simulate next WP
		reportWaypointReached();
		if ((currentWaypoint != waypoint->end()) && currentWaypoint->isOnPlatform()) {
			vcopy(currentWaypoint->pos(), &wpPlatPos);
		} else {	// alright this if-else is bad style :-(
			// simulate next WP
			reportWaypointReached();
			if ((currentWaypoint != waypoint->end()) && currentWaypoint->isOnPlatform()) {
				vcopy(currentWaypoint->pos(), &wpPlatPos);
			}
		}
		// restore values
		currentWaypoint = storeWP;
		currentPlat = storePF;
	}
	vcopy(&wpPlatPos, pos);
}



PB_Path_Waypoint PB_Path::getNextWaypoint()
// returns the next waypoint
{
	assert( waypoint != 0 );
	if (currentWaypoint != waypoint->end()) { // approaching another waypoint
		return (*currentWaypoint);
	} else {	// approaching the target navpoint
		//PB_Navpoint n = getNavpoint( data.endId );
		return PB_Path_Waypoint(navpoint_pos(endNav()), WP_IS_NAVPOINT, 0 );
	}
}


void PB_Path::getLastWaypointPos( EDICT *playerEnt, Vec3D *pos)
// returns the position of the last waypoint reached
{
	assert( waypoint != 0 );
	if (lastReachedWaypoint != waypoint->end()) { // at least one waypoint has been reached
		vcopy(lastReachedWaypoint->pos(playerEnt), pos);
	} else {	// nothing reached yet
		NAVPOINT *n = getNavpoint( data.startId );
		navpoint_pos(n, playerEnt, pos);
	}
}


void PB_Path::reportWaypointReached()
// confirms the waypoint as reached, internally choose next waypoint
{
	assert( waypoint != 0 );
	assert( platformPos != 0 );
	if (currentWaypoint == waypoint->end()) return;	// nothing more to reach...

	lastReachedWaypoint = currentWaypoint;
	lastReachedPlat = currentPlat;
	if (forwardPass) {								// path in forward direction ?
		if ( currentWaypoint->isOnPlatform() ) currentPlat++;
		currentWaypoint++;
	} else {
		if ( currentWaypoint->isOnPlatform() ) {
			if (currentPlat != platformPos->begin()) currentPlat--;
			else currentPlat = platformPos->end();
		}
		if (currentWaypoint != waypoint->begin()) {	// approaching another waypoint
			currentWaypoint--;
		} else {
			currentWaypoint = waypoint->end();		// manually set to end (backward!)			
		}
	}
}

void PB_Path::reportWaypointFailed()
// message that the waypoint has not been reached in time, roll back possible changes
{
	//startTime += 1.5;
}
