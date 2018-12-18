#include <float.h>
#include "parabot.h"
#include "sectors.h"
#include "focus.h"
#include "vistable.h"
#include "cell.h"
#include "priorityqueue.h"
#include "mapcells.h"
#include <algorithm>

static MAPCELLS map;
/*
PB_MapCells::PB_MapCells() : 
	cellArray( MAX_CELLS, 256 )
{
	clear();
}


PB_MapCells::~PB_MapCells()
{
}
*/

CELL *
mapcells_getcell(int index)
{
	return &map.cellArray[index];
}

void
mapcells_clear()
{
	map.numCells = 0;
	memset( &map.cellHash, NO_CELL_REGISTERED, sizeof map.cellHash );
	vistable_clear(&map.vis);
}

int
mapcells_updatevisibility(int maxUpdates)
{
	Vec3D dir;
	int c1, c2, trCount = 0;

	while (trCount < maxUpdates && vistable_needtrace(&map.vis, c1, c2 ) ) {
		if (LOSExists(cell_pos2(&map.cellArray[c1]), cell_pos2(&map.cellArray[c2]))) {
			vistable_addtrace(&map.vis, true);
			vsub(cell_pos2(&map.cellArray[c2]), cell_pos2(&map.cellArray[c1]), &dir);
			focus_adddir(&dir, map.cellArray[c1].data.sectors);
			vinv(&dir);
			focus_adddir(&dir, map.cellArray[c2].data.sectors);
		} else {
			vistable_addtrace(&map.vis, false);
		}
		trCount++;
	}
	return trCount;
}

int
mapcells_lastvisupdate()
{
	int c1 = 0, c2;
	vistable_needtrace(&map.vis, c1, c2);
	return c1;
}

static int
mapcells_gethashcode(const Vec3D *pos)
{
	int ix = (((int)pos->x + 4096) & 0x007F80) >> 7;
	int iy = (((int)pos->y + 4096) & 0x007F80) >> 1;
	int hashcode = ix + iy;
	assert( hashcode >= 0 );
	assert( hashcode < 4096 );
	return hashcode;
}

static void
mapcells_getallignedpos(const Vec3D *pos, Vec3D *apos)
{
	int ix, iy;

	ix = (((int)pos->x + 4096) & 0x007F80) + 64;
	iy = (((int)pos->y + 4096) & 0x007F80) + 64;

	apos->x = (float)(ix - 4096);
	apos->y = (float)(iy - 4096);
	apos->z = pos->z;
}

static void
mapcells_checkbucket(int hcode, const Vec3D *pos, float *closestDist, int *closestid, float maxdist)
{
	Vec3D dir;
	float dist;

	short cellId = map.cellHash[hcode];
	int dbgCnt = 0;
	while (cellId != NO_CELL_REGISTERED && dbgCnt++ < 1000) {
		vsub(pos, cell_pos2(&map.cellArray[cellId]), &dir);
		dist = vlen(&dir);
		if (dist < maxdist) {
			map.cellFound[map.numCellsFound++] = (FOUNDCELL){.dist = dist, .index = cellId};
			if (dist < *closestDist) {
				*closestDist = dist;
				*closestid = cellId;
			}
		}
		cellId = cell_nextcell(&map.cellArray[cellId]);
	}
}

