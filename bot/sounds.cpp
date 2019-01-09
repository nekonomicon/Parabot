#include "parabot.h"
#include "dllwrap.h"
#include "sounds.h"
#include "bot.h"
#include "weapon.h"
#include "chat.h"

extern int mod_id;
extern int clientWeapon[32];
extern bool fatalParabotError;
extern NAVPOINT* getNearestNavpoint( EDICT *pEdict );
STEPSOUNDS players[32];

bool headToBunker;
float airStrikeTime;
float nextAirstrikeTime = 500;

#define CHAR_TEX_CONCRETE	'C'			// texture types
#define CHAR_TEX_METAL		'M'
#define CHAR_TEX_DIRT		'D'
#define CHAR_TEX_VENT		'V'
#define CHAR_TEX_GRATE		'G'
#define CHAR_TEX_TILE		'T'
#define CHAR_TEX_SLOSH		'S'
#define CHAR_TEX_WOOD		'W'
#define CHAR_TEX_COMPUTER	'P'
#define CHAR_TEX_GLASS		'Y'
#define CHAR_TEX_GEARBOX_SNOW	'O'
#define CHAR_TEX_CSTRIKE_SNOW	'N'
#define CHAR_TEX_CSTRIKE_GRASS	'X'


void
sounds_getAllClientSounds()
{
	bool writeResult = true;
	
	float hearSteps = footsteps->value;
	if ( mod_id == DMC_DLL ) hearSteps = 0;		// no step sounds in DMC

	EDICT *player = 0;
	for (int i = 1; i <= com.globals->maxclients; i++) {
		player = edictofindex( i );
		if (!player) continue;							// skip invalid players
		if (!is_alive(player)) continue;	// skip player if not alive
		if (player->v.solid == SOLID_NOT) continue;	
		
		// get step sounds
		if (hearSteps > 0) sounds_calcStepSound( i - 1, player, writeResult );
		// get attack sounds
		if ((player->v.button & (ACTION_ATTACK1 | ACTION_ATTACK2))) {
			int wid = clientWeapon[i-1];
			WEAPON w;
			weapon_construct2(&w, wid);
			float sensDist = weapon_getaudibledistance(&w, player->v.button );
			float trackDist = sensDist / 3.0f;
			if (sensDist > players[i - 1].stepSensableDist) players[i - 1].stepSensableDist = sensDist;
			if (trackDist > players[i - 1].stepTrackableDist) players[i - 1].stepTrackableDist = trackDist;
		}
		// get jump sounds
		if ( mod_id == HOLYWARS_DLL || mod_id == DMC_DLL ) {
			if ( (player->v.button & ACTION_JUMP) && !is_underwater(player) ) {
				float sensDist = 300;
				float trackDist = 150;
				if (sensDist > players[i - 1].stepSensableDist) players[i - 1].stepSensableDist = sensDist;
				if (trackDist > players[i - 1].stepTrackableDist) players[i - 1].stepTrackableDist = trackDist;
			}
		}
		// get reload sounds
		if ( player->v.button & ACTION_RELOAD ) {
			float sensDist = 200;
			float trackDist = 100;
			if (sensDist > players[i - 1].stepSensableDist) players[i - 1].stepSensableDist = sensDist;
			if (trackDist > players[i - 1].stepTrackableDist) players[i - 1].stepTrackableDist = trackDist;
		}
	}
}


