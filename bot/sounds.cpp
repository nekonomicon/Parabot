#include "sounds.h"
#include "bot.h"
#include "pb_weapon.h"
#include "pb_chat.h"
#include "pb_mapgraph.h"


extern int mod_id;
extern int clientWeapon[32];
extern PB_MapGraph mapGraph;	// mapgraph for waypoints
extern bool fatalParabotError;
extern PB_Chat chat;
extern PB_Navpoint* getNearestNavpoint( edict_t *pEdict );

bool headToBunker = false;
float airStrikeTime = 0;
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
#define CHAR_TEX_FLESH		'F'

bool UTIL_IsOnLadder( edict_t *ent )
{
	return (ent->v.movetype == MOVETYPE_FLY);
}

void Sounds::init()
{
	stepsounds_t nullStruct = {0};
	static bool firstInit = true;

	if( !firstInit )
		player.clear();
	else
		firstInit = false;

	player = std::vector<stepsounds_t>( 32, nullStruct );
	//player = std::vector<stepsounds_t>( gpGlobals->maxClients, nullStruct );
}

void Sounds::getAllClientSounds()
{
	bool writeResult = true;
	
	float hearSteps = CVAR_GET_FLOAT( "mp_footsteps" );
	if ( mod_id == DMC_DLL ) hearSteps = 0;		// no step sounds in DMC

	edict_t *pPlayer = 0;
	for (int i=1; i<=gpGlobals->maxClients; i++) {
		pPlayer = INDEXENT( i );
		if (!pPlayer) continue;							// skip invalid players
		if (!isAlive( ENT(pPlayer) )) continue;	// skip player if not alive
		if (pPlayer->v.solid == SOLID_NOT) continue;	
		
		// get step sounds
		if (hearSteps > 0) calcStepSound( i-1, pPlayer, writeResult );
		// get attack sounds
		if ( (pPlayer->v.button & (IN_ATTACK|IN_ATTACK2)) ) {
			int wid = clientWeapon[i-1];
			PB_Weapon w( wid );
			float sensDist = w.getAudibleDistance( pPlayer->v.button );
			float trackDist = sensDist/3;
			if (sensDist > player[i - 1].stepSensableDist) player[i - 1].stepSensableDist = sensDist;
			if (trackDist > player[i - 1].stepTrackableDist) player[i - 1].stepTrackableDist = trackDist;
		}
		// get jump sounds
		if ( mod_id==HOLYWARS_DLL || mod_id==DMC_DLL ) {
			if ( (pPlayer->v.button & IN_JUMP) && !isUnderwater( ENT(pPlayer) ) ) {
				float sensDist = 300;
				float trackDist = 150;
				if (sensDist > player[i - 1].stepSensableDist) player[i - 1].stepSensableDist = sensDist;
				if (trackDist > player[i - 1].stepTrackableDist) player[i - 1].stepTrackableDist = trackDist;
			}
		}
		// get reload sounds
		if ( pPlayer->v.button & IN_RELOAD ) {
			float sensDist = 200;
			float trackDist = 100;
			if (sensDist > player[i - 1].stepSensableDist) player[i - 1].stepSensableDist = sensDist;
			if (trackDist > player[i - 1].stepTrackableDist) player[i - 1].stepTrackableDist = trackDist;
		}
	}
}


