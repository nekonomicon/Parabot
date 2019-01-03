#pragma once
#ifndef PB_PERCEPTION_H
#define PB_PERCEPTION_H

#include "pb_global.h"
#include <list>

enum { // perception-item classification
	PI_PLAYER = 1,	// unidentified player in team-game
	PI_FRIEND,	// teammate in team-game
	PI_FOE,		// enemy
	PI_HOSTAGE,	// CS
	PI_BOMB,	// CS
	PI_WEAPONBOX,	// weaponbox
	PI_EXPLOSIVE,	// grenade, satchel
	PI_LASERDOT,	// RPG laserdot
	PI_TRIPMINE,	// Beam-Tripmine
	PI_HALO,	// HW
	PI_DAMAGE,	// damage perception
	PI_SNARK,	// snark
	PI_NEWAREA	// a big area that bot hasn't seen before
};

#define MAX_PERCEPTION PI_NEWAREA

enum { // perception-item state
	PI_VISIBLE =	BIT(0),	// is seen
	PI_AUDIBLE =	BIT(1),	// is heard but can't be tracked
	PI_TRACKABLE =	BIT(2),	// is heard and trackable
	PI_TACTILE =	BIT(3),	// is attacking
	PI_PREDICTED =	BIT(4),	// has been seen and is predicted now
	PI_ORIG_KNOWN =	BIT(5),	// for damage: origin/inflictor is known

	PI_UNKNOWN =	BIT(31)	// unknown model index
};

enum { // flags
	PI_BEST_ARMED =		BIT(0),	// best weapon has been armed for that enemy
	PI_ALERT =		BIT(1),	// enemy knows that bot has seen it
	PI_HIGH_PRIORITY =	BIT(2),	// e.g. the Saint in HW
	PI_FOCUS1 =		BIT(3),	// bot is attacking this frame
	PI_FOCUS2 =		BIT(4),	// bot is attacking last frame
	PI_UNREACHABLE =	BIT(5),	// bot can't reach this item by roaming
	PI_DISAPPEARED =	BIT(6),	// item has just become not visible
	PI_PREEMPTIVE =		BIT(7)	// bot uses preemptive fire for that enemy
};

// distances
#define MAX_DIST_VP		3000	// max. visible player detection distance
#define MAX_DIST_VPR	 800	// max. visible player recognition distance
#define MAX_DIST_VI		1200	// max. visible item detection distance

#define UNKNOWN_POS		&zerovector

typedef struct percept {	
	int	 id;
	float	 botsensitivity;	// sensitivity of bot that perceived item
	EDICT	*entity;		// pointer to edict
	short	 pclass;		// classification of item
	short	 state;		// kind of perception
	int	 model;			// model index of item
	float	 update;		// worldtime when goal-queue should be updated
	float	 firstdetection;	// worldtime first detection was registered
	float	 lastdetection;		// worldtime last detection was registered
	float	 lastseentime;		// worldtime last visible contact was registered
	Vec3D	 lastpos;		// last observed position (seen or heard)
	Vec3D	 lastseenpos;		// last seen position
	Vec3D	 lastseenvelocity;	// last seen velocity

	// for enemy:
	float	 rating;		// weapon-rating
	float	 orientation;		// dotproduct if item is facing the bot
	float	 distance;		// distance to bot
	int	 flags;
	Vec3D	 predtarget;		// position the item is supposed to reach
	float	 lastcalctarget;	// last worldtime target was predicted
	Vec3D	 predappearance;	// position where the item is supposed to show up
	float	 lastcalcappearance;	// last worldtime appearance was predicted
} PERCEPT;

void	 percept_construct(PERCEPT *percept, float botSens, EDICT *ent, short state, short realClass, float dist);

float	 percept_getreactiontime(PERCEPT *percept, EDICT *ent, short state, short realClass, float dist);
// returns the time needed to react to the item (time to enter goal-queue)

void	 percept_predictedposition(PERCEPT *percept, const Vec3D *botPos, Vec3D *lastPos);
// returns a predicted position for an enemy that is not perceived anymore

Vec3D	*percept_predictedappearance(PERCEPT *percept, const Vec3D *botPos);
// returns the position where a not visible enemy is most likely to get into line-of-sight

bool	 percept_hasbeenvisible(PERCEPT *percept);
bool	 percept_isvisible(PERCEPT *percept);
bool	 percept_istrackable(PERCEPT *percept);
bool	 percept_inflictorknown(PERCEPT *percept);
bool	 percept_hasbeenspotted(PERCEPT *percept);
bool	 percept_ismoving(PERCEPT *percept);
bool	 percept_isfacingbot(PERCEPT *percept);
bool	 percept_isaimingatbot(PERCEPT *percept);
bool	 percept_ishurtingbot(PERCEPT *percept);
bool	 percept_isalert(PERCEPT *percept);
bool	 percept_canbeattackedbest(PERCEPT *percept);
bool	 percept_hashighpriority(PERCEPT *percept);
bool	 percept_hasfocus(PERCEPT *percept);
bool	 percept_isreachable(PERCEPT *percept);
bool	 percept_hasjustdisappeared(PERCEPT *percept);
bool	 percept_isunderpreemptivefire(PERCEPT *percept);
float	 percept_targetaccuracy(PERCEPT *percept);

typedef std::list<PERCEPT> tPerceptionList;

typedef struct perception {
	EDICT		*botent;
	int		 numenemies, numunidentified;	// just for fast queries
	float		 sensitivity;
	int		 cdet, odet;
	tPerceptionList	 detections[2];	// two lists to be able to compare old and actual
	tPerceptionList	 tactiledetections;
	float		 maxspeed;
} PERCEPTION;

void		 perception_init(PERCEPTION *perception, EDICT *ent);
// initializes all necessary variables

void		 perception_setsensitivity(PERCEPTION *perception, int skill);

void		 perception_addattack(PERCEPTION *perception, EDICT *att, int dmg);
// adds a damage perception from entity att (maybe 0) with amount dmg

void		 perception_addnewarea(PERCEPTION *perception, Vec3D *viewDir);
// adds a new area perception in the direction viewDir

bool		 perception_isnewperception(PERCEPTION *perception, tPerceptionList &pList, PERCEPT *perc);

bool		 perception_isnewtactileperception(PERCEPTION *perception, tPerceptionList &pList, PERCEPT *perc);

void		 perception_collectdata(PERCEPTION *perception);

void		 perception_resetplayerclassifications(PERCEPTION *perception);
// enforces new classifications for all players

EDICT		*perception_getnearesttripmine(PERCEPTION *perception);
// returns nearest tripmine or 0 if no tripmine seen

bool		 perception_underfire(PERCEPTION *perception);
// returns true is bot is currently being attacked

tPerceptionList	*perception_list(PERCEPTION *perception);

#endif
