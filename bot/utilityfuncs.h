#pragma once
#if !defined(UTILITY_FUNCS_H)
#define UTILITY_FUNCS_H

// for HolyWars:
bool	 is_thesaint(EDICT *player);
bool	 is_heretic(EDICT *player);

// common funcs
EDICT	*laserdotowner(EDICT *laser);
bool	 worldtime_reached(float time);
bool	 buttontriggers(EDICT *button, EDICT *target);
char	*memfgets(byte *pMemFile, int fileSize, int *filePos);
int	 getnearestplayerindex(Vec3D *pos);

float	 worldtime();
void	 boxcenter(EDICT *e, Vec3D *pos);
void	 eyepos(EDICT *e, Vec3D *pos);
bool	 playerexists(EDICT *player);
bool	 is_alive(EDICT *player);
bool	 is_underwater(EDICT *player);
bool	 needsair(EDICT *player);
bool	 canshootat(EDICT *bot, Vec3D *target);

// for DMC:
bool	 has_quaddamage(EDICT *player);
bool	 is_invisible(EDICT *player);
bool	 is_invulnerable(EDICT *player);
bool	 is_onladder(EDICT *player);
bool	 is_hostowner(EDICT *player);
bool	 is_validedict(EDICT *e);
float	 servermaxspeed();
float	 getframerate();
byte	 getframerateinterval();
bool	 fileexists(const char *filename);
#endif
