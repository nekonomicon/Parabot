#include "mapimport.h"
#include "parabot.h"
#include "pb_global.h"
#include "sectors.h"
#include "vistable.h"
#include "cell.h"
#include "mapcells.h"
#include "bot.h"

extern int mod_id;

char actualMapname[100];

void
saveleveldata()
{
	char fileName[100];

	// Directory exists?
	strcpy( fileName, com.modname );
	strcat( fileName, "/addons/parabot/navpoints/" );
	CreateDirectory( fileName, NULL );

	strcat( fileName, actualMapname );
	strcat( fileName, ".pnf" );
	INFO_MSG( "\nSaving level data to %s\n", fileName );
	mapgraph_save( fileName );

	fileName[strlen(fileName) - 4] = '\0'; // cut file extention
	strcat( fileName, ".pcf" );
	INFO_MSG( "\nSaving cell data to %s\n", fileName );
	mapcells_save( fileName );
}

static void
importnav(int code, const char *modelName)
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
		mapgraph_addnavpoint(&n);
	}
}

static void
importhl_specifics()
{
	// import weapons
	importnav( NAV_W_CROSSBOW, NULL);
	importnav( NAV_W_CROWBAR, NULL);
	importnav( NAV_W_EGON, NULL);
	importnav( NAV_W_GAUSS, NULL);
	importnav( NAV_W_HANDGRENADE, NULL);
	importnav( NAV_W_HORNETGUN, NULL);
	importnav( NAV_W_MP5, NULL);
	importnav( NAV_W_9MMAR, NULL);
	importnav( NAV_W_PYTHON, NULL);
	importnav( NAV_W_357, NULL);
	importnav( NAV_W_RPG, NULL);
	importnav( NAV_W_SATCHEL, NULL);
	importnav( NAV_W_SHOTGUN, NULL);
	importnav( NAV_W_SNARK, NULL);
	importnav( NAV_W_TRIPMINE, NULL);
	importnav( NAV_W_GLOCK, NULL);
	importnav( NAV_W_9MMHANDGUN, NULL);
	// import ammo
	importnav( NAV_A_CROSSBOW, NULL);
	importnav( NAV_A_CROSSBOW_BOLT, NULL);
	importnav( NAV_A_EGONCLIP, NULL);
	importnav( NAV_A_GAUSSCLIP, NULL);
	importnav( NAV_A_MP5CLIP, NULL);
	importnav( NAV_A_MP5GRENADES, NULL );
	importnav( NAV_A_9MMAR, NULL);
	importnav( NAV_A_9MMBOX, NULL);
	importnav( NAV_A_9MMCLIP, NULL);
	importnav( NAV_A_ARGRENADES, NULL);
	importnav( NAV_A_357, NULL);
	importnav( NAV_A_RPGCLIP, NULL);
	importnav( NAV_A_RPG_ROCKET, NULL);
	importnav( NAV_A_BUCKSHOT, NULL);
	importnav( NAV_A_GLOCKCLIP, NULL);
	// import funcs	
	importnav( NAV_F_TANK, NULL);
	importnav( NAV_F_TANKCONTROLS, NULL);
	importnav( NAV_F_RECHARGE, NULL);
	importnav( NAV_F_HEALTHCHARGER, NULL);
	// import items
	importnav(  NAV_I_HEALTHKIT, NULL);
	importnav(  NAV_I_LONGJUMP, NULL);
	importnav(  NAV_I_AIRTANK, NULL);
	importnav(  NAV_I_BATTERY, NULL);
}

static void
importcs_specifics()
{
	importnav( NAV_CS_BOMB_TARGET, NULL);		// Bombzone in DE-maps
	importnav( NAV_CS_BUYZONE, NULL);
	importnav( NAV_CS_ESCAPEZONE, NULL);			// Escapezone for terrorists in ES-maps
//	importnav( NAV_CS_GRENCATCH, NULL);
	importnav( NAV_CS_HOSTAGE_RESCUE, NULL);		// Rescuezone for hostages in CS-maps
//	importnav( NAV_CS_VEHICLE, NULL);
	importnav( NAV_CS_VEHICLECONTROLS, NULL);
	importnav( NAV_CS_VIP_SAFETYZONE, NULL);		// Rescuezone for VIP in AS-maps
//	importnav( NAV_CS_WEAPONCHECK, NULL);
	importnav( NAV_CS_HOSTAGE_ITY, NULL);
//	importnav( NAV_CSI_BOMB_TARGET, NULL);
//	importnav( NAV_CSI_HOSTAGE_RESCUE, NULL);
//	importnav( NAV_CSI_MAP_PARAMETERS, NULL);
	importnav( NAV_CSI_VIP_START, NULL);			// VIP-spawnpoint
	importnav( NAV_INFO_PLAYER_START, NULL);		// CT-spawnpoint (T-spawnpoint=PLAYER_DM)
}

