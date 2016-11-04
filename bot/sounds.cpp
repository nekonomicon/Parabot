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

PB_Navpoint* getNearestNavpoint( edict_t *pEdict );


bool headToBunker = false;
float airStrikeTime = 0;
float nextAirstrikeTime = 500;



#define MAX_TEXTURES 385

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


char szTextureName[MAX_TEXTURES][CBTEXTURENAMEMAX] = { 
"DUCT_FLR01",	"DUCT_FLR01A",	"DUCT_FLR02A",	"DUCT_VNT",		"DUCT_VNT2",	"DUCT_WALL01",	"DUCT_WALL02",	"DUCT_WALL03",	"DUCT_WALL04",	"SILO2_COR", 
"OUT_GRVL1",	"OUT_GRVL2",	"OUT_GRVL2B",	"OUT_GRVL3",	"OUT_MUD1",		"OUT_SND1",		"OUT_SND2",		"OUT_SND2B",	"OUT_SND2C",	"OUT_WLK", 
"OUT_GRSS1",	"OUT_DIRT1",	"OUT_DIRT2",	"GENERIC48",	"GENERIC49",	"GENERIC51",	"GENERIC52",	"GENERIC93",	"OUT_PAVE2",	"OUT_SNBAG", 
"OUT_SNBAG2",	"OUT_SNBAGB",	"OUT_SNBAGC",	"OUT_TNT2",		"OUT_TNT1",		"OUT_TNT1B",	"OUT_TNT1C",	"OUT_TNT3",		"OUT_TNT3B",	"OUT_NET1", 
"OUT_NET1B",	"FROSTSEWFLR",	"GENERIC109",	"GENERIC015C",	"GENERIC015D",	"GENERIC015E",	"GENERIC015F",	"GENERIC015G",	"GENERIC015H",	"GENERIC015I", 
"GENERIC015",	"GENERIC015A",	"GENERIC015B",	"BABTECH_FLR0", "CRETE2_FLR03", "CRETE2_FLR03", "CRETE2_FLR03", "CRETE2_FLR03", "ELEV1_FLR",	"ELEV2_FLR", 
"ELEV_FLR",		"GRATE1",		"GRATE2",		"GRATE2A",		"GRATE3A",		"GRID1",		"GRID2",		"TNNL_FLR12",	"TNNL_FLR12A",	"TNNL_FLR1A", 
"LAB1_STAIR2A", "FROSTFLOOR",	"GRATE4A",		"GRATE3B",		"GRATESTEP1",	"GRATESTEP2",	"GRATESTEP3",	"GRID1",		"GRID2",		"GENERIC015V", 
"C1A1_FLR1",	"BABFL",		"C2A3TURBINE1", "C2A3TURBINE2", "C2A3TURBINE3", "C2A3TURBINE4", "SILO2_P2",		"SILO2_P2B",	"SILO2_P3",		"SILO2_P4", 
"SILO2_PAN2",	"SILO2_W1",		"SILO2_W1A",	"SILO2_WALL1",	"SILO2_W2",		"BABTECH_C5",	"LAB3_FLR2A",	"LAB3_FLR2B",	"FIFTIES_CMP3", "LAB1_STAIR2B", 
"OUT_QUNST11",	"FREEZER_FLR1", "GENERIC015N",	"GENERIC015P",	"GENERIC015R",	"GENERIC0150",	"GENERIC015S",	"GENERIC015T",	"GENERIC015U",	"GENERIC015V", 
"GENERIC015V2", "C1A4_DOME3",	"GENERIC_114",	"WATERBLUE",	"WATERGREEN",	"TOXICGRN",		"FLUID1A",		"FLUID1B",		"FLUID2",		"FLUID3", 
"FLUID4",		"WATERSILO",	"WATERSILO2",	"WATERF",		"FLUID1A",		"FIFT_BLOODFL", "FIFT_BLOODFL", "FIFTIES_F01",	"FIFTIES_F02",	"FIFTIES_F03", 
"FIFTIES_F03B", "FIFTIES_FLR0", "FIFTIES_FLR0", "FIFTIES_FLR0", "FIFTIES_FLR0", "FIFTIES_FLR0", "FIFTIES_FLR0", "FIFTIES_FLR5", "LAB1_BLUXFLR", "LAB1_BLUXFLR", 
"LAB1_BLUX1",	"LAB1_BLUX1B",	"LAB1_C4003",	"LAB1_C4A001",	"LAB1_C4B002",	"LAB1_C4D2",	"LAB1_CAB2",	"LAB1_FLOOR2A", "LAB1_FLOOR2B", "LAB1_FLOOR3", 
"LAB1_FLOOR4",	"LAB1_FLOOR5",	"LAB1_FLOOR6",	"LAB1_FLOOR10", "LAB1_FLR3",	"LAB1_FLR4",	"LAB1_FLR4B",	"LAB1_FLR4C",	"LAB1_FLR4D",	"LAB1_FLR5B", 
"LAB1_FLR5C",	"LAB1_FLR5D",	"LAB1_FLR5D",	"LAB1_FLR6B",	"LAB1_FLR6C",	"LAB1_FLR6D",	"LAB1_W8FLR1",	"LAB1_W8FLR1B", "LAB1_W8FLR1C", "LAB1_W8FLR1D", 
"C2A4_FLR6",	"C1A2_FLR1",	"C1A2_FLR2",	"C1A0_LABFLRB", "C1A0_LABFLRC", "C1A0_LABFLRD", "C1A0_LABFLRE", "C1A0_LABFLR",	"CRATE01",		"CRATE02", 
"CRATE02B",		"CRATE03",		"CRATE04",		"CRATE05",		"CRATE06",		"CRATE07",		"CRATE08",		"CRATE08B",		"CRATE09",		"CRATE09B", 
"CRATE09C",		"CRATE19",		"CRATE20",		"CRATE21",		"CRATE22",		"CRATE23",		"CRATE25",		"FIFTIES_DR6",	"FIFTIES_DR6A", "FIFTIES_CCH1", 
"FIFTIES_CCH2", "FIFTIES_CCH3", "FIFTIES_CCH4", "FIFTIES_DR1K", "FIFTIES_DR2",	"FIFTIES_DR7",	"FIFTIES_DR8",	"FIFTIES_DR9",	"FIFTIES_DSK1", "OUT_SLAT01", 
"OUT_WD",		"OUT_CAC1",		"OUT_CAC2",		"OUT_CAC2",		"BCRATE02",		"BCRATE03",		"BCRATE04",		"BCRATE05",		"BCRATE06",		"BCRATE07", 
"BCRATE08",		"BCRATE12",		"BCRATE14",		"BCRATE15",		"BCRATE16",		"BCRATE17",		"BCRATE18",		"BCRATE25",		"BCRATE26",		"CRATE10", 
"CRATE11",		"CRATE12",		"CRATE13",		"CRATE24",		"CRATE27",		"C3A1_NRC2",	"C1A1_GAD1",	"C1A1_GAD2",	"C1A1_GAD3",	"C1A1_GAD4", 
"C1A1_GAD4A",	"C1A1_GGT8",	"C1A4_PAN1A",	"C1A4_PAN1B",	"C1A4_SWTCH1",	"C2A2_SATORB",	"C2A4_CMP1",	"C2A4_GAD2",	"C3A2A_W1D",	"C3A2A_W1E", 
"C3A2A_W2D",	"C3A2A_W2E",	"DRKMTL_SCRN2", "DRKMTL_SCRN3", "DRKMTL_SCRN4", "DRKMTLT_WALL", "FIFTIES_GGT8", "FIFTIES_MON1", "FIFTIES_MON1", "FIFTIES_MON2", 
"FIFTIES_MON2", "FIFTIES_MON3", "FIFTIES_MON3", "FIFTIES_MON4", "FIFTIES_PAN2", "GENERIC105",	"GENERIC106",	"GENERIC107",	"GENERIC114",	"GENERIC87A", 
"GENERIC88A",	"GENERIC89A",	"GENERIC113",	"LAB1_CMPM1",	"LAB1_CMPM2",	"LAB1_CMPM3",	"LAB1_CMPM4",	"LAB1_CMPM5",	"LAB1_CMPM6",	"LAB1_CMPM7", 
"LAB1_CMPM8",	"LAB1_COMP1",	"LAB1_COMP2",	"LAB1_COMP3",	"LAB1_COMP2A",	"LAB1_COMP10A", "LAB1_COMP10B", "LAB1_COMP10C", "LAB1_COMP10D", "LAB1_COMP10E", 
"LAB1_COMP7",	"LAB1_COMP8",	"LAB1_COMP9A",	"LAB1_COMP9A2", "LAB1_COMP9B",	"LAB1_COMP9C",	"LAB1_COMP9D",	"LAB1_GAD2",	"LAB1_GAD3",	"LAB1_GAD4", 
"LAB1_RADSCRN", "LAB4_GAD3",	"LAB4_GAD4",	"LAB4_SWTCH",	"LAB_COMPM4",	"RECHARGEA",	"TNNL_GAD1",	"TNNL_GAD2",	"TNNL_GAD4",	"C2A4C2C", 
"C2A4W1B",		"LAB1_CMP",		"LAB1_CMP1",	"LAB1_CMP2",	"LAB1_COMP3",	"LAB1_COMP3A",	"LAB1_COMP3B",	"LAB1_COMP3C",	"LAB1_COMP3D",	"LAB1_COMP4", 
"LAB1_COMP5",	"LAB1_COMP7",	"LAB1_COMP8",	"LAB1_SW1",		"LAB_CRT1",		"LAB_CRT10A",	"LAB_CRT10B",	"LAB_CRT10C",	"LAB_CRT10D",	"LAB_CRT2", 
"LAB_CRT3",		"LAB_CRT4",		"LAB_CRT5",		"LAB_CRT6",		"LAB_CRT7",		"LAB_CRT8",		"LAB_CRT9C",	"CRETESTCH01A", "DRKMTLLGT1",	"FIFTS_LGHT01", 
"FIFTS_LGHT3",	"FIFTS_LGHT4",	"FIFTIES_LGT2", "FIFTS_LGHT5",	"GYMLIGHT",		"LIGHT1",		"LIGHT2A",		"LIGHT3A",		"LIGHT3B",		"LIGHT3C", 
"LIGHT3D",		"LIGHT3E",		"LIGHT3F",		"LIGHT4A",		"LIGHT5A",		"LIGHT5B",		"LIGHT5C",		"LIGHT5D",		"LIGHT5E",		"LIGHT5F", 
"LIGHT6A",		"METALSTCH2",	"TNNL_LGT1",	"TNNL_LGT2",	"TNNL_LGT3",	"TNNL_LGT4",	"LAB1_GAD2",	"ELEV1_DWN",	"SPOTBLUE",		"SPOTGREEN", 
"SPOTRED",		"SPOTYELLOW",	"LAB1_GAD3",	"LAB1_GAD3B",	"ELEV2_PAN",	"ELEV1_PAN",	"C2A4X_C3",		"C2A4X_C1",		"MEDKIT",		"GLASS_BRIGHT", 
"GLASS_DARK",	"GLASS_MED",	"GLASSBLUE1",	"GLASSBLUE2",	"GLASSGREEEN" 
};

