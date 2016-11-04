#if !defined( PB_NAVPOINT_H )
#define PB_NAVPOINT_H


#include "extdll.h"


#define NAV_W_CROSSBOW			1	
#define NAV_W_CROWBAR			2
#define NAV_W_EGON				3
#define NAV_W_GAUSS				4
#define NAV_W_HANDGRENADE		5
#define NAV_W_HORNETGUN			6
#define NAV_W_MP5				7
#define NAV_W_9MMAR				8	// ok
#define NAV_W_PYTHON			9
#define NAV_W_357				10	// ok
#define NAV_W_RPG				11
#define NAV_W_SATCHEL			12
#define NAV_W_SHOTGUN			13
#define NAV_W_SNARK				14
#define NAV_W_TRIPMINE			15
#define NAV_W_GLOCK				16
#define NAV_W_9MMHANDGUN		17

#define NAV_A_CROSSBOW			20	// ok
#define NAV_A_CROSSBOW_BOLT		21
#define NAV_A_EGONCLIP			22
#define NAV_A_GAUSSCLIP			23	// ok
#define NAV_A_MP5CLIP			24
#define NAV_A_MP5GRENADES		25
#define NAV_A_9MMAR				26	// ok
#define NAV_A_9MMBOX			27
#define NAV_A_9MMCLIP			28
#define NAV_A_ARGRENADES		29	// ok
#define NAV_A_357				30	// ok
#define NAV_A_RPGCLIP			31	// ok
#define NAV_A_RPG_ROCKET		32
#define NAV_A_BUCKSHOT			33	// ok
#define NAV_A_GLOCKCLIP			34

#define NAV_F_BUTTON			40
#define NAV_F_ROT_BUTTON		41
#define NAV_F_DOOR				42
#define NAV_F_DOOR_ROTATING		43
#define NAV_F_TANK				44
#define NAV_F_TANKCONTROLS		45
#define NAV_F_PLAT				46
#define NAV_F_PLATROT			47
#define NAV_F_LADDER_TOP		48
#define NAV_F_LADDER_BOTTOM		49
#define NAV_F_RECHARGE			50
#define NAV_F_HEALTHCHARGER		51
#define NAV_F_BREAKABLE			52
#define NAV_F_TRAIN				53

#define NAV_I_HEALTHKIT			60
#define NAV_I_LONGJUMP			61
#define NAV_I_AIRTANK			62
#define NAV_I_BATTERY			63

#define NAV_INFO_TELEPORT_DEST	70
#define NAV_INFO_PLAYER_DM		71
#define NAV_INFO_PLAYER_START	72

#define NAV_TRIG_TELEPORT		80

#define NAV_S_CAMPING			90
#define NAV_S_SPAWNPOINT		91
#define NAV_S_USE_TRIPMINE		92
#define NAV_S_AIRSTRIKE_COVER	93
#define NAV_S_AIRSTRIKE_BUTTON	94
#define NAV_S_BUTTON_SHOT		95

// CS - Navpoints
#define NAV_CS_BOMB_TARGET		100
#define NAV_CS_BUYZONE			101
#define NAV_CS_ESCAPEZONE		102
//#define NAV_CS_GRENCATCH		103
#define NAV_CS_HOSTAGE_RESCUE	104
//#define NAV_CS_VEHICLE			105
#define NAV_CS_VEHICLECONTROLS	106
#define NAV_CS_VIP_SAFETYZONE	107
//#define NAV_CS_WEAPONCHECK		108
#define NAV_CS_HOSTAGE_ENTITY	109
//#define NAV_CSI_BOMB_TARGET		110
//#define NAV_CSI_HOSTAGE_RESCUE	111
//#define NAV_CSI_MAP_PARAMETERS	112
#define NAV_CSI_VIP_START		113

// HolyWars Navpoints
#define NAV_HWW_DOUBLESHOTGUN	120
#define NAV_HWW_JACKHAMMER		121
#define NAV_HWW_RAILGUN			122
#define NAV_HWW_MACHINEGUN		123
#define NAV_HWW_ROCKETLAUNCHER	124

#define NAV_HWA_DOUBLESHOTGUN	130
#define NAV_HWA_RAILGUN			131
#define NAV_HWA_MACHINEGUN		132
#define NAV_HWA_ROCKETLAUNCHER	133

#define NAV_HW_HALOBASE			140
#define NAV_HW_JUMPPAD_TARGET	141
#define NAV_HW_TRIG_JUMPPAD		142
#define NAV_HW_JUMPPAD_SIGN		143

