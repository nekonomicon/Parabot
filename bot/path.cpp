#include "parabot.h"
#include "path.h"
#include "pb_global.h"
#include <stdio.h>

int journeyMode = JOURNEY_FAST;

void setJourneyMode(int mode)
// sets global journey mode
{
	journeyMode = mode;
}

void
path_waypoint_construct(PATH_WAYPOINT *wp)
{
	wp->act = WP_NOT_INITIALIZED;
}

bool
path_waypoint_reached(PATH_WAYPOINT *wp, EDICT *ent)
// returns true if waypoint is reached from pos
{
	if (wp->act & WP_NOT_REACHABLE)
		return false;

	Vec3D wpDir, vdir;
	vsub(path_waypoint_pos(wp, ent), &ent->v.origin, &wpDir);
	float dist = vlen(&wpDir);

	if (dist < 16.0)
		return true;

	if (dist > 40.0)
		return false;

	vdir = ent->v.velocity;
	vscale(&wpDir, 1.0f / dist, &wpDir);
	normalize(&vdir);
	float dres = dotproduct(&vdir, &wpDir);
	if (dres < 0)
		return true;	// waypoint has been passed

	return false;

/*	float requiredDist = 40;
	// approach jumps and platforms up to 30 units
	if (action() == BOT_JUMP || isOnPlatform() ) requiredDist = 30;
	// approach ladder waypoints up to 20 units
	if (data.act & WP_ON_LADDER) requiredDist = 20;
	if (dist < requiredDist) return true;
	else return false;*/
}

void
path_waypoint_reset(PATH_WAYPOINT *wp)
{
	wp->act = WP_NOT_INITIALIZED;
}

bool
path_waypoint_valid(PATH_WAYPOINT *wp)
{
	return !(wp->act & WP_NOT_INITIALIZED);
}

Vec3D *
path_waypoint_pos(PATH_WAYPOINT *wp)
{
	return &wp->pos;
}

Vec3D *
path_waypoint_pos(PATH_WAYPOINT *wp, EDICT *ent) 
// adjusts positions if ent is on ladder
{
	assert(ent != 0);
	if (!is_onladder(ent))
		return &wp->pos;

	wp->pos.z += 20.0f;

	return &wp->pos;
}

int
path_waypoint_action(PATH_WAYPOINT *wp)
{
	return (wp->act & 0x0000FFFF);
}

bool
path_waypoint_isonladder(PATH_WAYPOINT *wp)
{
	return ((wp->act & WP_ON_LADDER) > 0);
}

bool
path_waypoint_isonplatform(PATH_WAYPOINT *wp)
{
	return ((wp->act & WP_ON_PLATFORM) > 0);
}

bool
path_waypoint_isatplatformstart(PATH_WAYPOINT *wp)
{
	return ((wp->act & WP_AT_PLATFORM_START) > 0);
}

bool
path_waypoint_isnavpoint(PATH_WAYPOINT *wp)
{
	return ((wp->act & WP_IS_NAVPOINT) > 0);
}

bool
path_waypoint_needstriggerforplat(PATH_WAYPOINT *wp)
{
	return ((wp->act & WP_PLAT_NEEDS_TRIGGER) > 0);
}

bool
path_waypoint_causeddamage(PATH_WAYPOINT *wp)
{
	return ((wp->act & WP_DMG_OCURRED) > 0);
}

/*
void
path_construct()
{
	data.startid = 0;
	data.endid = 0;
	waypoint = 0;
	hiddenattack = 0;
	platformpos = 0;
	readytodelete = false;
}

PB_Path::~PB_Path()
{*/
/*	don't delete anything because:
 *  1) if path is member of mapgrap the clear function is called there
 *  2) if path is temporal it might contain waypoint/hiddenattack-pointers to
 *     paths in mapgraph which must not be deleted
 */
//}

int
path_startid(PATH *path)
{
	return path->data.startid;
}

int
path_endid(PATH *path)
{
	return path->data.endid;
}

NAVPOINT *
path_startnav(PATH *path)
// returns the start navpoint
{
	return getNavpoint(path->data.startid);
}

NAVPOINT *
path_endnav(PATH *path)
// returns the end navpoint
{
	return getNavpoint(path->data.endid);
}