static void
importtfc_specifics()
{
	importnav( NAV_TFC_TEAMSPAWN, NULL);
	importnav( NAV_TFC_GOAL, NULL);
	importnav( NAV_TFC_ARMOR1, NULL);
	importnav( NAV_I_HEALTHKIT, NULL);
}

static void
importhw_specifics()
{
	importnav( NAV_F_HEALTHCHARGER, NULL);
	importnav( NAV_I_HEALTHKIT, NULL);
	importnav( NAV_I_BATTERY, NULL);

	importnav( NAV_HWW_DOUBLESHOTGUN, NULL);
	importnav( NAV_HWW_JACKHAMMER, NULL);
	importnav( NAV_HWW_RAILGUN, NULL);
	importnav( NAV_HWW_MACHINEGUN, NULL);
	importnav( NAV_HWW_ROCKETLAUNCHER, NULL);

	importnav( NAV_HWA_DOUBLESHOTGUN, NULL);
	importnav( NAV_HWA_RAILGUN, NULL);
	importnav( NAV_HWA_MACHINEGUN, NULL);
	importnav( NAV_HWA_ROCKETLAUNCHER, NULL);

	importnav( NAV_HW_HALOBASE, NULL);
	importnav( NAV_HW_JUMPPAD_TARGET, NULL);
//	importnav( NAV_HW_TRIG_JUMPPAD, NULL);
//	importnav( NAV_HW_JUMPPAD_SIGN, NULL);
}

static void
importdmc_specifics()
{
	importnav( NAV_DMCI_ARMOR1, NULL);
	importnav( NAV_DMCI_ARMOR2, NULL);
	importnav( NAV_DMCI_ARMOR3, NULL);
	importnav( NAV_DMCI_ARMOR_INV, NULL);
	importnav( NAV_DMCI_ENVIROSUIT, NULL);
	importnav( NAV_DMCI_INVISIBILITY, NULL);
	importnav( NAV_DMCI_INVULNERABILITY, NULL);
	importnav( NAV_DMCI_SUPERDAMAGE, NULL);
	importnav( NAV_DMCI_CELLS, NULL);
	importnav( NAV_DMCI_HEALTH_SMALL, "models/w_medkits.mdl" );
	importnav( NAV_DMCI_HEALTH_NORM,  "models/w_medkit.mdl" );
	importnav( NAV_DMCI_HEALTH_LARGE, "models/w_medkitl.mdl" );
	importnav( NAV_DMCI_ROCKETS, NULL);
	importnav( NAV_DMCI_SHELLS, NULL);
	importnav( NAV_DMCI_SPIKES, NULL);
	importnav( NAV_DMCI_WEAPON, NULL);
	importnav( NAV_DMCW_SUPERSHOTGUN, NULL);
	importnav( NAV_DMCW_NAILGUN, NULL);
	importnav( NAV_DMCW_SUPERNAILGUN, NULL);
	importnav( NAV_DMCW_GRENLAUNCHER, NULL);
	importnav( NAV_DMCW_ROCKETLAUNCHER, NULL);
	importnav( NAV_DMCW_LIGHTNING, NULL);
}

static void
importgearbox_specifics()
{
	importhl_specifics();
	importnav( NAV_OFW_GRAPPLE, NULL);
	importnav( NAV_OFW_EAGLE, NULL);
	importnav( NAV_OFW_PIPEWRENCH, NULL);
	importnav( NAV_OFW_M249, NULL);
	importnav( NAV_OFW_DISPLACER, NULL);
	importnav( NAV_OFW_SHOCKRIFLE, NULL);
	importnav( NAV_OFW_SPORELAUNCHER, NULL);
	importnav( NAV_OFW_SNIPERRIFLE, NULL);
	importnav( NAV_OFW_KNIFE, NULL);
	importnav( NAV_OFW_PENGUIN, NULL);
	importnav( NAV_OFA_556, NULL);
	importnav( NAV_OFA_762, NULL);
	importnav( NAV_OFA_EAGLECLIP, NULL);
	importnav( NAV_OFA_SPORE, NULL);
}