char chTextureType[MAX_TEXTURES] = { 
'V', 'V', 'V', 'V', 'V', 'V', 'V', 'V', 'V', 'V', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 
'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 
'D', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 
'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 
'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 
'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'M', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 
'S', 'S', 'S', 'S', 'S', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 
'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 
'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'W', 'W', 
'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 
'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 
'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'P', 'P', 'P', 'P', 'P', 
'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 
'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 
'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 
'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 
'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 
'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 
'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'Y', 
'Y', 'Y', 'Y', 'Y', 'Y'
};


bool UTIL_IsOnLadder( edict_t *ent )
{
	return (ent->v.movetype == MOVETYPE_FLY);
}



Sounds::Sounds()
{
	for (int i=0; i<32; i++) {
		stepSensableDist[i] = 0;
		stepTrackableDist[i] = 0;
		itemSensableDist[i] = 0;
		itemTrackableDist[i] = 0;
		timeItemSound[i] = 0;
		timeStepSound[i] = 0;
		oldTimeStepSound[i] = 0;
	}
}


char Sounds::findTextureType( char *name )
{
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		if (!_strnicmp(name, &(szTextureName[i][0]), CBTEXTURENAMEMAX-1))
			return (chTextureType[i]);
	}

	return CHAR_TEX_CONCRETE;
}


