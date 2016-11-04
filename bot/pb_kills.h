#if !defined( PB_KILLS_H )
#define PB_KILLS_H


#include "pb_sectors.h"


class PB_Kills : PB_Sectors
{


public:

	PB_Kills();

	void addDir( Vector dir );
	short forDir( Vector dir );

	bool load( FILE *fp );
	bool save( FILE *fp );


private:

	short numKillsInSector[NUM_SECTORS];

};

#endif