void Sounds::calcStepSound( int clientIndex, edict_t *ent, bool writeResult )
// calculates the volume of step sounds for given player
// if writeResult is true the method sets the values in CBasePlayer, otherwise it just
// reads them out
{
	int	fWalking;
	float fvol = 0;
	char szbuffer[64];
	const char *pTextureName;
	Vector start, end;
	float rgfl1[3];
	float rgfl2[3];
	Vector knee;
	Vector feet;
	Vector center;
	float height;
	float speed;
	float velrun;
	float velwalk;
	float flduck;
	int	fLadder;
	int step;
	float sensableDist = 0;
	float trackableDist = 0;


	//CBasePlayer *pl = (CBasePlayer*) GET_PRIVATE( ent );

	if (writeResult) {
		if ( worldTime() <= player[clientIndex].timeStepSound ) {
			player[clientIndex].stepSensableDist = 0;
			player[clientIndex].stepTrackableDist = 0;
			// check for mapchange -> start playing sounds
			if ( (player[clientIndex].timeStepSound - worldTime()) > 1.0 ) 
				player[clientIndex].timeStepSound = worldTime();
			else return;
		}
	}
	else {
		if ( player[clientIndex].timeStepSound == player[clientIndex].oldTimeStepSound ) {
			player[clientIndex].stepSensableDist = 0;
			player[clientIndex].stepTrackableDist = 0;
			return;
		}
		else player[clientIndex].oldTimeStepSound = player[clientIndex].timeStepSound;
	}

	speed = ent->v.velocity.Length();

	// determine if we are on a ladder
	fLadder = UTIL_IsOnLadder( ent );

	// UNDONE: need defined numbers for run, walk, crouch, crouch run velocities!!!!	
	if (FBitSet(ent->v.flags, FL_DUCKING) || fLadder)
	{
		//velwalk = 60;		// These constants should be based on cl_movespeedkey * cl_forwardspeed somehow
		velrun = 80;		// UNDONE: Move walking to server
		flduck = 0.1;
	}
	else
	{
		//velwalk = 120;
		velrun = 210;
		flduck = 0.0;
	}

	// ALERT (at_console, "vel: %f\n", vecVel.Length());
	
	// if we're on a ladder or on the ground, and we're moving fast enough,
	// play step sound.  Also, if m_flTimeStepSound is zero, get the new
	// sound right away - we just started moving in new level.

	if ((fLadder || FBitSet (ent->v.flags, FL_ONGROUND)) && (speed >= velrun))
	{
		fWalking = speed < velrun;		

		center = knee = feet = (ent->v.absmin + ent->v.absmax) * 0.5;
		height = ent->v.absmax.z - ent->v.absmin.z;

		knee.z = ent->v.absmin.z + height * 0.2;
		feet.z = ent->v.absmin.z;

		// find out what we're stepping in or on...
		if (fLadder)
		{
			//step = STEP_LADDER;
			fvol = 0.35;
			sensableDist = 600;
			trackableDist = 400;
			if (writeResult) player[clientIndex].timeStepSound = gpGlobals->time + 0.35;
		}
		else if ( UTIL_PointContents ( knee ) == CONTENTS_WATER )
		{
			//step = STEP_WADE;
			fvol = 0.65;
			sensableDist = 200;
			trackableDist = 200;
			if (writeResult) player[clientIndex].timeStepSound = gpGlobals->time + 0.6;
		}
		else if (UTIL_PointContents ( feet ) == CONTENTS_WATER )
		{
			//step = STEP_SLOSH;
			fvol = fWalking ? 0.2 : 0.5;
			sensableDist = 200;
			trackableDist = 200;
			if (writeResult) player[clientIndex].timeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;		
		}
		else
		{	
			if (writeResult) {
				// find texture under player, if different from current texture, 
				// get material type
				
				start = end = center;							// center point of player BB
				start.z = end.z = ent->v.absmin.z;				// copy zmin
				start.z += 4.0;									// extend start up
				end.z -= 24.0;									// extend end down
				
				start.CopyToArray( rgfl1 );
				end.CopyToArray( rgfl2 );
				
				pTextureName = TRACE_TEXTURE( ENT( ent->v.groundentity), rgfl1, rgfl2 );
				if ( pTextureName )
				{
					// strip leading '-0' or '{' or '!'
					if (*pTextureName == '-')
						pTextureName += 2;
					if (*pTextureName == '{' || *pTextureName == '!')
						pTextureName++;
					
					if (_strnicmp(pTextureName, player[clientIndex].textureName, CBTEXTURENAMEMAX-1))
					{
						// current texture is different from texture player is on...
						// set current texture
						strcpy(szbuffer, pTextureName);
						szbuffer[CBTEXTURENAMEMAX - 1] = 0;
						strcpy(player[clientIndex].textureName, szbuffer);
						
						//debugMsg( "texture: ", player[clientIndex].textureName, "\n" );
						
						// get texture type
						player[clientIndex].textureType = MDLL_PM_FindTextureType( player[clientIndex].textureName );	
					}
				}
			}
			
			switch (player[clientIndex].textureType)
			{
			default:
			case CHAR_TEX_CONCRETE:
				//step = STEP_CONCRETE;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 200;
				trackableDist = 100;
				if (writeResult) player[clientIndex].timeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_METAL:	
				//step = STEP_METAL;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 600;
				trackableDist = 300;
				if (writeResult) player[clientIndex].timeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_DIRT:	
				//step = STEP_DIRT;
				fvol = fWalking ? 0.25 : 0.55;
				sensableDist = 200;
				trackableDist = 100;
				if (writeResult) player[clientIndex].timeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_VENT:	
				//step = STEP_VENT;
				fvol = fWalking ? 0.4 : 0.7;
				sensableDist = 600;
				trackableDist = 400;
				if (writeResult) player[clientIndex].timeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_GRATE:
				//step = STEP_GRATE;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 600;
				trackableDist = 400;
				if (writeResult) player[clientIndex].timeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_TILE:	
				//step = STEP_TILE;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 200;
				trackableDist = 100;
				if (writeResult) player[clientIndex].timeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_SLOSH:
				//step = STEP_SLOSH;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 200;
				trackableDist = 200;
				if (writeResult) player[clientIndex].timeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;
			}
		}

		if (writeResult) player[clientIndex].timeStepSound += flduck; // slower step time if ducking
	
		// 35% volume if ducking
		//if ( ent->v.flags & FL_DUCKING ) fvol *= 0.35;
		
		/*char *name = (char*) STRING( ent->v.netname );
		debugMsg( "%.1f: ", worldTime() );
		debugMsg( name, " on ", textureName[clientIndex] );
		debugMsg( " (Code %i)", step );
		debugMsg( " -> vol %.2f\n", fvol );*/
	}

	player[clientIndex].stepSensableDist = sensableDist;
	player[clientIndex].stepTrackableDist = trackableDist;
}

