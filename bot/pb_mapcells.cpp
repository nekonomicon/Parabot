#include "parabot.h"
#include "sectors.h"
#include "focus.h"
#include "vistable.h"
#include "cell.h"
#include "pb_mapcells.h"
#include <algorithm>

PB_MapCells::PB_MapCells() : 
	cellArray( MAX_CELLS, 256 )
{
	clear();
}


PB_MapCells::~PB_MapCells()
{
}


void PB_MapCells::clear()
{
	numCells = 0;
	memset( &cellHash, NO_CELL_REGISTERED, sizeof cellHash );
	vistable_clear(&vis);
}


int PB_MapCells::updateVisibility( int maxUpdates )
{
	Vec3D dir;
	int c1, c2, trCount = 0;

	while (trCount < maxUpdates && vistable_needtrace(&vis, c1, c2 ) ) {
		if (LOSExists(cell_pos2(&cellArray[c1]), cell_pos2(&cellArray[c2]))) {
			vistable_addtrace(&vis, true);
			vsub(cell_pos2(&cellArray[c2]), cell_pos2(&cellArray[c1]), &dir);
			focus_adddir(&dir, cellArray[c1].data.sectors);
			vinv(&dir);
			focus_adddir(&dir, cellArray[c2].data.sectors);
		} else {
			vistable_addtrace(&vis, false);
		}
		trCount++;
	}
	return trCount;
}

int PB_MapCells::lastVisUpdate()
{
	int c1 = 0, c2;
	vistable_needtrace(&vis, c1, c2 );
	return c1;
}

int PB_MapCells::getHashcode( const Vec3D *pos )
{
	int ix = (((int)pos->x + 4096) & 0x007F80) >> 7;
	int iy = (((int)pos->y + 4096) & 0x007F80) >> 1;
	int hashcode = ix + iy;
	assert( hashcode >= 0 );
	assert( hashcode < 4096 );
	return hashcode;
}

void PB_MapCells::getAllignedPos(const Vec3D *pos, Vec3D *apos)
{
	int ix, iy;

	ix = (((int)pos->x + 4096) & 0x007F80) + 64;
	iy = (((int)pos->y + 4096) & 0x007F80) + 64;

	apos->x = (float)(ix - 4096);
	apos->y = (float)(iy - 4096);
	apos->z = pos->z;
}

int dbgCnt;

void PB_MapCells::checkbucket(int hcode, const Vec3D *pos, float *closestDist, int *closestid, float maxdist)
{
	Vec3D dir;
	float dist;

	short cellId = cellHash[hcode];
	dbgCnt = 0;
	while (cellId != NO_CELL_REGISTERED && dbgCnt++ < 1000) {
		vsub(pos, cell_pos2(&cellArray[cellId]), &dir);
		dist = vlen(&dir);
		if (dist < maxdist) {
			cellFound[numCellsFound++] = PBT_FoundCell(dist, cellId);
			if (dist < *closestDist) {
				*closestDist = dist;
				*closestid = cellId;
			}
		}
		cellId = cell_nextcell(&cellArray[cellId]);
	}
}

