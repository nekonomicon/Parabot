#include "pb_global.h"
#include "pb_mapgraph.h"
#include "pb_mapcells.h"
#include "entity_state.h"
#include "bot.h"


extern PB_MapGraph mapGraph;
extern PB_MapCells map;
extern int mod_id;


char actualMapname[100] = "";



void saveLevelData()
{
	char fileName[100];
		
	strcpy( fileName, actualMapname );
	strcat( fileName, ".pnf" );
	infoMsg( "\nSaving level data to ", fileName, "\n" );
	mapGraph.save( fileName );

	strcpy( fileName, actualMapname );
	strcat( fileName, ".pcf" );
	infoMsg( "\nSaving cell data to ", fileName, "\n" );
	map.save( fileName );
	return;
}

   

void importNav( int code )
{
	CBaseEntity *pOther = NULL;
	PB_Navpoint n;
	Vector pos;
	
	char *classname = PB_Navpoint::classname( code ); 

	while ( (pOther = UTIL_FindEntityByClassname (pOther, classname)) != NULL) {
		pos = (pOther->pev->absmax + pOther->pev->absmin) * 0.5;
		n.init( pos, code, 0 );
		mapGraph.addNavpoint( n );
	}
}


void importNav( int code, const char *modelName )
{
	CBaseEntity *pOther = NULL;
	PB_Navpoint n;
	Vector pos;
	
	char *classname = PB_Navpoint::classname( code ); 

	while ( (pOther = UTIL_FindEntityByClassname (pOther, classname)) != NULL) {
		if (!FStrEq( STRING(pOther->pev->model), modelName )) continue;
		pos = (pOther->pev->absmax + pOther->pev->absmin) * 0.5;
		n.init( pos, code, 0 );
		mapGraph.addNavpoint( n );
	}
}


void importHL_Specifics()
{
	// import weapons
	importNav( NAV_W_CROSSBOW );
	importNav( NAV_W_CROWBAR );
	importNav( NAV_W_EGON );
	importNav( NAV_W_GAUSS );
	importNav( NAV_W_HANDGRENADE );
	importNav( NAV_W_HORNETGUN );
	importNav( NAV_W_MP5 );
	importNav( NAV_W_9MMAR );
	importNav( NAV_W_PYTHON );
	importNav( NAV_W_357 );
	importNav( NAV_W_RPG );
	importNav( NAV_W_SATCHEL );
	importNav( NAV_W_SHOTGUN );
	importNav( NAV_W_SNARK );
	importNav( NAV_W_TRIPMINE );
	importNav( NAV_W_GLOCK );
	importNav( NAV_W_9MMHANDGUN );
	// import ammo
	importNav( NAV_A_CROSSBOW );
	importNav( NAV_A_CROSSBOW_BOLT );
	importNav( NAV_A_EGONCLIP );
	importNav( NAV_A_GAUSSCLIP	);
	importNav( NAV_A_MP5CLIP );
	importNav( NAV_A_MP5GRENADES );
	importNav( NAV_A_9MMAR	);
	importNav( NAV_A_9MMBOX );
	importNav( NAV_A_9MMCLIP );
	importNav( NAV_A_ARGRENADES );
	importNav( NAV_A_357 );
	importNav( NAV_A_RPGCLIP );
	importNav( NAV_A_RPG_ROCKET );
	importNav( NAV_A_BUCKSHOT );
	importNav( NAV_A_GLOCKCLIP	);
	// import funcs	
	importNav( NAV_F_TANK );
	importNav( NAV_F_TANKCONTROLS );
	importNav( NAV_F_RECHARGE );
	importNav( NAV_F_HEALTHCHARGER );
	// import items
	importNav(  NAV_I_HEALTHKIT );
	importNav(  NAV_I_LONGJUMP );
	importNav(  NAV_I_AIRTANK );
	importNav(  NAV_I_BATTERY );
}


