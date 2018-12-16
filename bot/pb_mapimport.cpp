#include "parabot.h"
#include "pb_global.h"
#include "sectors.h"
#include "pb_mapgraph.h"
#include "vistable.h"
#include "cell.h"
#include "pb_mapcells.h"
#include "bot.h"


extern PB_MapGraph mapGraph;
extern PB_MapCells map;
extern int mod_id;

char actualMapname[100];

void saveLevelData()
{
	char fileName[100];

	// Directory exists?
	strcpy( fileName, com.modname );
	strcat( fileName, "/addons/parabot/navpoints/" );
	CreateDirectory( fileName, NULL );

	strcat( fileName, actualMapname );
	strcat( fileName, ".pnf" );
	INFO_MSG( "\nSaving level data to %s\n", fileName );
	mapGraph.save( fileName );

	fileName[strlen(fileName) - 4] = '\0'; // cut file extention
	strcat( fileName, ".pcf" );
	INFO_MSG( "\nSaving cell data to %s\n", fileName );
	map.save( fileName );
	return;
}

void importNav(int code, const char *modelName)
{
	EDICT *pOther = NULL;
	NAVPOINT n;
	Vec3D pos;
	
	const char *classname = navpoint_classname(code);

	while ((pOther = find_entitybyclassname(pOther, classname))) {
		if (modelName && !Q_STREQ( STRING(pOther->v.model), modelName ))
			continue;

		boxcenter(pOther, &pos);
		navpoint_init(&n, &pos, code, 0);
		mapGraph.addNavpoint(&n);
	}
}

void importHL_Specifics()
{
	// import weapons
	importNav( NAV_W_CROSSBOW, NULL);
	importNav( NAV_W_CROWBAR, NULL);
	importNav( NAV_W_EGON, NULL);
	importNav( NAV_W_GAUSS, NULL);
	importNav( NAV_W_HANDGRENADE, NULL);
	importNav( NAV_W_HORNETGUN, NULL);
	importNav( NAV_W_MP5, NULL);
	importNav( NAV_W_9MMAR, NULL);
	importNav( NAV_W_PYTHON, NULL);
	importNav( NAV_W_357, NULL);
	importNav( NAV_W_RPG, NULL);
	importNav( NAV_W_SATCHEL, NULL);
	importNav( NAV_W_SHOTGUN, NULL);
	importNav( NAV_W_SNARK, NULL);
	importNav( NAV_W_TRIPMINE, NULL);
	importNav( NAV_W_GLOCK, NULL);
	importNav( NAV_W_9MMHANDGUN, NULL);
	// import ammo
	importNav( NAV_A_CROSSBOW, NULL);
	importNav( NAV_A_CROSSBOW_BOLT, NULL);
	importNav( NAV_A_EGONCLIP, NULL);
	importNav( NAV_A_GAUSSCLIP, NULL);
	importNav( NAV_A_MP5CLIP, NULL);
	importNav( NAV_A_MP5GRENADES, NULL );
	importNav( NAV_A_9MMAR, NULL);
	importNav( NAV_A_9MMBOX, NULL);
	importNav( NAV_A_9MMCLIP, NULL);
	importNav( NAV_A_ARGRENADES, NULL);
	importNav( NAV_A_357, NULL);
	importNav( NAV_A_RPGCLIP, NULL);
	importNav( NAV_A_RPG_ROCKET, NULL);
	importNav( NAV_A_BUCKSHOT, NULL);
	importNav( NAV_A_GLOCKCLIP, NULL);
	// import funcs	
	importNav( NAV_F_TANK, NULL);
	importNav( NAV_F_TANKCONTROLS, NULL);
	importNav( NAV_F_RECHARGE, NULL);
	importNav( NAV_F_HEALTHCHARGER, NULL);
	// import items
	importNav(  NAV_I_HEALTHKIT, NULL);
	importNav(  NAV_I_LONGJUMP, NULL);
	importNav(  NAV_I_AIRTANK, NULL);
	importNav(  NAV_I_BATTERY, NULL);
}


void importCS_Specifics()
{
	importNav( NAV_CS_BOMB_TARGET, NULL);		// Bombzone in DE-maps
	importNav( NAV_CS_BUYZONE, NULL);
	importNav( NAV_CS_ESCAPEZONE, NULL);			// Escapezone for terrorists in ES-maps
//	importNav( NAV_CS_GRENCATCH, NULL);
	importNav( NAV_CS_HOSTAGE_RESCUE, NULL);		// Rescuezone for hostages in CS-maps
//	importNav( NAV_CS_VEHICLE, NULL);
	importNav( NAV_CS_VEHICLECONTROLS, NULL);
	importNav( NAV_CS_VIP_SAFETYZONE, NULL);		// Rescuezone for VIP in AS-maps
//	importNav( NAV_CS_WEAPONCHECK, NULL);
	importNav( NAV_CS_HOSTAGE_ITY, NULL);
//	importNav( NAV_CSI_BOMB_TARGET, NULL);
//	importNav( NAV_CSI_HOSTAGE_RESCUE, NULL);
//	importNav( NAV_CSI_MAP_PARAMETERS, NULL);
	importNav( NAV_CSI_VIP_START, NULL);			// VIP-spawnpoint
	importNav( NAV_INFO_PLAYER_START, NULL);		// CT-spawnpoint (T-spawnpoint=PLAYER_DM)
}