void Sounds::getAllClientSounds()
{
	bool writeResult = true;
	
	float hearSteps = CVAR_GET_FLOAT( "mp_footsteps" );
	if ( mod_id == DMC_DLL ) hearSteps = 0;		// no step sounds in DMC

	CBaseEntity *pPlayer = 0;
	for (int i=1; i<=gpGlobals->maxClients; i++) {
		pPlayer = UTIL_PlayerByIndex( i );
		if (!pPlayer) continue;							// skip invalid players
		if (!isAlive( ENT(pPlayer->pev) )) continue;	// skip player if not alive
		if (pPlayer->pev->solid == SOLID_NOT) continue;	
		
		// get step sounds
		if (hearSteps > 0) calcStepSound( i-1, pPlayer->edict(), writeResult );
		// get attack sounds
		if ( (pPlayer->pev->button & (IN_ATTACK|IN_ATTACK2)) ) {
			int wid = clientWeapon[i-1];
			PB_Weapon w( wid );
			float sensDist = w.getAudibleDistance( pPlayer->pev->button );
			float trackDist = sensDist/3;
			if (sensDist > stepSensableDist[i-1]) stepSensableDist[i-1] = sensDist;
			if (trackDist > stepTrackableDist[i-1]) stepTrackableDist[i-1] = trackDist;
		}
		// get jump sounds
		if ( mod_id==HOLYWARS_DLL || mod_id==DMC_DLL ) {
			if ( (pPlayer->pev->button & IN_JUMP) && !isUnderwater( ENT(pPlayer->pev) ) ) {
				float sensDist = 300;
				float trackDist = 150;
				if (sensDist > stepSensableDist[i-1]) stepSensableDist[i-1] = sensDist;
				if (trackDist > stepTrackableDist[i-1]) stepTrackableDist[i-1] = trackDist;
			}
		}
		// get reload sounds
		if ( pPlayer->pev->button & IN_RELOAD ) {
			float sensDist = 200;
			float trackDist = 100;
			if (sensDist > stepSensableDist[i-1]) stepSensableDist[i-1] = sensDist;
			if (trackDist > stepTrackableDist[i-1]) stepTrackableDist[i-1] = trackDist;
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


	CBasePlayer *pl = (CBasePlayer*) GET_PRIVATE( ent );

	if (writeResult) {
		if ( worldTime() <= timeStepSound[clientIndex] ) {
			stepSensableDist[clientIndex] = 0;
			stepTrackableDist[clientIndex] = 0;
			// check for mapchange -> start playing sounds
			if ( (timeStepSound[clientIndex]-worldTime()) > 1.0 ) 
				timeStepSound[clientIndex] = worldTime();
			else return;
		}
	}
	else {
		if ( timeStepSound[clientIndex] == oldTimeStepSound[clientIndex] ) {
			stepSensableDist[clientIndex] = 0;
			stepTrackableDist[clientIndex] = 0;
			return;
		}
		else oldTimeStepSound[clientIndex] = timeStepSound[clientIndex];
	}


	speed = ent->v.velocity.Length();

	// determine if we are on a ladder
	fLadder = UTIL_IsOnLadder( ent );

	// UNDONE: need defined numbers for run, walk, crouch, crouch run velocities!!!!	
	if (FBitSet(ent->v.flags, FL_DUCKING) || fLadder)
	{
		velwalk = 60;		// These constants should be based on cl_movespeedkey * cl_forwardspeed somehow
		velrun = 80;		// UNDONE: Move walking to server
		flduck = 0.1;
	}
	else
	{
		velwalk = 120;
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
			step = STEP_LADDER;
			fvol = 0.35;
			sensableDist = 600;
			trackableDist = 400;
			if (writeResult) timeStepSound[clientIndex] = gpGlobals->time + 0.35;
		}
		else if ( UTIL_PointContents ( knee ) == CONTENTS_WATER )
		{
			step = STEP_WADE;
			fvol = 0.65;
			sensableDist = 200;
			trackableDist = 200;
			if (writeResult) timeStepSound[clientIndex] = gpGlobals->time + 0.6;
		}
		else if (UTIL_PointContents ( feet ) == CONTENTS_WATER )
		{
			step = STEP_SLOSH;
			fvol = fWalking ? 0.2 : 0.5;
			sensableDist = 200;
			trackableDist = 200;
			if (writeResult) timeStepSound[clientIndex] = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;		
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
					
					if (_strnicmp(pTextureName, textureName[clientIndex], CBTEXTURENAMEMAX-1))
					{
						// current texture is different from texture player is on...
						// set current texture
						strcpy(szbuffer, pTextureName);
						szbuffer[CBTEXTURENAMEMAX - 1] = 0;
						strcpy(textureName[clientIndex], szbuffer);
						
						//debugMsg( "texture: ", textureName[clientIndex], "\n" );
						
						// get texture type
						textureType[clientIndex] = findTextureType( textureName[clientIndex] );	
					}
				}
			}
			
			switch (textureType[clientIndex])
			{
			default:
			case CHAR_TEX_CONCRETE:
				step = STEP_CONCRETE;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 200;
				trackableDist = 100;
				if (writeResult) timeStepSound[clientIndex] = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_METAL:	
				step = STEP_METAL;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 600;
				trackableDist = 300;
				if (writeResult) timeStepSound[clientIndex] = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_DIRT:	
				step = STEP_DIRT;
				fvol = fWalking ? 0.25 : 0.55;
				sensableDist = 200;
				trackableDist = 100;
				if (writeResult) timeStepSound[clientIndex] = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_VENT:	
				step = STEP_VENT;
				fvol = fWalking ? 0.4 : 0.7;
				sensableDist = 600;
				trackableDist = 400;
				if (writeResult) timeStepSound[clientIndex] = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_GRATE:
				step = STEP_GRATE;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 600;
				trackableDist = 400;
				if (writeResult) timeStepSound[clientIndex] = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_TILE:	
				step = STEP_TILE;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 200;
				trackableDist = 100;
				if (writeResult) timeStepSound[clientIndex] = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;

			case CHAR_TEX_SLOSH:
				step = STEP_SLOSH;
				fvol = fWalking ? 0.2 : 0.5;
				sensableDist = 200;
				trackableDist = 200;
				if (writeResult) timeStepSound[clientIndex] = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
				break;
			}
		}

		if (writeResult) timeStepSound[clientIndex] += flduck; // slower step time if ducking
	
		// 35% volume if ducking
		if ( ent->v.flags & FL_DUCKING ) fvol *= 0.35;
		
		/*char *name = (char*) STRING( ent->v.netname );
		debugMsg( "%.1f: ", worldTime() );
		debugMsg( name, " on ", textureName[clientIndex] );
		debugMsg( " (Code %i)", step );
		debugMsg( " -> vol %.2f\n", fvol );*/
	}

	stepSensableDist[clientIndex] = sensableDist;
	stepTrackableDist[clientIndex] = trackableDist;
}