// DMC Navpoints
#define NAV_DMCI_ARMOR1			150		// armor = 100
#define NAV_DMCI_ARMOR2			151		// armor = 150
#define NAV_DMCI_ARMOR3			152		// ???
#define NAV_DMCI_ARMOR_INV		153		// armor = 200
#define NAV_DMCI_ENVIROSUIT		154		// ???
#define NAV_DMCI_INVISIBILITY	155
#define NAV_DMCI_INVULNERABILITY	156
#define NAV_DMCI_SUPERDAMAGE	157
#define NAV_DMCI_HEALTH_SMALL	159		// +15 health
#define NAV_DMCI_HEALTH_NORM	160		// +25 health
#define NAV_DMCI_HEALTH_LARGE	161		// ++100 health
#define NAV_DMCI_CELLS			162
#define NAV_DMCI_ROCKETS		163
#define NAV_DMCI_SHELLS			164
#define NAV_DMCI_SPIKES			165
#define NAV_DMCI_WEAPON			166
//LINK_ENTITY_TO_FUNC( quake_nail, "quake_nail" );
//LINK_ENTITY_TO_FUNC( quake_rocket, "quake_rocket" );
//LINK_ENTITY_TO_FUNC( teledeath, "teledeath" );
#define NAV_DMCW_QUAKEGUN		170
#define NAV_DMCW_SUPERSHOTGUN	171
#define NAV_DMCW_NAILGUN		172
#define NAV_DMCW_SUPERNAILGUN	173
#define NAV_DMCW_GRENLAUNCHER	174
#define NAV_DMCW_ROCKETLAUNCHER	175
#define NAV_DMCW_LIGHTNING		176

// TFC - Navpoints
#define NAV_TFC_TEAMSPAWN		180
#define NAV_TFC_ARMOR1			181
#define NAV_TFC_GOAL			182

// Gearbox - Navpoints
#define NAV_OFW_GRAPPLE			190
#define NAV_OFW_EAGLE			191
#define NAV_OFW_PIPEWRENCH		192
#define NAV_OFW_M249			193
#define NAV_OFW_DISPLACER		194
#define NAV_OFW_SHOCKRIFLE		195
#define NAV_OFW_SPORELAUNCHER	196
#define NAV_OFW_SNIPERRIFLE		197
#define NAV_OFW_KNIFE			198
#define NAV_OFW_PENGUIN			199
#define NAV_OFA_556				200
#define NAV_OFA_762				201
#define NAV_OFA_EAGLECLIP		202
#define NAV_OFA_SPORE			203

#define MAX_NAV_TYPES			204		// keep up to date!!!





class PB_Navpoint
{

public:	

	static char* classname( int code );
	
	void init( Vector &pos, int type, int special );
	void setId( int id ) { data.privateId = id; }
	void initEntityPtr();
	// gets a pointer to the corresponding t_edict if possible and stores it

	int id() { return data.privateId; }
	int type() { return data.type; }
	char* classname();
	int special() { return data.special; }
	edict_t* entity() { return ent; }
	
	Vector pos() { return data.pos; }
	Vector pos( edict_t *playerEnt );

	//bool offers( int classType );
	// returns true if navpoint is of type or belongs to superclass type

	bool doorOpen( edict_t *playerEnt );
	// call only for NAV_F_DOOR and NAV_F_DOOR_ROTATING
	bool visible( edict_t *playerEnt );
	bool reached( edict_t *playerEnt );

	void reportVisit( edict_t *player, float time );
	// reports a visit to the navpoint
	void doNotVisitBefore( edict_t *player, float time );
	// tells that player should not visit this navpoint before time
	float nextVisit( edict_t *player );
	// returns the worldTime after which navpoint can be visited again

	bool offersHealth(); 
	bool offersArmor(); 
	bool offersCamping() { return (data.type==NAV_S_CAMPING); }

	bool needsTriggering() { return needsTrigger; };
	bool isTriggerFor( PB_Navpoint &wp );
	bool isTriggered();
	
	void load( FILE *fp );
	void save( FILE *fp );
	void print();	// debugMsg navName
	void printPos();	// debugMsg navName


	// operators for graph algorithms:
	bool operator==(const PB_Navpoint& O) const  {  return data.privateId == O.data.privateId; }
    bool operator<(const PB_Navpoint& O) const   {  return data.privateId < O.data.privateId;  }
	

private:

	typedef struct {
		int		privateId;
		int		type;		// type of navpoint
		Vector	pos;		// position
		int		visits;		// number of visits
		int		special;	// e.g. viewangle for camping-spots
		float	damageComp;	// ((damage done to others) - (own damage)) for camping-spots
	} TSaveData;

	TSaveData	data;
	edict_t		*ent;			// pointer to entity
	float		lastVisitedAt, nextVisitAt;	// in worldtime
	edict_t		*lastVisitor;	// last player visiting this navpoint
	bool		needsTrigger;
	int			normalState;
};


#endif
