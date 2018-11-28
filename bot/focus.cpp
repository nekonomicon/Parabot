#include "parabot.h"
#include "sectors.h"
#include "focus.h"

/*
PB_Focus::PB_Focus()
{
	memset( &numCellsInSector, 0, sizeof numCellsInSector );
	for (int i=0; i<NUM_SECTORS; i++) {
		focusValue[i] = 1;
	}
}
*/

void focus_adddir(Vec3D *dir, SECTOR *sectors)
{
	if (dir->x == 0 && dir->y == 0)
		return;

	int sector = getsector(dir);
	sectors[sector].cells++;

	// calc new focusValues for ALL sectors:
	for (sector = 0; sector < NUM_SECTORS; sector++) {
		int sumOtherSectors = 0;
		for (int i = 0; i < NUM_SECTORS; i++) {
			if (i != sector)
				sumOtherSectors += sectors[i].cells;
		}
		if (sumOtherSectors == 0)
			sectors[sector].focusvalue = 6.0f * ((float)sectors[sector].cells);
		else
			sectors[sector].focusvalue = 3.0f * ((float)sectors[sector].cells) / ((float)sumOtherSectors);
	}
}

float focus_fordir(Vec3D *dir, SECTOR *sectors)
{
	if (dir->x == 0 && dir->y == 0)
		return 0;

	return sectors[getsector(dir)].focusvalue;
}

int focus_cellsfordir(Vec3D *dir, SECTOR *sectors)
{
	if (dir->x == 0 && dir->y == 0)
		return 0;

	return sectors[getsector(dir)].cells;
}