void importCS_Specifics()
{
	importNav( NAV_CS_BOMB_TARGET );		// Bombzone in DE-maps
	importNav( NAV_CS_BUYZONE );
	importNav( NAV_CS_ESCAPEZONE );			// Escapezone for terrorists in ES-maps
//	importNav( NAV_CS_GRENCATCH );
	importNav( NAV_CS_HOSTAGE_RESCUE );		// Rescuezone for hostages in CS-maps
//	importNav( NAV_CS_VEHICLE );
	importNav( NAV_CS_VEHICLECONTROLS );
	importNav( NAV_CS_VIP_SAFETYZONE );		// Rescuezone for VIP in AS-maps
//	importNav( NAV_CS_WEAPONCHECK );
	importNav( NAV_CS_HOSTAGE_ENTITY );
//	importNav( NAV_CSI_BOMB_TARGET );
//	importNav( NAV_CSI_HOSTAGE_RESCUE );
//	importNav( NAV_CSI_MAP_PARAMETERS );
	importNav( NAV_CSI_VIP_START );			// VIP-spawnpoint
	importNav( NAV_INFO_PLAYER_START );		// CT-spawnpoint (T-spawnpoint=PLAYER_DM)
}


void importTFC_Specifics()
{
	importNav( NAV_TFC_TEAMSPAWN );
	importNav( NAV_TFC_GOAL );
	importNav( NAV_TFC_ARMOR1 );
	importNav( NAV_I_HEALTHKIT );
}


void importHW_Specifics()
{
	importNav( NAV_F_HEALTHCHARGER );
	importNav( NAV_I_HEALTHKIT );
	importNav( NAV_I_BATTERY );

	importNav( NAV_HWW_DOUBLESHOTGUN );
	importNav( NAV_HWW_JACKHAMMER );
	importNav( NAV_HWW_RAILGUN );
	importNav( NAV_HWW_MACHINEGUN );
	importNav( NAV_HWW_ROCKETLAUNCHER );

	importNav( NAV_HWA_DOUBLESHOTGUN );
	importNav( NAV_HWA_RAILGUN );
	importNav( NAV_HWA_MACHINEGUN );
	importNav( NAV_HWA_ROCKETLAUNCHER );

	importNav( NAV_HW_HALOBASE );
	importNav( NAV_HW_JUMPPAD_TARGET );
//	importNav( NAV_HW_TRIG_JUMPPAD );
//	importNav( NAV_HW_JUMPPAD_SIGN );
}


void importDMC_Specififcs()
{
	importNav( NAV_DMCI_ARMOR1 );
	importNav( NAV_DMCI_ARMOR2 );
	importNav( NAV_DMCI_ARMOR3 );
	importNav( NAV_DMCI_ARMOR_INV );
	importNav( NAV_DMCI_ENVIROSUIT );
	importNav( NAV_DMCI_INVISIBILITY );
	importNav( NAV_DMCI_INVULNERABILITY );
	importNav( NAV_DMCI_SUPERDAMAGE );
	importNav( NAV_DMCI_CELLS );
	importNav( NAV_DMCI_HEALTH_SMALL, "models/w_medkits.mdl" );
	importNav( NAV_DMCI_HEALTH_NORM,  "models/w_medkit.mdl" );
	importNav( NAV_DMCI_HEALTH_LARGE, "models/w_medkitl.mdl" );
	importNav( NAV_DMCI_ROCKETS );
	importNav( NAV_DMCI_SHELLS );
	importNav( NAV_DMCI_SPIKES );
	importNav( NAV_DMCI_WEAPON );
	importNav( NAV_DMCW_SUPERSHOTGUN );
	importNav( NAV_DMCW_NAILGUN );
	importNav( NAV_DMCW_SUPERNAILGUN );
	importNav( NAV_DMCW_GRENLAUNCHER );
	importNav( NAV_DMCW_ROCKETLAUNCHER );
	importNav( NAV_DMCW_LIGHTNING );
}