int UTIL_GetNearestPlayerIndex( Vector &pos )
{
	float dist, bestDist = 10000;
	int	  bestPlayer = 0;
	edict_t *pPlayer = 0;

	for (int i=1; i<=gpGlobals->maxClients; i++) {
		pPlayer = INDEXENT( i );
		if (!pPlayer) continue;							// skip invalid players
		if (!isAlive( ENT(pPlayer) )) continue;	// skip player if not alive
		if (pPlayer->v.solid == SOLID_NOT) continue;	
		
		dist = (pPlayer->v.origin - pos).Length();
		if (dist < bestDist) {
			bestDist = dist;
			bestPlayer = i;
		}
	}
	return bestPlayer;
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


void Sounds::parseSound( edict_t *ent, const char *sample, float vol )
{
	if (fatalParabotError) return;

	int clientIndex;

	switch( mod_id ) {
	case AG_DLL:
	case HUNGER_DLL:
	case VALVE_DLL:
	case GEARBOX_DLL:
	case HOLYWARS_DLL:
		if ( FStrEq( sample, "items/gunpickup2.wav" ) ) {		// weapon pickup
			clientIndex = ENTINDEX( ent ) - 1;
			player[clientIndex].itemSensableDist = AMMO_SENS_DIST;
			player[clientIndex].itemTrackableDist = AMMO_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up gun!\n" );
			PB_Navpoint *nearest = getNearestNavpoint( ent );
			//assert( nearest != 0 );
			if (!nearest) return;	// listen server before map is loaded
			const char *wpnName = nearest->classname();
			if (mod_id==VALVE_DLL || mod_id==AG_DLL || mod_id==HUNGER_DLL || mod_id==GEARBOX_DLL) {
				if ( FStrEq( wpnName, "weapon_rpg"      ) ||
					 FStrEq( wpnName, "weapon_gauss"    ) ||
					 FStrEq( wpnName, "weapon_egon"     ) ||
					 FStrEq( wpnName, "weapon_crossbow" ) ) chat.registerGotWeapon( ent, wpnName );
			}
			else {	// Holy Wars
				chat.registerGotWeapon( ent, wpnName );
			}
		}
		else if ( FStrEq( sample, "items/9mmclip1.wav" ) ) {	// ammo pickup
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			//ent = INDEXENT( clientIndex+1 );
			player[clientIndex].itemSensableDist = AMMO_SENS_DIST;
			player[clientIndex].itemTrackableDist = AMMO_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up ammo!\n" );
		}
		else if ( FStrEq( sample, "doors/doorstop6.wav" ) ) {	// lift usage
			Vector pos = 0.5 * (ent->v.absmin + ent->v.absmax);  
			pos.z = ent->v.absmax.z;
			clientIndex = UTIL_GetNearestPlayerIndex( pos ) - 1;
			//ent = INDEXENT( clientIndex+1 );
			player[clientIndex].itemSensableDist = LIFT_SENS_DIST;
			player[clientIndex].itemTrackableDist = LIFT_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " used lift!\n" );
		}
		else if ( FStrEq( sample, "items/suitcharge1.wav" ) ) {// battery charger
			Vector pos = 0.5 * (ent->v.absmin + ent->v.absmax);  
			clientIndex = UTIL_GetNearestPlayerIndex( pos ) - 1;
			//ent = INDEXENT( clientIndex+1 );
			if (vol > 0) {
				player[clientIndex].itemSensableDist = LOAD_SENS_DIST;
				player[clientIndex].itemTrackableDist = LOAD_TRACK_DIST;
				player[clientIndex].timeItemSound = worldTime() + 10.0;
				//debugMsg( STRING(ent->v.netname), " started using suitcharger!\n" );
			}
			else {
				player[clientIndex].itemSensableDist = 0;
				player[clientIndex].itemTrackableDist = 0;
				player[clientIndex].timeItemSound = 0;
				//debugMsg( STRING(ent->v.netname), " stopped using suitcharger!\n" );
			}
		}
		else if ( FStrEq( sample, "items/medcharge4.wav" ) ) {	// med charger
			Vector pos = 0.5 * (ent->v.absmin + ent->v.absmax);  
			clientIndex = UTIL_GetNearestPlayerIndex( pos ) - 1;
			//ent = INDEXENT( clientIndex+1 );
			if (vol > 0) {
				player[clientIndex].itemSensableDist = LOAD_SENS_DIST;
				player[clientIndex].itemTrackableDist = LOAD_TRACK_DIST;
				player[clientIndex].timeItemSound = worldTime() + 10.0;
				//debugMsg( STRING(ent->v.netname), " started using healthcharger!\n" );
			}
			else {
				player[clientIndex].itemSensableDist = 0;
				player[clientIndex].itemTrackableDist = 0;
				player[clientIndex].timeItemSound = 0;
				//debugMsg( STRING(ent->v.netname), " stopped using healthcharger!\n" );
			}
		}
		else if ( FStrEq( sample, "items/smallmedkit1.wav" ) ) {	// medkit pickup
			clientIndex = ENTINDEX( ent ) - 1;
			player[clientIndex].itemSensableDist = ITEM_SENS_DIST;
			player[clientIndex].itemTrackableDist = ITEM_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up medkit!\n" );
		}
		else if ( FStrEq( sample, "!336" ) ) {						// longjump pickup
			clientIndex = ENTINDEX( ent ) - 1;
			player[clientIndex].itemSensableDist = SPEC_ITEM_SENS_DIST;
			player[clientIndex].itemTrackableDist = SPEC_ITEM_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 3.0;
			debugMsg( STRING(ent->v.netname), " picked up longjump!\n" );
		}
		else if ( FStrEq( sample, "misc/jumppad.wav" ) ) {			// HW-jumppad 
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			//ent = INDEXENT( clientIndex+1 );
			player[clientIndex].itemSensableDist = LIFT_SENS_DIST;
			player[clientIndex].itemTrackableDist = LIFT_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " used HW-jumppad!\n" );
		}
		else if ( FStrEq( sample, "player/drink3.wav" ) ) {		// HW-bottle
			clientIndex = ENTINDEX( ent ) - 1;
			player[clientIndex].itemSensableDist = ITEM_SENS_DIST;
			player[clientIndex].itemTrackableDist = ITEM_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up HW-bottle!\n" );
		}
		/*else {
			if (ent) debugMsg( STRING(ent->v.classname), " caused ", sample, " !\n" );
			else debugMsg( "NullEnt caused ", sample, " !\n" );
		}*/
		break;

	case DMC_DLL:
		if ( FStrEq( sample, "common/null.wav" ) ||
			 FStrEq( sample, "player/lburn1.wav" ) ||
			 FStrEq( sample, "player/lburn2.wav" ) ||
			 FStrEq( sample, "items/itembk2.wav" ) ) {	// ignore
		}
		else if ( FStrEq( sample, "weapons/pkup.wav" ) ||		// weapon pickup
				  FStrEq( sample, "weapons/lock4.wav" ) ) {		// ammo pickup
			const char *wpnName = STRING( ent->v.classname );
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			ent = INDEXENT( clientIndex+1 );
			player[clientIndex].itemSensableDist = AMMO_SENS_DIST;
			player[clientIndex].itemTrackableDist = AMMO_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up gun or ammo: ", wpnName, "\n" );
			if ( FStrEq( wpnName, "weapon_lightning" ) ||
				 FStrEq( wpnName, "weapon_rocketlauncher" ) ||
				 FStrEq( wpnName, "weapon_grenadelauncher" ) ||
				 FStrEq( wpnName, "weapon_supernailgun" ) ) chat.registerGotWeapon( ent, wpnName );
		}
		else if ( FStrEq( sample, "plats/freightmove2.wav" ) ) {	// lift usage
			Vector pos = 0.5 * (ent->v.absmin + ent->v.absmax);  
			pos.z = ent->v.absmax.z;
			clientIndex = UTIL_GetNearestPlayerIndex( pos ) - 1;
			//ent = INDEXENT( clientIndex+1 );
			player[clientIndex].itemSensableDist = LIFT_SENS_DIST;
			player[clientIndex].itemTrackableDist = LIFT_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " used lift!\n" );
		}
		else if ( FStrEq( sample, "items/button4.wav" ) ) {	// button usage
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			//ent = INDEXENT( clientIndex+1 );
			player[clientIndex].itemSensableDist = LIFT_SENS_DIST;
			player[clientIndex].itemTrackableDist = LIFT_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " pressed button!\n" );
		}
		else if ( FStrEq( sample, "items/health1.wav" ) ||
				  FStrEq( sample, "items/r_item2.wav" ) ) {	// medkit or armor pickup
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			//ent = INDEXENT( clientIndex+1 );
			player[clientIndex].itemSensableDist = ITEM_SENS_DIST;
			player[clientIndex].itemTrackableDist = ITEM_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up medkit or armor!\n" );
		}
		else if ( FStrEq( sample, "items/armor1.wav" ) ) {	// armor pickup
			clientIndex = ENTINDEX( ent ) - 1;
			player[clientIndex].itemSensableDist = ITEM_SENS_DIST;
			player[clientIndex].itemTrackableDist = ITEM_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up armor!\n" );
		}
		else if ( FStrEq( sample, "items/damage.wav" ) ) {			// quad-damage
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			//ent = INDEXENT( clientIndex+1 );
			player[clientIndex].itemSensableDist = SPEC_ITEM_SENS_DIST;
			player[clientIndex].itemTrackableDist = SPEC_ITEM_TRACK_DIST;
			player[clientIndex].timeItemSound = worldTime() + 2.0;
			//debugMsg( STRING(ent->v.netname), " picked up quad-damage!\n" );
		}
		/*else {
			if (ent) debugMsg( STRING(ent->v.classname), " caused ", sample, " !\n" );
			else debugMsg( "NullEnt caused ", sample, " !\n" );
		}*/
		break;
	}
}

