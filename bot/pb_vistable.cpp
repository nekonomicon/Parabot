// PB_VisTable.cpp: implementation of the PB_VisTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( _DEBUG )
#define NDEBUG			// no assert!
#endif

#include <assert.h>
#include <malloc.h>
#include "PB_VisTable.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


PB_VisTable::PB_VisTable()
{
	// init bitMask table
	for (int i=0; i<32; i++) {
		int bm = 1 << i;
		bitMask[i] = bm;
	}
	numCells = 0;
	clear();
}


PB_VisTable::~PB_VisTable()
{
	clear();
}


void PB_VisTable::clear()
{
	for (int i=0; i<numCells; i++) free( visTable[i] );
	numCells = 0;
	traceCell = 0;
	traceBit = 0;
}


int* PB_VisTable::getMem( int numBits )
{
	int numInts = ((numBits-1) >> 5) + 1;
	return (int*)calloc( numInts, sizeof(int) );
}


void PB_VisTable::addCell()
{
	assert( numCells < MAX_CELLS );
	visTable[numCells] = getMem( numCells+1 );
	numCells++;
}


bool PB_VisTable::needTrace( int &cell1, int &cell2 )
{
	if ((traceBit == traceCell) && (traceCell < numCells)) addTrace( true );

	if (traceCell < numCells) {
		cell1 = traceCell;
		cell2 = traceBit;
		return true;
	}
	return false;
}


void PB_VisTable::addTrace( bool visible )
{
	setVisibility( traceCell, traceBit, visible );
	traceBit++;
	if (traceBit > traceCell) {
		traceCell++;
		traceBit = 0;
	}
}


bool PB_VisTable::isVisible( int cell1, int cell2 )
{
	if (cell1 < cell2) {
		// cell1 must be >= cell2
		int h = cell1;	cell1 = cell2;	cell2 = h;
	}
	int ofs = cell2 >> 5;
	int bit = cell2 & 31;
	return ((visTable[cell1][ofs] & bitMask[bit]) !=0 );

}


void PB_VisTable::setVisibility( int cell1, int cell2, bool visible )
{
	assert( cell1 >= cell2 );
	int ofs = cell2 >> 5;
	int bit = cell2 & 31;
	if (visible) visTable[cell1][ofs] |= bitMask[bit];
	else		 visTable[cell1][ofs] &= ~(bitMask[bit]);
}


void PB_VisTable::load( FILE *fp )
{
	fread( &numCells, sizeof(int), 1, fp );
	fread( &traceCell, sizeof(int), 1, fp );
	fread( &traceBit, sizeof(int), 1, fp );
	for (int i=0; i<numCells; i++) {
		//visTable[i] = getMem( i+1 );
		int numInts = (i >> 5) + 1;
		fread( visTable[i], sizeof(int), numInts, fp );
	}
}


void PB_VisTable::save( FILE *fp )
{
	fwrite( &numCells, sizeof(int), 1, fp );
	fwrite( &traceCell, sizeof(int), 1, fp );
	fwrite( &traceBit, sizeof(int), 1, fp );
	for (int i=0; i<numCells; i++) {
		int numInts = (i >> 5) + 1;
		fwrite( visTable[i], sizeof(int), numInts, fp );
	}
}
