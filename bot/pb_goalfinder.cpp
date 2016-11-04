#include "pb_goalfinder.h"
//#include "pb_goals.h"
#include "parabot.h"




PB_GoalFinder::~PB_GoalFinder()
{
	knownGoals.clear();
}


void PB_GoalFinder::init( CParabot *pb )
// initializes all necessary variables
{
	bot = pb;
	bot->goalMove[0] = 0;
	bot->goalView[0] = 0;
	bot->goalAct[0] = 0;
	for (int i=0; i<MAX_GOALS; i++) { 
		bestGoalFunction[i] = 0;
		responsiblePercept[i] = 0;
		bestWeight[i] = 0;
	}
}


void PB_GoalFinder::addGoal( int goalClass, int triggerId, tGoalFunc gf, tWeightFunc wf )
{
	tGoal g;
	g.type = goalClass;
	g.goal = gf;
	g.weight = wf;
	knownGoals.insert( std::make_pair(triggerId, g) );
}


void PB_GoalFinder::analyze( PB_Perception &senses )
{
	float weight;
	int   type;
	tPerceptionList *perception = senses.perceptionList();
	tPerceptionList::iterator pli = perception->begin();

	while ( pli != perception->end() ) {
		/*PB_Percept dmgp = *pli;
		if ( pli->pClass == PI_DAMAGE ) {
			debugMsg( "damage sensed\n" );
		}*/
		if ( worldTime() >= pli->update ) {
			// rating only necessary for enemies:
			if (pli->pClass==PI_FOE) {
				assert (pli->entity != 0);
				// check if enemy can see the bot
				UTIL_MakeVectors( pli->entity->v.v_angle );
				Vector dir = (bot->botPos() - pli->entity->v.origin).Normalize();
				pli->orientation = DotProduct( gpGlobals->v_forward, dir );
				// weapon rating ( -5 .. +5 )
				if (pli->hasBeenSpotted()) pli->rating = bot->combat.getRating( (*pli) ); 
				else pli->rating = 0;
				
				//debugMsg( "Rating %s = %f\n", STRING(pli->entity->v.netname), pli->rating );
				//float dst = (bot->botPos() - pli->entity->v.origin).Length();
				//float hp = bot->action.targetAccuracy();
				//debugMsgF("HitProb = %.2f\n", hp );
			}	
			tGoalList::iterator gli = knownGoals.find(pli->pClass);
			while ( gli!=knownGoals.end() ) {
				//debugMsg( "." );
				assert( gli->first == pli->pClass );
				type = gli->second.type;
				tWeightFunc wf = gli->second.weight;
				if (wf) weight = (*wf)( bot, &(*pli) );
				else weight = -100;

				if (weight > bestWeight[type]) {
					bestGoalFunction[type] = gli->second.goal;
					responsiblePercept[type] = &(*pli);
					assert( responsiblePercept[type]->pClass > 0 && responsiblePercept[type]->pClass <= MAX_PERCEPTION );
					bestWeight[type] = weight;
				}
				// next goal for this perception:
				do gli++; while ((gli != knownGoals.end()) && (gli->first != pli->pClass));
			}
			//debugMsg( "\n" );
		}
		pli++;
		//check();
	}
	//check();
}


void PB_GoalFinder::analyzeUnconditionalGoals()
{
	float weight;
	int   type;
	tGoalList::iterator gli = knownGoals.begin();

	while ( gli != knownGoals.end() ) {
		if (gli->first == GOAL_UNCONDITIONAL) {
			type = gli->second.type;
			tWeightFunc wf = gli->second.weight;
			if (wf) weight = (*wf)( bot, 0 );
			else weight = -100;

			if (weight > bestWeight[type]) {
				bestGoalFunction[type] = gli->second.goal;
				responsiblePercept[type] = 0;
				bestWeight[type] = weight;
			}
		}
		gli++;
	}
}


void PB_GoalFinder::synchronize()
// deletes goals that can't be executed at the same time
{
	if ( bestWeight[G_MOVE|G_VIEW] > (bestWeight[G_MOVE]+bestWeight[G_VIEW]) ) {
		bestGoalFunction[G_MOVE] = 0;
		responsiblePercept[G_MOVE] = 0;
		bestGoalFunction[G_VIEW] = 0;
		responsiblePercept[G_VIEW] = 0;
	}
	else {
		bestGoalFunction[G_MOVE|G_VIEW] = 0;
		responsiblePercept[G_MOVE|G_VIEW] = 0;
	}
}


void PB_GoalFinder::check()
{
	for (int i=0; i<MAX_GOALS; i++) {
		if (responsiblePercept[i] != 0) {
			assert( responsiblePercept[i]->pClass > 0 && responsiblePercept[i]->pClass <= MAX_PERCEPTION );
		}
	}
}