void importGearbox_Specifics()
{
	importHL_Specifics();
	importNav( NAV_OFW_GRAPPLE );
	importNav( NAV_OFW_EAGLE );
	importNav( NAV_OFW_PIPEWRENCH );
	importNav( NAV_OFW_M249 );
	importNav( NAV_OFW_DISPLACER );
	importNav( NAV_OFW_SHOCKRIFLE );
	importNav( NAV_OFW_SPORELAUNCHER );
	importNav( NAV_OFW_SNIPERRIFLE );
	importNav( NAV_OFW_KNIFE );
	importNav( NAV_OFW_PENGUIN );
	importNav( NAV_OFA_556 );
	importNav( NAV_OFA_762 );
	importNav( NAV_OFA_EAGLECLIP );
	importNav( NAV_OFA_SPORE );
}


bool loadLevelData()
{
	char fileName[100];
	
	if (strcmp( STRING(gpGlobals->mapname), actualMapname )==0) return true;

	if ( strcmp( actualMapname, "" ) != 0 && 
		 mapGraph.numberOfNavpoints() > 0    ) saveLevelData();
	
	mapGraph.clear();
	map.clear();
	strcpy( actualMapname, STRING(gpGlobals->mapname) );
	strcpy( fileName, STRING(gpGlobals->mapname) );
	strcat( fileName, ".pnf" );
	if (!mapGraph.load( fileName ))	{
		infoMsg( "Importing level data...\n" );
			
	// import funcs
		importNav( NAV_F_BUTTON	);
		importNav( NAV_F_ROT_BUTTON	);
		importNav( NAV_F_DOOR );
		importNav( NAV_F_DOOR_ROTATING );
		importNav( NAV_F_PLAT );
		importNav( NAV_F_PLATROT );
		importNav( NAV_F_TRAIN );
		importNav( NAV_F_BREAKABLE );

	// import player-start
		importNav(  NAV_INFO_PLAYER_DM );

	// teleport
		importNav(  NAV_INFO_TELEPORT_DEST );	
		importNav(  NAV_TRIG_TELEPORT );	
		
	// import ladders
		CBaseEntity *pOther = NULL;
		PB_Navpoint n;
		Vector posUp, posDown;
	
		while ( (pOther = UTIL_FindEntityByClassname (pOther, "func_ladder")) != NULL) {
			posUp = (pOther->pev->absmax + pOther->pev->absmin) * 0.5;
			posUp.z = pOther->pev->absmax.z;
			posDown = posUp;
			posDown.z = pOther->pev->absmin.z;
		/*	Vector dir = pOther->v.absmax - pOther->v.absmin;
			dir.z = 0;
			dir = CrossProduct( dir, Vector(0,0,1));
			dir = 30 * dir.Normalize();	
			TraceResult tr;
			UTIL_TraceLine( posUp, posDown+dir, ignore_monsters, NULL, &tr );
			if (tr.flFraction<1.0) posDown = posDown - dir;
			else				   posDown = posDown + dir; */
			posUp.z += 36;
			posDown.z += 36;
			n.init( posUp, NAV_F_LADDER_TOP, 0 );
			mapGraph.addNavpoint( n );
			n.init( posDown, NAV_F_LADDER_BOTTOM, 0 );
			mapGraph.addNavpoint( n );
		}

		// import MOD-specifics
		switch( mod_id ) {
		case VALVE_DLL:		importHL_Specifics();		break;
		case CSTRIKE_DLL:	importCS_Specifics();		break;
		case TFC_DLL:		importTFC_Specifics();		break;
		case HOLYWARS_DLL:	importHW_Specifics();		break;
		case DMC_DLL:		importDMC_Specififcs();		break;
		case GEARBOX_DLL:	importGearbox_Specifics();	break;
		default:			errorMsg( "Unsupported MOD in pb_mapimport.cpp!\n" );
		}
		
		// import specials
		if (mod_id==VALVE_DLL && strcmp( STRING(gpGlobals->mapname), "crossfire" )==0) {
			PB_Navpoint n;
			n.init( Vector( 0,-2236,-1852 ), NAV_S_AIRSTRIKE_BUTTON, 0 );
			mapGraph.addNavpoint( n );
		}
	}
	else {
		strcpy( fileName, STRING(gpGlobals->mapname) );
		strcat( fileName, ".pcf" );
		map.load( fileName );
		infoMsg( "Loaded level data.\n" );
	}
	if (mapGraph.numberOfNavpoints() > 0 ) return true;
	else return false;
}


