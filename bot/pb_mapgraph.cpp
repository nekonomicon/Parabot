#pragma warning( disable : 4786 )	// disable 29 graph warnings

#include "PB_MapGraph.h"
#include "dynpq.h"
#include "pb_global.h"
#include "pb_needs.h"


void invalidateMapGraphCash();

// id used in waypoint files
char *PNFidString = "Parabot Waypoint File 0.8      ";



PB_MapGraph::PB_MapGraph() : 
	graph( 1024, 64 )
{
	nextId = 0;
	nextPathId = 0;	// Path ID
	passCount = 0;
	for (int i=0; i<MAX_NAV_TYPES; i++) availableItems[i] = false;
}


PB_MapGraph::~PB_MapGraph()
{
	clear();
}


void PB_MapGraph::clear()
// delete all data
{
	int numPaths;

	for (int i=0; i<graph.size(); i++) {
		numPaths = graph[i].second.size();
		
		// clear each path that has own data
    	AdjList::iterator adj = graph[i].second.begin();
		while (adj != graph[i].second.end()) {
			if (adj->second.hasData()) adj->second.clear();
			adj++;
		}
		graph[i].second.clear();
	}
	graph.clear();
	nextId = 0;
	nextPathId = 0;
	invalidateMapGraphCash();
}


int PB_MapGraph::numberOfPaths()
// returns the number of paths
{
	int count = 0;
    for (int i=0; i<graph.size(); i++) {
		// count each path that is not deleted
    	AdjList::iterator adj = graph[i].second.begin();
		while (adj != graph[i].second.end()) {
			if (!adj->second.deleted()) count++;
			adj++;
		}
	}
    return count;
}


AdjPtr PB_MapGraph::findPath( int id, int startId, bool &found )
// find path with id beginning at navpoint startId
{
	found = false;
//  vielleicht schneller:
//	GraphType::Successor::iterator si =graph[startId].second.find(...);
//	(*si).second.init( 0, 0 );

	AdjPtr adj = graph[startId].second.begin();

	while (adj != graph[startId].second.end() && !found) {
		if ((*adj).second.id()==id) {
			found = true;
			return adj;
		}
		adj++;
	}
	return adj;
}


PB_Path* PB_MapGraph::findPath( int id )
// find path with id
{
	bool found = false;
	int  start = 0;
	AdjPtr adj;

	while ( !found && start<numberOfNavpoints() ) {
		adj = findPath( id, start, found );
		start++;
	}
	if (found) return &(adj->second);
	else return 0;
}


AdjPtr PB_MapGraph::findLinkedPath( int dataId, int startId, bool &found )
// find path with id beginning at navpoint startId
{
	found = false;
	AdjPtr adj = graph[startId].second.begin();

	while (adj != graph[startId].second.end() && !found) {
		if (adj->second.dataId()==dataId) {
			found = true;
			return adj;
		}
		adj++;
	}
	return adj;
}


PB_Navpoint* PB_MapGraph::getNearestNavpoint( Vector &pos )
// returns the nearest navpoint to pos existing in the graph, NULL if graph is empty
{
	float dx, dy, dz, dist, minDist = 1000000000000;
	int   minId = -1;

	for (int i=0; i<numberOfNavpoints(); i++) {
		dx = pos.x - graph[i].first.pos().x;
		dy = pos.y - graph[i].first.pos().y;
		dz = pos.z - graph[i].first.pos().z;
		dist = dx*dx + dy*dy + dz*dz;
		if (dist<minDist) {
			minDist = dist;
			minId = i;
		}
	}
	if (minId>=0) return (PB_Navpoint*) &(graph[minId].first);
	else return 0;
}


PB_Navpoint* PB_MapGraph::getNearestNavpoint( Vector &pos, int type )
// returns the nearest navpoint with given type to pos existing in the graph, 
// NULL if graph doesn't contain navpoints of the given type
{
	float dx, dy, dz, dist, minDist = 1000000000000;
	int   minId = -1;

	for (int i=0; i<numberOfNavpoints(); i++) if (graph[i].first.type()==type) {
		dx = pos.x - graph[i].first.pos().x;
		dy = pos.y - graph[i].first.pos().y;
		dz = pos.z - graph[i].first.pos().z;
		dist = dx*dx + dy*dy + dz*dz;
		if (dist<minDist) {
			minDist = dist;
			minId = i;
		}
	}
	if (minId>=0) return (PB_Navpoint*) &(graph[minId].first);
	else return 0;
}


int PB_MapGraph::linkedNavpointsFrom( PB_Navpoint *nav )
// returns the number of other nodes linked to nav in the graph
{
	if (!nav) return 0;
	else { 
		int count = 0;
		AdjPtr adj = graph[nav->id()].second.begin();
		while (adj != graph[nav->id()].second.end()) {
			if ( !(adj->second.deleted()) ) count++;	// only count valid paths
			adj++;
		}
		return count;
	}
}


