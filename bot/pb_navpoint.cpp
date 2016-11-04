#include "pb_navpoint.h"
#include "pb_global.h"


extern int mod_id;




static char navpointClasses[MAX_NAV_TYPES][32] = { "unknown",
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
"ammo_spore"
};



void PB_Navpoint::init( Vector &pos, int type, int special )
{
	data.pos = pos;
	data.type = type;
	data.visits = 0;
	data.special = special;
	data.damageComp = 0;
	ent = 0;
	lastVisitor = 0;
	lastVisitedAt = -100;
	nextVisitAt = 0;
}


bool UTIL_ButtonTriggers( edict_t *button, edict_t *target );


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
		 debugMsg( "initEntityPtr() failed for %i\n", type() );
	}*/
	if ( (type() == NAV_F_BREAKABLE)	 ||
	     (type() == NAV_F_HEALTHCHARGER) || 
		 (type() == NAV_F_RECHARGE)      || 
		 (type() == NAV_F_BUTTON)		 || 
		 (type() == NAV_F_PLAT)			 || 
		 (type() == NAV_F_TRAIN)		 || 
		 (type() == NAV_F_DOOR)             ) 
	{
		char *name = classname();
		ent = getEntity( name, data.pos );
		if (ent==0) { 
			debugMsg( "initEntityPtr() failed for %i\n", type() );
		}
		else {
			if (!FStringNull( ent->v.targetname )) {
				// we have a targetname, let's see if there are buttons for it...
				CBaseEntity *pButton = NULL;
				while ((pButton = UTIL_FindEntityByClassname( pButton, "func_button" )) != NULL) {
					if (UTIL_ButtonTriggers( ENT(pButton->pev), ent )) {
						needsTrigger = true;
						break;
					}
				}
			}
			if ((type() == NAV_F_PLAT)	|| 
				(type() == NAV_F_TRAIN)	|| 
				(type() == NAV_F_DOOR)  || 
				(type() == NAV_F_BUTTON)   ) 
			{
				CBaseToggle *toggleClass = (CBaseToggle*)GET_PRIVATE( ent );
				normalState = toggleClass->m_toggle_state;
			}
		}
	}
	else ent = 0;
}


Vector PB_Navpoint::pos( edict_t *playerEnt ) 
// adjusts positions if playerEnt is on ladder
{ 
	assert( playerEnt!=0 );
	if ( playerEnt->v.movetype != MOVETYPE_FLY ) return data.pos; 
	else return (data.pos + Vector( 0,0,20 ));
}


bool PB_Navpoint::isTriggerFor( PB_Navpoint &wp )
{
	edict_t *targetEnt = wp.entity();
	edict_t *triggerEnt = ent;
	if (type()==NAV_S_BUTTON_SHOT) triggerEnt = getNavpoint( data.special ).entity();
	if (!targetEnt || !triggerEnt) return false;
	if (FStringNull( targetEnt->v.targetname ) || FStringNull( triggerEnt->v.target )) return false;
	if (UTIL_ButtonTriggers( triggerEnt, targetEnt )) return true;
	return false;
}


bool PB_Navpoint::isTriggered()
{
	#define SF_PLAT_TOGGLE		0x0001
	#define SF_DOOR_START_OPEN	0x0001

	if (!needsTriggering()) return false;
	assert( ent != 0 );

	CBaseToggle *toggleClass = (CBaseToggle*)GET_PRIVATE( ent );

	if (mod_id != DMC_DLL) {
		if (toggleClass->m_toggle_state == normalState) return false;
		else return true;
	}

	// for DMC:
	if (type()==NAV_F_DOOR) {
		if (ent->v.spawnflags & SF_DOOR_START_OPEN) {
			if (toggleClass->m_toggle_state == TS_AT_BOTTOM) return false;
			else return true;
		}
		else {
			if (toggleClass->m_toggle_state == TS_AT_TOP) return false;
			else return true;
		}
	}
	else {
		if (toggleClass->m_toggle_state == TS_AT_TOP) return false;
		else return true;
	}
}


