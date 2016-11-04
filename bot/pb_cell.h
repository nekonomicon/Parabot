#if !defined( PB_CELL_H )
#define PB_CELL_H


#include "pb_global.h"
#include "pb_focus.h"
#include "pb_kills.h"
#include "pb_navpoint.h"


#pragma warning( disable : 4786 )	// disable warnings


#define NO_CELL_REGISTERED -1
#define MAX_NBS		10


class PB_Cell
{

// flags:
#define CFL_UNDERWATER	(1<<0)	// cell is underwater
#define CFL_LOW_CEILING	(1<<1)	// player can only pass ducked

// group-memberships:
#define CG_ROOM		1
#define CG_HALLWAY	2
#define CG_YARD		3
#define CG_ALLEY	4
#define CG_ROOF		5


public:

	static Vector makePos( edict_t *pEdict ) {
		return (pEdict->v.origin + pEdict->v.view_ofs);
	}

	PB_Cell() {};
	PB_Cell( edict_t *pEdict );
	PB_Cell( FILE *fp );

	bool save( FILE *fp );

	Vector pos()						{ return data.position;		}
	short getNeighbour( int i )			{ return data.neighbour[i]; }
	float getWeight( int i )			{ return data.weight[i];	}
	int visits()						{ return data.visits;		}
	short nextCell()					{ return next;				}
	void setNextCell( short nextId )	{ next = nextId;			}
	bool setNeighbour( short nbId, float nbWeight );
	bool addTraffic( short nbId, float nbWeight );
	void addEnvDamage( float dmg );
	float getEnvDamage();
	bool isSuitableRoamingTarget( edict_t *traveller );
	PB_Navpoint* getNavpoint();
	short getTraffic( short nbId );
	bool delNeighbour( short nbId );
	short getGround()					{ return data.ground;		}

	PB_Focus	focus;
	PB_Kills	kills;


private:

	typedef struct {
		Vector	position;
		short	neighbour[MAX_NBS];	// ids of neighbouring cells
		float	weight[MAX_NBS];		// weight to neighbouring cells (for A*)
		short	traffic[MAX_NBS];	// count of observed moves to neighbours
		int		visits;			// count of overall visits
		int		flags;
		float	envDamage;
		short	navpoint;		// -1 if no navpoint in cell, else id of nearest nav
		short	ground;			// -1 if world, else id of ground-entity
		//byte	group;			// group membership
	} TSaveData;

	TSaveData	data;
	short	next;			// id of next cell in linked list (for hashtable)

};


#endif