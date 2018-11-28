#pragma once
#if !defined(SOUNDS_H)
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

#define CBTEXTURENAMEMAX	13		// only load first n chars of name

typedef char TextureName[CBTEXTURENAMEMAX];

void	sounds_getAllClientSounds();
void	sounds_parseSound( EDICT *ent, const char *sample, float vol );
void	sounds_parseAmbientSound( EDICT *ent, const char *sample, float vol );

float	sounds_getSensableDist( int clientIndex );
float	sounds_getTrackableDist( int clientIndex );
typedef struct stepsounds {
	float stepSensableDist;
	float stepTrackableDist;
	float itemSensableDist;
	float itemTrackableDist;
	float timeItemSound;
	float timeStepSound;
	float oldTimeStepSound;
	TextureName textureName;
	char textureType;
} STEPSOUNDS;

void	sounds_calcStepSound( int clientIndex, EDICT *ent, bool writeResult );

#endif