void
path_startrecord(PATH *path, int startPnt, float worldtime)
// start recording a path from given point at given time
{
	path->data.privateid = -1;
	path->data.dataid = -1;			// needs to be initialized in PB_MapGraph::addPath
	path->data.startid = startPnt;
	path->data.endid = -1;			// = recording
	path->starttime = worldtime;
	path->data.attempts = 0;
	path->data.lastattempt = getTotalAttempts();	// init as new
	path->data.successful = 0;
	path->data.losthealth = 0;
	path->data.enemyencounters = 0;
	path->readytodelete = false;
	path->specialid = -1;
	path->waypoint = new WaypointList;	
	path->hiddenattack = new AttackList;
	path->platformpos = new PlatformList;
}

void
path_addWaypoint(PATH *path, PATH_WAYPOINT *wp)
// adds the waypoint to the path
{
	assert(path->waypoint != 0);
	path->waypoint->push_back(*wp);
}

void
path_addwaypoint(PATH *path, Vec3D *pos, int action, float arrival)
// adds the waypoint to the path
{
	PATH_WAYPOINT wp = {.pos = *pos, .act = action, .arrival = (arrival - path->starttime)};
	assert(path->waypoint != 0);
	path->waypoint->push_back(wp);
}

void
path_addplatforminfo(PATH *path, int navid, Vec3D *pos)
// adds platform info
{
	PATH_PLATFORM pf = {.navid = navid, .pos = *pos};
	assert(platformpos != 0);
	path->platformpos->push_back(pf);
}

void
path_stoprecord(PATH *path, int endPnt, float worldtime, int mode)
// stops recording
{
	path->data.endid = endPnt;
	path->data.scheduledtime = worldtime - path->starttime;
	path->data.passtime = path->data.scheduledtime;
	path->data.specials = mode;
}

void
path_clear(PATH *path)
// deletes all path-data, cancels recording
{
	if (path->waypoint) {
		path->waypoint->clear();
		delete path->waypoint;
		path->waypoint = 0;
	}
	if (path->hiddenattack) {
		path->hiddenattack->clear();
		delete path->hiddenattack;
		path->hiddenattack = 0;
	}
	if (path->platformpos) {
		path->platformpos->clear();
		delete path->platformpos;
		path->platformpos = 0;
	}
	path->data.endid = 0;			// not recording anything
	path->readytodelete = true;	// just in case...
}

void
path_markfordelete(PATH *path)
{
	path->readytodelete = true;
}

void
path_makeindependant(PATH *path)
{
	path->data.dataid = path->data.privateid;
}

void
path_makedependantof(PATH *path, int id)
{
	path->data.dataid = id;
}

#if _DEBUG
void
path_print(PATH *path)
{
	DEBUG_MSG( "path from " );
	path_startnav().print();
	DEBUG_MSG( " to " );
	path_endnav().print();
}
#endif

#include "marker.h"
#include <queue>

#if _DEBUG
void
path_mark(PATH *path)
// mark all waypoints
{
	WaypointList::iterator wpi = waypoint->begin();
	PB_Path_Waypoint wp;
	while (wpi != waypoint->end()) {
		glMarker.newMarker( wpi->pos(), 1 );
		wpi++;
	}
	if (currentwaypoint != waypoint->end()) {
		glMarker.newMarker( Vector(0,0,10)+currentwaypoint->pos(), 1 );
	} else
		DEBUG_MSG( "Current wp = end\n" );
	if (lastreachedwaypoint != waypoint->end()) {
		glMarker.newMarker( Vector(0,0,10)+lastreachedwaypoint->pos(), 2 );
	} else
		DEBUG_MSG( "Last reached wp = end\n" );
}
#endif //_DEBUG