void sounds_calcStepSound( int clientIndex, EDICT *player, bool writeResult )
// calculates the volume of step sounds for given player
// if writeResult is true the method sets the values in CBasePlayer, otherwise it just
// reads them out
{
	int	fWalking;
	float fvol = 0;
	char szbuffer[64];
	const char *pTextureName;
	Vec3D start, end;
	Vec3D knee;
	Vec3D feet;
	Vec3D center;
	float height;
	float speed;
	float velrun;
	//float velwalk;
	float flduck;
	int	fLadder;
	//int step;
	float sensableDist = 0;
	float trackableDist = 0;

	if (writeResult) {
		if ( worldtime() <= players[clientIndex].timeStepSound ) {
			players[clientIndex].stepSensableDist = 0;
			players[clientIndex].stepTrackableDist = 0;
			// check for mapchange -> start playing sounds
			if ( (players[clientIndex].timeStepSound - worldtime()) > 1.0 ) 
				players[clientIndex].timeStepSound = worldtime();
			else
				return;
		}
	} else {
		if ( players[clientIndex].timeStepSound == players[clientIndex].oldTimeStepSound ) {
			players[clientIndex].stepSensableDist = 0;
			players[clientIndex].stepTrackableDist = 0;
			return;
		} else
			players[clientIndex].oldTimeStepSound = players[clientIndex].timeStepSound;
	}

	speed = vlen(&player->v.velocity);

	// determine if we are on a ladder
	fLadder = is_onladder(player);

	// UNDONE: need defined numbers for run, walk, crouch, crouch run velocities!!!!	
	if ((player->v.flags & FL_DUCKING) || fLadder) {
		//velwalk = 60;		// These constants should be based on cl_movespeedkey * cl_forwardspeed somehow
		velrun = 80;		// UNDONE: Move walking to server
		flduck = 0.1;
	} else {
		//velwalk = 120;
		velrun = 210;
		flduck = 0.0;
	}

	// ALERT (at_console, "vel: %f\n", vlen(vecVel));
	
	// if we're on a ladder or on the ground, and we're moving fast enough,
	// play step sound.  Also, if m_flTimeStepSound is zero, get the new
	// sound right away - we just started moving in new level.

	if ((fLadder || (player->v.flags & FL_ONGROUND)) && (speed >= velrun))
	{
		fWalking = speed < velrun;		

		boxcenter(player, &feet);
		knee = feet;
		center = feet;
		height = player->v.absmax.z - player->v.absmin.z;

		knee.z = player->v.absmin.z + height * 0.2f;
		feet.z = player->v.absmin.z;

		// find out what we're stepping in or on...
		if (fLadder) {
			//step = STEP_LADDER;
			fvol = 0.35;
			sensableDist = 600;
			trackableDist = 400;
			if (writeResult) players[clientIndex].timeStepSound = com.globals->time + 0.35f;
		} else if (pointcontents(&knee) == CONTENTS_WATER) {
			//step = STEP_WADE;
			fvol = 0.65;
			sensableDist = 200;
			trackableDist = 200;
			if (writeResult) players[clientIndex].timeStepSound = com.globals->time + 0.6;
		} else if (pointcontents(&feet) == CONTENTS_WATER) {
			//step = STEP_SLOSH;
			fvol = fWalking ? 0.2 : 0.5;
			sensableDist = 200;
			trackableDist = 200;
			if (writeResult)
				players[clientIndex].timeStepSound = fWalking ? com.globals->time + 0.4 : com.globals->time + 0.3;		
		} else {
			if (writeResult) {
				// find texture under player, if different from current texture, 
				// get material type

				start = center;
				end = center;
				start.z = end.z = player->v.absmin.z;				// copy zmin
				start.z += 4.0;									// extend start up
				end.z -= 24.0;									// extend end down

				pTextureName = trace_texture(player->v.groundentity, &start, &end);
				if (pTextureName) {
					// strip leading '-0' or '{' or '!'
					if (*pTextureName == '-')
						pTextureName += 2;
					if (*pTextureName == '{' || *pTextureName == '!')
						pTextureName++;
					
					if (_strnicmp(pTextureName, players[clientIndex].textureName, CBTEXTURENAMEMAX - 1)) {
						// current texture is different from texture player is on...
						// set current texture
						strcpy(szbuffer, pTextureName);
						szbuffer[CBTEXTURENAMEMAX - 1] = 0;
						strcpy(players[clientIndex].textureName, szbuffer);
						
						// DEBUG_MSG( "texture: %s\n", player[clientIndex].textureName );

						// get texture type
						players[clientIndex].textureType = find_texturetype( players[clientIndex].textureName );	
					}
				}
			}
			
			switch (players[clientIndex].textureType)
			{
			default:
			case CHAR_TEX_CONCRETE:
				//step = STEP_CONCRETE;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 200;
				trackableDist = 100;
				if (writeResult) players[clientIndex].timeStepSound = fWalking ? com.globals->time + 0.4 : com.globals->time + 0.3;
				break;

			case CHAR_TEX_METAL:	
				//step = STEP_METAL;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 600;
				trackableDist = 300;
				if (writeResult) players[clientIndex].timeStepSound = fWalking ? com.globals->time + 0.4 : com.globals->time + 0.3;
				break;

			case CHAR_TEX_DIRT:	
				//step = STEP_DIRT;
				fvol = fWalking ? 0.25 : 0.55;
				sensableDist = 200;
				trackableDist = 100;
				if (writeResult) players[clientIndex].timeStepSound = fWalking ? com.globals->time + 0.4 : com.globals->time + 0.3;
				break;

			case CHAR_TEX_VENT:	
				//step = STEP_V;
				fvol = fWalking ? 0.4 : 0.7;
				sensableDist = 600;
				trackableDist = 400;
				if (writeResult) players[clientIndex].timeStepSound = fWalking ? com.globals->time + 0.4 : com.globals->time + 0.3;
				break;

			case CHAR_TEX_GRATE:
				//step = STEP_GRATE;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 600;
				trackableDist = 400;
				if (writeResult) players[clientIndex].timeStepSound = fWalking ? com.globals->time + 0.4 : com.globals->time + 0.3;
				break;

			case CHAR_TEX_TILE:	
				//step = STEP_TILE;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 200;
				trackableDist = 100;
				if (writeResult) players[clientIndex].timeStepSound = fWalking ? com.globals->time + 0.4 : com.globals->time + 0.3;
				break;

			case CHAR_TEX_SLOSH:
				//step = STEP_SLOSH;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 200;
				trackableDist = 200;
				if (writeResult) players[clientIndex].timeStepSound = fWalking ? com.globals->time + 0.4 : com.globals->time + 0.3;
				break;
			}
		}

		if (writeResult)
			players[clientIndex].timeStepSound += flduck; // slower step time if ducking
	
		// 35% volume if ducking
		//if ( player->v.flags & FL_DUCKING ) fvol *= 0.35;
		
		/*char *name = (char*) STRING( player->v.netname );
		DEBUG_MSG( "%.1f: %s on %s (Code %i) -> vol %.2f\n", worldtime(),
		    name, players[clientIndex].textureName, step, fvol );*/
	}

	players[clientIndex].stepSensableDist = sensableDist;
	players[clientIndex].stepTrackableDist = trackableDist;
}

