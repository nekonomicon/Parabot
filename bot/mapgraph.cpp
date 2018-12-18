#pragma warning( disable : 4786 )	// disable 29 graph warnings

#include "parabot.h"
// #include "mapgraph.h"
#include "dynpq.h"
#include "pb_global.h"
#include <stdio.h>

static MAPGRAPH mapgraph;

// id used in waypoint files
static const char *PNFidString = "Parabot Waypoint File 0.8      ";

NAVPOINT* cashedNavpoint[32+1];

void
invalidateMapGraphCash()
{
	for (int i = 0; i <= 32; i++)
		cashedNavpoint[i] = 0;
}

void
cachePlayerData()
{
	for (int i = 1; i <= com.globals->maxclients; i++) {
		EDICT *player = edictofindex( i );
		if (playerexists(player)) {
			cashedNavpoint[i] = mapgraph_getnearestnavpoint(&player->v.origin);
		}
	}
}

NAVPOINT *
getNearestNavpoint( EDICT *pEdict )
{
	int i = indexofedict(pEdict);
	if (i < 0 || i > 32) return 0;
	return cashedNavpoint[i];
}

/*
PB_MapGraph::PB_MapGraph() : 
	graph( 1024, 64 )
{
	nextid = 0;
	nextpathid = 0;	// Path ID
	passcount = 0;
	memset( &mapgraph.availableitems, false, sizeof mapgraph.availableitems );
}


PB_MapGraph::~PB_MapGraph()
{
	clear();
}
*/

void
mapgraph_clear()
// delete all data
{
	//int numPaths;

	for (int i=0; i< 1024/*graph.size()*/; i++) {
		//numPaths = mapgraph.graph[i].second.size();

		// clear each path that has own data
    	AdjList::iterator adj = mapgraph.graph[i].second.begin();
		while (adj != mapgraph.graph[i].second.end()) {
			if (adj->second.hasData()) adj->second.clear();
			adj++;
		}
		mapgraph.graph[i].second.clear();
	}
	//graph.clear();
	mapgraph.nextid = 0;
	mapgraph.nextpathid = 0;
	invalidateMapGraphCash();
}

void
mapgraph_incpasses()
{
	mapgraph.passcount++;
}

int
mapgraph_getpasscount()
{
	return mapgraph.passcount;
}

int
mapgraph_numberofnavpoints()
{
	return 1024; //graph.size();
}

int
mapgraph_numberofpaths()
// returns the number of paths
{
	int count = 0;
    for (int i = 0; i < 1024/*graph.size()*/; i++) {
		// count each path that is not deleted
    	AdjList::iterator adj = mapgraph.graph[i].second.begin();
		while (adj != mapgraph.graph[i].second.end()) {
			if (!adj->second.deleted()) count++;
			adj++;
		}
	}
    return count;
}

Node *
mapgraph_getnode(int i)
{
	return &mapgraph.graph[i];
}

bool
mapgraph_itemavailable(int itemCode)
{
	return mapgraph.availableitems[itemCode];
}

static AdjPtr
mapgraph_findpath(int id, int startId, bool &found)
// find path with id beginning at navpoint startId
{
	found = false;
//  vielleicht schneller:
//	GraphType::Successor::iterator si = mapgraph.graph[startId].second.find(...);
//	(*si).second.init( 0, 0 );

	AdjPtr adj = mapgraph.graph[startId].second.begin();

	while (adj != mapgraph.graph[startId].second.end() && !found) {
		if ((*adj).second.id()==id) {
			found = true;
			return adj;
		}
		adj++;
	}
	return adj;
}

PB_Path *
mapgraph_findpath(int id)
// find path with id
{
	bool found = false;
	int  start = 0;
	AdjPtr adj;

	while ( !found && start<mapgraph_numberofnavpoints() ) {
		adj = mapgraph_findpath( id, start, found );
		start++;
	}
	if (found) return &(adj->second);
	return 0;
}

static AdjPtr
mapgraph_findlinkedpath(int dataId, int startId, bool &found)
// find path with id beginning at navpoint startId
{
	found = false;
	AdjPtr adj = mapgraph.graph[startId].second.begin();

	while (adj != mapgraph.graph[startId].second.end() && !found) {
		if (adj->second.dataId()==dataId) {
			found = true;
			return adj;
		}
		adj++;
	}
	return adj;
}