PB_Navpoint* PB_MapGraph::getNearestRoamingNavpoint( edict_t *traveller, PB_Navpoint *ignore )
// returns a navpoint to approach in roaming mode, prefers linked navpoints and the ones
// that are z-reachable, nextVisit-Time is required to be reached
// should never return zero
{
	float dx, dy, dz, dist, minDist = 1000000000000, minDistR = 1000000000000;
	int   minId = -1, minIdR = -1;
	int	  ignoreId = -1;
	Vector &pos = traveller->v.origin;

	if (ignore) ignoreId = ignore->id();
	
	for (int i=0; i<numberOfNavpoints(); i++) 
	if ( ( i != ignoreId ) && ( worldTime() >= graph[i].first.nextVisit( traveller ) ) ) {
		dx = pos.x - graph[i].first.pos().x;
		dy = pos.y - graph[i].first.pos().y;
		dz = pos.z - graph[i].first.pos().z;
		dist = dx*dx + dy*dy + dz*dz;
		if ( graph[i].second.size() > 0 ) dist/=2;	// prefer linked navpoints
		if (dist<minDist) {
			if (UTIL_PointContents( graph[i].first.pos() )==CONTENTS_WATER) continue; 
			minDist = dist;
			minId = i;
		}
		if ( (graph[i].first.pos().z<(pos.z+45)) && (graph[i].first.pos().z>(pos.z-50))	) {
			// this one is roaming reachable! 
			if (dist<minDistR) {
				if (UTIL_PointContents( graph[i].first.pos() )==CONTENTS_WATER) continue; 
				minDistR = dist;
				minIdR = i;
			}
		}
	}
	if (minIdR>=0) return (PB_Navpoint*) &(graph[minIdR].first);
	else if (minId>=0) return (PB_Navpoint*) &(graph[minId].first);
	else return (PB_Navpoint*) &(graph[0].first);	// fuck it, we *HAVE* to return something!
}


bool PB_MapGraph::addNavpoint( PB_Navpoint &navpoint )
// add a new navpoint to the graph
{
	navpoint.setId( nextId++ );
	navpoint.initEntityPtr();
	int gs = graph.size();
	Node x = Node(navpoint, AdjList());
    graph.push_back( x );
	availableItems[navpoint.type()] = true;
	return true;
}


bool PB_MapGraph::addPath( PB_Path &path, int directed, bool idInit )
// add a new path to the graph
{
	if (idInit) {
		path.setId( nextPathId++ );
		if (path.dataId()<0) path.makeIndependant();
	}
	(graph[path.startId()].second).insert( std::make_pair(path.endId(), path) );

	if (directed==GRAPH_BIDIRECTIONAL) {
		PB_Path rp;
		rp.initReturnOf( path );
		if (idInit) rp.setId( nextId++ );
		(graph[rp.startId()].second).insert( std::make_pair(rp.endId(), rp) );
	}
	return true;
}


void PB_MapGraph::addIfImprovement( PB_Path &path, bool addReturn )
// checks if any of the directions is improvement to graph, any improvement is added
// deleting existing (slower) path
// if path is not used, all path data get deleted!
{
	char *cname = path.endNav().classname();
	bool used = false;
	std::deque<int> journey;
	float oldWeight = shortestJourney( path.startId(), path.endId(), path.mode(), journey ) - 0.1;

	if (path.weight() < oldWeight) {
		addPath( path, GRAPH_ONEWAY );
		used = true;
/*		debugMsg( "Added path %i from ", path.id() );
		path.startNav().print();
		debugMsg( " to " );
		path.endNav().print();*/
	}
/*	else {
		debugMsg( "Discarded path to " );
		debugMsg( cname );
		debugMsg( "\n" );
	}*/
	if (addReturn) {
		PB_Path rp;
		rp.initReturnOf( path );
		//journey.clear();	now in function
		oldWeight = shortestJourney( rp.startId(), rp.endId(), rp.mode(), journey ) - 0.1;
		
		//if (rp.weight() < oldWeight) {		// insert return as well?
		
		if (journey.size()==0) {	// only insert return if no alternative path
			addPath( rp, GRAPH_ONEWAY );	// don't delete anything: return is uncertain...
			used = true;
			//debugMsg( ", added return path %i", rp.id() );
		} 
	}
	//debugMsg( "\n" );
	if (!used) path.clear();
}

