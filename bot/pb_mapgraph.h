#if !defined( PB_MAPGRAPH_H )
#define PB_MAPGRAPH_H

#pragma warning( disable : 4786 )	// disable warnings

#include <cassert>
#include <map>
#include <stack>
#include "checkvec.h"
#include "pbt_dynarray.cpp"
#include "PB_Journey.h"
class PB_Needs;



#define GRAPH_ONEWAY		0
#define GRAPH_BIDIRECTIONAL 1

	typedef std::multimap<int, PB_Path> AdjList;		// key is target navpoint
	typedef std::pair<PB_Navpoint, AdjList> Node;
    typedef PBT_DynArray<Node> GraphType;

    //typedef GraphType::iterator Iterator;
    //typedef GraphType::const_iterator ConstIterator;
	typedef AdjList::iterator AdjPtr;

class PB_MapGraph
{

public:

	

	PB_MapGraph();
	~PB_MapGraph();
	void clear();

	void incPasses() { passCount++; }
	int	 getPassCount() { return passCount; }
	
	int numberOfNavpoints() const { return graph.size(); }
	int numberOfPaths();

    Node& operator[](int i)  { return graph[i]; }	// access to node i
	
	PB_Path* findPath( int id );
	// returns a pointer to path id or 0 if it doesn't exist
		
	bool itemAvailable( int itemCode ) { return availableItems[itemCode]; }
	// returns true if a navpoint of type itemCode exists in Graph

	bool addNavpoint( PB_Navpoint &navpoint );
	// adds navpoint to graph

    bool addPath( PB_Path &path, int directed, bool idInit = true );
	// adds path to graph, if idInit is true path.id gets initialized

	void addIfImprovement( PB_Path &path, bool addReturn = true );
	// checks if any of the directions is improvement to graph, any improvement is added
	// deleting existing (slower) path
	// if path is not used, all path data get deleted!
	
	PB_Navpoint* getNearestNavpoint( Vector &pos );
	// returns the nearest navpoint to pos existing in the graph, NULL if graph is empty
	
	PB_Navpoint* getNearestNavpoint( Vector &pos, int type );
	// returns the nearest navpoint with given type to pos existing in the graph, 
	// NULL if graph doesn't contain navpoints of the given type

	int linkedNavpointsFrom( PB_Navpoint *nav );
	// returns the number of other nodes linked to nav in the graph

	PB_Navpoint* getNearestRoamingNavpoint( edict_t *traveller, PB_Navpoint *ignore=0 );
	// returns a navpoint to approach in roaming mode, prefers linked navpoints and the ones
	// that are z-reachable, nextVisit-Time is required to be reached
	// should never return zero

	bool getJourney( int start, int target, int mode, PB_Journey &journey );
	// returns a journey from start to target or false if none available in mode

	int getWishJourney( int start, PB_Needs &needs, int mode, PB_Journey &journey, edict_t *traveller );
	// returns the nav-id that according to wishList is the best to head for and the best
	// journey to get there if possible in mode, if not returns -1 and an empty journey

	float shortestJourney( int start, int target, int mode, std::deque<int> &path );
	// returns in path the shortest journey from start to target (pathIds)

	bool load( char *filename );
	// returns true if succesful, false if error occured
	bool save( char *filename );



private:

	GraphType graph;    // container
	int nextId;			// Node ID
	int nextPathId;		// Path ID
	int passCount;		// increment for every path the bots pass
	bool availableItems[MAX_NAV_TYPES];	// existing navpoints


	void initBackwardPaths();
	// called after load(), initializes data structures of backward paths

	void prepareBackwardPaths();
	// called before save(), copies data structures of deleted paths to backward paths

	AdjPtr findPath( int id, int startId, bool &found  );
	// returns (if found=true) iterator to a path with given id

	AdjPtr findLinkedPath( int dataId, int startId, bool &found );
	// returns (if found=true) iterator to a path with given dataId

	void Dijkstra( std::vector<float> &dist, std::vector<int> &path, 
				   int start, int searchMode );
	// classic Dijkstra algo

	int DijkstraToWish( std::vector<float>& dist, std::vector<int>& path, 
						    int start, PB_Needs &needs, int searchMode, edict_t *traveller );

	
};

#endif