NAVPOINT *
mapgraph_getnearestnavpoint(const Vec3D *pos)
// returns the nearest navpoint to pos existing in the graph, NULL if graph is empty
{
	float dist, minDist = 999999;
	int   minId = -1;
	Vec3D dir;

	for (int i = 0; i < mapgraph_numberofnavpoints(); i++) {
		vsub(pos, navpoint_pos(&mapgraph.graph[i].first), &dir);
		dist = dotproduct(&dir, &dir);
		if (dist < minDist) {
			minDist = dist;
			minId = i;
		}
	}
	if (minId >= 0) return &mapgraph.graph[minId].first;
	return 0;
}

NAVPOINT *
mapgraph_getnearestnavpoint(const Vec3D *pos, int type)
// returns the nearest navpoint with given type to pos existing in the graph, 
// NULL if graph doesn't contain navpoints of the given type
{
	float dist, minDist = 999999;
	int   minId = -1;
	Vec3D dir;

	for (int i = 0; i < mapgraph_numberofnavpoints(); i++) {
		if (navpoint_type(&mapgraph.graph[i].first)==type) {
			vsub(pos, navpoint_pos(&mapgraph.graph[i].first), &dir);
			dist = dotproduct(&dir, &dir);
			if (dist < minDist) {
				minDist = dist;
				minId = i;
			}
		}
	}
	if (minId >= 0) return &mapgraph.graph[minId].first;

	return 0;
}

int
mapgraph_linkednavpointsfrom(NAVPOINT *nav)
// returns the number of other nodes linked to nav in the graph
{
	if (!nav) return 0;
	else { 
		int count = 0;
		AdjPtr adj = mapgraph.graph[navpoint_id(nav)].second.begin();
		while (adj != mapgraph.graph[navpoint_id(nav)].second.end()) {
			if ( !(adj->second.deleted()) ) count++;	// only count valid paths
			adj++;
		}
		return count;
	}
}


NAVPOINT *
mapgraph_getnearestroamingnavpoint(EDICT *traveller, NAVPOINT *ignore)
// returns a navpoint to approach in roaming mode, prefers linked navpoints and the ones
// that are z-reachable, nextVisit-Time is required to be reached
// should never return zero
{
	float dist, minDist = 999999, minDistR = 999999;
	int   minId = -1, minIdR = -1;
	int	  ignoreId = -1;
	Vec3D *pos = &traveller->v.origin, dir;

	if (ignore) ignoreId = navpoint_id(ignore);

	for (int i = 0; i < mapgraph_numberofnavpoints(); i++) 
	if ((i != ignoreId) && (worldtime() >= navpoint_nextvisit(&mapgraph.graph[i].first, traveller))) {
		vsub(pos, navpoint_pos(&mapgraph.graph[i].first), &dir);
		dist = dotproduct(&dir, &dir);
		if (mapgraph.graph[i].second.size() > 0)
			dist *= 0.5f;	// prefer linked navpoints
		if (dist < minDist) {
			if (pointcontents(navpoint_pos(&mapgraph.graph[i].first)) == CONTENTS_WATER)
				continue; 
			minDist = dist;
			minId = i;
		}
		if ((navpoint_pos(&mapgraph.graph[i].first)->z < (pos->z + 45.0f)) && (navpoint_pos(&mapgraph.graph[i].first)->z > (pos->z - 50.0f))) {
			// this one is roaming reachable!
			if (dist < minDistR) {
				if (pointcontents(navpoint_pos(&mapgraph.graph[i].first)) == CONTENTS_WATER)
					continue;
				minDistR = dist;
				minIdR = i;
			}
		}
	}
	if (minIdR >= 0)
		return &mapgraph.graph[minIdR].first;
	else if (minId >= 0)
		return &mapgraph.graph[minId].first;

	return &mapgraph.graph[0].first;	// fuck it, we *HAVE* to return something!
}

bool
mapgraph_addnavpoint(NAVPOINT *navpoint)
// add a new navpoint to the graph
{
	navpoint_setid(navpoint, mapgraph.nextid++);
	navpoint_initentityptr(navpoint);
	//int gs = graph.size();
	Node x = Node(*navpoint, AdjList());
	// graph.push_back(x);
	mapgraph.availableitems[navpoint_type(navpoint)] = true;
	return true;
}

