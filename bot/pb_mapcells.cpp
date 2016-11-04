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
	for (int i=0; i<4096; i++) cellHash[i] = NO_CELL_REGISTERED;
	vis.clear();
}


int PB_MapCells::updateVisibility( int maxUpdates )
{
	int c1, c2, trCount = 0;

	while (trCount < maxUpdates && vis.needTrace( c1, c2 ) ) {
		if (LOSExists( cellArray[c1].pos(), cellArray[c2].pos() )) {
			vis.addTrace( true );
			Vector dir = cellArray[c2].pos() - cellArray[c1].pos();
			cellArray[c1].focus.addDir( dir );
			cellArray[c2].focus.addDir( -dir );
		}
		else {
			vis.addTrace( false );
		}
		trCount++;
	}
	return trCount;
}


int PB_MapCells::lastVisUpdate()
{
	int c1=0, c2;
	vis.needTrace( c1, c2 );
	return c1;
}


int PB_MapCells::getHashcode( Vector &pos )
{
	int ix = (((int)pos.x + 4096) & 0x007F80) >> 7;
	int iy = (((int)pos.y + 4096) & 0x007F80) >> 1;
	int hashcode = ix + iy;
	assert( hashcode >= 0 );
	assert( hashcode < 4096 );
	return hashcode;
}


Vector PB_MapCells::getAllignedPos( Vector pos )
{
	int ix = (((int)pos.x + 4096) & 0x007F80) + 64;
	int iy = (((int)pos.y + 4096) & 0x007F80) + 64;
	return Vector( (float)(ix-4096), (float)(iy-4096), pos.z );
}

int dbgCnt;

#define CHECK_BUCKET( hcode )													\
																				\
	cellId = cellHash[hcode];													\
	dbgCnt = 0; \
	while (cellId != NO_CELL_REGISTERED && dbgCnt++ < 1000) {										\
		if ( (dist = (pos-cellArray[cellId].pos()).Length()) < maxDist ) {		\
			cellFound[numCellsFound++] = PBT_FoundCell( dist, cellId );			\
			if (dist < closestDist) {											\
				closestDist = dist;												\
				closestId = cellId;												\
			}																	\
		}																		\
		cellId = cellArray[cellId].nextCell();									\
	}


int PB_MapCells::getCellId( Vector &pos, float maxDist )
{
	float dist, closestDist = maxDist;	// max dist that found WP can have to pos
	int closestId = NO_CELL_FOUND;			// is returned when no cell is found
	short cellId;
	int xStride=0, yStride=0;
	float xbDist, ybDist;	// distance to next bucket in x- and y- direction

	numCellsFound = 0;		// no cells found yet

	// search first (direct) bucket:
	int hashcode = getHashcode( pos );
	CHECK_BUCKET( hashcode )

	Vector cellStride = pos - getAllignedPos( pos );
	if (cellStride.x > 0) {	// chance that better WP in E bucket?
		xbDist = 64-cellStride.x;
		if (pos.x < (4096-128) && closestDist > xbDist) {
			xStride = +1;
			CHECK_BUCKET( hashcode+xStride )
		}
	}
	else {	// chance that better WP in W bucket?
		xbDist = 64+cellStride.x;
		if (pos.x > (-4096+128) && closestDist > xbDist) {
			xStride = -1;
			CHECK_BUCKET( hashcode+xStride )
		}
	}
	if (cellStride.y > 0) {	// chance that better WP in S bucket?
		ybDist = 64-cellStride.y;
		if (pos.y < (4096-128) && closestDist > ybDist) {
			yStride = +64;
			CHECK_BUCKET( hashcode+yStride )
		}
	}
	else {	// chance that better WP in N bucket?
		ybDist = 64+cellStride.y;
		if (pos.y > (-4096+128) && closestDist > ybDist) {
			yStride = -64;
			CHECK_BUCKET( hashcode+yStride )
		}
	}
	if (xStride != 0 && yStride != 0) {	// chance that better WP in diagonal bucket?
		float diagonalDist = sqrt( xbDist*xbDist + ybDist*ybDist );
		if (closestDist > diagonalDist)	CHECK_BUCKET( hashcode+xStride+yStride )
	}

	// any cells close?
	if (numCellsFound == 0) return NO_CELL_FOUND;

	// is the closest one visible as well?
	if ( LOSExists( pos, cellArray[closestId].pos() ) &&
		 ( (cellArray[closestId].pos().z - pos.z) <= 45 ) ) return closestId;
	
	// if not, sort all found cells
	std::sort( cellFound, cellFound+numCellsFound );
	for (int testCell=1; testCell<numCellsFound; testCell++) {
		int testId = cellFound[testCell].index;
		if (LOSExists( pos, cellArray[testId].pos() ) &&
			( (cellArray[testId].pos().z - pos.z) <= 45 ) ) return testId;
	}
	return NO_CELL_FOUND;
}


