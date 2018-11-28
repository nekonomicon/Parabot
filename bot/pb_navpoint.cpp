#include "parabot.h"
#include "sectors.h"
#include "pb_navpoint.h"
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



void PB_Navpoint::init(Vec3D *pos, int type, int special )
{
	vcopy(pos, &data.pos);
	data.type = type;
	data.visits = 0;
	data.special = special;
	data.damageComp = 0;
	ent = 0;
	lastVisitor = 0;
	lastVisitedAt = -100;
	nextVisitAt = 0;
}

void PB_Navpoint::initEntityPtr()
// gets a pointer to the corresponding t_edict if possible and stores it
{
	needsTrigger = false;
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
	if ( (type() == NAV_F_BREAKABLE)	 ||
	     (type() == NAV_F_HEALTHCHARGER) || 
		 (type() == NAV_F_RECHARGE)      || 
		 (type() == NAV_F_BUTTON)		 || 
		 (type() == NAV_F_PLAT)			 || 
		 (type() == NAV_F_TRAIN)		 || 
		 (type() == NAV_F_DOOR)             ) 
	{
		const char *name = classname();
		ent = getEntity( name, &data.pos );
		if (ent==0) { 
			DEBUG_MSG( "initEntityPtr() failed for %i\n", type() );
		} else {
			if (ent->v.targetname) {
				// we have a targetname, let's see if there are buttons for it...
				EDICT *pButton = NULL;
				while ((pButton = find_entitybyclassname(pButton, "func_button"))) {
					if (buttontriggers( pButton, ent )) {
						needsTrigger = true;
						break;
					}
				}
			}
			if ((type() == NAV_F_PLAT)	|| 
				(type() == NAV_F_TRAIN)	|| 
				(type() == NAV_F_DOOR)  || 
				(type() == NAV_F_BUTTON)) {
				int toggle_state = *(int *)((char*)ent->pvPrivateData + 128);
				normalState = toggle_state;
			}
		}
	}
	else ent = 0;
}


void PB_Navpoint::pos( EDICT *player, Vec3D *pos)
// adjusts positions if playerEnt is on ladder
{
	assert(player != 0);
	vcopy(&data.pos, pos);
	if (!is_onladder(player))
		return; 

	pos->z += 20.0f;
}


bool PB_Navpoint::isTriggerFor( PB_Navpoint &wp )
{
	EDICT *targetEnt = wp.entity();
	EDICT *triggerEnt = ent;

	if (type() == NAV_S_BUTTON_SHOT) triggerEnt = getNavpoint( data.special ).entity();
	if (!targetEnt || !triggerEnt) return false;
	if (!(targetEnt->v.targetname) || !(triggerEnt->v.target)) return false;
	if (buttontriggers( triggerEnt, targetEnt )) return true;

	return false;
}


bool PB_Navpoint::isTriggered()
{
	#define SF_PLAT_TOGGLE		0x0001
	#define SF_DOOR_START_OPEN	0x0001

	if (!needsTriggering()) return false;
	assert( ent != 0 );

	int toggle_state = *(int *)((char*)ent->pvPrivateData + 128);

	if (mod_id != DMC_DLL) {
		if (toggle_state == normalState) return false;
		else return true;
	}

	// for DMC:
	if (type()==NAV_F_DOOR) {
		if (ent->v.spawnflags & SF_DOOR_START_OPEN) {
			if (toggle_state == STATE_BOTTOM) return false;
			else return true;
		} else {
			if (toggle_state == STATE_TOP) return false;
			else return true;
		}
	} else {
		if (toggle_state == STATE_TOP) return false;
		else return true;
	}
}


bool PB_Navpoint::visible( EDICT *player )
// may not work for ladders since ent is not initialized
{
	TRACERESULT tr;

	assert( player != 0 );
	Vec3D eyePos;

	eyepos(player, &eyePos);
	trace_line( pos(), &eyePos, false, true, ent, &tr);	
	
	if ( tr.fraction != 1.0 ) {
		if ( tr.hit == player) return true;

		if (tr.hit) {	// maybe traceline hit object itself
			const char *hitClass = STRING( tr.hit->v.classname );
			if ( Q_STREQ( hitClass, classname() ) )
				return true;

			return false;
		}
	}
	return true;
}


bool PB_Navpoint::doorOpen( EDICT *playerEnt )
// call only for NAV_F_DOOR and NAV_F_DOOR_ROTATING
{
	TRACERESULT tr;

	assert( playerEnt != 0 );

	Vec3D passPos;
	vcopy(pos(), &passPos);
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
	else return true;
}