void importTFC_Specifics()
{
	importNav( NAV_TFC_TEAMSPAWN, NULL);
	importNav( NAV_TFC_GOAL, NULL);
	importNav( NAV_TFC_ARMOR1, NULL);
	importNav( NAV_I_HEALTHKIT, NULL);
}

void importHW_Specifics()
{
	importNav( NAV_F_HEALTHCHARGER, NULL);
	importNav( NAV_I_HEALTHKIT, NULL);
	importNav( NAV_I_BATTERY, NULL);

	importNav( NAV_HWW_DOUBLESHOTGUN, NULL);
	importNav( NAV_HWW_JACKHAMMER, NULL);
	importNav( NAV_HWW_RAILGUN, NULL);
	importNav( NAV_HWW_MACHINEGUN, NULL);
	importNav( NAV_HWW_ROCKETLAUNCHER, NULL);

	importNav( NAV_HWA_DOUBLESHOTGUN, NULL);
	importNav( NAV_HWA_RAILGUN, NULL);
	importNav( NAV_HWA_MACHINEGUN, NULL);
	importNav( NAV_HWA_ROCKETLAUNCHER, NULL);

	importNav( NAV_HW_HALOBASE, NULL);
	importNav( NAV_HW_JUMPPAD_TARGET, NULL);
//	importNav( NAV_HW_TRIG_JUMPPAD, NULL);
//	importNav( NAV_HW_JUMPPAD_SIGN, NULL);
}


void importDMC_Specififcs()
{
	importNav( NAV_DMCI_ARMOR1, NULL);
	importNav( NAV_DMCI_ARMOR2, NULL);
	importNav( NAV_DMCI_ARMOR3, NULL);
	importNav( NAV_DMCI_ARMOR_INV, NULL);
	importNav( NAV_DMCI_ENVIROSUIT, NULL);
	importNav( NAV_DMCI_INVISIBILITY, NULL);
	importNav( NAV_DMCI_INVULNERABILITY, NULL);
	importNav( NAV_DMCI_SUPERDAMAGE, NULL);
	importNav( NAV_DMCI_CELLS, NULL);
	importNav( NAV_DMCI_HEALTH_SMALL, "models/w_medkits.mdl" );
	importNav( NAV_DMCI_HEALTH_NORM,  "models/w_medkit.mdl" );
	importNav( NAV_DMCI_HEALTH_LARGE, "models/w_medkitl.mdl" );
	importNav( NAV_DMCI_ROCKETS, NULL);
	importNav( NAV_DMCI_SHELLS, NULL);
	importNav( NAV_DMCI_SPIKES, NULL);
	importNav( NAV_DMCI_WEAPON, NULL);
	importNav( NAV_DMCW_SUPERSHOTGUN, NULL);
	importNav( NAV_DMCW_NAILGUN, NULL);
	importNav( NAV_DMCW_SUPERNAILGUN, NULL);
	importNav( NAV_DMCW_GRENLAUNCHER, NULL);
	importNav( NAV_DMCW_ROCKETLAUNCHER, NULL);
	importNav( NAV_DMCW_LIGHTNING, NULL);
}


void importGearbox_Specifics()
{
	importHL_Specifics();
	importNav( NAV_OFW_GRAPPLE, NULL);
	importNav( NAV_OFW_EAGLE, NULL);
	importNav( NAV_OFW_PIPEWRENCH, NULL);
	importNav( NAV_OFW_M249, NULL);
	importNav( NAV_OFW_DISPLACER, NULL);
	importNav( NAV_OFW_SHOCKRIFLE, NULL);
	importNav( NAV_OFW_SPORELAUNCHER, NULL);
	importNav( NAV_OFW_SNIPERRIFLE, NULL);
	importNav( NAV_OFW_KNIFE, NULL);
	importNav( NAV_OFW_PENGUIN, NULL);
	importNav( NAV_OFA_556, NULL);
	importNav( NAV_OFA_762, NULL);
	importNav( NAV_OFA_EAGLECLIP, NULL);
	importNav( NAV_OFA_SPORE, NULL);
}

void importAG_Specifics()
{
	importHL_Specifics();
	importNav( NAV_AGINFO_HMCTFDETECT, NULL);
	importNav( NAV_AGINFO_PLAYER_ITEM1, NULL);
	importNav( NAV_AGINFO_PLAYER_ITEM2, NULL);
	importNav( NAV_AGI_FLAG_TEAM1, NULL);
	importNav( NAV_AGI_FLAG_TEAM2, NULL);
	importNav( NAV_AGI_DOM_CONTROLPOINT, NULL);
}