int PB_MapCells::getCellId(const Vec3D *pos, float maxDist )
{
	float dist, closestDist = maxDist;	// max dist that found WP can have to pos
	int closestid = NO_CELL_FOUND;			// is returned when no cell is found
	int xStride = 0, yStride = 0;
	Vec2D bdir;	// distance to next bucket in x- and y- direction
	Vec3D cellStride;

	numCellsFound = 0;		// no cells found yet

	// search first (direct) bucket:
	int hashcode = getHashcode(pos);
	checkbucket(hashcode, pos, &closestDist, &closestid, maxDist);

	getAllignedPos(pos, &cellStride);
	vsub(pos, &cellStride, &cellStride);
	if (cellStride.x > 0) {	// chance that better WP in E bucket?
		bdir.x = 64.0f - cellStride.x;
		if (pos->x < (4096.0f - 128.0f) && closestDist > bdir.x) {
			xStride = +1;
			checkbucket(hashcode + xStride, pos, &closestDist, &closestid, maxDist);
		}
	} else {	// chance that better WP in W bucket?
		bdir.x = 64.0f + cellStride.x;
		if (pos->x > (-4096.0f + 128.0f) && closestDist > bdir.x) {
			xStride = -1;
			checkbucket(hashcode + xStride, pos, &closestDist, &closestid, maxDist);
		}
	}
	if (cellStride.y > 0) {	// chance that better WP in S bucket?
		bdir.y = 64.0f - cellStride.y;
		if (pos->y < (4096.0f - 128.0f) && closestDist > bdir.y) {
			yStride = +64;
			checkbucket(hashcode + yStride, pos, &closestDist, &closestid, maxDist);
		}
	} else {	// chance that better WP in N bucket?
		bdir.y = 64.0f + cellStride.y;
		if (pos->y > (-4096.0f + 128.0f) && closestDist > bdir.y) {
			yStride = -64;
			checkbucket(hashcode + yStride, pos, &closestDist, &closestid, maxDist);
		}
	}
	if (xStride != 0 && yStride != 0) {	// chance that better WP in diagonal bucket?
		float diagonalDist = vlen2d(&bdir);
		if (closestDist > diagonalDist)
			checkbucket(hashcode + xStride + yStride, pos, &closestDist, &closestid, maxDist);
	}

	// any cells close?
	if (numCellsFound == 0) return NO_CELL_FOUND;

	// is the closest one visible as well?
	if (LOSExists(pos, cell_pos2(&cellArray[closestid])) &&
		 ((cell_pos2(&cellArray[closestid])->z - pos->z) <= 45.0f)) return closestid;

	// if not, sort all found cells
	std::sort(cellFound, cellFound + numCellsFound);
	for (int testCell = 1; testCell < numCellsFound; testCell++) {
		int testId = cellFound[testCell].index;
		if (LOSExists(pos, cell_pos2(&cellArray[testId])) &&
			((cell_pos2(&cellArray[testId])->z - pos->z) <= 45.0f)) return testId;
	}
	return NO_CELL_FOUND;
}

int PB_MapCells::addCell( CELL newCell, bool initNbs, int addedFrom )
{
	Vec3D pos;

	cell_pos(&newCell, &pos);
	int hashcode = getHashcode(&pos);
	// DEBUG_MSG("Pos=(%.f,%.f,%.f)", pos.x, pos.y, pos.z);
	// DEBUG_MSG(", Hashcode = %x", hashcode);
	// insert cell into array:
	cellArray.add(numCells, newCell);

	// insert id into hashtable:
	short cellId = cellHash[hashcode];
	if (cellId == NO_CELL_REGISTERED)
		cellHash[hashcode] = numCells;
	else {
		dbgCnt = 0;
		while (cell_nextcell(&cellArray[cellId]) != NO_CELL_REGISTERED  && dbgCnt++ < 1000) {
			cellId = cell_nextcell(&cellArray[cellId]);
		}
		/*if (dbgCnt == 1000) {
			FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
			fprintf( dfp, ">1000 recursions in addCell()!\n" ); 
			fclose( dfp );
		}*/
		cell_setnextcell(&cellArray[cellId], numCells);
		if (cellId == numCells) ERROR_MSG( "CellId=numCells!\n" );
	}
	vistable_addcell(&vis);
	if (initNbs) initNeighbours( numCells, addedFrom );
	numCells++;
	return (numCells - 1);		// has already been incremented
}