bool
mapgraph_addpath(PB_Path &path, int directed, bool idInit)
// add a new path to the graph
{
	if (idInit) {
		path.setId( mapgraph.nextpathid++ );
		if (path.dataId() < 0) path.makeIndependant();
	}
	(mapgraph.graph[path.startId()].second).insert( std::make_pair(path.endId(), path) );

	if (directed==GRAPH_BIDIRECTIONAL) {
		PB_Path rp;
		rp.initReturnOf(path);
		if (idInit)
			rp.setId(mapgraph.nextid++);
		(mapgraph.graph[rp.startId()].second).insert(std::make_pair(rp.endId(), rp));
	}
	return true;
}

void
mapgraph_addifimprovement(PB_Path &path, bool addReturn)
// checks if any of the directions is improvement to graph, any improvement is added
// deleting existing (slower) path
// if path is not used, all path data get deleted!
{
	//char *cname = path.endNav().classname();
	bool used = false;
	std::deque<int> journey;
	float oldWeight = mapgraph_shortestjourney( path.startId(), path.endId(), path.mode(), journey ) - 0.1;
	if (path.weight() < oldWeight) {
		mapgraph_addpath( path, GRAPH_ONEWAY );
		used = true;
/*		DEBUG_MSG( "Added path %i from ", path.id() );
		path.startNav().print();
		DEBUG_MSG( " to " );
		path.endNav().print();*/
	}
/*	else {
		DEBUG_MSG( "Discarded path to %s\n", cname );
	}*/
	if (addReturn) {
		PB_Path rp;
		rp.initReturnOf(path);
		//journey.clear();	now in function
		//oldWeight = mapgraph_shortestjourney(rp.startId(), rp.endId(), rp.mode(), journey) - 0.1;
		mapgraph_shortestjourney(rp.startId(), rp.endId(), rp.mode(), journey);
		//if (rp.weight() < oldWeight) {		// insert return as well?
		
		if (journey.size() == 0) {	// only insert return if no alternative path
			mapgraph_addpath(rp, GRAPH_ONEWAY);	// don't delete anything: return is uncertain...
			used = true;
			// DEBUG_MSG( ", added return path %i", rp.id() );
		} 
	}
	// DEBUG_MSG( "\n" );
	if (!used) path.clear();
}

/*
bool
mapgraph_deletepath(int id, int startId)
{
	bool found;
	AdjPtr adj = mapgraph_findpath( id, startId, found );

	if (!found) return false;	
	if (adj->second.hasData()) {					// look if other path shares data...
		AdjPtr back = mapgraph_findlinkedpath( id, adj->second.endId(), found );
		if (found) {
			back->second.makeIndependant();						// ...yes -> swap sharing!
			adj->second.makeDependantOf( back->second.id() );	// only 1 reference to data!
		}
	}
	adj->second.markForDelete();	// don't delete instantly! (bots may be using it)
	return true;
}
*/

static void
mapgraph_dijkstra(std::vector<float>& dist, std::vector<int>& path, int start, int searchMode)
{
	int actualVertex;

	// init distances
	dist = std::vector<float>(1024/*graph.size()*/, MAX_PATH_WEIGHT);
	dist[start] = 0;

	// init paths
	path = std::vector<int>(1024/*graph.size()*/, -1);

	// init priority queue
	dynamic_priority_queue<float> queue(dist);

	/* In the next step, all vertices are extracted  one by one from
	the priority queue, and precisely in the order of the estimated
	distance towards the starting vertex. Obviously, the starting
	vertex is dealt with first. No vertex is looked at twice. */

	while(!queue.empty()) {
		actualVertex = queue.topIndex();   // extract vertex with minimum
		queue.pop();
          
		// improve estimates for all neighbors of u
		AdjList::iterator I = mapgraph.graph[actualVertex].second.begin();	// I cycles through paths

		while(I != mapgraph.graph[actualVertex].second.end()) {
			PB_Path p = (*I).second;
			if (p.valid(searchMode)) {
				int Neighbor = (*I).first;
				float d = p.weight();

				// Relaxation
				if (dist[Neighbor] > (dist[actualVertex] + d)) {
					// improve estimate
					queue.changeKeyAt(Neighbor, dist[actualVertex] + d);
					// actualVertex is predecessor of the neighbor
					path[Neighbor] = p.id();
				}
			}
			++I;
		}
	}
}

