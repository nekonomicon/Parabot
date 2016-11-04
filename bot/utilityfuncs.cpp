#include "pb_global.h"
#include "pb_weapon.h"


extern int clientWeapon[32];
extern int mod_id;



bool playerExists( edict_t *pEdict )
{
	if (!pEdict) return false;
	if (ENTINDEX( pEdict )) {
		if (!pEdict->free && pEdict->v.netname != 0) return true;
	}
	return false;
}


bool isAlive( edict_t *pEdict )
{
	if (!pEdict) return false;
	return ((pEdict->v.deadflag == DEAD_NO) &&
            (pEdict->v.health > 0) && (pEdict->v.movetype != MOVETYPE_NOCLIP));
}


bool isTheSaint( edict_t *ent )
{
	char *playerClass = (char*)GET_PRIVATE( ent );
	assert( playerClass != 0);
	int *groupCode = (int*) (playerClass+252);
	if ( (*groupCode) == 0x00C3 ) return true;
	else return false;
}


bool isHeretic( edict_t *ent )
{
	char *playerClass = (char*)GET_PRIVATE( ent );
	assert( playerClass != 0);
	int *groupCode = (int*) (playerClass+252);
	if ( (*groupCode) == 0x003C ) return true;
	else return false;
}


bool hasQuadDamage( edict_t *ent )
{
	if (ent->v.renderfx==19 && ent->v.rendercolor.z==255) return true;
	return false;
}


bool isInvisible( edict_t *ent )
{
	if (ent->v.renderfx==19 && ent->v.renderamt==5) return true;
	return false;
}


bool isInvulnerable( edict_t *ent )
{
	if (ent->v.renderfx==19 && ent->v.rendercolor.x==255) return true;
	return false;
}


bool isUnderwater( edict_t *ent )
{
	if (ent->v.waterlevel == 3) return true;
	return false;
}


bool needsAir( edict_t *ent )
{
	if (worldTime() > (ent->v.air_finished-2.0)) return true;
	return false;
}


bool canShootAt( edict_t *bot, Vector target )
{
	TraceResult tr;

	assert( bot != 0 );
	UTIL_TraceLine( target, bot->v.origin, dont_ignore_monsters, bot, &tr);	
	if ( tr.flFraction == 1.0 ) return true;
	else return false;
}


edict_t* tripmineOwner( CBaseEntity *tripmine )
// TODO: check if working!
// returns the edict of the player that laid the tripmine
{
	char **ownerEdict = (char**)( ((char*)tripmine) + 680 );
	return (edict_t*)(*ownerEdict);
}


edict_t* laserdotOwner( edict_t *laser )
{
	CBaseEntity *pPlayer = 0;
	for (int i=1; i<=gpGlobals->maxClients; i++) {
		pPlayer = UTIL_PlayerByIndex( i );
		if (!pPlayer) continue;							// skip invalid players
		if (!isAlive( ENT(pPlayer->pev) )) continue;	// skip player if not alive
		if (pPlayer->pev->solid == SOLID_NOT) continue;	
		int wId = clientWeapon[i-1];
		if (wId != VALVE_WEAPON_RPG && wId != GEARBOX_WEAPON_EAGLE) continue;
	
		edict_t *ent = pPlayer->edict();
		UTIL_MakeVectors( ent->v.v_angle );
		Vector vecSrc = ent->v.origin + ent->v.view_ofs;
		Vector vecAiming = gpGlobals->v_forward;
		TraceResult tr;
		UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ent, &tr );
		Vector aimingPos = tr.vecEndPos;
		if (laser->v.origin == aimingPos) return ent;
	}
	return 0;
}


CBaseEntity* getActiveItem( edict_t *player )
{
	if (!player) return 0;
	char *playerClass = (char*) GET_PRIVATE( player );
	if (!playerClass) return 0;

	CBaseEntity **weaponClass;
	switch( mod_id ) {
	case VALVE_DLL:		weaponClass = (CBaseEntity**)(playerClass+1204);
						break;
	case HOLYWARS_DLL:	weaponClass = (CBaseEntity**)(playerClass+832);
						break;
	case DMC_DLL:		weaponClass = (CBaseEntity**)(playerClass+1172);
						break;	
	case GEARBOX_DLL:	weaponClass = (CBaseEntity**)(playerClass+1404);//1556);
						break;
	}
/*	if (*weaponClass) {
		debugMsg( "." );
	}*/
	return (*weaponClass);
}