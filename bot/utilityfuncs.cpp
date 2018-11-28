#include "parabot.h"
#include "pb_global.h"
#include "pb_weapon.h"
#include "bot.h"
#include "utilityfuncs.h"

extern bot_t bots[32];
extern int clientWeapon[32];
extern int mod_id;

// for HolyWars:
bool
is_thesaint(EDICT *player)
{
	char *playerClass = (char*)(player->pvPrivateData);
	assert(playerClass != 0);
	int *groupCode = (int*)(playerClass + 252);

	if ((*groupCode) == 0x00C)
		return true;

	return false;
}

bool
is_heretic(EDICT *player)
{
	char *playerClass = (char*)(player->pvPrivateData);
	assert(playerClass != 0);
	int *groupCode = (int*) (playerClass + 252);

	if ((*groupCode) == 0x003C)
		return true;

	return false;
}

EDICT *
laserdotowner(EDICT *laser)
{
	EDICT *player = 0;
	Vec3D vecSrc, vecAiming, aimingPos;
	TRACERESULT tr;

	for (int i = 1; i <= com.globals->maxclients; i++) {
		player = edictofindex(i);
		if (!is_alive(player)		// skip player if not alive
		    || (player->v.solid == SOLID_NOT))
			continue;	
		int wId = clientWeapon[i - 1];
		if (wId != VALVE_WEAPON_RPG && (mod_id != GEARBOX_DLL && wId != GEARBOX_WEAPON_EAGLE))
			continue;
	
		makevectors(&player->v.v_angle);
		eyepos(player, &vecSrc);
		vcopy(&com.globals->fwd, &vecAiming);
		vma(&vecSrc, 8192.0f, &vecAiming, &aimingPos);
		trace_line(&vecSrc, &aimingPos, false, false, player, &tr);
		vcopy(&tr.endpos, &aimingPos);
		if (vcomp(&laser->v.origin, &aimingPos))
			return player;
	}
	return 0;
}

bool
worldtime_reached(float time)
{
	static float lastTime = 0;

	// take care of level changes:
	if (worldtime() < lastTime) return true;	
	lastTime = worldtime();

	if (worldtime() > time) return true;
	else return false;
}

bool
buttontriggers(EDICT *button, EDICT *target)
{
	const char *targetName = STRING(target->v.targetname);
	const char *buttonTarget = STRING(button->v.target);

	if(Q_STREQ(buttonTarget, targetName))
		return true;

	// multimanager in between?
	EDICT *bTarget = find_entitybytargetname(NULL, buttonTarget);
	if (!bTarget)
		return false;
	if (Q_STREQ(STRING(bTarget->v.classname), "multi_manager")) {
		string *szTargetName = (string *)((char *)bTarget->pvPrivateData + 272);

		// check all multimanager targets:
		for ( ; *szTargetName; szTargetName++ ) {
			if (Q_STREQ(targetName, STRING(*szTargetName)))
				return true;
		}
	}

	return false;
}

char *
memfgets(byte *pMemFile, int fileSize, int *filePos)
{
	static char buffer[512];

	assert(pMemFile != NULL);
	assert(*filePos < fileSize);

	int i = *filePos;
	int last = fileSize;

	// fgets always NULL terminates, so only read buffer size - 1 characters
	if( last - *filePos > ( sizeof( buffer ) - 1 ) )
		last = *filePos + ( sizeof( buffer ) - 1 );

	// Stop at the next newline (inclusive) or end of buffer
	while( i < last )
	{
		if( pMemFile[i] == '\n' )
		{
			i++;
			break;
		}
		i++;
	}

	// If we actually advanced the pointer, copy it over
	if( i != *filePos) {
		// We read in size bytes
		int size = i - *filePos;
		// copy it out
		memcpy( buffer, pMemFile + *filePos, sizeof(byte) * size );

		// null terminate
		buffer[size] = 0;

		// Update file pointer
		*filePos = i;
		return buffer;
	}

	// No data read, bail
	return NULL;
}

int
getnearestplayerindex(Vec3D *pos)
{
	float dist, bestDist = 10000;
	int	  bestPlayer = 0;
	EDICT *player = 0;
	Vec3D dir;

	for (int i = 1; i <= com.globals->maxclients; i++) {
		player = edictofindex(i);
		if (!is_alive(player)		// skip player if not alive
		    || (player->v.solid == SOLID_NOT))
			continue;	

		vsub(&player->v.origin, pos, &dir);
		dist = vlen(&dir);
		if (dist < bestDist) {
			bestDist = dist;
			bestPlayer = i;
		}
	}
	return bestPlayer;
}

float
worldtime()
{
	return com.globals->time;
}

void
boxcenter(EDICT *e, Vec3D *pos)
{
	vadd(&e->v.absmin, &e->v.absmax, pos);
	vscale(pos, 0.5f, pos);
}

void
eyepos(EDICT *e, Vec3D *pos)
{
	vadd(&e->v.origin, &e->v.view_ofs, pos);
}

bool
playerexists(EDICT *player)
{
	if (player 
	    && indexofedict(player)
	    && !player->free
	    && player->v.netname) {
		return true;
	}

	return false;
}

bool
is_alive(EDICT *player)
{
	if (player
	    && (player->v.deadflag == DEAD_NO)
	    && (player->v.health > 0)
	    && (player->v.movetype != MOVETYPE_NOCLIP)) {
		return true;
	}

	return false;
}

bool
is_underwater(EDICT *player)
{
	if (player->v.waterlevel == 3)
		return true;

	return false;
}

bool
needsair(EDICT *player)
{
	if (worldtime() > (player->v.air_finished - 2.0f))
		return true;

	return false;
}

bool
canshootat(EDICT *bot, Vec3D *target)
{
	TRACERESULT tr;

	assert(bot != 0);
	trace_line(target, &bot->v.origin, false, false, bot, &tr);	
	if (tr.fraction == 1.0f)
		return true;
	else
		return false;
}

// for DMC:
bool
has_quaddamage(EDICT *player)
{
	if (player->v.renderfx == RF_GLOW && player->v.rendercolor.z == 255)
		return true;

	return false;
}

bool
is_invisible(EDICT *player)
{
	if (player->v.renderfx == RF_GLOW && player->v.renderamt == 5)
		return true;

	return false;
}

bool
is_invulnerable(EDICT *player)
{
	if (player->v.renderfx == RF_GLOW && player->v.rendercolor.x == 255)
		return true;

	return false;
}

bool
is_onladder(EDICT *player)
{
	return (player->v.movetype == MOVETYPE_FLY);
}

bool
is_hostowner(EDICT *player)
{
	if(player == edictofindex(1))
		return true;

	return false;
}

bool
is_validedict(EDICT *e)
{
	if(e && indexofedict(e)) // index 0 -> worldspawn
		return true;

	return false;
}

float
servermaxspeed()
{
	return maxspeed->value;
}

int
getframerateinterval()
{
	// Jumbot 2.4 has same code
	return (int)(com.globals->frametime * 1000.0f);
}

bool
fileexists(const char *filename)
{
        struct stat checkfile;
        if(0 > stat(filename, &checkfile))
                return false;

        return true;
}