bool PB_Navpoint::visible( edict_t *playerEnt )
// may not work for ladders since ent is not initialized
{
	TraceResult tr;

	assert( playerEnt != 0 );
	Vector eyePos = playerEnt->v.origin + playerEnt->v.view_ofs;

	UTIL_TraceLine( pos(), eyePos, dont_ignore_monsters, ignore_glass, ent, &tr);	
	
	if ( tr.flFraction != 1.0 ) {
		if ( tr.pHit == playerEnt ) return true;

		if (tr.pHit) {	// maybe traceline hit object itself
			const char *hitClass = STRING( tr.pHit->v.classname );
			if ( strcmp(hitClass, classname()) == 0 ) return true;
			return false;
		}
	}
	return true;
}


bool PB_Navpoint::doorOpen( edict_t *playerEnt )
// call only for NAV_F_DOOR and NAV_F_DOOR_ROTATING
{
	TraceResult tr;

	assert( playerEnt != 0 );

	Vector passPos = pos();
	passPos.z = playerEnt->v.origin.z;

	UTIL_TraceHull( playerEnt->v.origin, passPos, dont_ignore_monsters, head_hull, playerEnt, &tr);	
	if (tr.flFraction != 1.0) {
		/*if (tr.pHit) {
			if (!FStringNull(tr.pHit->v.classname))
				debugMsg( "Door blocked by ", STRING(tr.pHit->v.classname), "\n" );
			else
				debugMsg( "Door blocked by unknown class\n" );
		}*/
		return false;
	}
	else return true;
}


bool PB_Navpoint::reached( edict_t *playerEnt )
// maybe better because ignores bot entity?
{
	#define SF_BUTTON_TOUCH_ONLY	256	// button only fires as a result of USE key.

	assert( playerEnt != 0 );
	if ((playerEnt->v.origin - pos(playerEnt)).Length()<55) {
		if (type()==NAV_F_DOOR || type()==NAV_F_DOOR_ROTATING) {
			// doors are only reached if they are open
			return doorOpen( playerEnt );
		}
		else if (type()==NAV_F_BUTTON) {
			assert( ent != 0 );
			if ( FBitSet ( ent->v.spawnflags, SF_BUTTON_TOUCH_ONLY ) ) { // touchable button
				//CBaseToggle *toggleClass = (CBaseToggle*)GET_PRIVATE( ent );
				//if (toggleClass->m_toggle_state == normalState) return false;	// not pressed
				if (((Vector)playerEnt->v.velocity).Length() > 20) return false;
			}
		}
		else if (type()==NAV_S_AIRSTRIKE_BUTTON) {
			if (((Vector)playerEnt->v.velocity).Length() > 20) return false;
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


void PB_Navpoint::print()
{
	debugMsg( classname() );
	debugMsg( " (ID %i)", id() );
}


void PB_Navpoint::printPos()
{
	debugMsg( classname() );
	debugMsg( " (ID %i) at (", id() );
	debugMsg( "%.f, ", pos().x ); 
	debugMsg( "%.f, ", pos().y ); 
	debugMsg( "%.f)", pos().z );
}


void PB_Navpoint::reportVisit( edict_t *player, float time ) 
// reports a visit to the navpoint
{
	if ( (time > (lastVisitedAt+0.2)) || (time < lastVisitedAt) ) 
		data.visits++;	// don't count visits twice
	lastVisitor = player;
	lastVisitedAt = time;
	switch( mod_id ) {
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


void PB_Navpoint::doNotVisitBefore( edict_t *player, float time )
// tells that player should not visit this navpoint before time
{
	lastVisitor = player;
	nextVisitAt	= time; 
}


float PB_Navpoint::nextVisit( edict_t *player )
// returns the worldTime after which navpoint can be visited again by player
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


char* PB_Navpoint::classname()
{
	assert( data.type >= 0 );
	assert( data.type < MAX_NAV_TYPES );
	return navpointClasses[data.type];
}


char* PB_Navpoint::classname( int code )
{
	assert( code >= 0 );
	assert( code < MAX_NAV_TYPES );
	return navpointClasses[code];
}