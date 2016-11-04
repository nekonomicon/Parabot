#if !defined( PB_MapCells_H )
#define PB_MapCells_H

#include "pb_cell.h"
#include "pb_vistable.h"
#include "pbt_priorityqueue.h"
#include "pbt_dynarray.cpp"


class PBT_FoundCell
{
public:
	PBT_FoundCell() {}
	PBT_FoundCell( float d, short i ) {  dist=d; index=i;  }
	operator==(const PBT_FoundCell& O) const {  return dist == O.dist; }
	operator<(const PBT_FoundCell& O) const {  return dist < O.dist; }
	
	float dist;
	short index;
};


class PB_MapCells 
{

#define MAX_CELLS 8192
#define CELL_SIZE 100		// waypoint size (sphere diameter)
#define NO_CELL_FOUND -1	// returned by getCellId() when no cell is found


public:

	PB_MapCells();
	~PB_MapCells();
	
	PB_Cell& cell( int index ) {  return cellArray[index];  }

	int getCellId( Vector &pos, float maxDist=CELL_SIZE );
	int getCellId( edict_t *pEdict ) {  return getCellId( PB_Cell::makePos( pEdict ) );  }
	
	// returns index of newCell:
	int addCell( PB_Cell newCell, bool initNbs, int addedFrom=NO_CELL_FOUND );	
	// returns number of found neighbours:
	int initNeighbours( int cellIndex, int firstNb  );
	int numberOfCells() { return numCells; }

	bool lineOfSight( int cell1, int cell2 ) {  return vis.isVisible( cell1, cell2 );  }
	int updateVisibility( int maxUpdates );
	int lastVisUpdate();

	// searches a path from startId to targetId, returns pathlength or -1
	int getPath( short startId, short targetId, short pathNodes[] );
	int getPathToCover( short startId, short enemyId, short pathNodes[] );
	int getPathForSneakyEscape( short startId, short enemyId, short pathNodes[] );
	int getPathToAttack( short startId, short enemyId, short pathNodes[] );
	int getOffensivePath( short startId, short enemyId, float minDist, short pathNodes[] );
	int getDirectedPathToAttack( short startId, short enemyId, Vector dir, short pathNodes[] );
	int getPathToRoamingTarget( short startId, edict_t *botEnt, short pathNodes[] );
	int predictPlayerPos( short startId, short ownId, short pathNodes[] );
		
	bool load( char *mapname );
	bool save( char *mapname );
	void clear();



private:

	PBT_DynArray<PB_Cell> cellArray;

	short		cellHash[4096];
	int			numCells;
		
	PBT_FoundCell	cellFound[256];	// holds all WPs found by getCellId()
	int			numCellsFound;		// number of elements in cellSearch
	int			lastCellFound;		// last passed element

	PB_VisTable vis;

	int getHashcode( Vector &pos );
	Vector getAllignedPos( Vector pos );

};


#endif