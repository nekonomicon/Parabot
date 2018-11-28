#pragma once
#if !defined(PB_NAVPOINT_H)
#define PB_NAVPOINT_H
#include "sdk_common.h"

enum {
	NAV_W_CROSSBOW = 1,
	NAV_W_CROWBAR,
	NAV_W_EGON,
	NAV_W_GAUSS,
	NAV_W_HANDGRENADE,
	NAV_W_HORNETGUN,
	NAV_W_MP5,
	NAV_W_9MMAR,	// ok
	NAV_W_PYTHON,
	NAV_W_357,// ok
	NAV_W_RPG,
	NAV_W_SATCHEL,
	NAV_W_SHOTGUN,
	NAV_W_SNARK,
	NAV_W_TRIPMINE,
	NAV_W_GLOCK,
	NAV_W_9MMHANDGUN,

	NAV_A_CROSSBOW = 20, // ok
	NAV_A_CROSSBOW_BOLT,
	NAV_A_EGONCLIP,
	NAV_A_GAUSSCLIP, // ok
	NAV_A_MP5CLIP,
	NAV_A_MP5GRENADES,
	NAV_A_9MMAR,	// ok
	NAV_A_9MMBOX,
	NAV_A_9MMCLIP,
	NAV_A_ARGRENADES,	// ok
	NAV_A_357,	// ok
	NAV_A_RPGCLIP,	// ok
	NAV_A_RPG_ROCKET,
	NAV_A_BUCKSHOT,	// ok
	NAV_A_GLOCKCLIP,

	NAV_F_BUTTON = 40,
	NAV_F_ROT_BUTTON,
	NAV_F_DOOR,
	NAV_F_DOOR_ROTATING,
	NAV_F_TANK,
	NAV_F_TANKCONTROLS,
	NAV_F_PLAT,
	NAV_F_PLATROT,
	NAV_F_LADDER_TOP,
	NAV_F_LADDER_BOTTOM,
	NAV_F_RECHARGE,
	NAV_F_HEALTHCHARGER,
	NAV_F_BREAKABLE,
	NAV_F_TRAIN,

	NAV_I_HEALTHKIT	= 60,
	NAV_I_LONGJUMP,
	NAV_I_AIRTANK,
	NAV_I_BATTERY,

	NAV_INFO_TELEPORT_DEST = 70,
	NAV_INFO_PLAYER_DM,
	NAV_INFO_PLAYER_START,

	NAV_TRIG_TELEPORT = 80,

	NAV_S_CAMPING = 90,
	NAV_S_SPAWNPOINT,
	NAV_S_USE_TRIPMINE,
	NAV_S_AIRSTRIKE_COVER,
	NAV_S_AIRSTRIKE_BUTTON,
	NAV_S_BUTTON_SHOT,

// CS - Navpoints
	NAV_CS_BOMB_TARGET = 100,
	NAV_CS_BUYZONE,
	NAV_CS_ESCAPEZONE,
//	NAV_CS_GRENCATCH,
	NAV_CS_HOSTAGE_RESCUE = 104,
//	NAV_CS_VEHICLE,
	NAV_CS_VEHICLECONTROLS = 106,
	NAV_CS_VIP_SAFETYZONE,
//	NAV_CS_WEAPONCHECK,
	NAV_CS_HOSTAGE_ITY = 109,
//	NAV_CSI_BOMB_TARGET,
//	NAV_CSI_HOSTAGE_RESCUE,
//	NAV_CSI_MAP_PARAMETERS,
	NAV_CSI_VIP_START = 113,

// HolyWars Navpoints
	NAV_HWW_DOUBLESHOTGUN = 120,
	NAV_HWW_JACKHAMMER,
	NAV_HWW_RAILGUN,
	NAV_HWW_MACHINEGUN,
	NAV_HWW_ROCKETLAUNCHER,

	NAV_HWA_DOUBLESHOTGUN = 130,
	NAV_HWA_RAILGUN,
	NAV_HWA_MACHINEGUN,
	NAV_HWA_ROCKETLAUNCHER,