int
mapcells_getcellid(const Vec3D *pos, float maxDist)
{
	float dist, closestDist = maxDist;	// max dist that found WP can have to pos
	int closestid = NO_CELL_FOUND;			// is returned when no cell is found
	int xStride = 0, yStride = 0;
	Vec2D bdir;	// distance to next bucket in x- and y- direction
	Vec3D cellStride;

	map.numCellsFound = 0;		// no cells found yet

	// search first (direct) bucket:
	int hashcode = mapcells_gethashcode(pos);
	mapcells_checkbucket(hashcode, pos, &closestDist, &closestid, maxDist);

	mapcells_getallignedpos(pos, &cellStride);
	vsub(pos, &cellStride, &cellStride);
	if (cellStride.x > 0) {	// chance that better WP in E bucket?
		bdir.x = 64.0f - cellStride.x;
		if (pos->x < (4096.0f - 128.0f) && closestDist > bdir.x) {
			xStride = +1;
			mapcells_checkbucket(hashcode + xStride, pos, &closestDist, &closestid, maxDist);
		}
	} else {	// chance that better WP in W bucket?
		bdir.x = 64.0f + cellStride.x;
		if (pos->x > (-4096.0f + 128.0f) && closestDist > bdir.x) {
			xStride = -1;
			mapcells_checkbucket(hashcode + xStride, pos, &closestDist, &closestid, maxDist);
		}
	}
	if (cellStride.y > 0) {	// chance that better WP in S bucket?
		bdir.y = 64.0f - cellStride.y;
		if (pos->y < (4096.0f - 128.0f) && closestDist > bdir.y) {
			yStride = +64;
			mapcells_checkbucket(hashcode + yStride, pos, &closestDist, &closestid, maxDist);
		}
	} else {	// chance that better WP in N bucket?
		bdir.y = 64.0f + cellStride.y;
		if (pos->y > (-4096.0f + 128.0f) && closestDist > bdir.y) {
			yStride = -64;
			mapcells_checkbucket(hashcode + yStride, pos, &closestDist, &closestid, maxDist);
		}
	}
	if (xStride != 0 && yStride != 0) {	// chance that better WP in diagonal bucket?
		float diagonalDist = vlen2d(&bdir);
		if (closestDist > diagonalDist)
			mapcells_checkbucket(hashcode + xStride + yStride, pos, &closestDist, &closestid, maxDist);
	}

	// any cells close?
	if (map.numCellsFound == 0) return NO_CELL_FOUND;

	// is the closest one visible as well?
	if (LOSExists(pos, cell_pos2(&map.cellArray[closestid])) &&
		 ((cell_pos2(&map.cellArray[closestid])->z - pos->z) <= 45.0f)) return closestid;

	// if not, sort all found cells
	// std::sort(map.cellFound, map.cellFound + map.numCellsFound);
	for (int testCell = 1; testCell < map.numCellsFound; testCell++) {
		int testId = map.cellFound[testCell].index;
		if (LOSExists(pos, cell_pos2(&map.cellArray[testId])) &&
			((cell_pos2(&map.cellArray[testId])->z - pos->z) <= 45.0f)) return testId;
	}
	return NO_CELL_FOUND;
}

int
mapcells_getcellid(EDICT *pEdict)
{
	Vec3D pos;

	eyepos(pEdict, &pos);
	return mapcells_getcellid(&pos);
}

int
mapcells_addcell(CELL newCell, bool initNbs, int addedFrom)
{
	Vec3D pos;

	cell_pos(&newCell, &pos);
	int hashcode = mapcells_gethashcode(&pos);
	// DEBUG_MSG("Pos=(%.f,%.f,%.f)", pos.x, pos.y, pos.z);
	// DEBUG_MSG(", Hashcode = %x", hashcode);
	// insert cell into array:
	// map.cellArray.add(map.numCells, newCell);

	// insert id into hashtable:
	short cellId = map.cellHash[hashcode];
	if (cellId == NO_CELL_REGISTERED)
		map.cellHash[hashcode] = map.numCells;
	else {
		int dbgCnt = 0;
		while (cell_nextcell(&map.cellArray[cellId]) != NO_CELL_REGISTERED  && dbgCnt++ < 1000) {
			cellId = cell_nextcell(&map.cellArray[cellId]);
		}
		/*if (dbgCnt == 1000) {
			FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
			fprintf( dfp, ">1000 recursions in addCell()!\n" ); 
			fclose( dfp );
		}*/
		cell_setnextcell(&map.cellArray[cellId], map.numCells);
		if (cellId == map.numCells) ERROR_MSG( "CellId=numCells!\n" );
	}
	vistable_addcell(&map.vis);
	if (initNbs) mapcells_initneighbours( map.numCells, addedFrom );
	map.numCells++;
	return (map.numCells - 1);		// has already been incremented
}

