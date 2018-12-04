#pragma once
#if !defined(PB_CELL_H)
#define PB_CELL_H

#include "pb_global.h"
#include "pb_navpoint.h"

#define NO_CELL_REGISTERED -1
#define MAX_NBS		10

enum { // flags:
	CFL_UNDERWATER = BIT(0),	// cell is underwater
	CFL_LOW_CEILING	= BIT(1)	// player can only pass ducked
};

enum { // group-memberships:
	CG_ROOM = 1,
	CG_HALLWAY,
	CG_YARD,
	CG_ALLEY,
	CG_ROOF
};

typedef struct cell_savedata {
	Vec3D	position;
	short	neighbour[MAX_NBS];	// ids of neighbouring cells
	float	weight[MAX_NBS];	// weight to neighbouring cells (for A*)
	short	traffic[MAX_NBS];	// count of observed moves to neighbours
	int	visits;			// count of overall visits
	int	flags;
	float	envdamage;
	short	navpoint;		// -1 if no navpoint in cell, else id of nearest nav
	short	ground;			// -1 if world, else id of ground-entity
	SECTOR  sectors[NUM_SECTORS];
	//byte	group;			// group membership
} CELL_SAVEDATA;

typedef struct cell {
	CELL_SAVEDATA	data;
	short		next;			// id of next cell in linked list (for hashtable)
} CELL;

void		 cell_construct(CELL *cell, EDICT *pEdict);
void		 cell_load(CELL *cell, FILE *fp);
bool		 cell_save(CELL *cell, FILE *fp);
void		 cell_pos(CELL *cell, Vec3D *pos);
Vec3D		*cell_pos2(CELL *cell);
short		 cell_getneighbour(CELL *cell, int i);
float		 cell_getweight(CELL *cell, int i);
// int		 cell_visits(CELL *cell);
short		 cell_nextcell(CELL *cell);
void		 cell_setnextcell(CELL *cell, short nextId);
bool		 cell_setneighbour(CELL *cell, short nbId, float nbWeight);
bool		 cell_addtraffic(CELL *cell, short nbId, float nbWeight);
void		 cell_addenvdamage(CELL *cell, float dmg);
float		 cell_getenvdamage(CELL *cell);
bool		 cell_issuitableroamingtarget(CELL *cell, EDICT *traveller);
PB_Navpoint	*cell_getnavpoint(CELL *cell);
short		 cell_gettraffic(CELL *cell, short nbId);
bool		 cell_delneighbour(CELL *cell, short nbId);
short		 cell_getground(CELL *cell);

#endif