float
mapgraph_shortestjourney(int start, int target, int mode, std::deque<int> &journey)
{
	std::vector<float> dist;
	std::vector<int> lastPath;
	PB_Path *path;

	if (!journey.empty())
		journey.clear();	// delete journey 
	mapgraph_dijkstra( dist, lastPath, start, mode );
	float d = dist[target];

	while (lastPath[target] >= 0) {
		journey.push_back( lastPath[target] );
		path = mapgraph_findpath( lastPath[target] );
		if (path) target = path->startId();
		else break;
	}
	return d;
}

bool
mapgraph_getjourney(int start, int target, int mode, JOURNEY *journey)
// returns a journey from start to target or false if none available in mode
{
	//journey.pathList.clear();	now in function
	mapgraph_shortestjourney( start, target, mode, journey->pathlist );
	if (journey->pathlist.empty())
		return false;
	return true;
}

static int
mapgraph_dijkstratowish(std::vector<float>& dist, std::vector<int>& path, int start, NEEDS *needs, int searchMode, EDICT *traveller)
{
	// init distances
	dist = std::vector<float>(1024/*graph.size()*/, MAX_PATH_WEIGHT);
	dist[start] = 0;

	// init scores
	std::vector<float> score = std::vector<float>(1024/*graph.size()*/, -1);
	score[start] = 0;

	// init paths
	path = std::vector<int>(1024/*graph.size()*/, -1);

	// init priority queue
	dynamic_priority_queue<float> queue(dist);

	float navWeight = 1.0;
	int actualVertex;
	int targetNav = -1;
	bool targetFound = false;

	while(!queue.empty() && !targetFound)
	{
		actualVertex = queue.topIndex();   // extract vertex with minimum
		queue.pop();
		if (score[actualVertex] >= 5) {	// cut
			targetFound=true;
			targetNav = actualVertex;
			break;
		}

		// improve estimates for all neighbors of actualVertex
		AdjList::iterator adj = mapgraph.graph[actualVertex].second.begin();	

		// I cycles through paths		
		while(adj != mapgraph.graph[actualVertex].second.end()) {
			PB_Path p = adj->second;
			if ( p.valid(searchMode) ) {
				int neighbor = adj->first;
				float d = p.weight();
				
				// Relaxation
				if (dist[neighbor] > (dist[actualVertex] + d)) {
					// improve estimate
					queue.changeKeyAt(neighbor, dist[actualVertex] + d);
					// actualVertex is predecessor of the neighbor
					path[neighbor] = p.id();
					if (navpoint_nextvisit(&mapgraph.graph[neighbor].first, traveller) < worldtime() ) {
						navWeight += 0.1;
						score[neighbor] = score[actualVertex] + 
						    (needs_desirefor(needs, navpoint_type(&mapgraph.graph[neighbor].first))
						    / navWeight );
					}
				}
			}
			adj++;
		}
	}

	if ( targetFound ) {	// in case of cut
		queue.clear();	
	} else {
		targetNav = ( std::max_element( score.begin(), score.end() ) - score.begin() );
	}
	return targetNav;
}

int
mapgraph_getwishjourney(int start, NEEDS *needs, int mode, JOURNEY *journey, EDICT *traveller)
// returns the nav-id that according to wishList is the best to head for and the best
// journey to get there if possible in mode, if not returns -1 and an empty journey
{
	journey->pathlist.clear();

	std::vector<float> dist;
	std::vector<int> lastPath;
	PB_Path *path;

	int bestTarget = mapgraph_dijkstratowish( dist, lastPath, start, needs, mode, traveller );
	if (bestTarget == start) return -1;

	int target = bestTarget;
	while (lastPath[target] >= 0) {
		journey->pathlist.push_back(lastPath[target]);
		path = mapgraph_findpath(lastPath[target]);
		if (path)
			target = path->startId();
		else
			break;
	}
	if (journey->pathlist.empty()) return -1;
	return bestTarget;
}

static void
mapgraph_initbackwardpaths()
// called after load(), initializes data structures of backward paths
{
	// int numPaths;
	PB_Path p;
	AdjPtr rp;
	bool ok;

	for (int i = 0; i < mapgraph_numberofnavpoints(); i++) {
		//numPaths = mapgraph.graph[i].second.size();
		AdjList::iterator adj = mapgraph.graph[i].second.begin();
		while (adj != mapgraph.graph[i].second.end()) {
			p = adj->second;
			if (!p.hasData()) {
				rp = mapgraph_findpath(p.dataId(), p.endId(), ok);	// return path starts at end
				if (ok) {
					adj->second.waypoint = rp->second.waypoint;
					adj->second.hiddenAttack = rp->second.hiddenAttack;
					adj->second.platformPos = rp->second.platformPos;
				} else {
					DEBUG_MSG("FATAL ERROR: Return path %i could not be initialized!\n", p.id());
				}
			}
			adj++;
		}
	}
}