// weapons and ammo
#define AMMO_SENS_DIST			400
#define AMMO_TRACK_DIST			200
// chargers
#define LOAD_SENS_DIST		   1200
#define LOAD_TRACK_DIST			800
// platforms
#define LIFT_SENS_DIST			900
#define LIFT_TRACK_DIST			600
// healthpacks and armor
#define ITEM_SENS_DIST			600
#define ITEM_TRACK_DIST			400
// longjump, quad-damage etc.
#define SPEC_ITEM_SENS_DIST	   1200
#define SPEC_ITEM_TRACK_DIST   1200


void sounds_parseSound( EDICT *player, const char *sample, float vol )
{
	if (fatalParabotError) return;

	int clientIndex;

	switch( mod_id ) {
	default:
		if ( Q_STREQ( sample, "items/gunpickup2.wav" ) ) {		// weapon pickup
			clientIndex = indexofedict( player ) - 1;
			players[clientIndex].itemSensableDist = AMMO_SENS_DIST;
			players[clientIndex].itemTrackableDist = AMMO_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 0.3;
			//DEBUG_MSG( "%s picked up gun!\n", STRING(player->v.netname) );
			NAVPOINT *nearest = getNearestNavpoint( player );
			//assert( nearest != 0 );
			if (!nearest) return;	// listen server before map is loaded
			const char *wpnName = navpoint_classname(nearest);
			if (mod_id==VALVE_DLL || mod_id==AG_DLL || mod_id==HUNGER_DLL || mod_id==GEARBOX_DLL) {
				if ( Q_STREQ( wpnName, "weapon_rpg"      ) ||
					 Q_STREQ( wpnName, "weapon_gauss"    ) ||
					 Q_STREQ( wpnName, "weapon_egon"     ) ||
					 Q_STREQ( wpnName, "weapon_crossbow" ) ) chat_registergotweapon( player, wpnName );
			} else {	// Holy Wars
				chat_registergotweapon( player, wpnName );
			}
		} else if ( Q_STREQ( sample, "items/9mmclip1.wav" ) ) {	// ammo pickup
			clientIndex = getnearestplayerindex(&player->v.origin) - 1;
			//ent = edictofindex( clientIndex+1 );
			players[clientIndex].itemSensableDist = AMMO_SENS_DIST;
			players[clientIndex].itemTrackableDist = AMMO_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 0.3;
			//DEBUG_MSG( "%s picked up ammo!\n", STRING(player->v.netname) );
		} else if ( Q_STREQ( sample, "doors/doorstop6.wav" ) ) {	// lift usage
			Vec3D pos;
			boxcenter(player, &pos);
			pos.z = player->v.absmax.z;
			clientIndex = getnearestplayerindex(&pos) - 1;
			//ent = edictofindex( clientIndex+1 );
			players[clientIndex].itemSensableDist = LIFT_SENS_DIST;
			players[clientIndex].itemTrackableDist = LIFT_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 0.3;
			//DEBUG_MSG( "%s used lift!\n", STRING(player->v.netname) );
		} else if ( Q_STREQ( sample, "items/suitcharge1.wav" ) ) {// battery charger
			Vec3D pos;
			boxcenter(player, &pos);  
			clientIndex = getnearestplayerindex(&pos) - 1;
			//ent = edictofindex( clientIndex+1 );
			if (vol > 0) {
				players[clientIndex].itemSensableDist = LOAD_SENS_DIST;
				players[clientIndex].itemTrackableDist = LOAD_TRACK_DIST;
				players[clientIndex].timeItemSound = worldtime() + 10.0;
				//DEBUG_MSG( "%s started using suitcharger!\n", STRING(ent->v.netname) );
			} else {
				players[clientIndex].itemSensableDist = 0;
				players[clientIndex].itemTrackableDist = 0;
				players[clientIndex].timeItemSound = 0;
				//DEBUG_MSG( "%s stopped using suitcharger!\n", STRING(ent->v.netname) );
			}
		} else if ( Q_STREQ( sample, "items/medcharge4.wav" ) ) {	// med charger
			Vec3D pos;
			boxcenter(player, &pos);  
			clientIndex = getnearestplayerindex(&pos) - 1;
			//ent = edictofindex( clientIndex+1 );
			if (vol > 0) {
				players[clientIndex].itemSensableDist = LOAD_SENS_DIST;
				players[clientIndex].itemTrackableDist = LOAD_TRACK_DIST;
				players[clientIndex].timeItemSound = worldtime() + 10.0;
				//DEBUG_MSG( "%s started using healthcharger!\n", STRING(player->v.netname) );
			} else {
				players[clientIndex].itemSensableDist = 0;
				players[clientIndex].itemTrackableDist = 0;
				players[clientIndex].timeItemSound = 0;
				//DEBUG_MSG( "%s stopped using healthcharger!\n", STRING(player->v.netname) );
			}
		} else if ( Q_STREQ( sample, "items/smallmedkit1.wav" ) ) {	// medkit pickup
			clientIndex = indexofedict( player ) - 1;
			players[clientIndex].itemSensableDist = ITEM_SENS_DIST;
			players[clientIndex].itemTrackableDist = ITEM_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 0.3;
			//DEBUG_MSG( "%s picked up medkit!\n", STRING(player->v.netname) );
		} else if ( Q_STREQ( sample, "!336" ) ) {						// longjump pickup
			clientIndex = indexofedict( player ) - 1;
			players[clientIndex].itemSensableDist = SPEC_ITEM_SENS_DIST;
			players[clientIndex].itemTrackableDist = SPEC_ITEM_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 3.0;
			DEBUG_MSG( "%s picked up longjump!\n", STRING(player->v.netname) );
		} else if ( Q_STREQ( sample, "misc/jumppad.wav" ) ) {			// HW-jumppad 
			clientIndex = getnearestplayerindex(&player->v.origin) - 1;
			//ent = edictofindex( clientIndex+1 );
			players[clientIndex].itemSensableDist = LIFT_SENS_DIST;
			players[clientIndex].itemTrackableDist = LIFT_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 0.3;
			// DEBUG_MSG( "%s used HW-jumppad!\n", STRING(player->v.netname) );
		} else if ( Q_STREQ( sample, "player/drink3.wav" ) ) {		// HW-bottle
			clientIndex = indexofedict( player ) - 1;
			players[clientIndex].itemSensableDist = ITEM_SENS_DIST;
			players[clientIndex].itemTrackableDist = ITEM_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 0.3;
			// DEBUG_MSG( "%s picked up HW-bottle!\n", STRING(player->v.netname) );
		}/*else {
			if (player) DEBUG_MSG( "%s caused %s!\n", STRING(player->v.classname), sample);
			else DEBUG_MSG( "NullEnt caused %s!\n", sample );
		}*/
		break;

	case DMC_DLL:
		if ( Q_STREQ( sample, "common/null.wav" ) ||
			 Q_STREQ( sample, "player/lburn1.wav" ) ||
			 Q_STREQ( sample, "player/lburn2.wav" ) ||
			 Q_STREQ( sample, "items/itembk2.wav" ) ) {	// ignore
		} else if (Q_STREQ( sample, "weapons/pkup.wav" ) ||		// weapon pickup
			    Q_STREQ( sample, "weapons/lock4.wav" ) ) {		// ammo pickup
			const char *wpnName = STRING( player->v.classname );
			clientIndex = getnearestplayerindex(&player->v.origin) - 1;
			player = edictofindex( clientIndex+1 );
			players[clientIndex].itemSensableDist = AMMO_SENS_DIST;
			players[clientIndex].itemTrackableDist = AMMO_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 0.3;
			// DEBUG_MSG( "%s picked up gun or ammo: %s\n", STRING(player->v.netname), wpnName );
			if ( Q_STREQ( wpnName, "weapon_lightning" ) ||
				 Q_STREQ( wpnName, "weapon_rocketlauncher" ) ||
				 Q_STREQ( wpnName, "weapon_grenadelauncher" ) ||
				 Q_STREQ( wpnName, "weapon_supernailgun" ) ) chat_registergotweapon( player, wpnName );
		} else if ( Q_STREQ( sample, "plats/freightmove2.wav" ) ) {	// lift usage
			Vec3D pos;
			boxcenter(player, &pos);  
			pos.z = player->v.absmax.z;
			clientIndex = getnearestplayerindex(&pos) - 1;
			//ent = edictofindex( clientIndex+1 );
			players[clientIndex].itemSensableDist = LIFT_SENS_DIST;
			players[clientIndex].itemTrackableDist = LIFT_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 0.3;
			// DEBUG_MSG( "%s used lift!\n", STRING(player->v.netname) );
		} else if ( Q_STREQ( sample, "items/button4.wav" ) ) {	// button usage
			clientIndex = getnearestplayerindex(&player->v.origin) - 1;
			//player = edictofindex( clientIndex+1 );
			players[clientIndex].itemSensableDist = LIFT_SENS_DIST;
			players[clientIndex].itemTrackableDist = LIFT_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 0.3;
			// DEBUG_MSG( "%s pressed button!\n", STRING(player->v.netname) );
		} else if ( Q_STREQ( sample, "items/health1.wav" ) ||
				  Q_STREQ( sample, "items/r_item2.wav" ) ) {	// medkit or armor pickup
			clientIndex = getnearestplayerindex(&player->v.origin) - 1;
			//player = edictofindex( clientIndex+1 );
			players[clientIndex].itemSensableDist = ITEM_SENS_DIST;
			players[clientIndex].itemTrackableDist = ITEM_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 0.3;
			// DEBUG_MSG( "%s picked up medkit or armor!\n", STRING(player->v.netname) );
		} else if ( Q_STREQ( sample, "items/armor1.wav" ) ) {	// armor pickup
			clientIndex = indexofedict( player ) - 1;
			players[clientIndex].itemSensableDist = ITEM_SENS_DIST;
			players[clientIndex].itemTrackableDist = ITEM_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 0.3;
			// DEBUG_MSG( "%s picked up armor!\n", STRING(player->v.netname) );
		} else if ( Q_STREQ( sample, "items/damage.wav" ) ) {			// quad-damage
			clientIndex = getnearestplayerindex(&player->v.origin) - 1;
			//player = edictofindex( clientIndex+1 );
			players[clientIndex].itemSensableDist = SPEC_ITEM_SENS_DIST;
			players[clientIndex].itemTrackableDist = SPEC_ITEM_TRACK_DIST;
			players[clientIndex].timeItemSound = worldtime() + 2.0;
			// DEBUG_MSG( "%s picked up quad-damage!\n", STRING(player->v.netname) );
		}/*else {
			if (player) DEBUG_MSG( "%s caused %s!\n", STRING(player->v.classname), sample );
			else DEBUG_MSG( "NullEnt caused %s!\n", sample );
		}*/
		break;
	}
}

