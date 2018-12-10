#pragma once
#if !defined(PB_GOALFINDER_H)
#define PB_GOALFINDER_H

#include "pb_global.h"
#include "pb_perception.h"
#include <list>
#include <map>

#define GOAL_UNCONDITIONAL	0

// goal classes (flags!):
#define G_ACTION 0	// only impact on action
#define G_MOVE	 1	// impact on movement 
#define G_VIEW	 2	// impact on body-/viewangle

#define MAX_GOALS 4	// combinations!

class CParabot;
typedef void (*tGoalFunc)( CParabot*, PB_Percept* );
typedef float (*tWeightFunc)( CParabot*, PB_Percept* );

typedef struct {
	int		type;			// goal class (parallism!)
	tGoalFunc	goal;	
	tWeightFunc	weight;
} tGoal;
typedef std::multimap<int, tGoal> tGoalList;

typedef struct goalfinder {
	tGoalList	 knowngoals;
	CParabot	*bot;
	tGoalFunc	 bestgoalfunction[MAX_GOALS];		// for each class
	PB_Percept	*responsiblepercept[MAX_GOALS];
	float		 bestweight[MAX_GOALS];
} GOALFINDER;

void		 goalfinder_init(GOALFINDER *goalfinder, CParabot *pb);
// initializes all necessary variables

void		 goalfinder_addgoal(GOALFINDER *goalfinder, int goalClass, int triggerId, tGoalFunc gf, tWeightFunc wf);
void		 goalfinder_analyze(GOALFINDER *goalfinder, PB_Perception &senses);
void		 goalfinder_analyzeunconditionalgoals(GOALFINDER *goalfinder);
void		 goalfinder_check(GOALFINDER *goalfinder);
void		 goalfinder_synchronize(GOALFINDER *goalfinder);
// deletes goals that can't be executed at the same time

tGoalFunc	 goalfinder_bestgoal(GOALFINDER *goalfinder, int nr);
// returns a pointer to the best goal function

PB_Percept	*goalfinder_trigger(GOALFINDER *goalfinder, int nr);
// returns a pointer to the perception that caused the selection

#endif