static void
importag_specifics()
{
	importhl_specifics();
	importnav( NAV_AGINFO_HMCTFDETECT, NULL);
	importnav( NAV_AGINFO_PLAYER_ITEM1, NULL);
	importnav( NAV_AGINFO_PLAYER_ITEM2, NULL);
	importnav( NAV_AGI_FLAG_TEAM1, NULL);
	importnav( NAV_AGI_FLAG_TEAM2, NULL);
	importnav( NAV_AGI_DOM_CONTROLPOINT, NULL);
}

static void
importhunger_specifics()
{
	importhl_specifics();
	importnav( NAV_THW_AP9, NULL);
	importnav( NAV_THW_CHAINGUN, NULL);
	importnav( NAV_THW_EINAR1, NULL);
	importnav( NAV_THW_MEDKIT, NULL);
	importnav( NAV_THW_SNIPER, NULL);
	importnav( NAV_THW_SHOVEL, NULL);
	importnav( NAV_THW_SPANNER, NULL);
	importnav( NAV_THW_TAURUS, NULL);
	importnav( NAV_THA_AP9, NULL);
	importnav( NAV_THA_SNIPER, NULL);
	importnav( NAV_THA_TAURUS, NULL);
}

bool
loadleveldata()
{
	char fileName[100];
	
	if (Q_STREQ(STRING(com.globals->mapname), actualMapname))
		return true;

	if (!Q_STREQ(actualMapname, "") && mapgraph_numberofnavpoints() > 0)
		saveleveldata();
	
	mapgraph_clear();
	mapcells_clear();
	strcpy( actualMapname, STRING(com.globals->mapname) );
	strcpy( fileName, com.modname );
	strcat( fileName, "/addons/parabot/navpoints/" );
	strcat( fileName, STRING(com.globals->mapname) );
	strcat( fileName, ".pnf" );
	if (!mapgraph_load( fileName ))	{
		INFO_MSG( "Importing level data...\n" );

	// import funcs
		importnav( NAV_F_BUTTON, NULL);
		importnav( NAV_F_ROT_BUTTON, NULL);
		importnav( NAV_F_DOOR, NULL);
		importnav( NAV_F_DOOR_ROTATING, NULL);
		importnav( NAV_F_PLAT, NULL);
		importnav( NAV_F_PLATROT, NULL);
		importnav( NAV_F_TRAIN, NULL);
		importnav( NAV_F_BREAKABLE, NULL);

	// import player-start
		importnav(  NAV_INFO_PLAYER_DM, NULL);

	// teleport
		importnav(  NAV_INFO_TELEPORT_DEST, NULL);	
		importnav(  NAV_TRIG_TELEPORT, NULL);	

	// import ladders
		EDICT *pOther = NULL;
		NAVPOINT n;
		Vec3D posUp, posDown;

		while ((pOther = find_entitybyclassname(pOther, "func_ladder"))) {
			boxcenter(pOther, &posUp);
			posUp.z = pOther->v.absmax.z;
			posDown = posUp;
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
			mapgraph_addnavpoint(&n);
			navpoint_init(&n, &posDown, NAV_F_LADDER_BOTTOM, 0 );
			mapgraph_addnavpoint(&n);
		}

		// import MOD-specifics
		switch( mod_id ) {
		case AG_DLL:		importag_specifics();		break;
		case HUNGER_DLL:	importhunger_specifics();	break;
		case VALVE_DLL:		importhl_specifics();		break;
		case CSTRIKE_DLL:	importcs_specifics();		break;
		case TFC_DLL:		importtfc_specifics();		break;
		case HOLYWARS_DLL:	importhw_specifics();		break;
		case DMC_DLL:		importdmc_specifics();		break;
		case GEARBOX_DLL:	importgearbox_specifics();	break;
		default:			ERROR_MSG( "Unsupported MOD in pb_mapimport.cpp!\n" );
		}

		// import specials
		if ((mod_id==VALVE_DLL || mod_id==AG_DLL) && Q_STREQ( STRING(com.globals->mapname), "crossfire" ) ) {
			Vec3D v = { 0,-2236,-1852 };
			navpoint_init(&n, &v, NAV_S_AIRSTRIKE_BUTTON, 0 );
			mapgraph_addnavpoint(&n);
		}
	} else {
		fileName[strlen( fileName ) - 4] = '\0'; // cut file extention
		strcat( fileName, ".pcf" );
		mapcells_load( fileName );
		INFO_MSG( "Loaded level data.\n" );
	}
	if (mapgraph_numberofnavpoints() > 0 ) return true;
	return false;
}
