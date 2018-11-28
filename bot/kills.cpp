#include "parabot.h"
#include "sectors.h"
#include "kills.h"

void kills_adddir(Vec3D *dir, SECTOR *sectors)
{
	if (dir->x == 0 && dir->y == 0)
		return;

	int sector = getsector(dir);

	sectors[sector].kills++;
}

short kills_fordir(Vec3D *dir, SECTOR *sectors)
{
	if (dir->x == 0 && dir->y == 0)
		return 0;

	return sectors[getsector(dir)].kills;
}