int
mapcells_initneighbours(int cellIndex, int firstNb)
{
	float maxDist = 2.0f * CELL_SIZE;		// radius in which neighbours must lie
	float dist, closestDist = maxDist;	// required for macro...
	int closestid = NO_CELL_FOUND;
	Vec3D nbPos, pos, dif;

	map.numCellsFound = 0;		// no cells found yet

	cell_pos(&map.cellArray[cellIndex], &pos);
	int hashcode = mapcells_gethashcode(&pos);
	// search center and 8 surrounding buckets
	if (pos.y > (-4096.0f + 128.0f)) {
		if (pos.x > (-4096.0f + 128.0f))
			mapcells_checkbucket(hashcode - 65, &pos, &closestDist, &closestid, maxDist);
		mapcells_checkbucket(hashcode - 64, &pos, &closestDist, &closestid, maxDist);
		if (pos.x < (4096.0f - 128.0f))
			mapcells_checkbucket(hashcode - 63, &pos, &closestDist, &closestid, maxDist);
	}
	if (pos.x > (-4096.0f + 128.0f))
		mapcells_checkbucket(hashcode - 1, &pos, &closestDist, &closestid, maxDist);
	mapcells_checkbucket(hashcode, &pos, &closestDist, &closestid, maxDist);
	if (pos.x < (4096.0f - 128.0f))
		mapcells_checkbucket(hashcode + 1, &pos, &closestDist, &closestid, maxDist);
	if (pos.y < (4096.0f - 128.0f)) {
		if (pos.x > (-4096.0f + 128.0f))
			mapcells_checkbucket(hashcode + 63, &pos, &closestDist, &closestid, maxDist);
		mapcells_checkbucket(hashcode + 64, &pos, &closestDist, &closestid, maxDist);
		if (pos.x < (4096.0f - 128.0f))
			mapcells_checkbucket(hashcode + 65, &pos, &closestDist, &closestid, maxDist);
	}

	int numNb = 0;
	if (firstNb != NO_CELL_REGISTERED) {
		vsub(&pos, cell_pos2(&map.cellArray[firstNb]), &dif);
		float weightEst = vlen(&dif) / servermaxspeed();
		//if (map.cellArray[cellIndex].setNeighbour(firstNb, weightEst)) numNb++;
		// only add current cell as neighbour to previous one:
		cell_setneighbour(&map.cellArray[firstNb], cellIndex, weightEst);
	}

	for (int i = 0; i < map.numCellsFound; i++) {
		int nbId = map.cellFound[i].index;
		cell_pos(&map.cellArray[nbId], &pos);
		if (nbId != cellIndex && LOSExists(&pos, &nbPos)) {
			vsub(&nbPos, &pos, &dif);
			float weightEst = vlen(&dif) / servermaxspeed();
			if (dif.z <= 45) {	// neighbour z-reachable from cell
				if (cell_setneighbour(&map.cellArray[cellIndex], nbId, weightEst)) numNb++;
				//else printf("too many nbs found for cell %i!\n", cellIndex);
			} else if (dif.z >= -45) {	// cell z-reachable from neighbour
				cell_setneighbour(&map.cellArray[nbId], cellIndex, weightEst);
			}
			
		}
	}
	return numNb;
}

int
mapcells_numberofcells()
{
	return map.numCells;
}

bool
mapcells_lineofsight(int cell1, int cell2)
{
	return vistable_isvisible(&map.vis, cell1, cell2);
}

/* Generic A-Star template
 *
 * Parameters:
 * - goalReached: when this expression turns true, the search is terminated successful
 * - failed:	  when this expression turns true, the search has failed 
 * - avoid:		  these cells are not considered at all (k.o.-criteria)
 * - heuristic:   estimate of how much costs the rest of the path will produce
 *				  (has to be smaller than the real value!!!)
 * - penalty:	  additional costs
 */
#define GET_PATH( goalReached, failed, avoid, heuristic, penalty )					\
														\
	short pre[MAX_CELLS];															\
	memset( pre, NO_CELL_REGISTERED, sizeof pre );										\
	pre[startId] = startId;															\
																					\
	PBT_PriorityQueue queue;														\
	queue.addOrUpdate( startId, 0 );												\
																					\
	int searchedNodes = 0;															\
	short currentCell;																\
	while (!queue.empty()) {														\
		searchedNodes++;															\
		currentCell = queue.getFirst();												\
		if (goalReached) goto TargetReached;										\
		if (failed) goto TargetFailed;												\
		for (int n = 0; n < 10; n++) {													\
			short nb = cellArray[currentCell].getNeighbour( n );					\
			short nbg = cellArray[nb].getGround();									\
			if (nb == NO_CELL_REGISTERED) break;									\
			if ( cellArray[nb].getEnvDamage() > 20 ||								\
				 (nbg >= 0 && getNavpoint( nbg ).needsTriggering() && !getNavpoint( nbg ).isTriggered()) ||	\
				 avoid ) continue;													\
			float value = queue.getValue( currentCell ) +							\
						  cellArray[currentCell].getWeight( n ) + penalty;			\
			float estimate = value + heuristic;										\
			if (queue.neverContained( nb ) || estimate < queue.getWeight( nb )) {	\
				pre[nb] = currentCell;												\
				queue.addOrUpdate( nb, estimate, value );							\
			}																		\
		}																			\
	}																				\
																					\