void
path_load(PATH *path, FILE *fp)
// init path from file
{
	path->readytodelete = false;

	// read data
	int i;
	fread( &path->data, sizeof(PATH_SAVEDATA), 1, fp );

	if (path->data.dataid == path->data.privateid) {	// read lists if stored here
		// read waypoints
		path->waypoint = new WaypointList;
		int numWpts;
		fread( &numWpts, sizeof(int), 1, fp );
		PATH_WAYPOINT wp;
		assert(path->waypoint != 0);
		for (i = 0; i < numWpts; i++) {
			//printf( "w" );
			fread( &wp, sizeof(PATH_WAYPOINT), 1, fp );
			path->waypoint->push_back( wp );
		}

		// read attacks
		path->hiddenattack = new AttackList;
		int numAtts;
		fread( &numAtts, sizeof(int), 1, fp );
		PATH_ATTACK att;
		assert( path->hiddenattack != 0 );
		for (i = 0; i < numAtts; i++) {
			//printf( "a" );
			fread( &att, sizeof(PATH_ATTACK), 1, fp );
			path->hiddenattack->push_back( att );
		}

		// read platform info
		path->platformpos = new PlatformList;
		int numPlats;
		fread( &numPlats, sizeof(int), 1, fp );
		PATH_PLATFORM pf;
		assert(path->platformpos != 0 );
		for (i = 0; i < numPlats; i++) {
			fread(&pf.navid, sizeof(int), 1, fp);
			fread(&pf.pos, 3 * sizeof(float), 1, fp);
			if(pf.navid < 0) {
				path->readytodelete = true;
				DEBUG_MSG( "Deleted 1 path because of incorrect platform info\n" );
			}
			path->platformpos->push_back( pf );
		}
	} else {	// must be initialized later on when all paths are loaded...
		path->waypoint = 0;
		path->hiddenattack = 0;
		path->platformpos = 0;
	}
}

void
path_save(PATH *path, FILE *fp)
// save path to file if not readytodelete
{
	if (path->readytodelete) return;
	
	// save data
	//printf( "P" );
	int i;
	fwrite(&path->data, sizeof(PATH_SAVEDATA), 1, fp );
	
	if (path->data.dataid == path->data.privateid) {	// save lists if stored here
		// save waypoints
		assert( path->waypoint != 0 );
		int numWpts = path->waypoint->size();
		fwrite( &numWpts, sizeof(int), 1, fp );
		WaypointList::iterator wpi = path->waypoint->begin();
		// PATH_WAYPOINT wp;
		for (i = 0; i < numWpts; i++) {
			//printf( "W" );
			fwrite(&wpi, sizeof(PATH_WAYPOINT), 1, fp );
			wpi++;
		}
		
		// save attacks
		assert( hiddenattack != 0 );
		int numAtts = path->hiddenattack->size();
		fwrite( &numAtts, sizeof(int), 1, fp );
		AttackList::iterator atti = path->hiddenattack->begin();
		//PB_Path_Attack att;
		for (i = 0; i < numAtts; i++) {
			//printf( "A" );
			fwrite( &atti, sizeof(PATH_ATTACK), 1, fp );
			atti++;
		}

		// save platform pos
		assert( platformpos != 0 );
		int numPlats = path->platformpos->size();
		fwrite( &numPlats, sizeof(int), 1, fp );
		PlatformList::iterator pfi = path->platformpos->begin();
		for (i = 0; i < numPlats; i++) {
			assert(pfi->navid < 1000);
			fwrite(&pfi->navid, sizeof(int), 1, fp);
			fwrite(&pfi->pos, 3 * sizeof(float), 1, fp);
			pfi++;
		}
	}
}

void
path_initreturnof(PATH *newpath, PATH *oldpath)
{
	newpath->data.privateid = -1;
	newpath->data.dataid = oldpath->data.privateid;
	newpath->data.startid = path_endid(oldpath);
	newpath->data.endid = path_startid(oldpath);
	newpath->data.scheduledtime = oldpath->data.scheduledtime;
	newpath->data.passtime = newpath->data.scheduledtime;
	newpath->data.specials = 0;
	newpath->data.attempts = 0;
	newpath->data.lastattempt = 0;
	newpath->data.successful = 0;
	newpath->data.losthealth = 0;
	newpath->data.enemyencounters = 0;
	newpath->readytodelete = false;
	newpath->specialid = oldpath->specialid;
	newpath->waypoint = oldpath->waypoint;			// use same data 
	newpath->hiddenattack = oldpath->hiddenattack;
	newpath->platformpos = oldpath->platformpos;
}

void
path_setid(PATH *path, int id)
{
	path->data.privateid = id;
}

int
path_id(PATH *path)
{
	return path->data.privateid;
}

int
path_dataid(PATH *path)
{
	return path->data.dataid;
}

int
path_getspecialid(PATH *path)
{
	return path->specialid;
}

void
path_setspecialid(PATH *path, int sId)
{
	path->specialid = sId;
}

int
path_mode(PATH *path)
{
	return path->data.specials;
}

bool
path_deleted(PATH *path)
{
	return path->readytodelete;
}

bool
path_hasdata(PATH *path)
{
	return (path->data.dataid == path->data.privateid);
}

bool
path_isrecording(PATH *path)
{
	return (-1 == path->data.endid);
}

