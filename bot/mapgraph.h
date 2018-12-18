#pragma once
#if !defined( PB_MAPGRAPH_H )
#define PB_MAPGRAPH_H

#include <cassert>
#include <map>
#include <stack>
#include "pbt_dynarray.h"

#define GRAPH_ONEWAY		0
#define GRAPH_BIDIRECTIONAL	1

typedef std::multimap<int, PB_Path> AdjList;		// key is target navpoint
typedef std::pair<NAVPOINT, AdjList> Node;
typedef PBT_DynArray<Node> GraphType;

//typedef GraphType::iterator Iterator;
//typedef GraphType::const_iterator ConstIterator;
typedef AdjList::iterator AdjPtr;

typedef struct mapgraph {
	// GraphType	graph;				// container
	Node		graph[1024];
	int		nextid;				// Node ID
	int		nextpathid;			// Path ID
	int		passcount;			// increment for every path the bots pass
	bool		availableitems[MAX_NAV_TYPES];	// existing navpoints
} MAPGRAPH;

void		 mapgraph_clear();

void		 mapgraph_incpasses();
int		 mapgraph_getpasscount();

int		 mapgraph_numberofnavpoints();
int		 mapgraph_numberofpaths();

//Node& operator[](int i)  { return graph[i]; }	// access to node i
Node		*mapgraph_getnode(int i);

PB_Path		*mapgraph_findpath( int id );
// returns a pointer to path id or 0 if it doesn't exist

bool		 mapgraph_itemavailable(int itemCode);
// returns true if a navpoint of type itemCode exists in Graph

bool		 mapgraph_addnavpoint(NAVPOINT *navpoint);
// adds navpoint to graph

bool		 mapgraph_addpath(PB_Path &path, int directed, bool idInit = true);
// adds path to graph, if idInit is true path.id gets initialized

void		 mapgraph_addifimprovement(PB_Path &path, bool addReturn = true);
// checks if any of the directions is improvement to graph, any improvement is added
// deleting existing (slower) path
// if path is not used, all path data get deleted!

NAVPOINT	*mapgraph_getnearestnavpoint(const Vec3D *pos);
// returns the nearest navpoint to pos existing in the graph, NULL if graph is empty

NAVPOINT	*mapgraph_getnearestnavpoint(const Vec3D *pos, int type);
// returns the nearest navpoint with given type to pos existing in the graph, 
// NULL if graph doesn't contain navpoints of the given type
int mapgraph_linkednavpointsfrom( NAVPOINT *nav );
// returns the number of other nodes linked to nav in the graph

NAVPOINT	*mapgraph_getnearestroamingnavpoint( EDICT *traveller, NAVPOINT *ignore=0);
// returns a navpoint to approach in roaming mode, prefers linked navpoints and the ones
// that are z-reachable, nextVisit-Time is required to be reached
// should never return zero

bool		 mapgraph_getjourney(int start, int target, int mode, JOURNEY *journey);
// returns a journey from start to target or false if none available in mode

int		 mapgraph_getwishjourney(int start, NEEDS *needs, int mode, JOURNEY *journey, EDICT *traveller);
// returns the nav-id that according to wishList is the best to head for and the best
// journey to get there if possible in mode, if not returns -1 and an empty journey

float		 mapgraph_shortestjourney(int start, int target, int mode, std::deque<int> &path);
// returns in path the shortest journey from start to target (pathIds)

bool		 mapgraph_load(const char *filename);
// returns true if succesful, false if error occured
bool		 mapgraph_save(const char *filename);
#endif
