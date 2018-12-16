#include <float.h>
#include "parabot.h"
#include "sectors.h"
#include "focus.h"
#include "kills.h"
#include "cell.h"
#include "vistable.h"
#include "pb_mapgraph.h"
#include "pb_mapcells.h"

extern PB_MapGraph mapGraph;

NAVPOINT *getNearestNavpoint(EDICT *pEdict);

void
cell_construct(CELL *cell, EDICT *e) 
{
	eyepos(e, &cell->data.position);
	cell->data.visits = 0;
	cell->data.flags = 0;
	cell->data.envdamage = 0;
	cell->data.navpoint = -1;
	cell->data.ground = -1;
	for (int i = 0; i < MAX_NBS; i++) {
		cell->data.neighbour[i] = NO_CELL_REGISTERED;
		cell->data.weight[i] = FLT_MAX;
		cell->data.traffic[i] = 0;
	}
	cell->next = NO_CELL_REGISTERED;

	// find navpoint:
	NAVPOINT *np = getNearestNavpoint(e);
	if ( np ) {
		Vec3D dir;

		vsub(navpoint_pos(np), &cell->data.position, &dir);
		if (vlen(&dir) <= CELL_SIZE
		    && LOSExists(navpoint_pos(np), &cell->data.position)
		    && (navpoint_pos(np)->z - cell->data.position.z) <= 45) 
			cell->data.navpoint = navpoint_id(np);
	}
	// find ground:
	EDICT *ground = e->v.groundentity;
	if (ground) {
		const char *groundName = STRING( ground->v.classname );
		if ( !Q_STREQ( groundName, "worldspawn" ) ) {
			cell->data.ground = getNavpointIndex( ground );
			if (cell->data.ground < 0)
				cell->data.ground = -1;
		}
	}
}

void
cell_load(CELL *cell, FILE *fp)
{
	fread( &cell->data, sizeof(CELL_SAVEDATA), 1, fp );
	cell->next = NO_CELL_REGISTERED;
}

bool
cell_save(CELL *cell, FILE *fp)
{
	fwrite( &cell->data, sizeof(CELL_SAVEDATA), 1, fp );
	return true;
}

void
cell_pos(CELL *cell, Vec3D *pos)
{
	vcopy(&cell->data.position, pos);
}

Vec3D *
cell_pos2(CELL *cell)
{
	return &cell->data.position;
}

short
cell_getneighbour(CELL *cell, int i)
{
	return cell->data.neighbour[i];
}

float
cell_getweight(CELL *cell, int i)
{
	return cell->data.weight[i];
}

static int
cell_visits(CELL *cell)
{
	return cell->data.visits;
}

short
cell_nextcell(CELL *cell)
{
	return cell->next;
}

void
cell_setnextcell(CELL *cell, short nextId)
{
	cell->next = nextId;
}

bool
cell_setneighbour(CELL *cell, short nbId, float nbWeight )
{
	for (int i = 0; i < MAX_NBS; i++) {
		if ((cell->data.neighbour[i] == NO_CELL_REGISTERED) || (cell->data.neighbour[i] == nbId)) {
			cell->data.neighbour[i] = nbId;
			cell->data.weight[i] = nbWeight;
			return true;
		}
	}
	return false;
}

bool
cell_addtraffic(CELL *cell, short nbId, float nbWeight)
{
	cell->data.visits++;	// delayed increment (player just leaving cell)

	for (int i = 0; i < MAX_NBS; i++) {
		if (cell->data.neighbour[i] == nbId) {
			if (cell->data.traffic[i] < 32767) cell->data.traffic[i]++;	// take care with short overflow
			if (nbWeight < 1.5) {	// don't let camping spoil the traffic!
				float wFactor = 1 / ((float)(cell->data.traffic[i] + 1));
				cell->data.weight[i] = (1 - wFactor) * cell->data.weight[i] + wFactor * nbWeight;
			}
			//else DEBUG_MSG( "Player stopped\n" );
			return true;
		} else if (cell->data.neighbour[i] == NO_CELL_REGISTERED) {
			cell->data.traffic[i]++;
			cell->data.neighbour[i] = nbId;
			cell->data.weight[i] = nbWeight;
			return true;
		}
	}
	return false;
}

void
cell_addenvdamage(CELL *cell, float dmg)
{
	cell->data.envdamage += dmg;
}

float
cell_getenvdamage(CELL *cell)
{
	if (cell->data.visits == 0)
		return 0;

	return (cell->data.envdamage / ((float)cell->data.visits));
}

bool
cell_issuitableroamingtarget(CELL *cell, EDICT *traveller)
{ 
	if (cell->data.navpoint < 0)
		return false;

	if (worldtime() >= navpoint_nextvisit(&mapGraph[cell->data.navpoint].first, traveller))
		return true;

	return false;
}

NAVPOINT *
cell_getnavpoint(CELL *cell)
{
	if (cell->data.navpoint < 0)
		return 0;

	return &(mapGraph[cell->data.navpoint].first);
}

short
cell_gettraffic(CELL *cell, short nbId)
{
	for (int i = 0; i < MAX_NBS; i++) 
		if (cell->data.neighbour[i] == nbId)
			return cell->data.traffic[i];
	return -1;
}

bool
cell_delneighbour(CELL *cell, short nbId)
{
	for (int i = 0; i < MAX_NBS; i++) {
		if (cell->data.neighbour[i] == nbId) {
			for (int j = i; j < (MAX_NBS - 1); j++) {
				cell->data.neighbour[j] = cell->data.neighbour[j + 1];
				cell->data.weight[j] = cell->data.weight[j + 1];
				cell->data.traffic[j] = cell->data.traffic[j + 1];
			}
			cell->data.neighbour[MAX_NBS - 1] = NO_CELL_REGISTERED;
			cell->data.weight[MAX_NBS - 1] = FLT_MAX;
			cell->data.traffic[MAX_NBS - 1] = 0;
			return true;
		}
	}
	return false;
}

short
cell_getground(CELL *cell)
{
	return cell->data.ground;
}