int UTIL_GetNearestPlayerIndex( Vector &pos )
{
	float dist, bestDist = 10000;
	int	  bestPlayer = 0;
	CBaseEntity *pPlayer = 0;

	for (int i=1; i<=gpGlobals->maxClients; i++) {
		pPlayer = UTIL_PlayerByIndex( i );
		if (!pPlayer) continue;							// skip invalid players
		if (!isAlive( ENT(pPlayer->pev) )) continue;	// skip player if not alive
		if (pPlayer->pev->solid == SOLID_NOT) continue;	
		
		dist = (pPlayer->pev->origin - pos).Length();
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
	case VALVE_DLL:
	case GEARBOX_DLL:
	case HOLYWARS_DLL:
		if ( strcmp( sample, "items/gunpickup2.wav" ) == 0 ) {		// weapon pickup
			clientIndex = ENTINDEX( ent ) - 1;
			itemSensableDist[clientIndex] = AMMO_SENS_DIST;
			itemTrackableDist[clientIndex] = AMMO_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up gun!\n" );
			PB_Navpoint *nearest = getNearestNavpoint( ent );
			//assert( nearest != 0 );
			if (!nearest) return;	// listen server before map is loaded
			const char *wpnName = nearest->classname();
			if (mod_id==VALVE_DLL || mod_id==GEARBOX_DLL) {
				if ( strcmp( wpnName, "weapon_rpg"      ) == 0 ||
					 strcmp( wpnName, "weapon_gauss"    ) == 0 ||
					 strcmp( wpnName, "weapon_egon"     ) == 0 ||
					 strcmp( wpnName, "weapon_crossbow" ) == 0 ) chat.registerGotWeapon( ent, wpnName );
			}
			else {	// Holy Wars
				chat.registerGotWeapon( ent, wpnName );
			}
		}
		else if ( strcmp( sample, "items/9mmclip1.wav" ) == 0 ) {	// ammo pickup
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			ent = INDEXENT( clientIndex+1 );
			itemSensableDist[clientIndex] = AMMO_SENS_DIST;
			itemTrackableDist[clientIndex] = AMMO_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up ammo!\n" );
		}
		else if ( strcmp( sample, "doors/doorstop6.wav" ) == 0 ) {	// lift usage
			Vector pos = 0.5 * (ent->v.absmin + ent->v.absmax);  
			pos.z = ent->v.absmax.z;
			clientIndex = UTIL_GetNearestPlayerIndex( pos ) - 1;
			ent = INDEXENT( clientIndex+1 );
			itemSensableDist[clientIndex] = LIFT_SENS_DIST;
			itemTrackableDist[clientIndex] = LIFT_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " used lift!\n" );
		}
		else if ( strcmp( sample, "items/suitcharge1.wav" ) == 0 ) {// battery charger
			Vector pos = 0.5 * (ent->v.absmin + ent->v.absmax);  
			clientIndex = UTIL_GetNearestPlayerIndex( pos ) - 1;
			ent = INDEXENT( clientIndex+1 );
			if (vol > 0) {
				itemSensableDist[clientIndex] = LOAD_SENS_DIST;
				itemTrackableDist[clientIndex] = LOAD_TRACK_DIST;
				timeItemSound[clientIndex] = worldTime() + 10.0;
				//debugMsg( STRING(ent->v.netname), " started using suitcharger!\n" );
			}
			else {
				itemSensableDist[clientIndex] = 0;
				itemTrackableDist[clientIndex] = 0;
				timeItemSound[clientIndex] = 0;
				//debugMsg( STRING(ent->v.netname), " stopped using suitcharger!\n" );
			}
		}
		else if ( strcmp( sample, "items/medcharge4.wav" ) == 0 ) {	// med charger
			Vector pos = 0.5 * (ent->v.absmin + ent->v.absmax);  
			clientIndex = UTIL_GetNearestPlayerIndex( pos ) - 1;
			ent = INDEXENT( clientIndex+1 );
			if (vol > 0) {
				itemSensableDist[clientIndex] = LOAD_SENS_DIST;
				itemTrackableDist[clientIndex] = LOAD_TRACK_DIST;
				timeItemSound[clientIndex] = worldTime() + 10.0;
				//debugMsg( STRING(ent->v.netname), " started using healthcharger!\n" );
			}
			else {
				itemSensableDist[clientIndex] = 0;
				itemTrackableDist[clientIndex] = 0;
				timeItemSound[clientIndex] = 0;
				//debugMsg( STRING(ent->v.netname), " stopped using healthcharger!\n" );
			}
		}
		else if ( strcmp( sample, "items/smallmedkit1.wav" ) == 0 ) {	// medkit pickup
			clientIndex = ENTINDEX( ent ) - 1;
			itemSensableDist[clientIndex] = ITEM_SENS_DIST;
			itemTrackableDist[clientIndex] = ITEM_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up medkit!\n" );
		}
		else if ( strcmp( sample, "!336" ) == 0 ) {						// longjump pickup
			clientIndex = ENTINDEX( ent ) - 1;
			itemSensableDist[clientIndex] = SPEC_ITEM_SENS_DIST;
			itemTrackableDist[clientIndex] = SPEC_ITEM_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 3.0;
			debugMsg( STRING(ent->v.netname), " picked up longjump!\n" );
		}
		else if ( strcmp( sample, "misc/jumppad.wav" ) == 0 ) {			// HW-jumppad 
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			ent = INDEXENT( clientIndex+1 );
			itemSensableDist[clientIndex] = LIFT_SENS_DIST;
			itemTrackableDist[clientIndex] = LIFT_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " used HW-jumppad!\n" );
		}
		else if ( strcmp( sample, "player/drink3.wav" ) == 0 ) {		// HW-bottle
			clientIndex = ENTINDEX( ent ) - 1;
			itemSensableDist[clientIndex] = ITEM_SENS_DIST;
			itemTrackableDist[clientIndex] = ITEM_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up HW-bottle!\n" );
		}
		/*else {
			if (ent) debugMsg( STRING(ent->v.classname), " caused ", sample, " !\n" );
			else debugMsg( "NullEnt caused ", sample, " !\n" );
		}*/
		break;

	case DMC_DLL:
		if ( strcmp( sample, "common/null.wav" ) == 0 ||
			 strcmp( sample, "player/lburn1.wav" ) == 0 ||
			 strcmp( sample, "player/lburn2.wav" ) == 0 ||
			 strcmp( sample, "items/itembk2.wav" ) == 0    ) {	// ignore
		}
		else if ( strcmp( sample, "weapons/pkup.wav" ) == 0 ||		// weapon pickup
				  strcmp( sample, "weapons/lock4.wav" ) == 0) {		// ammo pickup
			const char *wpnName = STRING( ent->v.classname );
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			ent = INDEXENT( clientIndex+1 );
			itemSensableDist[clientIndex] = AMMO_SENS_DIST;
			itemTrackableDist[clientIndex] = AMMO_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up gun or ammo: ", wpnName, "\n" );
			if ( strcmp( wpnName, "weapon_lightning" ) == 0 ||
				 strcmp( wpnName, "weapon_rocketlauncher" ) == 0 ||
				 strcmp( wpnName, "weapon_grenadelauncher" ) == 0 ||
				 strcmp( wpnName, "weapon_supernailgun" ) == 0		 ) chat.registerGotWeapon( ent, wpnName );
		}
		else if ( strcmp( sample, "plats/freightmove2.wav" ) == 0 ) {	// lift usage
			Vector pos = 0.5 * (ent->v.absmin + ent->v.absmax);  
			pos.z = ent->v.absmax.z;
			clientIndex = UTIL_GetNearestPlayerIndex( pos ) - 1;
			ent = INDEXENT( clientIndex+1 );
			itemSensableDist[clientIndex] = LIFT_SENS_DIST;
			itemTrackableDist[clientIndex] = LIFT_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " used lift!\n" );
		}
		else if ( strcmp( sample, "items/button4.wav" ) == 0 ) {	// button usage
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			ent = INDEXENT( clientIndex+1 );
			itemSensableDist[clientIndex] = LIFT_SENS_DIST;
			itemTrackableDist[clientIndex] = LIFT_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " pressed button!\n" );
		}
		else if ( strcmp( sample, "items/health1.wav" ) == 0 ||
				  strcmp( sample, "items/r_item2.wav" ) == 0    ) {	// medkit or armor pickup
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			ent = INDEXENT( clientIndex+1 );
			itemSensableDist[clientIndex] = ITEM_SENS_DIST;
			itemTrackableDist[clientIndex] = ITEM_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up medkit or armor!\n" );
		}
		else if ( strcmp( sample, "items/armor1.wav" ) == 0 ) {	// armor pickup
			clientIndex = ENTINDEX( ent ) - 1;
			itemSensableDist[clientIndex] = ITEM_SENS_DIST;
			itemTrackableDist[clientIndex] = ITEM_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 0.3;
			//debugMsg( STRING(ent->v.netname), " picked up armor!\n" );
		}
		else if ( strcmp( sample, "items/damage.wav" ) == 0 ) {			// quad-damage
			clientIndex = UTIL_GetNearestPlayerIndex( ent->v.origin ) - 1;
			ent = INDEXENT( clientIndex+1 );
			itemSensableDist[clientIndex] = SPEC_ITEM_SENS_DIST;
			itemTrackableDist[clientIndex] = SPEC_ITEM_TRACK_DIST;
			timeItemSound[clientIndex] = worldTime() + 2.0;
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

	if ( strcmp( sample, "ambience/siren.wav" ) == 0 ) {
		headToBunker = true;
		// 4 to 14 minutes silence...
		nextAirstrikeTime = worldTime() + 240.0 + RANDOM_FLOAT( 0.0, 600.0 );
	}
	else if ( strcmp( sample, "weapons/mortarhit.wav" ) ==0 ) {
		airStrikeTime = worldTime()+1.0;
	}
	else if ( strcmp( sample, "debris/beamstart2.wav" ) == 0 ) {
		Vector pos = 0.5 * (ent->v.absmin + ent->v.absmax);  
		clientIndex = UTIL_GetNearestPlayerIndex( pos ) - 1;
		ent = INDEXENT( clientIndex+1 );
		if (!ent) return;
		itemSensableDist[clientIndex] = 800;
		itemTrackableDist[clientIndex] = 800;
		timeItemSound[clientIndex] = worldTime() + 0.3;
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

	float vol = stepSensableDist[clientIndex];
	float itemPlayTime = timeItemSound[clientIndex] - worldTime();
	if ( itemPlayTime > 0 ) {
		// check for mapchange
		if ( itemPlayTime > 20 ) timeItemSound[clientIndex] = 0;
		if (itemSensableDist[clientIndex] > vol) vol = itemSensableDist[clientIndex];
	}
	return vol;
}


float Sounds::getTrackableDist( int clientIndex )  
{  
	clientIndex--;	// array start with 0, index with 1

	float vol = stepTrackableDist[clientIndex];
	float itemPlayTime = timeItemSound[clientIndex] - worldTime();
	if ( itemPlayTime > 0 ) {
		// check for mapchange
		if ( itemPlayTime > 20 ) timeItemSound[clientIndex] = 0;
		if (itemTrackableDist[clientIndex] > vol) vol = itemTrackableDist[clientIndex];
	}
	return vol;
}