void importHunger_Specifics()
{
	importHL_Specifics();
	importNav( NAV_THW_AP9, NULL);
	importNav( NAV_THW_CHAINGUN, NULL);
	importNav( NAV_THW_EINAR1, NULL);
	importNav( NAV_THW_MEDKIT, NULL);
	importNav( NAV_THW_SNIPER, NULL);
	importNav( NAV_THW_SHOVEL, NULL);
	importNav( NAV_THW_SPANNER, NULL);
	importNav( NAV_THW_TAURUS, NULL);
	importNav( NAV_THA_AP9, NULL);
	importNav( NAV_THA_SNIPER, NULL);
	importNav( NAV_THA_TAURUS, NULL);
}

bool loadLevelData()
{
	char fileName[100];
	
	if (Q_STREQ(STRING(com.globals->mapname), actualMapname))
		return true;

	if (!Q_STREQ(actualMapname, "") && mapGraph.numberOfNavpoints() > 0)
		saveLevelData();
	
	mapGraph.clear();
	map.clear();
	strcpy( actualMapname, STRING(com.globals->mapname) );
	strcpy( fileName, com.modname );
	strcat( fileName, "/addons/parabot/navpoints/" );
	strcat( fileName, STRING(com.globals->mapname) );
	strcat( fileName, ".pnf" );
	if (!mapGraph.load( fileName ))	{
		INFO_MSG( "Importing level data...\n" );

	// import funcs
		importNav( NAV_F_BUTTON, NULL);
		importNav( NAV_F_ROT_BUTTON, NULL);
		importNav( NAV_F_DOOR, NULL);
		importNav( NAV_F_DOOR_ROTATING, NULL);
		importNav( NAV_F_PLAT, NULL);
		importNav( NAV_F_PLATROT, NULL);
		importNav( NAV_F_TRAIN, NULL);
		importNav( NAV_F_BREAKABLE, NULL);

	// import player-start
		importNav(  NAV_INFO_PLAYER_DM, NULL);

	// teleport
		importNav(  NAV_INFO_TELEPORT_DEST, NULL);	
		importNav(  NAV_TRIG_TELEPORT, NULL);	

	// import ladders
		EDICT *pOther = NULL;
		NAVPOINT n;
		Vec3D posUp, posDown;

		while ((pOther = find_entitybyclassname(pOther, "func_ladder"))) {
			boxcenter(pOther, &posUp);
			posUp.z = pOther->v.absmax.z;
			vcopy(&posUp, &posDown);
			posDown.z = pOther->v.absmin.z;
		/*	Vec3D dir = pOther->v.absmax - pOther->v.absmin;
			dir.z = 0;
			dir = CrossProduct(dir, nullvector);
			normalize(dir);
			dir = 30 * dir;	
			TRACERESULT tr;
			trace_line( posUp, posDown+dir, true, false, NULL, &tr );
			if (tr.fraction < 1.0f) posDown = posDown - dir;
			else				   posDown = posDown + dir; */
			posUp.z += 36;
			posDown.z += 36;
			navpoint_init(&n, &posUp, NAV_F_LADDER_TOP, 0 );
			mapGraph.addNavpoint(&n);
			navpoint_init(&n, &posDown, NAV_F_LADDER_BOTTOM, 0 );
			mapGraph.addNavpoint(&n);
		}

		// import MOD-specifics
		switch( mod_id ) {
		case AG_DLL:		importAG_Specifics();		break;
		case HUNGER_DLL:	importHunger_Specifics();	break;
		case VALVE_DLL:		importHL_Specifics();		break;
		case CSTRIKE_DLL:	importCS_Specifics();		break;
		case TFC_DLL:		importTFC_Specifics();		break;
		case HOLYWARS_DLL:	importHW_Specifics();		break;
		case DMC_DLL:		importDMC_Specififcs();		break;
		case GEARBOX_DLL:	importGearbox_Specifics();	break;
		default:			ERROR_MSG( "Unsupported MOD in pb_mapimport.cpp!\n" );
		}

		// import specials
		if ((mod_id==VALVE_DLL || mod_id==AG_DLL) && Q_STREQ( STRING(com.globals->mapname), "crossfire" ) ) {
			Vec3D v = { 0,-2236,-1852 };
			navpoint_init(&n, &v, NAV_S_AIRSTRIKE_BUTTON, 0 );
			mapGraph.addNavpoint(&n);
		}
	} else {
		fileName[strlen( fileName ) - 4] = '\0'; // cut file extention
		strcat( fileName, ".pcf" );
		map.load( fileName );
		INFO_MSG( "Loaded level data.\n" );
	}
	if (mapGraph.numberOfNavpoints() > 0 ) return true;
	else return false;
}
