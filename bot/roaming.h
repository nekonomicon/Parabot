#pragma once
#ifndef PB_ROAMING_H
#define PB_ROAMING_H

typedef struct {
	int	shouldjump;  // 0=no, 1=slow, 2=fast
	Vec3D	wallangle;
	float   walldistance;
	Vec3D	jumppos, landpos;	// for transmitting possible jumps
	bool	shouldduck;
	bool	blocked;
	bool	gap;
	bool	ontouch;
	bool	tooclose, toofar;
} checkWayRes;

typedef struct roaming {
	EDICT	*pev;
	ACTION	*action;
	int	 straferightcount, strafeleftcount;
	float	 passededge;
	Vec3D	 passageorigin;
	float	 passagedistance;
	int	 passagetries;
	Vec3D	 target, jumptarget;
	float	 lastxydist;	// last XY-Dist to target
#if _DEBUG
	int	 markerid;
#endif //_DEBUG
	bool	 followleft, followright, checkifpassage;
	bool	 debugway, debugtrace;
} ROAMING;

void	roaming_init(ROAMING *pathfinder, EDICT *botEnt, ACTION *act);
void	roaming_reset(ROAMING *pathfinder, const Vec3D *newTarget);
void	roaming_checkway(ROAMING *pathfinder, const Vec3D *targetPos);
bool	roaming_targetnotreachable(ROAMING *pathfinder);

#endif