float
path_weight(PATH *path)
// used by graph algo, returns weight of path depending of journeyMode
{ 
	float certainty = 0.5;		// base certainty for new paths
	float encounterProb = 0.1;	

	switch( journeyMode ) {

	case JOURNEY_FAST:			// return fastest journey
		return path->data.passtime; 

	case JOURNEY_RELIABLE:		// return most reliable journey
		if (path->data.attempts > 0)
			certainty = ((float)path->data.successful) / ((float)path->data.attempts);
		return (1.0001f - certainty);

	case JOURNEY_CROWDED:		// return journey with most enemy encounters
		if (path->data.attempts > 0) {
			encounterProb = ((float)path->data.enemyencounters) / ((float)path->data.attempts);
			if (encounterProb > 9.9) encounterProb = 9.9;
		}
		return (10.0f - encounterProb);	// encounterProb may be >1

	case JOURNEY_LONELY:
		if (path->data.attempts > 0) 
			encounterProb = ((float)path->data.enemyencounters) / ((float)path->data.attempts);
		return encounterProb;
	}
	DEBUG_MSG( "Unknown journeymode!\n" );
	return 0;
}	

bool
path_valid(PATH *path, int mode)
{
	if (path->readytodelete) return false;	// don't accept deleted paths

	int dif = mode ^ path->data.specials;	// differences between searchmode and path-specials
	int neg = mode ^ (-1);		// specials not allowed in searchmode
	return !(dif & neg);		// only valid if AND = 0
}

void
path_startattempt(PATH *path, float worldtime)
// start the path at given time
{
	Vec3D dir;

	assert(path->waypoint != 0);
	assert(path->platformpos != 0);

	if (path->waypoint->size() > 1) {
		WaypointList::iterator wpForw = path->waypoint->begin();
		vsub(path_waypoint_pos(&(*wpForw)), navpoint_pos(path_startnav(path)), &dir);
		float distForw = vlen(&dir);
		WaypointList::iterator wpBackw = path->waypoint->end();
		wpBackw--;
		vsub(path_waypoint_pos(&(*wpBackw)), navpoint_pos(path_startnav(path)), &dir);
		float distBackw = vlen(&dir);
		// path in forward direction ?
		if (distForw <= distBackw) {
			path->forwardpass = true;
			path->currentwaypoint = wpForw;
			path->currentplat = path->platformpos->begin();
		} else {
			path->forwardpass = false;
			path->currentwaypoint = wpBackw;
			path->currentplat = path->platformpos->end();	
			if (path->platformpos->size() > 1) path->currentplat--;
		}
	} else {	// no waypoints stored: simulate forward
		path->forwardpass = true;
		path->currentwaypoint = path->waypoint->begin();
		path->currentplat = path->platformpos->begin();
	}

	path->lastreachedwaypoint = path->waypoint->end();	// nothing reached yet
	path->lastreachedplat = path->platformpos->end();
	path->starttime = worldtime;
	path->data.attempts++;
	incTotalAttempts();
	path->data.lastattempt = getTotalAttempts();
	path->lastwaitingplat = 0;
	path->ignoredplat = 0;
}

void
path_cancelattempt(PATH *path)
// cancels the pass, no reportTarget needs to be called
{
	if (path->data.attempts > 0) path->data.attempts--;
}

void
path_reporttargetreached(PATH *path, EDICT *traveller, float worldtime)
// confirms path as finished, internally set new success-variables
{
	float allTime   = (path->data.passtime  * ((float)path->data.successful)) 
					+ (worldtime - path->starttime);

	path->data.successful++;
	if (path->data.successful == 0) {
		ERROR_MSG( "reportTargetReached!" );
		return;
	}
	path->data.passtime = allTime / ((float)path->data.successful);		// set new passtime
	navpoint_reportvisit(path_endnav(path), traveller, worldtime ); //now in observer
	if (path->ignoredplat) {	// we ignored a plat but arrived nevertheless?
		WaypointList::iterator wpi = path->waypoint->begin();
		PlatformList::iterator pfi = path->platformpos->begin();
		while (wpi!=path->waypoint->end()) {
			if (path_waypoint_isonplatform(&(*wpi))) {
				EDICT *plat = navpoint_entity(getNavpoint(pfi->navid));
				// delete the platform flag for these waypoints
				if (plat==path->ignoredplat) wpi->act &= (~WP_ON_PLATFORM);
				pfi++;
			}
			wpi++;
		}
	}
}

