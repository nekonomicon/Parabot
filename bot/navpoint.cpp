#include "parabot.h"
#include "sectors.h"
#include "navpoint.h"
#include "pb_global.h"
#include <stdio.h>

extern int mod_id;

static const char *navpointClasses[MAX_NAV_TYPES] = { "unknown",
"weapon_crossbow",
"weapon_crowbar",
"weapon_egon",
"weapon_gauss",
"weapon_handgrenade",
"weapon_hornetgun",
"weapon_mp5",
"weapon_9mmAR",
"weapon_python",
"weapon_357",
"weapon_rpg",
"weapon_satchel",
"weapon_shotgun",
"weapon_snark",
"weapon_tripmine",
"weapon_glock",
"weapon_9mmhandgun",
"unknown",
"unknown",
"ammo_crossbow",
"crossbow_bolt",
"ammo_egonclip",
"ammo_gaussclip",
"ammo_mp5clip",
"ammo_mp5grenades",
"ammo_9mmAR",
"ammo_9mmbox",
"ammo_9mmclip",
"ammo_ARgrenades",
"ammo_357",
"ammo_rpgclip",
"rpg_rocket",
"ammo_buckshot",
"ammo_glockclip",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"func_button",
"func_rot_button",
"func_door",
"func_door_rotating",
"func_tank",
"func_tankcontrols",
"func_plat",
"func_platrot",
"func_ladder (top)",
"func_ladder (bottom)",
"func_recharge",
"func_healthcharger",
"func_breakable",
"func_train",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",	
"item_healthkit",
"item_longjump",
"item_airtank",
"item_battery",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",			
"info_teleport_destination",
"info_player_deathmatch",
"info_player_start",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",				
"trigger_teleport",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"special_camp_point",
"special_spawn_point",
"special_tripmine_hint",
"special_airstrike_cover",
"special_airstrike_button",
"special_button_shot",
"unknown",
"unknown",
"unknown",
"unknown",		
"func_bomb_target",
"func_buyzone",
"func_escapezone",
"unknown",//	case NAV_CS_GRENCATCH		: strcpy( classname, "func_grencatch");			break;
"func_hostage_rescue",
"unknown",//	case NAV_CS_VEHICLE			: strcpy( classname, "func_vehicle");			break;
"func_vehiclecontrols",
"func_vip_safetyzone",
"unknown",//	case NAV_CS_WEAPONCHECK		: strcpy( classname, "func_weaponcheck");		break;
"hostage_entity",
"unknown",//	case NAV_CSI_BOMB_TARGET	: strcpy( classname, "info_bomb_target");		break;
"unknown",//	case NAV_CSI_HOSTAGE_RESCUE	: strcpy( classname, "info_hostage_rescue");	break;
"unknown",//	case NAV_CSI_MAP_PARAMETERS	: strcpy( classname, "info_map_parameters");	break;
"info_vip_start",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"weapon_doubleshotgun",
"weapon_jackhammer",
"weapon_machinegun",
"weapon_railgun",
"weapon_rocketlauncher",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"ammo_doubleshotgun",
"ammo_railgun",
"ammo_machinegun",
"ammo_rocketlauncher",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"halo_base",
"info_jumppad_target",
"trigger_jumppad",
"jumppad_sign",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"item_armor1",
"item_armor2",
"item_armor3",
"item_armorInv",
"item_artifact_envirosuit",
"item_artifact_invisibility",
"item_artifact_invulnerability",
"item_artifact_super_damage",
"unknown",
"item_health",
"item_health",
"item_health",
"item_cells",
"item_rockets",
"item_shells",
"item_spikes",
"item_weapon",
"unknown",
"unknown",
"unknown",
"weapon_quakegun",
"weapon_supershotgun",
"weapon_nailgun",
"weapon_supernailgun",
"weapon_grenadelauncher",
"weapon_rocketlauncher",
"weapon_lightning",
"unknown",
"unknown",
"unknown",
"info_player_teamspawn",
"item_armor1",
"info_tfgoal",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"weapon_grapple",
"weapon_eagle",
"weapon_pipewrench",
"weapon_m249",
"weapon_displacer",
"weapon_shockrifle",
"weapon_sporelauncher",
"weapon_sniperrifle",
"weapon_knife",
"weapon_penguin",
"ammo_556",
"ammo_762",
"ammo_eagleclip",
"ammo_spore",
"weapon_th_ap9",
"weapon_th_chaingun",
"weapon_einar1",
"weapon_th_medkit",
"weapon_th_sniper",
"weapon_th_shovel",
"weapon_th_spanner",
"weapon_th_taurus",
"ammo_th_ap9",
"ammo_th_sniper",
"ammo_th_taurus",
"info_hmctfdetect",
"info_player_team1",
"info_player_team2",
"item_flag_team1",
"item_flag_team2",
"item_dom_controlpoint"
};

