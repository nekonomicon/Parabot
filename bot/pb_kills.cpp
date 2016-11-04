#include "pb_kills.h"


PB_Kills::PB_Kills()
{
	for (int i=0; i<NUM_SECTORS; i++) {
		numKillsInSector[i] = 0;
	}
}


void PB_Kills::addDir( Vector dir )
{
	if (dir.x==0 && dir.y==0) return;
	int sector = getSector( dir );

	numKillsInSector[sector]++;
}


short PB_Kills::forDir( Vector dir )
{
	if (dir.x==0 && dir.y==0) return 0;
	return numKillsInSector[getSector( dir )];
}


bool PB_Kills::load( FILE *fp )
{
	fread( &numKillsInSector[0], sizeof(short), NUM_SECTORS, fp );
	return true;
}


bool PB_Kills::save( FILE *fp )
{
/*	FILE *f=fopen( "killstats.txt", "a" ); 
	fprintf( f, "%i %i %i %i\n", numKillsInSector[0], numKillsInSector[1],numKillsInSector[2],numKillsInSector[3] ); 
	fclose( f );*/
	fwrite( &numKillsInSector[0], sizeof(short), NUM_SECTORS, fp );
	return true;
}