/*
bool PB_MapGraph::deletePath( int id, int startId )
{
	bool found;
	AdjPtr adj = findPath( id, startId, found );

	if (!found) return false;	
	if (adj->second.hasData()) {					// look if other path shares data...
		AdjPtr back = findLinkedPath( id, adj->second.endId(), found );
		if (found) {
			back->second.makeIndependant();						// ...yes -> swap sharing!
			adj->second.makeDependantOf( back->second.id() );	// only 1 reference to data!
		}
	}
	adj->second.markForDelete();	// don't delete instantly! (bots may be using it)
	return true;
}
*/

void PB_MapGraph::Dijkstra( std::vector<float>& dist, std::vector<int>& path, 
						    int start, int searchMode )
{
    // init distances
	dist = std::vector<float>(graph.size(), MAX_PATH_WEIGHT);
    dist[start] = 0;

    // init paths
    path = std::vector<int>(graph.size(), -1);

	// init priority queue
    dynamic_priority_queue<float> queue(dist);

    /* In the next step, all vertices are extracted  one by one from
       the priority queue, and precisely in the order of the estimated
       distance towards the starting vertex. Obviously, the starting
       vertex is dealt with first. No vertex is looked at twice. */

    int actualVertex;
    while(!queue.empty())
    {
       actualVertex = queue.topIndex();   // extract vertex with minimum
       queue.pop();
          
        // improve estimates for all neighbors of u
	   AdjList::iterator          
		   I = graph[actualVertex].second.begin();		// I cycles through paths

        while(I != graph[actualVertex].second.end())
        {
			PB_Path p = (*I).second;
			if ( p.valid(searchMode) ) {
				int Neighbor = (*I).first;
				float d = p.weight();

				// Relaxation
				if (dist[Neighbor] > (dist[actualVertex] + d))
				{
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


float PB_MapGraph::shortestJourney( int start, int target, int mode, std::deque<int> &journey )
{
	std::vector<float> dist;
    std::vector<int> lastPath;
	PB_Path *path;
    
	if (!journey.empty()) journey.clear();	// delete journey 
    Dijkstra( dist, lastPath, start, mode );
	float d = dist[target];

	while (lastPath[target]>=0) {
		journey.push_back( lastPath[target] );
		path = findPath( lastPath[target] );
		if (path) target = path->startId();
		else break;
	}
	return d;
}


bool PB_MapGraph::getJourney( int start, int target, int mode, PB_Journey &journey )
// returns a journey from start to target or false if none available in mode
{
	//journey.pathList.clear();	now in function
	shortestJourney( start, target, mode, journey.pathList );
	if (journey.pathList.empty()) return false;
	else return true;
}


int PB_MapGraph::DijkstraToWish( std::vector<float>& dist, std::vector<int>& path, 
						    int start, PB_Needs &needs, int searchMode, edict_t *traveller )
{
    // init distances
	dist = std::vector<float>(graph.size(), MAX_PATH_WEIGHT);
    dist[start] = 0;
	// init scores
	std::vector<float> score = std::vector<float>(graph.size(), -1);
	score[start] = 0;
    // init paths
    path = std::vector<int>(graph.size(), -1);
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
		AdjList::iterator adj = graph[actualVertex].second.begin();	
		// I cycles through paths
		
		while(adj != graph[actualVertex].second.end())
        {
			PB_Path p = adj->second;
			if ( p.valid(searchMode) ) {
				int neighbor = adj->first;
				float d = p.weight();
				
				// Relaxation
				if (dist[neighbor] > (dist[actualVertex] + d))
				{
					// improve estimate
					queue.changeKeyAt(neighbor, dist[actualVertex] + d);
					// actualVertex is predecessor of the neighbor
					path[neighbor] = p.id();
					if ( graph[neighbor].first.nextVisit( traveller ) < worldTime() ) {
						navWeight += 0.1;
						score[neighbor] = score[actualVertex] + 
											( needs.desireFor( graph[neighbor].first.type() )
											  / navWeight );
					}
				}
            }
            adj++;
        }
    }
	if ( targetFound ) {	// in case of cut
		queue.clear();	
	}
	else {
		targetNav = ( std::max_element( score.begin(), score.end() ) - score.begin() );
	}
	return targetNav;
}


int PB_MapGraph::getWishJourney( int start, PB_Needs &needs, int mode, PB_Journey &journey, edict_t *traveller )
// returns the nav-id that according to wishList is the best to head for and the best
// journey to get there if possible in mode, if not returns -1 and an empty journey
{
	journey.pathList.clear();

	std::vector<float> dist;
    std::vector<int> lastPath;
	PB_Path *path;
    
    int bestTarget = DijkstraToWish( dist, lastPath, start, needs, mode, traveller );
	if (bestTarget==start) return -1;

	int target = bestTarget;
	while (lastPath[target]>=0) {
		journey.pathList.push_back( lastPath[target] );
		path = findPath( lastPath[target] );
		if (path) target = path->startId();
		else break;
	}
	if (journey.pathList.empty()) return -1;
	else return bestTarget;
}


void PB_MapGraph::initBackwardPaths()
// called after load(), initializes data structures of backward paths
{
	int numPaths;
	PB_Path p;
	AdjPtr rp;
	bool ok;

	for (int i=0; i<numberOfNavpoints(); i++) {
		numPaths = graph[i].second.size();
		
    	AdjList::iterator adj = graph[i].second.begin();
		while (adj != graph[i].second.end()) {
			p = adj->second;
			if (!p.hasData()) {
				rp = findPath( p.dataId(), p.endId(), ok );	// return path starts at end
				if (ok) {
					adj->second.waypoint = rp->second.waypoint;
					adj->second.hiddenAttack = rp->second.hiddenAttack;
					adj->second.platformPos = rp->second.platformPos;
				}
				else {
					debugMsg( "FATAL ERROR: Return path %i could not be initialized!\n", p.id() );
				}
			}
			adj++;
		}
	}
}


void PB_MapGraph::prepareBackwardPaths()
// called before save(), copies data structures of deleted paths to backward paths
{
	AdjPtr	adj, back;
	bool	found;

	for ( int startId=0; startId < numberOfNavpoints(); startId++ ) {
		adj = graph[startId].second.begin();
		while (adj != graph[startId].second.end()) {
			if ( (adj->second.deleted()) && (adj->second.hasData()) ) {	// if path deleted
				back = findLinkedPath( adj->second.id(), adj->second.endId(), found );
				if (found) {
					back->second.makeIndependant();						// ...yes -> swap sharing!
					adj->second.makeDependantOf( back->second.id() );	// only 1 reference to data!
				}
			}
			adj++;
		}
	}
}


bool PB_MapGraph::load( char *mapname )
{
	FILE *fp;
	char filename[256] = "parabot/navigationfiles/";
	strcat( filename, mapname );

	if ( (fp = fopen( filename, "rb" )) == NULL ) return false;

	char idString[32];	// check for correct version
	fread( &idString, 32, 1, fp );
	if (strcmp( idString, PNFidString) !=0 ) {
		fclose( fp );
		return false;
	}

	fread( &passCount, sizeof(int), 1, fp );

	int i, numNpts, numPaths;
	PB_Navpoint navpoint;
	PB_Path path;

	clear();		// clear all data
	nextId = 0;
	nextPathId = 0;

	fread( &numNpts, sizeof(int), 1, fp );
						
	for (i=0; i<numNpts; i++) {
		//printf( "n" );
		navpoint.load( fp );
		addNavpoint( navpoint );
	}
	
	fread( &numPaths, sizeof(int), 1, fp );

	for (int j=0; j<numPaths; j++) {
		//printf( "p" );
		path.load( fp );
		if (path.id() > nextPathId) nextPathId = path.id();
		addPath( path, GRAPH_ONEWAY, false );
	}
	fclose( fp );
	nextPathId++;	// largest ID + 1
	initBackwardPaths();
	return true;
}


bool PB_MapGraph::save( char *mapname )
{
	FILE *fp;
	int i, numNpts, numPaths;
	char filename[256] = "parabot/navigationfiles/";
	strcat( filename, mapname );

	if ( (fp = fopen( filename, "wb" )) == NULL ) return false;

	// sort out unused paths
	int maxNotUsed = 40 * numberOfNavpoints();	// maximum unused time after which 
	int notUsed;								// pathes are deleted
	for (i=0; i<graph.size(); i++) {
		AdjList::iterator adj = graph[i].second.begin();
		while (adj != graph[i].second.end()) {
			notUsed = passCount - adj->second.data.lastAttempt;
			if (notUsed > maxNotUsed) adj->second.markForDelete();
			adj++;
		}
	}
	prepareBackwardPaths();

	fwrite( PNFidString, 32, 1, fp );

	// save number of passes
	fwrite( &passCount, sizeof(int), 1, fp );

		//save number of navpoints
	numNpts = numberOfNavpoints();
	fwrite( &numNpts, sizeof(int), 1, fp );

	for (i=0; i<numNpts; i++) {
		// save navpoint
		//printf( "N%i", graph[i].first.id() );
		graph[i].first.save( fp );
	}

	numPaths = numberOfPaths();
	fwrite( &numPaths, sizeof(int), 1, fp );
	//printf( "%i ", numPaths );

	for (i=0; i<numNpts; i++) {
		// save each path
    	AdjList::iterator adj = graph[i].second.begin();
		while (adj != graph[i].second.end()) {
			PB_Path p = adj->second;
			p.save( fp );
			adj++;
		}
	}
	fclose( fp );
	return true;
}