void Sounds::parseAmbientSound( edict_t *ent, const char *sample, float vol )
{
	if (fatalParabotError) return;

	int clientIndex;

	if ( FStrEq( sample, "ambience/siren.wav" ) ) {
		headToBunker = true;
		// 4 to 14 minutes silence...
		nextAirstrikeTime = worldTime() + 240.0 + RANDOM_FLOAT( 0.0, 600.0 );
	}
	else if ( FStrEq( sample, "weapons/mortarhit.wav" ) ) {
		airStrikeTime = worldTime()+1.0;
	}
	else if ( FStrEq( sample, "debris/beamstart2.wav" ) ) {
		Vector pos = 0.5 * (ent->v.absmin + ent->v.absmax);  
		clientIndex = UTIL_GetNearestPlayerIndex( pos ) - 1;
		ent = INDEXENT( clientIndex+1 );
		if (!ent) return;
		player[clientIndex].itemSensableDist = 800;
		player[clientIndex].itemTrackableDist = 800;
		player[clientIndex].timeItemSound = worldTime() + 0.3;
		//debugMsg( STRING(ent->v.netname), " used teleporter!\n" );
	}
	/*else {
		if (ent) debugMsg( STRING(ent->v.classname), " caused ", sample, " !\n" );
		else debugMsg( "NullEnt caused ", sample, " !\n" );
	}*/

}

float Sounds::getSensableDist( int clientIndex )
{
	clientIndex--;	// array start with 0, index with 1

	float vol = player[clientIndex].stepSensableDist;
	float itemPlayTime = player[clientIndex].timeItemSound - worldTime();
	if ( itemPlayTime > 0 ) {
		// check for mapchange
		if ( itemPlayTime > 20 ) player[clientIndex].timeItemSound = 0;
		if (player[clientIndex].itemSensableDist > vol) vol = player[clientIndex].itemSensableDist;
	}
	return vol;
}

float Sounds::getTrackableDist( int clientIndex )
{
	clientIndex--;	// array start with 0, index with 1

	float vol = player[clientIndex].stepTrackableDist;
	float itemPlayTime = player[clientIndex].timeItemSound - worldTime();
	if ( itemPlayTime > 0 ) {
		// check for mapchange
		if ( itemPlayTime > 20 ) player[clientIndex].timeItemSound = 0;
		if (player[clientIndex].itemTrackableDist > vol) vol = player[clientIndex].itemTrackableDist;
	}
	return vol;
}
