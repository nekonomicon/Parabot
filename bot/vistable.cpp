// PB_VisTable.cpp: implementation of the PB_VisTable class.
//
//////////////////////////////////////////////////////////////////////
#include <assert.h>
#if HAVE_MALLOC_H
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include "parabot.h"
#include "sectors.h"
#include "vistable.h"
#include "cell.h"
#include "mapcells.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void updateVisTable()
{
	int trCount = mapcells_updatevisibility( 128 );
	if (trCount > 0) {
		pb2dMsg( 20, 100, "Tracing Visibility (%i/%i)...", mapcells_lastvisupdate(), mapcells_numberofcells() );
	}
}
/*
PB_VisTable::PB_VisTable()
{
	numCells = 0;
	clear();
}

PB_VisTable::~PB_VisTable()
{
	clear();
}
*/
void
vistable_clear(VISTABLE *vistable)
{
	for (int i = 0; i < vistable->numcells; i++)
		free(vistable->cells[i]);
	vistable->numcells = 0;
	vistable->tracecell = 0;
	vistable->tracebit = 0;
}

static int *
vistable_getmem(int numBits)
{
	int numInts = ((numBits - 1) >> 5) + 1;
	return (int*)calloc( numInts, sizeof(int) );
}

void
vistable_addcell(VISTABLE *vistable)
{
	assert(vistable->numcells < MAX_CELLS);
	vistable->cells[vistable->numcells] = vistable_getmem(vistable->numcells + 1);
	++vistable->numcells;
}

bool
vistable_needtrace(VISTABLE *vistable, int &cell1, int &cell2)
{
	if ((vistable->tracebit == vistable->tracecell)
	    && (vistable->tracecell < vistable->numcells))
		vistable_addtrace(vistable, true);

	if (vistable->tracecell < vistable->numcells) {
		cell1 = vistable->tracecell;
		cell2 = vistable->tracebit;
		return true;
	}
	return false;
}

static void
vistable_setvisibility(VISTABLE *vistable, int cell1, int cell2, bool visible)
{
	assert(cell1 >= cell2);
	int ofs = cell2 >> 5;
	int bit = cell2 & 31;
	if (visible)
		vistable->cells[cell1][ofs] |= BIT(bit);
	else
		vistable->cells[cell1][ofs] &= ~(BIT(bit));
}

void
vistable_addtrace(VISTABLE *vistable, bool visible)
{
	vistable_setvisibility(vistable, vistable->tracecell, vistable->tracebit, visible);
	++vistable->tracebit;
	if (vistable->tracebit > vistable->tracecell) {
		++vistable->tracecell;
		vistable->tracebit = 0;
	}
}

bool
vistable_isvisible(VISTABLE *vistable, int cell1, int cell2)
{
	if (cell1 < cell2) {
		// cell1 must be >= cell2
		int h = cell1;
		cell1 = cell2;
		cell2 = h;
	}
	int ofs = cell2 >> 5;
	int bit = cell2 & 31;
	return ((vistable->cells[cell1][ofs] & BIT(bit)) != 0);
}

void
vistable_load(VISTABLE *vistable, FILE *fp)
{
	fread(&vistable->numcells, sizeof(int), 1, fp);
	fread(&vistable->tracecell, sizeof(int), 1, fp);
	fread(&vistable->tracebit, sizeof(int), 1, fp);
	for (int i = 0; i < vistable->numcells; i++) {
		//visTable[i] = getMem(i + 1);
		int numInts = (i >> 5) + 1;
		fread(vistable->cells[i], sizeof(int), numInts, fp);
	}
}

void
vistable_save(VISTABLE *vistable, FILE *fp)
{
	fwrite(&vistable->numcells, sizeof(int), 1, fp);
	fwrite(&vistable->tracecell, sizeof(int), 1, fp);
	fwrite(&vistable->tracebit, sizeof(int), 1, fp);
	for (int i = 0; i < vistable->numcells; i++) {
		int numInts = (i >> 5) + 1;
		fwrite(vistable->cells[i], sizeof(int), numInts, fp);
	}
}
