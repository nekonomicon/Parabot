#ifndef PB_GOALFINDER_H
#define PB_GOALFINDER_H

#pragma warning( disable : 4786 )	// disable stl warnings


#include "pb_global.h"
#include "pb_perception.h"
#include <list>
#include <map>


class CParabot;
typedef void (*tGoalFunc)( CParabot*, PB_Percept* );
typedef float (*tWeightFunc)( CParabot*, PB_Percept* );

typedef struct {
	int	type;			// goal class (parallism!)
	tGoalFunc goal;	
	tWeightFunc weight;
} tGoal;
typedef std::multimap<int, tGoal> tGoalList;


#define GOAL_UNCONDITIONAL	0

// goal classes (flags!):
#define G_ACTION 0	// only impact on action
#define G_MOVE	 1	// impact on movement 
#define G_VIEW	 2	// impact on body-/viewangle

#define MAX_GOALS 4	// combinations!


class PB_GoalFinder
{

public:

	PB_GoalFinder() {};

	~PB_GoalFinder();

	void init( CParabot *pb );
	// initializes all necessary variables

	void addGoal( int goalClass, int triggerId, tGoalFunc gf, tWeightFunc wf );

	void analyze( PB_Perception &senses );

	void analyzeUnconditionalGoals();

	void check();

	void synchronize();
	// deletes goals that can't be executed at the same time

	tGoalFunc bestGoal( int nr ) { return bestGoalFunction[nr]; }
	// returns a pointer to the best goal function

	PB_Percept* trigger( int nr ) { return responsiblePercept[nr]; }
	// returns a pointer to the perception that caused the selection


private:

	tGoalList	knownGoals;
	CParabot	*bot;
	tGoalFunc	bestGoalFunction[MAX_GOALS];		// for each class
	PB_Percept	*responsiblePercept[MAX_GOALS];
	float		bestWeight[MAX_GOALS];

};

#endif