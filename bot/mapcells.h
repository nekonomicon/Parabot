#pragma once
#if !defined( PB_MapCells_H )
#define PB_MapCells_H

#include "pbt_priorityqueue.h"
#include "pbt_dynarray.h"

#define MAX_CELLS 8192
#define CELL_SIZE 100		// waypoint size (sphere diameter)
#define NO_CELL_FOUND -1	// returned by getCellId() when no cell is found

typedef struct foundcell {	
	float dist;
	short index;
} FOUNDCELL;

typedef struct mapcells {
	//PBT_DynArray<CELL>	cellArray;
	CELL			cellArray[MAX_CELLS];
	short			cellHash[4096];
	int			numCells;
	FOUNDCELL		cellFound[256];	// holds all WPs found by getCellId()
	int			numCellsFound;		// number of elements in cellSearch
	// int			lastCellFound;		// last passed element
	VISTABLE		vis;
} MAPCELLS;

CELL	*mapcells_getcell(int index);

int	 mapcells_getcellid(const Vec3D *pos, float maxDist=CELL_SIZE);
int	 mapcells_getcellid(EDICT *pEdict);

// returns index of newCell:
int	 mapcells_addcell(CELL newCell, bool initNbs, int addedFrom=NO_CELL_FOUND);
// returns number of found neighbours:
int	 mapcells_initneighbours(int cellIndex, int firstNb);
int	 mapcells_numberofcells();

bool	 mapcells_lineofsight(int cell1, int cell2);
int	 mapcells_updatevisibility(int maxUpdates);
int	 mapcells_lastvisupdate();

// searches a path from startId to targetId, returns pathlength or -1
int	 mapcells_getpath(short startId, short targetId, short pathNodes[]);
int	 mapcells_getpathtocover(short startId, short enemyId, short pathNodes[]);
int	 mapcells_getpathforsneakyescape(short startId, short enemyId, short pathNodes[]);
int	 mapcells_getpathtoattack(short startId, short enemyId, short pathNodes[]);
int	 mapcells_getoffensivepath(short startId, short enemyId, float minDist, short pathNodes[]);
int	 mapcells_getdirectedpathtoattack(short startId, short enemyId, Vec3D *dir, short pathNodes[]);
int	 mapcells_getpathtoroamingtarget(short startId, EDICT *botEnt, short pathNodes[]);
int	 mapcells_predictplayerpos(short startId, short ownId, short pathNodes[]);
// void mapcells_checkbucket(int hcode, const Vec3D *pos, float *closestDist, int *closestid, float maxdist);

bool	 mapcells_load(const char *mapname);
bool	 mapcells_save(const char *mapname);
void	 mapcells_clear();

#endif
