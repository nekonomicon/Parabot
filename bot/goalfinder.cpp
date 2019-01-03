#include "parabot.h"

/*
PB_GoalFinder::~PB_GoalFinder()
{
	knowngoals.clear();
}
*/

void
goalfinder_init(GOALFINDER *goalfinder, CParabot *pb)
// initializes all necessary variables
{
	goalfinder->bot = pb;
#if DEBUG
	goalfinderbot->goalMove[0] = 0;
	goalfinderbot->goalView[0] = 0;
	goalfinderbot->goalAct[0] = 0;
#endif
	memset(&goalfinder->bestgoalfunction, 0, sizeof goalfinder->bestgoalfunction);
	memset(&goalfinder->responsiblepercept, 0, sizeof goalfinder->responsiblepercept);
	memset(&goalfinder->bestweight, 0, sizeof goalfinder->bestweight);
}

void
goalfinder_addgoal(GOALFINDER *goalfinder, int goalClass, int triggerId, tGoalFunc gf, tWeightFunc wf)
{
	tGoal g;
	g.type = goalClass;
	g.goal = gf;
	g.weight = wf;
	goalfinder->knowngoals.insert(std::make_pair(triggerId, g));
}

void
goalfinder_analyze(GOALFINDER *goalfinder, PERCEPTION *senses)
{
	float weight;
	int   type;
	tPerceptionList *perception = perception_list(senses);
	tPerceptionList::iterator pli = perception->begin();

	while ( pli != perception->end() ) {
		/*PB_Percept dmgp = *pli;
		if ( pli->pclass == PI_DAMAGE ) {
			DEBUG_MSG( "damage sensed\n" );
		}*/
		if ( worldtime() >= pli->update ) {
			// rating only necessary for enemies:
			if (pli->pclass == PI_FOE) {
				assert (pli->entity != 0);
				// check if enemy can see the bot
				makevectors(&pli->entity->v.v_angle);
				Vec3D dir;
				vsub(goalfinder->bot->botPos(), &pli->entity->v.origin, &dir);
				normalize(&dir);
				pli->orientation = dotproduct(&com.globals->fwd, &dir);
				// weapon rating ( -5 .. +5 )
				if (percept_hasbeenspotted(&(*pli)))
					pli->rating = combat_getrating(&goalfinder->bot->combat, &(*pli));
				else
					pli->rating = 0;
				
				// DEBUG_MSG( "Rating %s = %f\n", STRING(pli->entity->v.netname), pli->rating );
				//float dst = (bot->botPos() - pli->entity->v.origin).Length();
				//float hp = bot->action.targetAccuracy();
				// DEBUG_MSG("HitProb = %.2f\n", hp );
			}	
			tGoalList::iterator gli = goalfinder->knowngoals.find(pli->pclass);
			while (gli != goalfinder->knowngoals.end()) {
				// DEBUG_MSG( "." );
				assert(gli->first == pli->pclass);
				type = gli->second.type;
				tWeightFunc wf = gli->second.weight;
				if (wf)
					weight = (*wf)( goalfinder->bot, &(*pli) );
				else
					weight = -100;

				if (weight > goalfinder->bestweight[type]) {
					goalfinder->bestgoalfunction[type] = gli->second.goal;
					goalfinder->responsiblepercept[type] = &(*pli);
					assert(responsiblepercept[type]->pclass > 0 && goalfinder->responsiblepercept[type]->pclass <= MAX_PERCEPTION);
					goalfinder->bestweight[type] = weight;
				}
				// next goal for this perception:
				do{
					gli++;
				} while ((gli != goalfinder->knowngoals.end()) && (gli->first != pli->pclass));
			}
			// DEBUG_MSG( "\n" );
		}
		pli++;
		// goalfinder_check();
	}
	// goalfinder_check();
}

void
goalfinder_analyzeunconditionalgoals(GOALFINDER *goalfinder)
{
	float weight;
	int   type;
	tGoalList::iterator gli = goalfinder->knowngoals.begin();

	while (gli != goalfinder->knowngoals.end()) {
		if (gli->first == GOAL_UNCONDITIONAL) {
			type = gli->second.type;
			tWeightFunc wf = gli->second.weight;
			if (wf)
				weight = (*wf)( goalfinder->bot, 0 );
			else
				weight = -100;

			if (weight > goalfinder->bestweight[type]) {
				goalfinder->bestgoalfunction[type] = gli->second.goal;
				goalfinder->responsiblepercept[type] = 0;
				goalfinder->bestweight[type] = weight;
			}
		}
		gli++;
	}
}

void
goalfinder_synchronize(GOALFINDER *goalfinder)
// deletes goals that can't be executed at the same time
{
	if (goalfinder->bestweight[G_MOVE|G_VIEW] > (goalfinder->bestweight[G_MOVE] + goalfinder->bestweight[G_VIEW])) {
		goalfinder->bestgoalfunction[G_MOVE] = 0;
		goalfinder->responsiblepercept[G_MOVE] = 0;
		goalfinder->bestgoalfunction[G_VIEW] = 0;
		goalfinder->responsiblepercept[G_VIEW] = 0;
	} else {
		goalfinder->bestgoalfunction[G_MOVE|G_VIEW] = 0;
		goalfinder->responsiblepercept[G_MOVE|G_VIEW] = 0;
	}
}

void
goalfinder_check(GOALFINDER *goalfinder)
{
	for (int i = 0; i < MAX_GOALS; i++) {
		if (goalfinder->responsiblepercept[i] != 0) {
			assert(goalfinder->responsiblepercept[i]->pclass > 0 && goalfinder->responsiblepercept[i]->pclass <= MAX_PERCEPTION);
		}
	}
}

tGoalFunc
goalfinder_bestgoal(GOALFINDER *goalfinder, int nr)
{
	return goalfinder->bestgoalfunction[nr];
}

PERCEPT *
goalfinder_trigger(GOALFINDER *goalfinder, int nr)
{
	return goalfinder->responsiblepercept[nr];
}