TargetFailed:																		\
	return -1;																		\
																					\
TargetReached:																		\
	int pathLength = 0;																\
	while (pre[currentCell] != currentCell) {										\
		pathNodes[pathLength++] = currentCell;										\
		currentCell = pre[currentCell];												\
	}																				\
	pathNodes[pathLength] = startId;												\
	pathNodes[pathLength+1] = startId;	/* error with [index+1] */					\
	return pathLength;



int
mapcells_getpath(short startId, short targetId, short pathNodes[])
// to targetId with shortest path
{
	Vec3D dir;

	short pre[MAX_CELLS];
	memset( pre, NO_CELL_REGISTERED, sizeof pre);
	pre[startId] = startId;

	PRIORITY_QUEUE queue = {.weight[EMPTY_KEY] = FLT_MAX};
	priorityqueue_addorupdate(&queue, startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!priorityqueue_empty(&queue)) {
		searchedNodes++;
		currentCell = priorityqueue_getfirst(&queue);
		if (currentCell == targetId) {
			int pathLength = 0;

			while (pre[currentCell] != currentCell) {
				pathNodes[pathLength++] = currentCell;
				currentCell = pre[currentCell];
			}
			pathNodes[pathLength] = startId;
			pathNodes[pathLength + 1] = startId;	/* error with [index+1] */

			return pathLength;
		}
		for (int n = 0; n < 10; n++) {
			short nb = cell_getneighbour(&map.cellArray[currentCell], n);
			short nbg = cell_getground(&map.cellArray[nb]);
			if (nb == NO_CELL_REGISTERED)
				break;
			if (cell_getenvdamage(&map.cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			float value = priorityqueue_getvalue(&queue, currentCell) + cell_getweight(&map.cellArray[currentCell], n);
			vsub(cell_pos2(&map.cellArray[targetId]), cell_pos2(&map.cellArray[nb]), &dir);
			float heuristic = vlen(&dir);
			float estimate = value + heuristic;
			if (priorityqueue_nevercontained(&queue, nb)
			    || estimate < priorityqueue_getweight(&queue, nb)) {
				pre[nb] = currentCell;
				priorityqueue_addorupdate(&queue, nb, estimate, value);
			}
		}
	}

	return -1;
}

int
mapcells_getpathtocover(short startId, short enemyId, short pathNodes[])
// to second cell not visible from enemyId, bigger distance preferred
{
	Vec3D dir;

	short pre[MAX_CELLS];
	memset(pre, NO_CELL_REGISTERED, sizeof pre);
	pre[startId] = startId;

	PRIORITY_QUEUE queue = {.weight[EMPTY_KEY] = FLT_MAX};
	priorityqueue_addorupdate(&queue, startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!priorityqueue_empty(&queue)) {
		searchedNodes++;
		currentCell = priorityqueue_getfirst(&queue);
		if (!mapcells_lineofsight(currentCell, enemyId)
		    && !mapcells_lineofsight(pre[currentCell], enemyId)) {
			int pathLength = 0;

			while (pre[currentCell] != currentCell) {
				pathNodes[pathLength++] = currentCell;
				currentCell = pre[currentCell];
			}
			pathNodes[pathLength] = startId;
			pathNodes[pathLength + 1] = startId;    /* error with [index+1] */

			return pathLength;
		}
		if (searchedNodes > 200)
			return -1;
		for (int n = 0; n < 10; n++) {
			short nb = cell_getneighbour(&map.cellArray[currentCell], n);
			short nbg = cell_getground(&map.cellArray[nb]);
			if (nb == NO_CELL_REGISTERED) break;
			if (cell_getenvdamage(&map.cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			vsub(cell_pos2(&map.cellArray[enemyId]), cell_pos2(&map.cellArray[nb]), &dir);
			float penalty = 15000 - vlen(&dir);
			float value = priorityqueue_getvalue(&queue, currentCell) + cell_getweight(&map.cellArray[currentCell], n) + penalty;
			float estimate = value;
			if (priorityqueue_nevercontained(&queue, nb) || estimate < priorityqueue_getweight(&queue, nb)) {
				pre[nb] = currentCell;
				priorityqueue_addorupdate(&queue, nb, estimate, value);
			}
		}
	}

	return -1;
}

int
mapcells_getpathforsneakyescape(short startId, short enemyId, short pathNodes[])
// to a cell far away from enemyId under best possible cover
{
	Vec3D dir;
	short pre[MAX_CELLS];
	memset( pre, NO_CELL_REGISTERED, sizeof pre );
	pre[startId] = startId;

	PRIORITY_QUEUE queue = {.weight[EMPTY_KEY] = FLT_MAX};
	priorityqueue_addorupdate(&queue, startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!priorityqueue_empty(&queue)) {
		searchedNodes++;
		currentCell = priorityqueue_getfirst(&queue);
		vsub(cell_pos2(&map.cellArray[currentCell]), cell_pos2(&map.cellArray[enemyId]), &dir);
		if (vlen(&dir) > 2000) {
			int pathLength = 0;

			while (pre[currentCell] != currentCell) {
				pathNodes[pathLength++] = currentCell;
				currentCell = pre[currentCell];
			}
			pathNodes[pathLength] = startId;
			pathNodes[pathLength + 1] = startId;	/* error with [index+1] */

			return pathLength;
		}
		for (int n = 0; n < 10; n++) {
			short nb = cell_getneighbour(&map.cellArray[currentCell], n);
			short nbg = cell_getground(&map.cellArray[nb]);
			if (nb == NO_CELL_REGISTERED) break;
			if ( cell_getenvdamage(&map.cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			float penalty = (100 * ((int)mapcells_lineofsight(nb, enemyId)));
			float value = priorityqueue_getvalue(&queue, currentCell) + cell_getweight(&map.cellArray[currentCell], n) + penalty;
			float estimate = value;
			if (priorityqueue_nevercontained(&queue, nb) || estimate < priorityqueue_getweight(&queue, nb)) {
				pre[nb] = currentCell;
				priorityqueue_addorupdate(&queue, nb, estimate, value);
			}
		}
	}

	return -1;
}

int
mapcells_getpathtoattack(short startId, short enemyId, short pathNodes[])
// to first cell with visibility to enemyId
{	
	short pre[MAX_CELLS];
	memset( pre, NO_CELL_REGISTERED, sizeof pre );
	pre[startId] = startId;

	PRIORITY_QUEUE queue = {.weight[EMPTY_KEY] = FLT_MAX};
	priorityqueue_addorupdate(&queue, startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!priorityqueue_empty(&queue)) {
		searchedNodes++;
		currentCell = priorityqueue_getfirst(&queue);
		if (mapcells_lineofsight(currentCell, enemyId)) {
			int pathLength = 0;

			while (pre[currentCell] != currentCell) {
				pathNodes[pathLength++] = currentCell;
				currentCell = pre[currentCell];
			}
			pathNodes[pathLength] = startId;
			pathNodes[pathLength + 1] = startId;    /* error with [index+1] */

			return pathLength;
		}
		for (int n = 0; n < 10; n++) {
			short nb = cell_getneighbour(&map.cellArray[currentCell], n);
			short nbg = cell_getground(&map.cellArray[nb]);
			if (nb == NO_CELL_REGISTERED)
				break;
			if (cell_getenvdamage(&map.cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			float value = priorityqueue_getvalue(&queue, currentCell) + cell_getweight(&map.cellArray[currentCell], n);
			float estimate = value;
			if (priorityqueue_nevercontained(&queue, nb) || estimate < priorityqueue_getweight(&queue, nb)) {
				pre[nb] = currentCell;
				priorityqueue_addorupdate(&queue, nb, estimate, value);
			}
		}
	}

	return -1;
}

int
mapcells_getdirectedpathtoattack(short startId, short enemyId, Vec3D *dir, short pathNodes[])
// to first cell in opposite direction to awayFromId with visibility to enemyId
{
	Vec3D opdir;

	normalize(dir);
	short pre[MAX_CELLS];
	memset( pre, NO_CELL_REGISTERED, sizeof pre );
	pre[startId] = startId;

	PRIORITY_QUEUE queue = {.weight[EMPTY_KEY] = FLT_MAX};
	priorityqueue_addorupdate(&queue, startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!priorityqueue_empty(&queue)) {
		searchedNodes++;
		currentCell = priorityqueue_getfirst(&queue);
		if (mapcells_lineofsight(currentCell, enemyId)) {
			int pathLength = 0;

			while (pre[currentCell] != currentCell) {
				pathNodes[pathLength++] = currentCell;
				currentCell = pre[currentCell];
			}
			pathNodes[pathLength] = startId;
			pathNodes[pathLength + 1] = startId;    /* error with [index+1] */

			return pathLength;
		}
		if (searchedNodes > 100)
			return -1;
		for (int n = 0; n < 10; n++) {
			short nb = cell_getneighbour(&map.cellArray[currentCell], n);
			short nbg = cell_getground(&map.cellArray[nb]);
			if (nb == NO_CELL_REGISTERED) break;
			vsub(cell_pos2(&map.cellArray[nb]), cell_pos2(&map.cellArray[startId]), &opdir);
			normalize(&opdir);
			if ( cell_getenvdamage(&map.cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg)))
			    || dotproduct(&opdir, dir) < 0.7f)
				continue;
			float value = priorityqueue_getvalue(&queue, currentCell) + cell_getweight(&map.cellArray[currentCell], n);
			float estimate = value;
			if (priorityqueue_nevercontained(&queue, nb) || estimate < priorityqueue_getweight(&queue, nb)) {
				pre[nb] = currentCell;
				priorityqueue_addorupdate(&queue, nb, estimate, value);
			}
		}
	}

	return -1;
}

int
mapcells_getoffensivepath(short startId, short enemyId, float minDist, short pathNodes[])
{
	short enemyTarget = enemyId;
	
	short predictedEnemyRoute[128];
	if (mapcells_getpathtocover(enemyId, startId, predictedEnemyRoute) > 0) {
		// enemy can find cover here:
		enemyTarget = predictedEnemyRoute[0];
	}
	Vec3D enemyPos, dir;		// cover will at least be as far...

	cell_pos(&map.cellArray[enemyId], &enemyPos);
	short pre[MAX_CELLS];
	memset(pre, NO_CELL_REGISTERED, sizeof pre);
	pre[startId] = startId;

	PRIORITY_QUEUE queue = {.weight[EMPTY_KEY] = FLT_MAX};
	priorityqueue_addorupdate(&queue, startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!priorityqueue_empty(&queue)) {
		searchedNodes++;
		currentCell = priorityqueue_getfirst(&queue);
		vsub(cell_pos2(&map.cellArray[currentCell]), &enemyPos, &dir);
		if (mapcells_lineofsight( currentCell, enemyTarget ) && vlen(&dir) > minDist) {
			int pathLength = 0;

			while (pre[currentCell] != currentCell) {
				pathNodes[pathLength++] = currentCell;
				currentCell = pre[currentCell];
			}
			pathNodes[pathLength] = startId;
			pathNodes[pathLength + 1] = startId;	/* error with [index+1] */
			return pathLength;
		}
		if (searchedNodes > 200)
			return -1;
		for (int n = 0; n < 10; n++) {
			short nb = cell_getneighbour(&map.cellArray[currentCell], n);
			short nbg = cell_getground(&map.cellArray[nb]);
			if (nb == NO_CELL_REGISTERED)
				break;
			if (cell_getenvdamage(&map.cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			float penalty = 0.5f * ((int)(!mapcells_lineofsight( nb, enemyId)));
			float value = priorityqueue_getvalue(&queue, currentCell) + cell_getweight(&map.cellArray[currentCell], n) + penalty;
			float estimate = value;
			if (priorityqueue_nevercontained(&queue, nb) || estimate < priorityqueue_getweight(&queue, nb)) {
				pre[nb] = currentCell;
				priorityqueue_addorupdate(&queue, nb, estimate, value);
			}
		}
	}

	return -1;
}

int
mapcells_predictplayerpos(short startId, short ownId, short pathNodes[])
{
	short pre[MAX_CELLS];
	memset(pre, NO_CELL_REGISTERED, sizeof pre);
	pre[startId] = startId;

	PRIORITY_QUEUE queue = {.weight[EMPTY_KEY] = FLT_MAX};
	priorityqueue_addorupdate(&queue, startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!priorityqueue_empty(&queue)) {
		searchedNodes++;
		currentCell = priorityqueue_getfirst(&queue);
		if (priorityqueue_size(&queue) > 5.0f) {
			int pathLength = 0;

			while (pre[currentCell] != currentCell) {
				pathNodes[pathLength++] = currentCell;
				currentCell = pre[currentCell];
			}
			pathNodes[pathLength] = startId;
			pathNodes[pathLength + 1] = startId;	/* error with [index+1] */
			return pathLength;
		}
		for (int n = 0; n < 10; n++) {
			short nb = cell_getneighbour(&map.cellArray[currentCell], n);
			short nbg = cell_getground(&map.cellArray[nb]);
			if (nb == NO_CELL_REGISTERED)
				break;
			if (cell_getenvdamage(&map.cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg)))
			    || mapcells_lineofsight(ownId, nb))
				continue;
			float value = priorityqueue_getvalue(&queue, currentCell) + cell_getweight(&map.cellArray[currentCell], n);
			float estimate = value;
			if (priorityqueue_nevercontained(&queue, nb)
			    || estimate < priorityqueue_getweight(&queue, nb)) {
				pre[nb] = currentCell;
				priorityqueue_addorupdate(&queue, nb, estimate, value);
			}
		}
	}

	return -1;
}

int
mapcells_getpathtoroamingtarget(short startId, EDICT *botEnt, short pathNodes[])
// to the nearest suitable roaming target
{
	short pre[MAX_CELLS];
	memset( pre, NO_CELL_REGISTERED, sizeof pre );
	pre[startId] = startId;

	PRIORITY_QUEUE queue = {.weight[EMPTY_KEY] = FLT_MAX};
	priorityqueue_addorupdate(&queue, startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!priorityqueue_empty(&queue)) {
		searchedNodes++;
		currentCell = priorityqueue_getfirst(&queue);
		if (cell_issuitableroamingtarget(&map.cellArray[currentCell], botEnt)) {
			int pathLength = 0;

			while (pre[currentCell] != currentCell) {
				pathNodes[pathLength++] = currentCell;
				currentCell = pre[currentCell];
			}
			pathNodes[pathLength] = startId;
			pathNodes[pathLength + 1] = startId;	/* error with [index+1] */

			return pathLength;
		}
		for (int n = 0; n < 10; n++) {
			short nb = cell_getneighbour(&map.cellArray[currentCell], n);
			short nbg = cell_getground(&map.cellArray[nb]);
			if (nb == NO_CELL_REGISTERED) break;
			if (cell_getenvdamage(&map.cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			float value = priorityqueue_getvalue(&queue, currentCell) + cell_getweight(&map.cellArray[currentCell], n);
			float estimate = value;
			if (priorityqueue_nevercontained(&queue,  nb) || estimate < priorityqueue_getweight(&queue, nb)) {
				pre[nb] = currentCell;
				priorityqueue_addorupdate(&queue, nb, estimate, value );
			}
		}
	}

	return -1;
}

bool
mapcells_load(const char *mapname)
{
	FILE *fp;
	CELL cell;

	if ((fp = fopen( mapname, "rb" )) == NULL)
		return false;

	int count;

	fread( &count, sizeof(int), 1, fp );
	for (int i = 0; i < count; i++) {
		cell_load(&cell, fp);
		mapcells_addcell(cell, false); // don't init neighbours
	}
	vistable_load(&map.vis, fp);

	fclose(fp);
	return true;
}

bool
mapcells_save(const char *mapname)
{
	FILE *fp;

	if ((fp = fopen(mapname, "wb")) == NULL) return false;

	fwrite(&map.numCells, sizeof(int), 1, fp);
	for (int i = 0; i < map.numCells; i++)
		cell_save(&map.cellArray[i], fp);

	vistable_save(&map.vis, fp);

	fclose(fp);

	return true;
}
