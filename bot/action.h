#pragma once
#if !defined(PB_ACTION_H)
#define PB_ACTION_H

// movement constants:
#define MAX_YAW		20
#define MAX_VDELAY	11

typedef struct action {
	EDICT		*e;                   // bot entity
	Vec3D		 moveangle, viewangle;		// actual move angle and view(shoot!)angle
	float		 speed;						// speed to move at
	float		 strafe;					// strafe speed (>0 = right, <0 = left )
	int		 action;					// instant action e.g. fire, jump, duck

	float		 jumppos;		// zpos where jump started
	Vec3D		 finejumppos;
	int		 longjumpstate;
	float		 nextjumptime;	// for delayed jump
	int		 viewprior, moveprior;		// priorities
	float		 maxspeed;

	int		 aimskill;
	
	Vec3D		 currentview;	// delayed view
	Vec3D		 deltaview;
	float		 targetdiff[MAX_VDELAY];		// storing the difference to target for the last frames
	float		 hitprob;
	float		 weaponcone;
	Vec3D		 targetpos, targetvel;
	float		 targetdist;

	int		 usecounter;
	float		 nextusetime;
	Vec3D		 nextusepos;

	float		 duckendtime;
	float		 stopendtime;

	float		 lastmove, lastmovecheck, nextviewupdate, vupdtime, aimaccuracy;
	float		 maxturn;
	int		 turncount;
	bool		 injump;		// true while ducked in jump
	bool		 finejump;
	bool		 notstucking;

	// returns an adequate msec-value to pass to the engine
} ACTION;

void	action_init(ACTION *action, EDICT *botEnt);
void	action_reset(ACTION *action);
void	action_setaimskill(ACTION *action, int skill);
void	action_setmoveangle(ACTION *action, Vec3D *angle);
void	action_setmoveangleyaw(ACTION *action,float angle);
void	action_setmovedir(ACTION *action, Vec3D *vec, int prior);
void	action_setviewangle(ACTION *action, Vec3D *angle, int prior);
void	action_setviewdir(ACTION *action, Vec3D *vec, int prior);
void	action_setviewlikemove(ACTION *action);
void	action_setaimdir(ACTION *action, Vec3D *currentPos, Vec3D *relVelocity);
void	action_setspeed(ACTION *action, float value);
void	action_setmaxspeed(ACTION *action);
float	action_getmaxspeed(ACTION *action);
void	action_add(ACTION *action, int code, Vec3D *exactPos);
bool	action_gotstuck(ACTION *action);
void	action_resetstuck(ACTION *action);
void	action_dontgetstuck(ACTION *action);
void	action_perform(ACTION *action);

float	action_moveangleyaw(ACTION *action);
bool	action_jumping(ACTION *action);
bool	action_pausing(ACTION *action);
float	action_targetaccuracy(ACTION *action);
// returns the accurcy for beforehand passed target
void	action_getmovedir(ACTION *action, Vec3D *movedir);
void	action_getviewangle(ACTION *action, Vec3D *viewangle);
int	action_getaimskill(ACTION *action);
float	action_estimatehitprob(ACTION *action);
void	action_setweaponcone(ACTION *action, float cone);
void	action_calcviewangle(ACTION *action, Vec3D *viewangle);
#endif
