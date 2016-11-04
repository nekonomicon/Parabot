#include <float.h>
#include "pb_cell.h"
#include "pb_mapgraph.h"
#include "pb_mapcells.h"


extern PB_MapGraph mapGraph;

PB_Navpoint* getNearestNavpoint( edict_t *pEdict );



PB_Cell::PB_Cell( edict_t *pEdict ) 
{
	data.position = makePos( pEdict );
	data.visits = 0;
	data.flags = 0;
	data.envDamage = 0;
	data.navpoint = -1;
	data.ground = -1;
	for (int i=0; i<MAX_NBS; i++) {
		data.neighbour[i] = NO_CELL_REGISTERED;
		data.weight[i] = FLT_MAX;
		data.traffic[i] = 0;
	}
	next = NO_CELL_REGISTERED;

	// find navpoint:
	PB_Navpoint *np = getNearestNavpoint( pEdict );
	if ( np ) {
		if ( (np->pos() - data.position).Length() <= CELL_SIZE  &&
			 LOSExists( np->pos(), data.position )				&&
			 (np->pos().z - data.position.z) <= 45 				   ) 
			 data.navpoint = np->id();
	}
	// find ground:
	edict_t *ground = pEdict->v.groundentity;
	if (ground) {
		const char *groundName = STRING( ground->v.classname );
		if (strcmp( groundName, "worldspawn" ) != 0) {
			data.ground = getNavpointIndex( ground );
			if (data.ground < 0) data.ground = -1;
		}
	}	
}


PB_Cell::PB_Cell( FILE *fp )
{
	fread( &data, sizeof(TSaveData), 1, fp );
	focus.load( fp );
	kills.load( fp );
	next = NO_CELL_REGISTERED;
}


bool PB_Cell::save( FILE *fp )
{
	fwrite( &data, sizeof(TSaveData), 1, fp );
	focus.save( fp );
	kills.save( fp );
	return true;
}


bool PB_Cell::setNeighbour( short nbId, float nbWeight )
{
	for (int i=0; i<MAX_NBS; i++) {
		if ((data.neighbour[i] == NO_CELL_REGISTERED) || (data.neighbour[i] == nbId)) {
			data.neighbour[i] = nbId;
			data.weight[i] = nbWeight;
			return true;
		}
	}
	return false;
}


bool PB_Cell::addTraffic( short nbId, float nbWeight )
{
	data.visits++;	// delayed increment (player just leaving cell)

	for (int i=0; i<MAX_NBS; i++) {
		if (data.neighbour[i] == nbId) {
			if (data.traffic[i] < 32767) data.traffic[i]++;	// take care with short overflow
			if (nbWeight < 1.5) {	// don't let camping spoil the traffic!
				float wFactor = 1 / ((float)(data.traffic[i]+1));
				data.weight[i] = (1-wFactor)*data.weight[i] + wFactor*nbWeight;
			}
			//else debugMsg( "Player stopped\n" );
			return true;
		}
		else if (data.neighbour[i] == NO_CELL_REGISTERED) {
			data.traffic[i]++;
			data.neighbour[i] = nbId;
			data.weight[i] = nbWeight;
			return true;
		}
	}
	return false;
}


void PB_Cell::addEnvDamage( float dmg )
{
	data.envDamage += dmg;
}


float PB_Cell::getEnvDamage() 
{ 
	if (data.visits == 0) return 0;
	else return ( data.envDamage / ((float)data.visits) ); 
}


bool PB_Cell::isSuitableRoamingTarget( edict_t *traveller )
{ 
	if (data.navpoint < 0) return false;
	if ( worldTime() >= mapGraph[data.navpoint].first.nextVisit( traveller ) ) return true;
	return false;
}


PB_Navpoint* PB_Cell::getNavpoint()
{
	if ( data.navpoint < 0 ) return 0;
	return &(mapGraph[data.navpoint].first);
}


short PB_Cell::getTraffic( short nbId )
{
	for (int i=0; i<MAX_NBS; i++) 
		if (data.neighbour[i] == nbId) return data.traffic[i];
	return -1;
}


bool PB_Cell::delNeighbour( short nbId )
{
	for (int i=0; i<MAX_NBS; i++) 
		if (data.neighbour[i] == nbId) {
			for (int j=i; j<(MAX_NBS-1); j++) {
				data.neighbour[j] = data.neighbour[j+1];
				data.weight[j] = data.weight[j+1];
				data.traffic[j] = data.traffic[j+1];
			}
			data.neighbour[MAX_NBS-1] = NO_CELL_REGISTERED;
			data.weight[MAX_NBS-1] = FLT_MAX;
			data.traffic[MAX_NBS-1] = 0;
			return true;
		}
	return false;
}
