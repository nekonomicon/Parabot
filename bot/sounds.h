#ifndef SOUNDS_H
#define SOUNDS_H


#include "pb_global.h"



#define STEP_CONCRETE	0		// default step sound
#define STEP_METAL		1		// metal floor
#define STEP_DIRT		2		// dirt, sand, rock
#define STEP_VENT		3		// ventillation duct
#define STEP_GRATE		4		// metal grating
#define STEP_TILE		5		// floor tiles
#define STEP_SLOSH		6		// shallow liquid puddle
#define STEP_WADE		7		// wading in liquid
#define STEP_LADDER		8		// climbing ladder


typedef char TextureName[CBTEXTURENAMEMAX];


class Sounds
{

public:

	Sounds();

	void getAllClientSounds();
	void parseSound( edict_t *ent, const char *sample, float vol );
	void parseAmbientSound( edict_t *ent, const char *sample, float vol );
		
	float getSensableDist( int clientIndex );
	float getTrackableDist( int clientIndex );
	


private:

	float stepSensableDist[32], stepTrackableDist[32];
	float itemSensableDist[32], itemTrackableDist[32];
	float timeItemSound[32];
	float timeStepSound[32], oldTimeStepSound[32];
	TextureName textureName[32];
	char textureType[32];


	char findTextureType( char *name );
	void calcStepSound( int clientIndex, edict_t *ent, bool writeResult );

	
};

#endif