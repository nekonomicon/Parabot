#pragma once
#if !defined(PB_CELL_H)
#define PB_CELL_H

#include "pb_global.h"
#include "pb_navpoint.h"


#pragma warning( disable : 4786 )	// disable warnings


#define NO_CELL_REGISTERED -1
#define MAX_NBS		10


class PB_Cell
{

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

public:

	PB_Cell() {};
	PB_Cell( EDICT *pEdict );
	PB_Cell( FILE *fp );

	bool save( FILE *fp );

	Vec3D *pos()						{ return &data.position;		}
	short getNeighbour( int i )			{ return data.neighbour[i]; }
	float getWeight( int i )			{ return data.weight[i];	}
	int visits()						{ return data.visits;		}
	short nextCell()					{ return next;				}
	void setNextCell( short nextId )	{ next = nextId;			}
	bool setNeighbour( short nbId, float nbWeight );
	bool addTraffic( short nbId, float nbWeight );
	void addEnvDamage( float dmg );
	float getEnvDamage();
	bool isSuitableRoamingTarget( EDICT *traveller );
	PB_Navpoint* getNavpoint();
	short getTraffic( short nbId );
	bool delNeighbour( short nbId );
	short getGround()					{ return data.ground;		}

	// PB_Focus	focus;

// private:

	typedef struct {
		Vec3D	position;
		short	neighbour[MAX_NBS];	// ids of neighbouring cells
		float	weight[MAX_NBS];		// weight to neighbouring cells (for A*)
		short	traffic[MAX_NBS];	// count of observed moves to neighbours
		int		visits;			// count of overall visits
		int		flags;
		float	envDamage;
		short	navpoint;		// -1 if no navpoint in cell, else id of nearest nav
		short	ground;			// -1 if world, else id of ground-entity
		SECTOR  sectors[NUM_SECTORS];
		//byte	group;			// group membership
	} TSaveData;

	TSaveData	data;
	short	next;			// id of next cell in linked list (for hashtable)

};
#endif
