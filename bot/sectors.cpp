#include "parabot.h"
#include "sectors.h"

int
getsector(Vec3D *dir)
{
	int sector = 0;

	if ((dir->x - dir->y) > 0.0f) 
		sector += 1;

	if ((dir->x + dir->y) > 0.0f)
		sector += 2;

	return sector;
}
