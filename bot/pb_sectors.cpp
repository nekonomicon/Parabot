#include "pb_sectors.h"


int PB_Sectors::getSector( Vector dir )
{
	int sector = 0;

	if ((dir.x-dir.y) > 0) sector += 1;
	if ((dir.x+dir.y) > 0) sector += 2;
	return sector;
}