bool PB_Navpoint::reached( EDICT *playerEnt )
// maybe better because ignores bot entity?
{
	Vec3D _pos, dir;
	#define SF_BUTTON_TOUCH_ONLY	256	// button only fires as a result of USE key.

	assert( playerEnt != 0 );
	pos(playerEnt, &_pos);
	vsub(&playerEnt->v.origin, &_pos, &dir);
	if (vlen(&dir) < 55) {
		if (type()==NAV_F_DOOR || type()==NAV_F_DOOR_ROTATING) {
			// doors are only reached if they are open
			return doorOpen( playerEnt );
		} else if (type()==NAV_F_BUTTON) {
			assert( ent != 0 );
			if (ent->v.spawnflags & SF_BUTTON_TOUCH_ONLY) { // touchable button
				//CBaseToggle *toggleClass = (CBaseToggle*)GET_PRIVATE( ent );
				//if (toggleClass->m_toggle_state == normalState) return false;	// not pressed
				if (vlen(&playerEnt->v.velocity) > 20) return false;
			}
		} else if (type()==NAV_S_AIRSTRIKE_BUTTON) {
			if (vlen(&playerEnt->v.velocity) > 20) return false;
		}
		if (visible( playerEnt )) return true;
	}
	return false;
}


void PB_Navpoint::load( FILE *fp )
{
	fread( &data, sizeof(TSaveData), 1, fp );
	lastVisitor = 0;
	lastVisitedAt = -100;
	nextVisitAt = 0;
}


void PB_Navpoint::save( FILE *fp )
{
	fwrite( &data, sizeof(TSaveData), 1, fp );
}

#if _DEBUG
void PB_Navpoint::print()
{
	DEBUG_MSG( "%s (ID %i)", classname(), id() );
}


void PB_Navpoint::printPos()
{
	DEBUG_MSG( "%s (ID %i) at (%.f, %.f, %.f)", classname(), id(), pos().x, pos().y, pos().z );
}
#endif

void PB_Navpoint::reportVisit( EDICT *player, float time ) 
// reports a visit to the navpoint
{
	if ( (time > (lastVisitedAt+0.2)) || (time < lastVisitedAt) ) 
		data.visits++;	// don't count visits twice
	lastVisitor = player;
	lastVisitedAt = time;
	switch( mod_id ) {
	case AG_DLL:
	case VALVE_DLL:	// weapons and ammo:
					if (type() < NAV_F_BUTTON)	
						nextVisitAt = (lastVisitedAt+20);	
					// items:
					else if ((type()>=NAV_I_HEALTHKIT) && (type()<NAV_INFO_TELEPORT_DEST)) 
						nextVisitAt = (lastVisitedAt+20);	
					// airstrike cover:
					else if (type() == NAV_S_AIRSTRIKE_COVER) 
						nextVisitAt = lastVisitedAt;
					else nextVisitAt = lastVisitedAt+10;		// no respawn
					break;
	case DMC_DLL:	// superhealth:
					if (type() == NAV_DMCI_HEALTH_LARGE)
						nextVisitAt = (lastVisitedAt+100);		
					// invisibility, invulnerability, superdamage:
					else if (type()>=NAV_DMCI_INVISIBILITY && type()<=NAV_DMCI_SUPERDAMAGE)
						nextVisitAt = (lastVisitedAt+60);	
					// armor and health:
					else if ( (type()>=NAV_DMCI_ARMOR1 && type()<=NAV_DMCI_ARMOR_INV) ||
							  type()==NAV_DMCI_HEALTH_SMALL || type()==NAV_DMCI_HEALTH_NORM )
						nextVisitAt = (lastVisitedAt+20);	
					// ammo:
					else if (type()>=NAV_DMCI_CELLS && type()<=NAV_DMCI_WEAPON)
						nextVisitAt = (lastVisitedAt+30);	
					// default to tour:
					else nextVisitAt = lastVisitedAt+10;	// no respawn
					break;
	default:		nextVisitAt = lastVisitedAt+10;		// forced to tour...
					break;
	}
}


void PB_Navpoint::doNotVisitBefore( EDICT *player, float time )
// tells that player should not visit this navpoint before time
{
	lastVisitor = player;
	nextVisitAt	= time; 
}


float PB_Navpoint::nextVisit( EDICT *player )
// returns the worldtime after which navpoint can be visited again by player
{
	if (lastVisitor!=player)		// only memorize own visits
		return 0;

	return nextVisitAt;
}


bool PB_Navpoint::offersHealth() 
{ 
	if (data.type!=NAV_F_HEALTHCHARGER) return false; 
	if (!ent) return false;		// instead of assert because of bug report
	if (ent->v.frame==0) return true;
	else return false;
}


bool PB_Navpoint::offersArmor() 
{ 
	if (data.type!=NAV_F_RECHARGE) return false; 
	if (!ent) return false;		// instead of assert because of bug report
	if (ent->v.frame==0) return true;
	else return false;
}


const char* PB_Navpoint::classname()
{
	assert( data.type >= 0 );
	assert( data.type < MAX_NAV_TYPES );
	return navpointClasses[data.type];
}


const char* PB_Navpoint::classname( int code )
{
	assert( code >= 0 );
	assert( code < MAX_NAV_TYPES );
	return navpointClasses[code];
}
