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
	void init();
	void getAllClientSounds();
	void parseSound( edict_t *ent, const char *sample, float vol );
	void parseAmbientSound( edict_t *ent, const char *sample, float vol );

	float getSensableDist( int clientIndex );
	float getTrackableDist( int clientIndex );
private:
	typedef struct
	{
		float stepSensableDist;
		float stepTrackableDist;
		float itemSensableDist;
		float itemTrackableDist;
		float timeItemSound;
		float timeStepSound;
		float oldTimeStepSound;
		TextureName textureName;
		char textureType;
	} stepsounds_t;

	void calcStepSound( int clientIndex, edict_t *ent, bool writeResult );
	std::vector<stepsounds_t> player;
};
#endif
