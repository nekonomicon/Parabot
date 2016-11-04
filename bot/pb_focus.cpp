#include "pb_focus.h"


PB_Focus::PB_Focus()
{
	for (int i=0; i<NUM_SECTORS; i++) {
		numCellsInSector[i] = 0;
		focusValue[i] = 1;
	}
}


void PB_Focus::addDir( Vector dir )
{
	if (dir.x==0 && dir.y==0) return;
	int sector = getSector( dir );
	numCellsInSector[sector]++;

	// calc new focusValues for ALL sectors:
	for (sector=0; sector<NUM_SECTORS; sector++) {
		int sumOtherSectors = 0;
		for (int i=0; i<NUM_SECTORS; i++) 
			if (i != sector) sumOtherSectors += numCellsInSector[i];
			
			if (sumOtherSectors==0)
				focusValue[sector] = 6*((float)numCellsInSector[sector]);
			else
				focusValue[sector] = 3*((float)numCellsInSector[sector]) / ((float)sumOtherSectors);
	}
}


float PB_Focus::forDir( Vector dir )
{
	if (dir.x==0 && dir.y==0) return 0;
	return focusValue[getSector( dir )];
}


int PB_Focus::cellsForDir( Vector dir )
{
	if (dir.x==0 && dir.y==0) return 0;
	return numCellsInSector[getSector( dir )];
}


bool PB_Focus::load( FILE *fp )
{
	fread( &numCellsInSector[0], sizeof(short), NUM_SECTORS, fp );
	fread( &focusValue[0], sizeof(float), NUM_SECTORS, fp );
	return true;
}


bool PB_Focus::save( FILE *fp )
{
	fwrite( &numCellsInSector[0], sizeof(short), NUM_SECTORS, fp );
	fwrite( &focusValue[0], sizeof(float), NUM_SECTORS, fp );
	return true;
}