int PB_MapCells::initNeighbours( int cellIndex, int firstNb )
{
	float maxDist = 2.0f * CELL_SIZE;		// radius in which neighbours must lie
	float dist, closestDist = maxDist;	// required for macro...
	int closestid = NO_CELL_FOUND;
	Vec3D nbPos, pos, dif;

	numCellsFound = 0;		// no cells found yet

	cell_pos(&cellArray[cellIndex], &pos);
	int hashcode = getHashcode(&pos);
	// search center and 8 surrounding buckets
	if (pos.y > (-4096.0f + 128.0f)) {
		if (pos.x > (-4096.0f + 128.0f))
			checkbucket(hashcode - 65, &pos, &closestDist, &closestid, maxDist);
		checkbucket(hashcode - 64, &pos, &closestDist, &closestid, maxDist);
		if (pos.x < (4096.0f - 128.0f))
			checkbucket(hashcode - 63, &pos, &closestDist, &closestid, maxDist);
	}
	if (pos.x > (-4096.0f + 128.0f))
		checkbucket(hashcode - 1, &pos, &closestDist, &closestid, maxDist);
	checkbucket(hashcode, &pos, &closestDist, &closestid, maxDist);
	if (pos.x < (4096.0f - 128.0f))
		checkbucket(hashcode + 1, &pos, &closestDist, &closestid, maxDist);
	if (pos.y < (4096.0f - 128.0f)) {
		if (pos.x > (-4096.0f + 128.0f))
			checkbucket(hashcode + 63, &pos, &closestDist, &closestid, maxDist);
		checkbucket(hashcode + 64, &pos, &closestDist, &closestid, maxDist);
		if (pos.x < (4096.0f - 128.0f))
			checkbucket(hashcode + 65, &pos, &closestDist, &closestid, maxDist);
	}

	int numNb = 0;
	if (firstNb != NO_CELL_REGISTERED) {
		vsub(&pos, cell_pos2(&cellArray[firstNb]), &dif);
		float weightEst = vlen(&dif) / servermaxspeed();
		//if (cellArray[cellIndex].setNeighbour(firstNb, weightEst)) numNb++;
		// only add current cell as neighbour to previous one:
		cell_setneighbour(&cellArray[firstNb], cellIndex, weightEst);
	}

	for (int i = 0; i < numCellsFound; i++) {
		int nbId = cellFound[i].index;
		cell_pos(&cellArray[nbId], &pos);
		if (nbId != cellIndex && LOSExists(&pos, &nbPos)) {
			vsub(&nbPos, &pos, &dif);
			float weightEst = vlen(&dif) / servermaxspeed();
			if (dif.z <= 45) {	// neighbour z-reachable from cell
				if (cell_setneighbour(&cellArray[cellIndex], nbId, weightEst)) numNb++;
				//else printf("too many nbs found for cell %i!\n", cellIndex);
			} else if (dif.z >= -45) {	// cell z-reachable from neighbour
				cell_setneighbour(&cellArray[nbId], cellIndex, weightEst);
			}
			
		}
	}
	return numNb;
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



int PB_MapCells::getPath( short startId, short targetId, short pathNodes[] )
// to targetId with shortest path
{
	Vec3D dir;

	short pre[MAX_CELLS];
	memset( pre, NO_CELL_REGISTERED, sizeof pre);
	pre[startId] = startId;

	PBT_PriorityQueue queue;
	queue.addOrUpdate(startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!queue.empty()) {
		searchedNodes++;
		currentCell = queue.getFirst();
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
			short nb = cell_getneighbour(&cellArray[currentCell], n);
			short nbg = cell_getground(&cellArray[nb]);
			if (nb == NO_CELL_REGISTERED)
				break;
			if (cell_getenvdamage(&cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			float value = queue.getValue(currentCell) + cell_getweight(&cellArray[currentCell], n);
			vsub(cell_pos2(&cellArray[targetId]), cell_pos2(&cellArray[nb]), &dir);
			float heuristic = vlen(&dir);
			float estimate = value + heuristic;
			if (queue.neverContained(nb)
			    || estimate < queue.getWeight(nb)) {
				pre[nb] = currentCell;
				queue.addOrUpdate(nb, estimate, value);
			}
		}
	}

	return -1;
}

int PB_MapCells::getPathToCover( short startId, short enemyId, short pathNodes[] )
// to second cell not visible from enemyId, bigger distance preferred
{
	Vec3D dir;

	short pre[MAX_CELLS];
	memset(pre, NO_CELL_REGISTERED, sizeof pre);
	pre[startId] = startId;

	PBT_PriorityQueue queue;
	queue.addOrUpdate(startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!queue.empty()) {
		searchedNodes++;
		currentCell = queue.getFirst();
		if (!lineOfSight(currentCell, enemyId)
		    && !lineOfSight(pre[currentCell], enemyId)) {
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
			short nb = cell_getneighbour(&cellArray[currentCell], n);
			short nbg = cell_getground(&cellArray[nb]);
			if (nb == NO_CELL_REGISTERED) break;
			if (cell_getenvdamage(&cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			vsub(cell_pos2(&cellArray[enemyId]), cell_pos2(&cellArray[nb]), &dir);
			float penalty = 15000 - vlen(&dir);
			float value = queue.getValue(currentCell) + cell_getweight(&cellArray[currentCell], n) + penalty;
			float estimate = value;
			if (queue.neverContained(nb) || estimate < queue.getWeight(nb)) {
				pre[nb] = currentCell;
				queue.addOrUpdate( nb, estimate, value );
			}
		}
	}

	return -1;
}

int PB_MapCells::getPathForSneakyEscape( short startId, short enemyId, short pathNodes[] )
// to a cell far away from enemyId under best possible cover
{
	Vec3D dir;
	short pre[MAX_CELLS];
	memset( pre, NO_CELL_REGISTERED, sizeof pre );
	pre[startId] = startId;

	PBT_PriorityQueue queue;
	queue.addOrUpdate( startId, 0 );

	int searchedNodes = 0;
	short currentCell;
	while (!queue.empty()) {
		searchedNodes++;
		currentCell = queue.getFirst();
		vsub(cell_pos2(&cellArray[currentCell]), cell_pos2(&cellArray[enemyId]), &dir);
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
			short nb = cell_getneighbour(&cellArray[currentCell], n);
			short nbg = cell_getground(&cellArray[nb]);
			if (nb == NO_CELL_REGISTERED) break;
			if ( cell_getenvdamage(&cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			float penalty = (100 * ((int)lineOfSight(nb, enemyId)));
			float value = queue.getValue(currentCell) + cell_getweight(&cellArray[currentCell], n) + penalty;
			float estimate = value;
			if (queue.neverContained(nb) || estimate < queue.getWeight(nb)) {
				pre[nb] = currentCell;
				queue.addOrUpdate(nb, estimate, value);
			}
		}
	}

	return -1;
}


int PB_MapCells::getPathToAttack( short startId, short enemyId, short pathNodes[] )
// to first cell with visibility to enemyId
{	
	short pre[MAX_CELLS];
	memset( pre, NO_CELL_REGISTERED, sizeof pre );
	pre[startId] = startId;

	PBT_PriorityQueue queue;
	queue.addOrUpdate( startId, 0 );

	int searchedNodes = 0;
	short currentCell;
	while (!queue.empty()) {
		searchedNodes++;
		currentCell = queue.getFirst();
		if (lineOfSight(currentCell, enemyId)) {
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
			short nb = cell_getneighbour(&cellArray[currentCell], n);
			short nbg = cell_getground(&cellArray[nb]);
			if (nb == NO_CELL_REGISTERED)
				break;
			if (cell_getenvdamage(&cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			float value = queue.getValue(currentCell) + cell_getweight(&cellArray[currentCell], n);
			float estimate = value;
			if (queue.neverContained(nb) || estimate < queue.getWeight(nb)) {
				pre[nb] = currentCell;
				queue.addOrUpdate(nb, estimate, value);
			}
		}
	}

	return -1;
}


int PB_MapCells::getDirectedPathToAttack( short startId, short enemyId, Vec3D *dir, short pathNodes[] )
// to first cell in opposite direction to awayFromId with visibility to enemyId
{
	Vec3D opdir;

	normalize(dir);
	short pre[MAX_CELLS];
	memset( pre, NO_CELL_REGISTERED, sizeof pre );
	pre[startId] = startId;

	PBT_PriorityQueue queue;
	queue.addOrUpdate(startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!queue.empty()) {
		searchedNodes++;
		currentCell = queue.getFirst();
		if (lineOfSight(currentCell, enemyId)) {
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
			short nb = cell_getneighbour(&cellArray[currentCell], n);
			short nbg = cell_getground(&cellArray[nb]);
			if (nb == NO_CELL_REGISTERED) break;
			vsub(cell_pos2(&cellArray[nb]), cell_pos2(&cellArray[startId]), &opdir);
			normalize(&opdir);
			if ( cell_getenvdamage(&cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg)))
			    || dotproduct(&opdir, dir) < 0.7f)
				continue;
			float value = queue.getValue(currentCell) + cell_getweight(&cellArray[currentCell], n);
			float estimate = value;
			if (queue.neverContained(nb) || estimate < queue.getWeight(nb)) {
				pre[nb] = currentCell;
				queue.addOrUpdate( nb, estimate, value );
			}
		}
	}

	return -1;
}


int PB_MapCells::getOffensivePath( short startId, short enemyId, float minDist, short pathNodes[] )
{
	short enemyTarget = enemyId;
	
	short predictedEnemyRoute[128];
	if (getPathToCover(enemyId, startId, predictedEnemyRoute) > 0) {
		// enemy can find cover here:
		enemyTarget = predictedEnemyRoute[0];
	}
	Vec3D enemyPos, dir;		// cover will at least be as far...

	cell_pos(&cellArray[enemyId], &enemyPos);
	short pre[MAX_CELLS];
	memset(pre, NO_CELL_REGISTERED, sizeof pre);
	pre[startId] = startId;

	PBT_PriorityQueue queue;
	queue.addOrUpdate(startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!queue.empty()) {
		searchedNodes++;
		currentCell = queue.getFirst();
		vsub(cell_pos2(&cellArray[currentCell]), &enemyPos, &dir);
		if (lineOfSight( currentCell, enemyTarget ) && vlen(&dir) > minDist) {
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
			short nb = cell_getneighbour(&cellArray[currentCell], n);
			short nbg = cell_getground(&cellArray[nb]);
			if (nb == NO_CELL_REGISTERED)
				break;
			if (cell_getenvdamage(&cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			float penalty = 0.5f * ((int)(!lineOfSight( nb, enemyId)));
			float value = queue.getValue(currentCell) + cell_getweight(&cellArray[currentCell], n) + penalty;
			float estimate = value;
			if (queue.neverContained(nb) || estimate < queue.getWeight(nb)) {
				pre[nb] = currentCell;
				queue.addOrUpdate(nb, estimate, value);
			}
		}
	}

	return -1;
}


int PB_MapCells::predictPlayerPos( short startId, short ownId, short pathNodes[] )
{
	short pre[MAX_CELLS];
	memset(pre, NO_CELL_REGISTERED, sizeof pre);
	pre[startId] = startId;

	PBT_PriorityQueue queue;
	queue.addOrUpdate(startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!queue.empty()) {
		searchedNodes++;
		currentCell = queue.getFirst();
		if (queue.size() > 5.0f) {
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
			short nb = cell_getneighbour(&cellArray[currentCell], n);
			short nbg = cell_getground(&cellArray[nb]);
			if (nb == NO_CELL_REGISTERED)
				break;
			if (cell_getenvdamage(&cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg)))
			    || lineOfSight(ownId, nb))
				continue;
			float value = queue.getValue(currentCell) + cell_getweight(&cellArray[currentCell], n);
			float estimate = value;
			if (queue.neverContained(nb)
			    || estimate < queue.getWeight(nb)) {
				pre[nb] = currentCell;
				queue.addOrUpdate(nb, estimate, value);
			}
		}
	}

	return -1;
}


int PB_MapCells::getPathToRoamingTarget( short startId, EDICT *botEnt, short pathNodes[] )
// to the nearest suitable roaming target
{
	short pre[MAX_CELLS];
	memset( pre, NO_CELL_REGISTERED, sizeof pre );
	pre[startId] = startId;

	PBT_PriorityQueue queue;
	queue.addOrUpdate(startId, 0);

	int searchedNodes = 0;
	short currentCell;
	while (!queue.empty()) {
		searchedNodes++;
		currentCell = queue.getFirst();
		if (cell_issuitableroamingtarget(&cellArray[currentCell], botEnt)) {
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
			short nb = cell_getneighbour(&cellArray[currentCell], n);
			short nbg = cell_getground(&cellArray[nb]);
			if (nb == NO_CELL_REGISTERED) break;
			if (cell_getenvdamage(&cellArray[nb]) > 20
			    || (nbg >= 0
			    && navpoint_needstriggering(getNavpoint(nbg))
			    && !navpoint_istriggered(getNavpoint(nbg))))
				continue;
			float value = queue.getValue(currentCell) + cell_getweight(&cellArray[currentCell], n);
			float estimate = value;
			if (queue.neverContained( nb ) || estimate < queue.getWeight(nb)) {
				pre[nb] = currentCell;
				queue.addOrUpdate( nb, estimate, value );
			}
		}
	}

	return -1;
}


bool PB_MapCells::load( char *mapname )
{
	FILE *fp;
	CELL cell;

	if ((fp = fopen( mapname, "rb" )) == NULL)
		return false;

	int count;

	fread( &count, sizeof(int), 1, fp );
	for (int i = 0; i < count; i++) {
		cell_load(&cell, fp);
		addCell(cell, false); // don't init neighbours
	}
	vistable_load(&vis, fp);

	fclose(fp);
	return true;
}


bool PB_MapCells::save( char *mapname )
{
	FILE *fp;

	if ((fp = fopen(mapname, "wb")) == NULL) return false;

	fwrite(&numCells, sizeof(int), 1, fp);
	for (int i = 0; i < numCells; i++)
		cell_save(&cellArray[i], fp);

	vistable_save(&vis, fp);

	fclose(fp);

	return true;
}