extern bool pb_pause;

void
path_reporttargetfailed(PATH *path)
// confirms path as failed, internally set new success-variables
{
	if (path->data.attempts == 0) return;	// must be...

	float certainty = ((float)path->data.successful) / ((float)path->data.attempts);	
	// check new certainty
	if (certainty < 0.5f) {
		path_markfordelete(path);
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

void
path_addattack(PATH *path, Vec3D *origin)
// stores the position of attack (inclusive actual position in path)
{
	assert( path->hiddenattack != 0 );

	PATH_ATTACK att = {.pos = *origin, .time = path->currentwaypoint->arrival};

	path->hiddenattack->push_back( att );
}

void
path_reportenemyspotted(PATH *path)
{
	path->data.enemyencounters++;
}

void
path_getviewpos(PATH *path, EDICT *traveller, int *prior, Vec3D *pos) 
// Priority values return 0 (nextWaypoint) or 2 (button)
{
	assert( path->waypoint != 0 );
	if ( path->lastreachedwaypoint != path->waypoint->end() ) {	// watch at buttons for pressing them
		if (path_waypoint_action(&(*path->lastreachedwaypoint)) == BOT_USE) {
			assert( traveller != 0 );
			// Vec3D t = traveller->v.origin;
			*pos = *navpoint_pos(path_startnav(path));
			*prior = 2;	// UNDONE: ButtonUse uses AimDir, not ViewDir!!!
			return;
		}
	}
	*prior = 0;

	PATH_WAYPOINT wp;
	path_getnextwaypoint(path, &wp);
	vadd(path_waypoint_pos(&wp, traveller), &traveller->v.view_ofs, pos);
}

bool
path_timeout(PATH *path, float worldtime)
// returns true if the next waypoint could not be reached in time
{
	float plan = path->data.scheduledtime;			// default = approaching path end

	assert( path->waypoint != 0 );
	if (path->currentwaypoint != path->waypoint->end()) {	// approaching another waypoint
		if (path->forwardpass) {						// path in forward direction ?
			plan = path->currentwaypoint->arrival;
		} else {	// backward passing
			plan = path->data.scheduledtime - path->currentwaypoint->arrival;
		}
	}

	if (worldtime > (path->starttime + plan + 3)) return true;
	return false;
}

bool
path_cannotbecontinued(PATH *path, EDICT *ent)
// returns true if the path has to be canceled
{
	Vec3D dir;

	PATH_WAYPOINT nextWP;
	path_getnextwaypoint(path, &nextWP);
	vsub(path_waypoint_pos(&nextWP, ent), &ent->v.origin, &dir);
	float dist = vlen(&dir);
	if (dist > 100) {
		// suppose bot has fallen...
		if (path_waypoint_isonplatform(&nextWP)) return true;	 
	}
	return false;
}

bool
path_finished(PATH *path, EDICT *traveller)
// returns true if all waypoints have been confirmed and end-navpoint is reached by traveller
{
	// UNDONE for teleporter hintpoints!
//	if (path->currentwaypoint == path->waypoint->end()) {	// no more waypoints to go...
	if (navpoint_reached(path_endnav(path), traveller)) return true;
//	}
	return false;
}

int
path_getnextaction(PATH *path)
// returns the action to perform at the next waypoint
{
	if (!path_hasdata(path)) return 0;					// don't do anything on return path

	assert( path->waypoint != 0 );
	if (path->currentwaypoint != path->waypoint->end()) {	// approaching another waypoint
		return path_waypoint_action(&(*path->currentwaypoint));
	}

	return 0;
}

bool
path_waitforplatform(PATH *path)
// return true if bot has to wait for a platform before continuing to the next waypoint
{
	#define MAX_PLAT_WAIT 2.0	// maximal time to wait if platform is not moving

	bool wait = false;
	assert( path->platformpos != 0 );
	if (path->platformpos->size() == 0) return false;	// no platform on path

	if ((path->currentwaypoint != path->waypoint->end()) && path_waypoint_isonplatform(&(*path->currentwaypoint))) {
		if (path->currentplat != path->platformpos->end()) {
			EDICT *plat = navpoint_entity(getNavpoint(path->currentplat->navid));
			assert (plat != 0);

			if (plat != path->lastwaitingplat) {
				path->lastwaitingplat = plat;
				path->waitplatendtime = worldtime() + MAX_PLAT_WAIT;
			}

			Vec3D tDir, vDir;
			vsub(&path->currentplat->pos, &plat->v.absmin, &tDir);
			float tLen = vlen(&tDir);
			vDir = plat->v.velocity;
			float vLen = vlen(&vDir);
			if (vLen > 0) path->waitplatendtime = worldtime() + MAX_PLAT_WAIT;

			if (tLen > 16) wait = true;

			if (worldtime() > path->waitplatendtime) {
				path->ignoredplat = plat;
				wait = false;
			}

	/*		if (getNavpoint( currentplat->navid ).type()==NAV_F_PLAT) {
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

void
path_nextplatformpos(PATH *path, Vec3D *pos)
// returns the next waypoint on a platform or null vector if no platform on path
{
	Vec3D wpPlatPos;
	assert( path->platformpos != 0 );
	if (path->platformpos->size() == 0) {	// no platform on path
		*pos = zerovector;
		return;
	}
	if ((path->currentwaypoint != path->waypoint->end()) && path_waypoint_isonplatform(&(*path->currentwaypoint))) {
		wpPlatPos = *path_waypoint_pos(&(*path->currentwaypoint));
	} else {
		WaypointList::iterator storeWP = path->currentwaypoint;
		PlatformList::iterator storePF = path->currentplat;
		// simulate next WP
		path_reportwaypointreached(path);
		if ((path->currentwaypoint != path->waypoint->end()) && path_waypoint_isonplatform(&(*path->currentwaypoint))) {
			wpPlatPos = *path_waypoint_pos(&(*path->currentwaypoint));
		} else {	// alright this if-else is bad style :-(
			// simulate next WP
			path_reportwaypointreached(path);
			if ((path->currentwaypoint != path->waypoint->end()) && path_waypoint_isonplatform(&(*path->currentwaypoint))) {
				wpPlatPos = *path_waypoint_pos(&(*path->currentwaypoint));
			}
		}
		// restore values
		path->currentwaypoint = storeWP;
		path->currentplat = storePF;
	}
	*pos = wpPlatPos;
}

void
path_getnextwaypoint(PATH *path, PATH_WAYPOINT *wp)
// returns the next waypoint
{
	assert(path->waypoint != 0);
	if (path->currentwaypoint != path->waypoint->end()) { // approaching another waypoint
		*wp = (*path->currentwaypoint);
	} else {	// approaching the target navpoint
		//PB_Navpoint n = getNavpoint( data.endid );
		*wp = (PATH_WAYPOINT){.pos = *navpoint_pos(path_endnav(path)), .act = WP_IS_NAVPOINT, .arrival = 0.0f};
	}
}

void
path_getlastwaypointpos(PATH *path, EDICT *playerEnt, Vec3D *pos)
// returns the position of the last waypoint reached
{
	assert( path->waypoint != 0 );
	if (path->lastreachedwaypoint != path->waypoint->end()) { // at least one waypoint has been reached
		*pos = *path_waypoint_pos(&(*path->lastreachedwaypoint), playerEnt);
	} else {	// nothing reached yet
		NAVPOINT *n = getNavpoint( path->data.startid );
		navpoint_pos(n, playerEnt, pos);
	}
}

void
path_reportwaypointreached(PATH *path)
// confirms the waypoint as reached, internally choose next waypoint
{
	assert( path->waypoint != 0 );
	assert( path->platformpos != 0 );
	if (path->currentwaypoint == path->waypoint->end()) return;	// nothing more to reach...

	path->lastreachedwaypoint = path->currentwaypoint;
	path->lastreachedplat = path->currentplat;
	if (path->forwardpass) {								// path in forward direction ?
		if (path_waypoint_isonplatform(&(*path->currentwaypoint))) path->currentplat++;
		path->currentwaypoint++;
	} else {
		if (path_waypoint_isonplatform(&(*path->currentwaypoint))) {
			if (path->currentplat != path->platformpos->begin()) path->currentplat--;
			else path->currentplat = path->platformpos->end();
		}
		if (path->currentwaypoint != path->waypoint->begin()) {	// approaching another waypoint
			path->currentwaypoint--;
		} else {
			path->currentwaypoint = path->waypoint->end();		// manually set to end (backward!)			
		}
	}
}

static void
path_reportwaypointfailed(PATH *path)
// message that the waypoint has not been reached in time, roll back possible changes
{
	//path->starttime += 1.5;
}