void
navpoint_init(NAVPOINT *navpoint, Vec3D *pos, int type, int special)
{
	vcopy(pos, &navpoint->data.pos);
	navpoint->data.type = type;
	navpoint->data.visits = 0;
	navpoint->data.special = special;
	navpoint->data.damagecomp = 0;
	navpoint->ent = 0;
	navpoint->lastvisitor = 0;
	navpoint->lastvisitedat = -100;
	navpoint->nextvisitat = 0;
}

void
navpoint_setid(NAVPOINT *navpoint, int id)
{
	navpoint->data.privateid = id;
}

void
navpoint_initentityptr(NAVPOINT *navpoint)
// gets a pointer to the corresponding t_edict if possible and stores it
{
	navpoint->needstrigger = false;
/*	char name[40];
	Nav2Classname( name, type() );
	ent = getEntity( name, data.pos );
	if ( (ent==0) &&
	// only crucial for certain objects...
	     ( (type() == NAV_F_BREAKABLE)	   ||
		   (type() == NAV_F_HEALTHCHARGER) || 
		   (type() == NAV_F_RECHARGE)      || 
		   (type() == NAV_F_TRAIN)		   || 
		   (type() == NAV_F_DOOR)             ) )
	{
		 DEBUG_MSG( "initEntityPtr() failed for %i\n", type() );
	}*/
	if ( (navpoint_type(navpoint) == NAV_F_BREAKABLE)	 ||
	     (navpoint_type(navpoint) == NAV_F_HEALTHCHARGER) || 
		 (navpoint_type(navpoint) == NAV_F_RECHARGE)      || 
		 (navpoint_type(navpoint) == NAV_F_BUTTON)		 || 
		 (navpoint_type(navpoint) == NAV_F_PLAT)			 || 
		 (navpoint_type(navpoint) == NAV_F_TRAIN)		 || 
		 (navpoint_type(navpoint) == NAV_F_DOOR)             ) 
	{
		const char *name = navpoint_classname(navpoint);
		navpoint->ent = getEntity( name, &navpoint->data.pos );
		if (navpoint->ent==0) { 
			DEBUG_MSG( "initEntityPtr() failed for %i\n", type() );
		} else {
			if (navpoint->ent->v.targetname) {
				// we have a targetname, let's see if there are buttons for it...
				EDICT *pButton = NULL;
				while ((pButton = find_entitybyclassname(pButton, "func_button"))) {
					if (buttontriggers( pButton, navpoint->ent )) {
						navpoint->needstrigger = true;
						break;
					}
				}
			}
			if ((navpoint_type(navpoint) == NAV_F_PLAT)	|| 
				(navpoint_type(navpoint) == NAV_F_TRAIN)	|| 
				(navpoint_type(navpoint) == NAV_F_DOOR)  || 
				(navpoint_type(navpoint) == NAV_F_BUTTON)) {
				int toggle_state = *(int *)((char*)navpoint->ent->pvPrivateData + 128);
				navpoint->normalstate = toggle_state;
			}
		}
	}
	else navpoint->ent = 0;
}

int
navpoint_id(NAVPOINT *navpoint)
{
	return navpoint->data.privateid;
}

int
navpoint_type(NAVPOINT *navpoint)
{
	return navpoint->data.type;
}

void
navpoint_pos(NAVPOINT *navpoint, EDICT *player, Vec3D *pos)
// adjusts positions if playerEnt is on ladder
{
	assert(player != 0);
	vcopy(&navpoint->data.pos, pos);
	if (!is_onladder(player))
		return; 

	pos->z += 20.0f;
}

bool
navpoint_needstriggering(NAVPOINT *navpoint)
{
	return navpoint->needstrigger;
}

bool
navpoint_istriggerfor(NAVPOINT *navpoint, NAVPOINT *wp)
{
	EDICT *targetEnt = navpoint_entity(wp);
	EDICT *triggerEnt = navpoint->ent;

	if (navpoint_type(navpoint) == NAV_S_BUTTON_SHOT) triggerEnt = navpoint_entity(getNavpoint(navpoint->data.special));
	if (!targetEnt || !triggerEnt) return false;
	if (!(targetEnt->v.targetname) || !(triggerEnt->v.target)) return false;
	if (buttontriggers( triggerEnt, targetEnt )) return true;

	return false;
}

