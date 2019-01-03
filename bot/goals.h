#pragma once
#if !defined(PB_GOALS_H)
#define PB_GOALS_H
class CParabot;
typedef struct percept PERCEPT;

void	goal_donothing(CParabot *pb, PERCEPT *item);

//***************************************************************************
//							COMBAT GOALS
//***************************************************************************

// hunt enemy that is trackable but currently not seen
void	goal_huntenemy(CParabot *pb, PERCEPT *item);
float	weight_huntenemy(CParabot *pb, PERCEPT *item);

// flee taking cover
void	goal_fleeenemy(CParabot *pb, PERCEPT *item);
float	weight_fleeenemy(CParabot *pb, PERCEPT *item);

// move to next cover from enemy position
void	goal_takecover(CParabot *pb, PERCEPT *item);
float	weight_takecover(CParabot *pb, PERCEPT *item);

// combat movements for short range
void	goal_closecombat(CParabot *pb, PERCEPT *item);
float	weight_closecombat(CParabot *pb, PERCEPT *item);

// combat movements for medium to long range
void	goal_rangeattack(CParabot *pb, PERCEPT *item);
float	weight_rangeattack(CParabot *pb, PERCEPT *item);

// prepared fatal attack
void	goal_silentattack(CParabot *pb, PERCEPT *item);
float	weight_silentattack(CParabot *pb, PERCEPT *item);

// move to ambush position and wait
void	goal_prepareambush(CParabot *pb, PERCEPT *item);
float	weight_prepareambush(CParabot *pb, PERCEPT *item);

// bunny hopping
void	goal_bunnyhop(CParabot *pb, PERCEPT *item);
float	weight_bunnyhop(CParabot *pb, PERCEPT *item);

// shooting
void	goal_shootatenemy(CParabot *pb, PERCEPT *item);
float	weight_shootatenemy(CParabot *pb, PERCEPT *item);
float	weight_shootatsnark(CParabot *pb, PERCEPT *item);

// chose best weapon
void	goal_armbestweapon(CParabot *pb, PERCEPT *item);
float	weight_armbestweapon(CParabot *pb, PERCEPT *item);

//***************************************************************************
//							COLLECT GOALS
//***************************************************************************

// chose route to collect static items
void	goal_collectitems(CParabot *pb, PERCEPT *item);
float	weight_collectitems(CParabot *pb, PERCEPT *item);

// get dynamic items
void	goal_getitem(CParabot *pb, PERCEPT *item);
float	weight_getweaponbox(CParabot *pb, PERCEPT *item);
float	weight_gethalo(CParabot *pb, PERCEPT *item);

//***************************************************************************
//							MISC MOVE GOALS
//***************************************************************************

// evade an item
void	goal_getaway(CParabot *pb, PERCEPT *item);
float	weight_getawayenemy(CParabot *pb, PERCEPT *item);
float	weight_getawaylaserdot(CParabot *pb, PERCEPT *item);
float	weight_getawayexplosive(CParabot *pb, PERCEPT *item);

// wait for an item
void	goal_waitatnavpoint(CParabot *pb, PERCEPT *item);
float	weight_waitforhalo(CParabot *pb, PERCEPT *item);

// pause
void	goal_pause(CParabot *pb, PERCEPT *item);
float	weight_pause(CParabot *pb, PERCEPT *item);

//***************************************************************************
//							LOOK GOALS
//***************************************************************************

// look around to discover enemy
void	goal_lookaround(CParabot *pb, PERCEPT *item);
float	weight_lookaroundlaserdot(CParabot *pb, PERCEPT *item);
float	weight_lookarounddamage(CParabot *pb, PERCEPT *item);
float	weight_lookarounddangeroussound(CParabot *pb, PERCEPT *item);
float	weight_lookaroundplayersound(CParabot *pb, PERCEPT *item);

// look around to check new area
void	goal_lookatnewarea(CParabot *pb, PERCEPT *item);
float	weight_lookatnewarea(CParabot *pb, PERCEPT *item);

// look closer at unidentified player
void	goal_reacttounidentified(CParabot *pb, PERCEPT *item);
float	weight_reacttounidentified(CParabot *pb, PERCEPT *item);

//***************************************************************************
//							TACTICAL GOALS
//***************************************************************************

// go sniping
void	goal_camp(CParabot *pb, PERCEPT *item);
float	weight_camp(CParabot *pb, PERCEPT *item);

// wait at a tank
void	goal_usetank(CParabot *pb, PERCEPT *item);
float	weight_usetank(CParabot *pb, PERCEPT *item);

// lay tripmines
void	goal_laytripmine(CParabot *pb, PERCEPT *item);
float	weight_laytripmine(CParabot *pb, PERCEPT *item);

// load health or armor
void	goal_loadhealthorarmor(CParabot *pb, PERCEPT *item);
float	weight_loadhealthorarmor(CParabot *pb, PERCEPT *item);

//***************************************************************************
//							TEAM GOALS
//***************************************************************************

// make room for a team-mamber to pass
void	goal_makeroom(CParabot *pb, PERCEPT *item);
float	weight_makeroom(CParabot *pb, PERCEPT *item);

// follow team-member
void	goal_follow(CParabot *pb, PERCEPT *item);
float	weight_followleader(CParabot *pb, PERCEPT *item);
float	weight_followenemy(CParabot *pb, PERCEPT *item);

// assist team-member in combat
void	goal_assistfire(CParabot *pb, PERCEPT *item);
float	weight_assistfire(CParabot *pb, PERCEPT *item);
#endif // PB_GOALS_H