	NAV_HW_HALOBASE = 140,
	NAV_HW_JUMPPAD_TARGET,
	NAV_HW_TRIG_JUMPPAD,
	NAV_HW_JUMPPAD_SIGN,

// DMC Navpoints
	NAV_DMCI_ARMOR1 = 150,		// armor = 100
	NAV_DMCI_ARMOR2,		// armor = 150
	NAV_DMCI_ARMOR3,		// ???
	NAV_DMCI_ARMOR_INV,		// armor = 200
	NAV_DMCI_ENVIROSUIT,		// ???
	NAV_DMCI_INVISIBILITY,
	NAV_DMCI_INVULNERABILITY,
	NAV_DMCI_SUPERDAMAGE,
	NAV_DMCI_HEALTH_SMALL,		// +15 health
	NAV_DMCI_HEALTH_NORM,		// +25 health
	NAV_DMCI_HEALTH_LARGE,		// ++100 health
	NAV_DMCI_CELLS,
	NAV_DMCI_ROCKETS,
	NAV_DMCI_SHELLS,
	NAV_DMCI_SPIKES,
	NAV_DMCI_WEAPON,
//LINK_ITY_TO_FUNC( quake_nail, "quake_nail" );
//LINK_ITY_TO_FUNC( quake_rocket, "quake_rocket" );
//LINK_ITY_TO_FUNC( teledeath, "teledeath" );
	NAV_DMCW_QUAKEGUN = 170,
	NAV_DMCW_SUPERSHOTGUN,
	NAV_DMCW_NAILGUN,
	NAV_DMCW_SUPERNAILGUN,
	NAV_DMCW_GRENLAUNCHER,
	NAV_DMCW_ROCKETLAUNCHER,
	NAV_DMCW_LIGHTNING,

// TFC - Navpoints
	NAV_TFC_TEAMSPAWN,
	NAV_TFC_ARMOR1,
	NAV_TFC_GOAL,

// Gearbox - Navpoints
	NAV_OFW_GRAPPLE	= 190,
	NAV_OFW_EAGLE,
	NAV_OFW_PIPEWRENCH,
	NAV_OFW_M249,
	NAV_OFW_DISPLACER,
	NAV_OFW_SHOCKRIFLE,
	NAV_OFW_SPORELAUNCHER,
	NAV_OFW_SNIPERRIFLE,
	NAV_OFW_KNIFE,
	NAV_OFW_PENGUIN,
	NAV_OFA_556,
	NAV_OFA_762,
	NAV_OFA_EAGLECLIP,
	NAV_OFA_SPORE,

// They Hunger - Navpoints
	NAV_THW_AP9,
	NAV_THW_CHAINGUN,
	NAV_THW_EINAR1,
	NAV_THW_MEDKIT,
	NAV_THW_SNIPER,
	NAV_THW_SHOVEL,
	NAV_THW_SPANNER,
	NAV_THW_TAURUS,
	NAV_THA_AP9,
	NAV_THA_SNIPER,
	NAV_THA_TAURUS,

// Adrenaline Gamer - Navpoints
	NAV_AGINFO_HMCTFDETECT,
	NAV_AGINFO_PLAYER_ITEM1,
	NAV_AGINFO_PLAYER_ITEM2,
	NAV_AGI_FLAG_TEAM1,
	NAV_AGI_FLAG_TEAM2,
	NAV_AGI_DOM_CONTROLPOINT,

	MAX_NAV_TYPES = NAV_AGI_DOM_CONTROLPOINT + 1		// keep up to date!!!
};

class PB_Navpoint
{

public:	

	static const char* classname( int code );
	
	void init( Vec3D *pos, int type, int special );
	void setId( int id ) { data.privateId = id; }
	void initEntityPtr();
	// gets a pointer to the corresponding t_edict if possible and stores it

	int id() { return data.privateId; }
	int type() { return data.type; }
	const char* classname();
	int special() { return data.special; }
	EDICT* entity() { return ent; }
	
	Vec3D *pos() { return &data.pos; }
	void pos( EDICT *playerEnt, Vec3D *pos);

	//bool offers( int classType );
	// returns true if navpoint is of type or belongs to superclass type

	bool doorOpen( EDICT *playerEnt );
	// call only for NAV_F_DOOR and NAV_F_DOOR_ROTATING
	bool visible( EDICT *playerEnt );
	bool reached( EDICT *playerEnt );

	void reportVisit( EDICT *player, float time );
	// reports a visit to the navpoint
	void doNotVisitBefore( EDICT *player, float time );
	// tells that player should not visit this navpoint before time
	float nextVisit( EDICT *player );
	// returns the worldtime after which navpoint can be visited again

	bool offersHealth(); 
	bool offersArmor(); 
	bool offersCamping() { return (data.type==NAV_S_CAMPING); }

	bool needsTriggering() { return needsTrigger; };
	bool isTriggerFor( PB_Navpoint &wp );
	bool isTriggered();
	
	void load( FILE *fp );
	void save( FILE *fp );
#if _DEBUG
	void print();	// DEBUG_MSG navName
	void printPos();	// DEBUG_MSG navName
#endif

	// operators for graph algorithms:
	bool operator==(const PB_Navpoint& O) const  {  return data.privateId == O.data.privateId; }
    bool operator<(const PB_Navpoint& O) const   {  return data.privateId < O.data.privateId;  }
	

private:

	typedef struct {
		int		privateId;
		int		type;		// type of navpoint
		Vec3D	pos;		// position
		int		visits;		// number of visits
		int		special;	// e.g. viewangle for camping-spots
		float	damageComp;	// ((damage done to others) - (own damage)) for camping-spots
	} TSaveData;

	TSaveData	data;
	EDICT		*ent;			// pointer to entity
	float		lastVisitedAt, nextVisitAt;	// in worldtime
	EDICT		*lastVisitor;	// last player visiting this navpoint
	bool		needsTrigger;
	int			normalState;
};


#endif