static void
mapgraph_preparebackwardpaths()
// called before save(), copies data structures of deleted paths to backward paths
{
	AdjPtr	adj, back;
	bool	found;

	for (int startId = 0; startId < mapgraph_numberofnavpoints(); startId++) {
		adj = mapgraph.graph[startId].second.begin();
		while (adj != mapgraph.graph[startId].second.end()) {
			if ((adj->second.deleted()) && (adj->second.hasData())) {	// if path deleted
				back = mapgraph_findlinkedpath(adj->second.id(), adj->second.endId(), found);
				if (found) {
					back->second.makeIndependant();						// ...yes -> swap sharing!
					adj->second.makeDependantOf(back->second.id());	// only 1 reference to data!
				}
			}
			adj++;
		}
	}
}

bool
mapgraph_load(const char *mapname)
{
	FILE *fp;

	if ((fp = fopen( mapname, "rb" )) == NULL)
		return false;

	char idString[32];	// check for correct version
	fread( &idString, 32, 1, fp );
	if (!Q_STREQ( idString, PNFidString)) {
		fclose(fp);
		return false;
	}

	fread( &mapgraph.passcount, sizeof(int), 1, fp );

	int i, numNpts, numPaths;
	NAVPOINT navpoint;
	PB_Path path;

	mapgraph_clear();		// clear all data
	mapgraph.nextid = 0;
	mapgraph.nextpathid = 0;

	fread( &numNpts, sizeof(int), 1, fp );

	for (i = 0; i < numNpts; i++) {
		//printf( "n" );
		navpoint_load(&navpoint, fp);
		mapgraph_addnavpoint(&navpoint);
	}

	fread( &numPaths, sizeof(int), 1, fp );

	for (int j = 0; j < numPaths; j++) {
		//printf( "p" );
		path.load( fp );
		if (path.id() > mapgraph.nextpathid)
			mapgraph.nextpathid = path.id();
		mapgraph_addpath( path, GRAPH_ONEWAY, false );
	}
	fclose( fp );
	mapgraph.nextpathid++;	// largest ID + 1
	mapgraph_initbackwardpaths();

	return true;
}

bool
mapgraph_save(const char *mapname)
{
	FILE *fp;
	int i, numNpts, numPaths;

	if ((fp = fopen(mapname, "wb")) == NULL) return false;

	// sort out unused paths
	int maxNotUsed = 40 * mapgraph_numberofnavpoints();	// maximum unused time after which 
	int notUsed;								// pathes are deleted
	for (i = 0; i < 1024/*graph.size()*/; i++) {
		AdjList::iterator adj = mapgraph.graph[i].second.begin();
		while (adj != mapgraph.graph[i].second.end()) {
			notUsed = mapgraph.passcount - adj->second.data.lastAttempt;
			if (notUsed > maxNotUsed) adj->second.markForDelete();
			adj++;
		}
	}
	mapgraph_preparebackwardpaths();

	fwrite( PNFidString, 32, 1, fp );

	// save number of passes
	fwrite( &mapgraph.passcount, sizeof(int), 1, fp );

	// save number of navpoints
	numNpts = mapgraph_numberofnavpoints();
	fwrite( &numNpts, sizeof(int), 1, fp );

	for (i = 0; i < numNpts; i++) {
		// save navpoint
		//printf( "N%i", navpoint_id(mapgraph.graph[i].first) );
		navpoint_save(&mapgraph.graph[i].first, fp);
	}

	numPaths = mapgraph_numberofpaths();
	fwrite( &numPaths, sizeof(int), 1, fp );
	//printf( "%i ", numPaths );

	for (i = 0; i < numNpts; i++) {
		// save each path
    	AdjList::iterator adj = mapgraph.graph[i].second.begin();
		while (adj != mapgraph.graph[i].second.end()) {
			PB_Path p = adj->second;
			p.save( fp );
			adj++;
		}
	}
	fclose( fp );

	return true;
}