void sounds_parseAmbientSound( EDICT *player, const char *sample, float vol )
{
	if (fatalParabotError) return;

	int clientIndex;

	if ( Q_STREQ( sample, "ambience/siren.wav" ) ) {
		headToBunker = true;
		// 4 to 14 minutes silence...
		nextAirstrikeTime = worldtime() + 240.0 + randomfloat( 0.0f, 600.0f );
	} else if ( Q_STREQ( sample, "weapons/mortarhit.wav")) {
		airStrikeTime = worldtime() + 1.0f;
	} else if ( Q_STREQ( sample, "debris/beamstart2.wav") ) {
		Vec3D pos;
		boxcenter(player, &pos);
		clientIndex = getnearestplayerindex(&pos) - 1;
		player = edictofindex( clientIndex+1 );
		if (!player) return;
		players[clientIndex].itemSensableDist = 800;
		players[clientIndex].itemTrackableDist = 800;
		players[clientIndex].timeItemSound = worldtime() + 0.3f;
		// DEBUG_MSG( "%s used teleporter!\n", STRING(player->v.netname) );
	}/*else {
		if (player) DEBUG_MSG( "%s caused %s!\n", STRING(player->v.classname), sample );
		else DEBUG_MSG( "NullEnt caused %s!\n", sample );
	}*/

}

float sounds_getSensableDist( int clientIndex )
{
	clientIndex--;	// array start with 0, index with 1

	float vol = players[clientIndex].stepSensableDist;
	float itemPlayTime = players[clientIndex].timeItemSound - worldtime();
	if (itemPlayTime > 0) {
		// check for mapchange
		if (itemPlayTime > 20)
			players[clientIndex].timeItemSound = 0;

		if (players[clientIndex].itemSensableDist > vol)
			vol = players[clientIndex].itemSensableDist;
	}
	return vol;
}

float sounds_getTrackableDist( int clientIndex )
{
	clientIndex--;	// array start with 0, index with 1

	float vol = players[clientIndex].stepTrackableDist;
	float itemPlayTime = players[clientIndex].timeItemSound - worldtime();
	if ( itemPlayTime > 0 ) {
		// check for mapchange
		if ( itemPlayTime > 20 ) players[clientIndex].timeItemSound = 0;
		if (players[clientIndex].itemTrackableDist > vol) vol = players[clientIndex].itemTrackableDist;
	}
	return vol;
}