bool
navpoint_istriggered(NAVPOINT *navpoint)
{
	#define SF_PLAT_TOGGLE		0x0001
	#define SF_DOOR_START_OPEN	0x0001

	if (!navpoint_needstriggering(navpoint)) return false;
	assert( navpoint->ent != 0 );

	int toggle_state = *(int *)((char*)navpoint->ent->pvPrivateData + 128);

	if (mod_id != DMC_DLL) {
		if (toggle_state == navpoint->normalstate) return false;
		return true;
	}

	// for DMC:
	if (navpoint_type(navpoint)==NAV_F_DOOR) {
		if (navpoint->ent->v.spawnflags & SF_DOOR_START_OPEN) {
			if (toggle_state == STATE_BOTTOM) return false;
			return true;
		} else {
			if (toggle_state == STATE_TOP) return false;
			return true;
		}
	} else {
		if (toggle_state == STATE_TOP) return false;
		return true;
	}
}

bool
navpoint_visible(NAVPOINT *navpoint, EDICT *player)
// may not work for ladders since ent is not initialized
{
	TRACERESULT tr;

	assert( player != 0 );
	Vec3D eyePos;

	eyepos(player, &eyePos);
	trace_line(navpoint_pos(navpoint), &eyePos, false, true, navpoint->ent, &tr);

	if ( tr.fraction != 1.0 ) {
		if ( tr.hit == player) return true;

		if (tr.hit) {	// maybe traceline hit object itself
			const char *hitClass = STRING( tr.hit->v.classname );
			if ( Q_STREQ( hitClass, navpoint_classname(navpoint) ) )
				return true;

			return false;
		}
	}
	return true;
}

bool
navpoint_dooropen(NAVPOINT *navpoint, EDICT *playerEnt)
// call only for NAV_F_DOOR and NAV_F_DOOR_ROTATING
{
	TRACERESULT tr;

	assert( playerEnt != 0 );

	Vec3D passPos;
	vcopy(navpoint_pos(navpoint), &passPos);
	passPos.z = playerEnt->v.origin.z;

	trace_hull( &playerEnt->v.origin, &passPos, false, 3, playerEnt, &tr);	
	if (tr.fraction != 1.0) {
		/*if (tr.hit) {
			if (tr.hit->v.classname)
				DEBUG_MSG( "Door blocked by %s\n", STRING(tr.hit->v.classname) );
			else
				DEBUG_MSG( "Door blocked by unknown class\n" );
		}*/
		return false;
	}
	return true;
}

bool
navpoint_reached(NAVPOINT *navpoint, EDICT *playerEnt)
// maybe better because ignores bot entity?
{
	Vec3D _pos, dir;
	#define SF_BUTTON_TOUCH_ONLY	256	// button only fires as a result of USE key.

	assert( playerEnt != 0 );
	navpoint_pos(navpoint, playerEnt, &_pos);
	vsub(&playerEnt->v.origin, &_pos, &dir);
	if (vlen(&dir) < 55) {
		if (navpoint_type(navpoint)==NAV_F_DOOR || navpoint_type(navpoint)==NAV_F_DOOR_ROTATING) {
			// doors are only reached if they are open
			return navpoint_dooropen(navpoint, playerEnt );
		} else if (navpoint_type(navpoint)==NAV_F_BUTTON) {
			assert( navpoint->ent != 0 );
			if (navpoint->ent->v.spawnflags & SF_BUTTON_TOUCH_ONLY) { // touchable button
				//CBaseToggle *toggleClass = (CBaseToggle*)GET_PRIVATE( ent );
				//if (toggleClass->m_toggle_state == normalstate) return false;	// not pressed
				if (vlen(&playerEnt->v.velocity) > 20) return false;
			}
		} else if (navpoint_type(navpoint)==NAV_S_AIRSTRIKE_BUTTON) {
			if (vlen(&playerEnt->v.velocity) > 20) return false;
		}
		if (navpoint_visible(navpoint, playerEnt )) return true;
	}
	return false;
}

void
navpoint_load(NAVPOINT *navpoint, FILE *fp)
{
	fread( &navpoint->data, sizeof(NAVPOINT_SAVEDATA), 1, fp );
	navpoint->lastvisitor = 0;
	navpoint->lastvisitedat = -100;
	navpoint->nextvisitat = 0;
}

void
navpoint_save(NAVPOINT *navpoint, FILE *fp)
{
	fwrite( &navpoint->data, sizeof(NAVPOINT_SAVEDATA), 1, fp );
}

#if _DEBUG
void
navpoint_print(NAVPOINT *navpoint)
{
	DEBUG_MSG( "%s (ID %i)", navpoint_classname(navpoint), navpoint_id(navpoint) );
}

void
navpoint_printpos(NAVPOINT *navpoint)
{
	DEBUG_MSG( "%s (ID %i) at (%.f, %.f, %.f)", navpoint_classname(navpoint), navpoint_id(navpoint), navpoint_pos(navpoint).x, navpoint_pos(navpoint).y, navpoint_pos(navpoint).z );
}
#endif