int PB_MapCells::addCell( PB_Cell newCell, bool initNbs, int addedFrom )
{
	Vector pos = newCell.pos();
	int hashcode = getHashcode( pos );
	//debugMsg( "Pos=(%.f,%.f,%.f)", pos.x, pos.y, pos.z );
	//debugMsg( ", Hashcode = %x", hashcode );
	// insert cell into array:
	cellArray.add( numCells, newCell );

	// insert id into hashtable:
	short cellId = cellHash[hashcode];
	if (cellId == NO_CELL_REGISTERED) cellHash[hashcode] = numCells;
	else {
		dbgCnt = 0;
		while (cellArray[cellId].nextCell() != NO_CELL_REGISTERED  && dbgCnt++ < 1000) {
			cellId = cellArray[cellId].nextCell();
		}
		if (dbgCnt == 1000) {
			FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
			fprintf( dfp, ">1000 recursions in addCell()!\n" ); 
			fclose( dfp );
		}
		cellArray[cellId].setNextCell( numCells );
		if (cellId==numCells) errorMsg( "CellId=numCells!\n" );
	}
	vis.addCell();
	if (initNbs) initNeighbours( numCells, addedFrom );
	numCells++;
	return (numCells-1);		// has already been incremented
}


int PB_MapCells::initNeighbours( int cellIndex, int firstNb )
{
	float maxDist = 2.0*CELL_SIZE;		// radius in which neighbours must lie

	float dist, closestDist = maxDist;	// required for macro...
	int closestId = NO_CELL_FOUND;
	short cellId;
	Vector nbPos;
	
	
	numCellsFound = 0;		// no cells found yet

	Vector pos = cellArray[cellIndex].pos();
	int hashcode = getHashcode( pos );
	// search center and 8 surrounding buckets
	if (pos.y > (-4096+128)) {
		if (pos.x > (-4096+128)) CHECK_BUCKET( hashcode-65 )
		CHECK_BUCKET( hashcode-64 )
		if (pos.x < (4096-128)) CHECK_BUCKET( hashcode-63 )
	}
	if (pos.x > (-4096+128)) CHECK_BUCKET( hashcode- 1 )
	CHECK_BUCKET( hashcode )
	if (pos.x < (4096-128)) CHECK_BUCKET( hashcode+ 1 )
	if (pos.y < (4096-128)) {
		if (pos.x > (-4096+128)) CHECK_BUCKET( hashcode+63 )
		CHECK_BUCKET( hashcode+64 )
		if (pos.x < (4096-128)) CHECK_BUCKET( hashcode+65 )
	}
	
	int numNb = 0;
	if (firstNb != NO_CELL_REGISTERED) {
		float weightEst = (pos - cellArray[firstNb].pos() ).Length() / serverMaxSpeed();
		//if (cellArray[cellIndex].setNeighbour( firstNb, weightEst )) numNb++;
		// only add current cell as neighbour to previous one:
		cellArray[firstNb].setNeighbour( cellIndex, weightEst );
	}

	for (int i=0; i<numCellsFound; i++) {
		int nbId = cellFound[i].index;
		if (nbId != cellIndex && LOSExists( pos, nbPos=cellArray[nbId].pos() )) {
			Vector dif = nbPos - pos;
			float weightEst = dif.Length() / serverMaxSpeed();
			if (dif.z <= 45) {	// neighbour z-reachable from cell
				if (cellArray[cellIndex].setNeighbour( nbId, weightEst )) numNb++;
				//else printf( "too many nbs found for cell %i!\n", cellIndex );
			}
			if (dif.z >= -45) {	// cell z-reachable from neighbour
				cellArray[nbId].setNeighbour( cellIndex, weightEst );
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
	for (int i=0; i<MAX_CELLS; i++) pre[i] = -1;									\
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
		for (int n=0; n<10; n++) {													\
			short nb = cellArray[currentCell].getNeighbour( n );					\
			short nbg = cellArray[nb].getGround();									\
			if (nb == NO_CELL_REGISTERED) break;									\
			if ( cellArray[nb].getEnvDamage() > 20 ||								\
				 (nbg>=0 && getNavpoint( nbg ).needsTriggering() && !getNavpoint( nbg ).isTriggered()) ||	\
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
	float tw = 0;																	\
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
	GET_PATH(
		currentCell==targetId,
		false,
		false,
		(cellArray[targetId].pos() - cellArray[nb].pos()).Length(),
		0
	)
}


int PB_MapCells::getPathToCover( short startId, short enemyId, short pathNodes[] )
// to second cell not visible from enemyId, bigger distance preferred
{
	GET_PATH(
		!lineOfSight( currentCell, enemyId ) && !lineOfSight( pre[currentCell], enemyId ),
		searchedNodes > 200,
		false,
		0,
		( 15000 - (cellArray[enemyId].pos() - cellArray[nb].pos()).Length() ) 
	)
}


int PB_MapCells::getPathForSneakyEscape( short startId, short enemyId, short pathNodes[] )
// to a cell far away from enemyId under best possible cover
{
	GET_PATH(
		(cellArray[currentCell].pos() - cellArray[enemyId].pos()).Length() > 2000,
		false,
		false,
		0,
		( 100.0*((int)lineOfSight( nb, enemyId )) ) 
	)
}


int PB_MapCells::getPathToAttack( short startId, short enemyId, short pathNodes[] )
// to first cell with visibility to enemyId
{	
	GET_PATH(
		lineOfSight( currentCell, enemyId ),
		false,
		false,
		0, 
		0
	)
}


int PB_MapCells::getDirectedPathToAttack( short startId, short enemyId, Vector dir, short pathNodes[] )
// to first cell in opposite direction to awayFromId with visibility to enemyId
{
	Vector startPos = cellArray[startId].pos();
	dir.Normalize();
	GET_PATH(
		lineOfSight( currentCell, enemyId ),
		searchedNodes > 100,
		DotProduct( (cellArray[nb].pos()-startPos).Normalize(), dir) < 0.7,
		0, 
		0
	)
}


int PB_MapCells::getOffensivePath( short startId, short enemyId, float minDist, short pathNodes[] )

{
	short enemyTarget = enemyId;
	
	short predictedEnemyRoute[128];
	if (getPathToCover( enemyId, startId, predictedEnemyRoute ) > 0) {
		// enemy can find cover here:
		enemyTarget = predictedEnemyRoute[0];
	}
	Vector enemyPos = cellArray[enemyId].pos();		// cover will at least be as far...

	GET_PATH(
		lineOfSight( currentCell, enemyTarget ) && (cellArray[currentCell].pos() - enemyPos).Length() > minDist,
		searchedNodes > 200,
		false,
		0, 
		( 0.5*((int)(!lineOfSight( nb, enemyId ))) )
	)
}


int PB_MapCells::predictPlayerPos( short startId, short ownId, short pathNodes[] )
{
	GET_PATH(
		queue.size()>5,
		false,
		lineOfSight(ownId, nb),
		0, 
		0
	)
}


int PB_MapCells::getPathToRoamingTarget( short startId, edict_t *botEnt, short pathNodes[] )
// to the nearest suitable roaming target
{
	GET_PATH(
		cellArray[currentCell].isSuitableRoamingTarget( botEnt ),
		false,
		false,
		0,
		0
	)	
}


bool PB_MapCells::load( char *mapname )
{
	FILE *fp;
	char filename[256] = "parabot/navigationfiles/";
	strcat( filename, mapname );

	if ( (fp = fopen( filename, "rb" )) == NULL ) return false;

	int count;

	fread( &count, sizeof(int), 1, fp );
	for (int i=0; i<count; i++) addCell( PB_Cell( fp ), false ); // don't init neighbours

	vis.load( fp );

	fclose( fp );
	return true;
}


bool PB_MapCells::save( char *mapname )
{
	FILE *fp;
	char filename[256] = "parabot/navigationfiles/";
	strcat( filename, mapname );

	if ( (fp = fopen( filename, "wb" )) == NULL ) return false;

	fwrite( &numCells, sizeof(int), 1, fp );
	for (int i=0; i<numCells; i++) cellArray[i].save( fp );

	vis.save( fp );

	fclose( fp );
	return true;
}