void
navpoint_reportvisit(NAVPOINT *navpoint, EDICT *player, float time) 
// reports a visit to the navpoint
{
	if ( (time > (navpoint->lastvisitedat+0.2)) || (time < navpoint->lastvisitedat) ) 
		navpoint->data.visits++;	// don't count visits twice
	navpoint->lastvisitor = player;
	navpoint->lastvisitedat = time;
	switch( mod_id ) {
	case AG_DLL:
	case VALVE_DLL:	// weapons and ammo:
					if (navpoint_type(navpoint) < NAV_F_BUTTON)
						navpoint->nextvisitat = (navpoint->lastvisitedat+20);	
					// items:
					else if ((navpoint_type(navpoint)>=NAV_I_HEALTHKIT) && (navpoint_type(navpoint)<NAV_INFO_TELEPORT_DEST)) 
						navpoint->nextvisitat = (navpoint->lastvisitedat+20);	
					// airstrike cover:
					else if (navpoint_type(navpoint) == NAV_S_AIRSTRIKE_COVER) 
						navpoint->nextvisitat = navpoint->lastvisitedat;
					else navpoint->nextvisitat = navpoint->lastvisitedat+10;		// no respawn
					break;
	case DMC_DLL:	// superhealth:
					if (navpoint_type(navpoint) == NAV_DMCI_HEALTH_LARGE)
						navpoint->nextvisitat = (navpoint->lastvisitedat+100);
					// invisibility, invulnerability, superdamage:
					else if (navpoint_type(navpoint)>=NAV_DMCI_INVISIBILITY && navpoint_type(navpoint)<=NAV_DMCI_SUPERDAMAGE)
						navpoint->nextvisitat = (navpoint->lastvisitedat+60);	
					// armor and health:
					else if ( (navpoint_type(navpoint)>=NAV_DMCI_ARMOR1 && navpoint_type(navpoint)<=NAV_DMCI_ARMOR_INV) ||
							  navpoint_type(navpoint)==NAV_DMCI_HEALTH_SMALL || navpoint_type(navpoint)==NAV_DMCI_HEALTH_NORM )
						navpoint->nextvisitat = (navpoint->lastvisitedat+20);	
					// ammo:
					else if (navpoint_type(navpoint)>=NAV_DMCI_CELLS && navpoint_type(navpoint)<=NAV_DMCI_WEAPON)
						navpoint->nextvisitat = (navpoint->lastvisitedat+30);	
					// default to tour:
					else navpoint->nextvisitat = navpoint->lastvisitedat+10;	// no respawn
					break;
	default:		navpoint->nextvisitat = navpoint->lastvisitedat+10;		// forced to tour...
					break;
	}
}

void
navpoint_donotvisitbefore(NAVPOINT *navpoint, EDICT *player, float time)
// tells that player should not visit this navpoint before time
{
	navpoint->lastvisitor = player;
	navpoint->nextvisitat = time; 
}

float
navpoint_nextvisit(NAVPOINT *navpoint, EDICT *player)
// returns the worldtime after which navpoint can be visited again by player
{
	if (navpoint->lastvisitor!=player)		// only memorize own visits
		return 0;

	return navpoint->nextvisitat;
}

bool
navpoint_offershealth(NAVPOINT *navpoint)
{
	if (navpoint->data.type!=NAV_F_HEALTHCHARGER) return false; 
	if (!navpoint->ent) return false;		// instead of assert because of bug report
	if (navpoint->ent->v.frame==0) return true;
	return false;
}

bool
navpoint_offersarmor(NAVPOINT *navpoint)
{ 
	if (navpoint->data.type!=NAV_F_RECHARGE) return false; 
	if (!navpoint->ent) return false;		// instead of assert because of bug report
	if (navpoint->ent->v.frame==0) return true;
	return false;
}

bool
navpoint_offerscamping(NAVPOINT *navpoint)
{
	return (navpoint->data.type == NAV_S_CAMPING);
}

const char *
navpoint_classname(NAVPOINT *navpoint)
{
	assert(navpoint->data.type >= 0 );
	assert(navpoint->data.type < MAX_NAV_TYPES );
	return navpointClasses[navpoint->data.type];
}

const char *
navpoint_classname(int code)
{
	assert(code >= 0);
	assert(code < MAX_NAV_TYPES);
	return navpointClasses[code];
}

int
navpoint_special(NAVPOINT *navpoint)
{
	return navpoint->data.special;
}

EDICT *
navpoint_entity(NAVPOINT *navpoint)
{
	return navpoint->ent;
}

Vec3D *
navpoint_pos(NAVPOINT *navpoint)
{
	return &navpoint->data.pos;
}
