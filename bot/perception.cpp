#include "parabot.h"
#include "bot.h"
#include "weapon.h"
#include "configuration.h"
#include "sounds.h"
#include "sectors.h"
#include "vistable.h"
#include "cell.h"
#include "mapcells.h"

extern int mod_id;
extern int clientWeapon[32];
extern bool haloOnBase;
// extern Sounds playerSounds;

int globalPerceptionId;

// maximum perception distances for different items
static float perceptionDist[MAX_PERCEPTION + 1] = { 0,
MAX_DIST_VP,	// player
MAX_DIST_VP,	// friend
MAX_DIST_VP,	// foe
MAX_DIST_VP,	// hostage
MAX_DIST_VI,	// bomb
MAX_DIST_VI,	// weaponbox
MAX_DIST_VI,	// explosive
MAX_DIST_VI,	// laserdot
MAX_DIST_VI,	// tripmine
MAX_DIST_VI,	// halo
0,		// damage
MAX_DIST_VI	// snark
};

float
percept_getreactiontime(PERCEPT *percept, EDICT *ent, short state, short realClass, float dist)
// returns the time needed to react to the item (time to enter goal-queue)
{
	float time;

	if (realClass <= PI_HOSTAGE) {
		// players
		time = (0.4f / percept->botsensitivity) - 0.2f;
		if (state == PI_VISIBLE && is_invisible( ent ))
			time *= 15.0f;	// DMC
	} else if (realClass == PI_DAMAGE) {
		// damage
		time = (0.2f / percept->botsensitivity) - 0.1f;
	} else if (realClass == PI_TRIPMINE) {
		// bots run into them too often anyway...
		time = 0.0f;
	} else {
		// items
		time = (0.4f / percept->botsensitivity) - 0.2f;
	}
		
	return time;
}

void
percept_construct(PERCEPT *percept, float botSens, EDICT *ent, short state, short realClass, float dist)
{
	percept->id = globalPerceptionId++;
	percept->botsensitivity = botSens;
	percept->distance = dist;
	percept->entity = ent;
	percept->firstdetection = worldtime();
	percept->lastdetection = worldtime();
	percept->lastseentime = -100;

	percept->model = PI_UNKNOWN;					// else mark as unknown
	percept->pclass = realClass;
	percept->state = state;
	percept->update = worldtime() + percept_getreactiontime(percept, ent, state, realClass, dist);	
	if (ent) {
		percept->lastpos = ent->v.origin;
		if (state == PI_VISIBLE) {
			vcopy(&ent->v.origin, &percept->lastseenpos);	// if visible, store position
			percept->lastseentime = worldtime();			// ...time
			vcopy(&ent->v.velocity, &percept->lastseenvelocity);	// ...velocity
			percept->model = ent->v.modelindex;			// ...and model
		}
	}
}

void
percept_predictedposition(PERCEPT *percept, const Vec3D *botPos, Vec3D *lastpos)
// returns a predicted position for an enemy that is not perceived anymore
{
	// not implemented yet!
	vcopy(botPos, lastpos);
}

Vec3D *
percept_predictedappearance(PERCEPT *percept, const Vec3D *botPos)
// returns the position where a not visible enemy is most likely to get into line-of-sight
{
	if (percept_isvisible(percept)
	    || percept->entity == 0)
		return &percept->lastpos;

	// update prediction every second:
	if (worldtime() - percept->lastcalcappearance > 1)
		vcopy(UNKNOWN_POS, &percept->predappearance);

	if (vcomp(&percept->predappearance, UNKNOWN_POS)) {
		short predictedPath[128];	// contains cell indices to target
		short origin = mapcells_getcellid(&percept->lastseenpos);
		short start = mapcells_getcellid(&percept->lastpos);
		short target = mapcells_getcellid(botPos);
		if (start == NO_CELL_FOUND || target == NO_CELL_FOUND) return &percept->predappearance;

		if (percept_hasbeenvisible(percept) && mapcells_lineofsight( origin, target )) {
			// bot is maintaining LOS -> probably enemy is searching cover!
			// first check if he might run through...
			if (mapcells_getdirectedpathtoattack(start, target, &percept->lastseenvelocity, predictedPath) > 0)
				cell_pos(mapcells_getcell(predictedPath[0]), &percept->predappearance);
			// if not, just pick shortest path
			else if (mapcells_getpathtoattack(start, target, predictedPath) > 0)
				cell_pos(mapcells_getcell(predictedPath[0]), &percept->predappearance);
			else
				vcopy(&percept->lastpos, &percept->predappearance);
		} else {
			// either enemy was never visible or bot has moved and lost LOS...
			// assume enemy is heading towards bot:
			if (mapcells_getpathtoattack(start, target, predictedPath) > 0 )
				cell_pos(mapcells_getcell(predictedPath[0]), &percept->predappearance);
			else
				vcopy(&percept->lastpos, &percept->predappearance);
		}

		percept->lastcalcappearance = worldtime();
	}

	return &percept->predappearance;
}

float
percept_targetaccuracy(PERCEPT *percept)
{
	if (percept->orientation < 0.5f)
		return 0.0f;

	float x = percept->distance * (1.0f - percept->orientation);
	if (x >= 5.48f)
		return 0.2f;

	if (x >= 0.77f)
		return 0.4f;

	if (x >= 0.152f)
		return 0.6f;

	if (x >= 0.021f)
		return 0.8f;

	return 1.0f;
}

bool
percept_isaimingatbot(PERCEPT *percept)
{
	// only notices aiming after a certain time according to distance (dist->time):
	// 200->0	400->1	600->2	800->3
	float recTime = (percept->distance - 200.0) / (percept->botsensitivity * 200);

	if (worldtime() >= (percept->firstdetection + recTime))
		return (percept->orientation > 0.95);

	return false;
}

bool
percept_hasbeenvisible(PERCEPT *percept)
{
	return (percept->state & PI_PREDICTED);
}

bool
percept_isvisible(PERCEPT *percept)
{
	return (percept->state & PI_VISIBLE);
}

bool
percept_istrackable(PERCEPT *percept)
{
	return (percept->state & (PI_TRACKABLE | PI_VISIBLE | PI_PREDICTED));
}

bool
percept_inflictorknown(PERCEPT *percept)
{
	return (percept->state & PI_ORIG_KNOWN);
}

bool
percept_hasbeenspotted(PERCEPT *percept)
{
	return (percept->state & (PI_VISIBLE | PI_PREDICTED));
}

bool
percept_ismoving(PERCEPT *percept)
{
	return (vlen(&percept->entity->v.velocity) > 5.0);
}

bool
percept_isfacingbot(PERCEPT *percept)
{
	return (percept->orientation > 0.7);
}

bool
percept_ishurtingbot(PERCEPT *percept)
{
	return (percept->state & PI_TACTILE);
}

bool
percept_isalert(PERCEPT *percept)
{
	return (percept->flags & PI_ALERT);
}

bool
percept_canbeattackedbest(PERCEPT *percept)
{
	return (percept->flags & PI_BEST_ARMED);
}

bool
percept_hashighpriority(PERCEPT *percept)
{
	return (percept->flags & PI_HIGH_PRIORITY);
}

bool
percept_hasfocus(PERCEPT *percept)
{
	return (percept->flags & (PI_FOCUS1 | PI_FOCUS2));
}

bool
percept_isreachable(PERCEPT *percept)
{
	return (!(percept->flags & PI_UNREACHABLE));
}

bool
percept_hasjustdisappeared(PERCEPT *percept)
{
	return (percept->flags & PI_DISAPPEARED);
}

bool
percept_isunderpreemptivefire(PERCEPT *percept)
{
	return (percept->flags & PI_PREEMPTIVE);
}

//****************************************************************************************
/*
PB_Perception::PB_Perception() 
{
	setSensitivity( 3 );
}


PB_Perception::~PB_Perception() 
{
	detections[0].clear();
	detections[1].clear();
	tactiledetections.clear();
}
*/
static bool perception_classify(PERCEPTION *perception, PERCEPT *perc);

void
perception_init(PERCEPTION *perception, EDICT *ent)
// initializes all necessary variables
{
	perception->botent = ent;
	perception->maxspeed = servermaxspeed();
	perception->detections[0].clear();
	perception->detections[1].clear();
	perception->tactiledetections.clear();
	perception->cdet = 0;
	perception->odet = 1;
	perception->numenemies = 0;
}

void
perception_setsensitivity(PERCEPTION *perception, int skill)
{
	//sensitivity = ((float)skill + 3.5f) / 9.0f ;
	perception->sensitivity =  30.0f / ((float)(20 - skill)) - 1.0f;	// worst=0.58 .. best=2.0
}

void
perception_addattack(PERCEPTION *perception, EDICT *inflictor, int dmg)
{
	Vec3D dir;
	PERCEPT tperc = {};

	// distance means amount of damage done!
	percept_construct(&tperc, perception->sensitivity, inflictor, PI_TACTILE, PI_DAMAGE, (float)dmg);
	perception->tactiledetections.push_back(tperc);
	if (inflictor) {
		// hack for raising skill:
		vsub(&inflictor->v.origin, &perception->botent->v.origin, &dir);
		float dist = vlen(&dir);
		PERCEPT newperc = {};
		percept_construct(&newperc, perception->sensitivity, inflictor, PI_VISIBLE, PI_FOE, dist);
		perception->detections[perception->cdet].push_back(newperc);
	}
}

void
perception_addnewarea(PERCEPTION *perception, Vec3D *viewDir)
{
	PERCEPT newArea = {};
	percept_construct(&newArea, perception->sensitivity, NULL, PI_PREDICTED, PI_NEWAREA, 0);
	vcopy(viewDir, &newArea.lastpos);
	perception->detections[perception->cdet].push_back(newArea);
}

bool
perception_classify(PERCEPTION *perception, PERCEPT *perc)
// identifies a PI_PLAYER perception as friend or foe if possible
{
	Vec3D dir;

	if (configuration_onpeacemode()) {	
		perc->pclass = PI_FRIEND;	// yeah, we love everybody (until he hurts us)
		return true;
	}

	if ((mod_id==VALVE_DLL || mod_id==HUNGER_DLL || mod_id==GEARBOX_DLL || mod_id==DMC_DLL) && !(com.gamedll_flags & GAMEDLL_TEAMPLAY)) {
		perc->pclass = PI_FOE;	// no friends in deathmatch...
		return true;
	} else if (mod_id==HOLYWARS_DLL) {
		perc->pclass = PI_FOE;	// no friends in deathmatch...
		return true;
/*		if (isTheSaint(perception->botent) || haloOnBase) {
			perc->pclass = PI_FOE;
			return true;
		}
		if (!(perc->state & PI_VISIBLE)) return false;	
		// only classify visible players:
		if (isTheSaint( perc->entity )) {
			perc->pclass = PI_FOE;
			perc->flags |= PI_HIGH_PRIORITY;
		}
		else if (isHeretic(perc->entity)) perc->pclass = PI_FOE;
		else perc->pclass = PI_FRIEND;
		return true;*/
	} else {
		if (!(perc->state & PI_VISIBLE)) return false;	
		// calculate time needed for identification:
		vsub(&perception->botent->v.origin, &perc->entity->v.origin, &dir);
		float distance = vlen(&dir);
		assert( sensitivity != 0 );
		float recTime = (distance - MAX_DIST_VPR) / (perception->sensitivity * 1000);

		if (worldtime() >= (perc->firstdetection + recTime)) {
			if (getteam(perc->entity) != getteam(perception->botent))
				perc->pclass = PI_FOE;
			else
				perc->pclass = PI_FRIEND;

			return true;
		}
	}
	return false;
}

bool
perception_isnewperception(PERCEPTION *perception, tPerceptionList &oldList, PERCEPT *perc)
// if perc exists in old perception list, delete it there, update perc and return false
{
	Vec3D dir;

	// new damages are in tactile list -> no check necessary:
	if (perc->pclass == PI_DAMAGE) return false;

	tPerceptionList::iterator pli = oldList.begin();
	tPerceptionList::iterator match = oldList.end();

	// special check for areas:
	if (perc->pclass == PI_NEWAREA) {
		while (pli != oldList.end()) {
			// only check for other areas:
			if (pli->pclass != PI_NEWAREA) {	
				pli++;
				continue; 
			}
			vsub(&perc->lastpos, &pli->lastpos, &dir);
			if (vlen(&dir) < 16.0f) {
				perc->id = pli->id;
				perc->lastpos = pli->lastpos;
				return false;
			}
		}
		return true;
	}

	// normal case:
	float bestDist = 10000;
	while (pli != oldList.end()) {
		// don't confuse different models!
		if (perc->model != PI_UNKNOWN && pli->model != PI_UNKNOWN && perc->model != pli->model) {
			pli++;
			continue;
		}

		// memorize every new damage:
		if (pli->pclass == PI_DAMAGE || pli->pclass == PI_NEWAREA) {	
			pli++;  continue; 
		}
		// check directions for area:
		
		// predicted position ok?
		float passedTime = perc->lastdetection - pli->lastdetection;
		vsub(&perc->lastpos, &pli->lastpos, &dir);
		if (vlen(&dir) > ((passedTime + 0.2f) * perception->maxspeed)) {
			pli++;
			continue;
		}
		// calculate deviation from predicted position
		vsub(&pli->lastseenpos, &perc->lastpos, &dir);
		float dev = vlen(&dir);
		if (dev < bestDist) {
			match = pli;
			bestDist = dev;
		}
		pli++;
	}

	if (match != oldList.end()) {	// match found?
		perc->id = match->id;
		// copy timings:
		perc->update = match->update;
		perc->firstdetection = match->firstdetection;
		perc->flags = match->flags & ~(PI_FOCUS1 | PI_FOCUS2 | PI_DISAPPEARED);
		if (match->flags & PI_FOCUS1) perc->flags |= PI_FOCUS2;	// remember focus once
		
		if (percept_isvisible(perc)) {
			perc->state |= PI_PREDICTED;	// add PREDICTED-flag
			perc->flags &= ~PI_PREEMPTIVE;	// stop preemptive fire
		} else {	// not visible:
			perc->lastseentime = match->lastseentime;
			perc->lastseenpos = match->lastseenpos;
			perc->lastseenvelocity = match->lastseenvelocity;
			perc->predtarget = match->predtarget;
			perc->lastcalctarget = match->lastcalctarget;
			perc->predappearance = match->predappearance;
			perc->lastcalcappearance = match->lastcalcappearance;
			if (match->state & PI_VISIBLE) {	// check if just disappeared
				perc->flags |= PI_DISAPPEARED;
			} else if (worldtime() > (perc->lastseentime + 2.0f)) {
				perc->flags &= ~PI_PREEMPTIVE;	// stop preemptive fire (only 2 secs!)
			}
		}

		// remember attacks and visual contact:
		perc->state |= (match->state & (PI_TACTILE | PI_PREDICTED)); 

		if (perc->model == PI_UNKNOWN) perc->model = match->model;	// remember model 
		if (perc->pclass == PI_PLAYER) {					// unidentified player:
			perception_classify(perception, perc);
			/*if (match->pclass!=PI_PLAYER) {				// if formerly known
				perc->pclass = match->pclass;			// ...remember class
			}*/
		}
		oldList.erase( match );
		return false;
	}

	// DEBUG_MSG( "New perception: %i!\n", perc->pclass );
	return true;
}

bool
perception_isnewtactileperception(PERCEPTION *perception, tPerceptionList &pList, PERCEPT *perc)
// if perc exists in perception list, delete it there, update perc and return false
{
	tPerceptionList::iterator pli = pList.begin();
	tPerceptionList::iterator match = pList.end();
	while (pli != pList.end()) {
		// only damages!
		if (pli->pclass != PI_DAMAGE) {
			pli++;
			continue;
		}
		
		// same entity?
		if (perc->entity != pli->entity) {
			pli++;
			continue;
		}
		match = pli;
		break;	
	}

	if (match != pList.end()) {	// match found?
		perc->id = match->id;
		// copy timings:	-> don't copy update since that is used for underFire() !
		perc->firstdetection = match->firstdetection;
		perc->distance += match->distance;	// add damages
		perc->state = match->state;	// origin known flag!
		pList.erase(match);
		//debugFile( "INFLICTOR KNOWN.\n" );
		return false;
	}

	// DEBUG_MSG( "New damage-perception!" );
	return true;
}

static bool
perception_addifvisible(PERCEPTION *perception, Vec3D *pos, EDICT *ent, int pclass)
{
	Vec3D botpos, dir;
	TRACERESULT tr;
	bool isvisible = false;

	if(!pos) {
                isvisible = true;
                pos = &ent->v.origin; //+ ent->v.view_ofs;
        }

	// check if in visible distance
	vadd(&perception->botent->v.origin, &perception->botent->v.view_ofs, &botpos);
	vsub(pos, &botpos, &dir);
	float dist = vlen(&dir);
	if (dist > (perception->sensitivity * perceptionDist[pclass]))
		return false;

	// check if in viewcone
	normalize(&dir);
	float dot = dotproduct(&com.globals->fwd, &dir);
	if (dot > 0.6) {
		trace_line(pos, &botpos, false, true, perception->botent, &tr);
		if ((tr.fraction == 1.0f) || (isvisible && tr.hit == ent)) {
			PERCEPT perc = {};
			percept_construct(&perc, perception->sensitivity, ent, isvisible ? PI_VISIBLE : PI_PREDICTED, pclass, dist);
			perception->detections[perception->cdet].push_back(perc);
			// entity itself is not supposed to be visible, therefore PI_PREDICTED
			return true;
		}
	}
	return false;
}

static void
perception_checkdamagefor(PERCEPTION *perception, PERCEPT *player)
{
	if( player->pclass >= PI_HOSTAGE || player->pclass == PI_FRIEND ) return;	// only players cause damage
	tPerceptionList::iterator cdi = perception->detections[perception->cdet].begin();
	while ( cdi != perception->detections[perception->cdet].end() ) {
		if ((cdi->pclass == PI_DAMAGE) && 
			(cdi->entity == player->entity || (cdi->entity == 0 && player->pclass == PI_FOE) )) {
			player->state |= PI_TACTILE;
			player->pclass = PI_FOE;
			cdi->state |= PI_ORIG_KNOWN;
			if (cdi->entity == 0) cdi->entity = player->entity;	// assume player is responsible
		}
		cdi++;
	}
}

static void
perception_checkinflictorfor(PERCEPTION *perception, PERCEPT *dmg)
{
	tPerceptionList::iterator cdi = perception->detections[perception->cdet].begin();
	while ( cdi != perception->detections[perception->cdet].end() ) {
		if ((cdi->pclass < PI_HOSTAGE && cdi->pclass != PI_FRIEND) && 
			(cdi->entity == dmg->entity || (dmg->entity == 0 && cdi->pclass == PI_FOE) )) {
			cdi->state |= PI_TACTILE;
			cdi->pclass = PI_FOE;
			dmg->state |= PI_ORIG_KNOWN;
			if (dmg->entity == 0)
				dmg->entity = cdi->entity;
		}
		cdi++;
	}
}

void
perception_collectdata(PERCEPTION *perception)
{
	Vec3D dir;
	EDICT *ent = 0;
	TRACERESULT tr;
	bool detected;

	makevectors(&perception->botent->v.v_angle);

	int h = perception->cdet;
	perception->cdet = perception->odet;
	perception->odet = h;	// switch lists
	perception->detections[perception->cdet].clear();

	while ((ent = find_entityinsphere(ent, &perception->botent->v.origin, perception->sensitivity * MAX_DIST_VP))) {
		const char *pclassname = STRING(ent->v.classname);

		// detect players
		if (Q_STREQ(pclassname, "player")) {
			// check if valid
			if (!is_alive(ent)		// skip player if not alive
			    || ent == perception->botent			// skip self
			    || ent->v.solid == SOLID_NOT)
				continue;	// skip player if observer
			
			detected = perception_addifvisible(perception, NULL, ent, PI_PLAYER);
			if (!detected)
				detected = perception_addifvisible(perception, &ent->v.absmin, ent, PI_PLAYER);

			if (!detected)
				detected = perception_addifvisible(perception, &ent->v.absmax, ent, PI_PLAYER);

			// if not detected check if audible by shooting
			if (!detected) {
				int clientIndex = indexofedict(ent);
				vsub(&ent->v.origin, &perception->botent->v.origin, &dir);
				float dist = vlen(&dir);
				float audibleDist = sounds_getSensableDist( clientIndex );
				if (dist < audibleDist * perception->sensitivity) {
					detected = true;
					float trackableDist = sounds_getTrackableDist( clientIndex );
					int state;
					PERCEPT perc = {};
					if (dist < trackableDist * perception->sensitivity) {
						// DEBUG_MSG("Player trackable\n");
						state = PI_TRACKABLE;
					} else {
						// DEBUG_MSG( "Player audible\n" );
						state = PI_AUDIBLE;
					}
					percept_construct(&perc, perception->sensitivity, ent, state, PI_PLAYER, dist);
					perception->detections[perception->cdet].push_back(perc);
				}
			}
   		} else if ((mod_id != DMC_DLL && Q_STREQ( pclassname, "weaponbox")) ||
				  (mod_id == DMC_DLL && Q_STREQ( pclassname, "item_backpack")))	{
			perception_addifvisible(perception, NULL, ent, PI_WEAPONBOX);
		} else if (Q_STREQ(pclassname, "halo")) {
			perception_addifvisible(perception, NULL, ent, PI_HALO);
		} else if (Q_STREQ(pclassname, "hostage_entity")) {
			perception_addifvisible(perception, NULL, ent, PI_HOSTAGE);
		} else if (Q_STREQ(pclassname, "weapon_c4")) {
			perception_addifvisible(perception, NULL, ent, PI_BOMB);
		} else if (Q_STREQ(pclassname, "laser_spot")) {
			// for (not) detecting own laserspot:
			Vec3D vecSrc;

			eyepos(perception->botent, &vecSrc);
			vma(&vecSrc, 8192.0f, &com.globals->fwd, &dir);
			trace_line(&vecSrc, &dir, false, false, perception->botent, &tr );
			if (!vcomp(&ent->v.origin, &tr.endpos) && !(ent->v.effects & EF_NODRAW)) {
				perception_addifvisible(perception, NULL, ent, PI_LASERDOT );
			}
		} else if (Q_STREQ(pclassname, "monster_satchel")) {
			perception_addifvisible(perception, NULL, ent, PI_EXPLOSIVE );
		} else if (Q_STREQ(pclassname, "grenade")) {
			if (!perception_addifvisible(perception, NULL, ent, PI_EXPLOSIVE )) {
				vsub(&ent->v.origin, &perception->botent->v.origin, &dir);
				float dist = vlen(&dir);
				if (dist < 200.0f * perception->sensitivity) {
					PERCEPT perc = {};
					percept_construct(&perc, perception->sensitivity, ent, PI_AUDIBLE, PI_EXPLOSIVE, dist);
					perception->detections[perception->cdet].push_back(perc);
				}
			}
		} else if (Q_STREQ(pclassname, "monster_snark")) {
			perception_addifvisible(perception, NULL, ent, PI_SNARK);
		} else if (Q_STREQ(pclassname, "monster_tripmine") || Q_STREQ(pclassname, "monster_tripsnark")) {
			if (ent->v.owner == perception->botent) {
				// remember own tripmines even without seeing them:
				vsub(&ent->v.origin, &perception->botent->v.origin, &dir);
				float dist = vlen(&dir);
				PERCEPT perc = {};
				percept_construct(&perc, perception->sensitivity, ent, PI_TRACKABLE, PI_TRIPMINE, dist);
				perception->detections[perception->cdet].push_back(perc);
			} else if (!perception_addifvisible(perception, NULL, ent, PI_TRIPMINE)) {		// check beamstart
				Vec3D mine_vecEnd, beamEnd, beamCenter;

				vma(&ent->v.origin, 2048.0f, &com.globals->fwd, &mine_vecEnd);
				trace_line(&ent->v.origin, &mine_vecEnd, false, false, ent, &tr );
				float mine_flBeamLength = tr.fraction;
				vscale(&mine_vecEnd, mine_flBeamLength, &beamEnd);
				if (!perception_addifvisible(perception, &beamEnd, ent, PI_TRIPMINE)) {	// check beamend
					vadd(&ent->v.origin, &beamEnd, &beamCenter);
					perception_addifvisible(perception, &beamCenter, ent, PI_TRIPMINE);	// check beamcenter
				}
			}
		}
	}

	// determine new perceptions
	tPerceptionList::iterator cdi = perception->detections[perception->cdet].begin();
	while (cdi != perception->detections[perception->cdet].end()) {
		// update values if perceipt is known:
		if (perception_isnewperception(perception, perception->detections[perception->odet], &(*cdi)))
			perception_checkdamagefor(perception, &(*cdi));

		// try to identify as friend or foe if not sure:
		if (cdi->pclass == PI_PLAYER)
			perception_classify(perception, &(*cdi));
		cdi++;
	}

	// if we lost some perceptions, don't forget these for a while
	tPerceptionList::iterator odi = perception->detections[perception->odet].begin();
	while (odi != perception->detections[perception->odet].end()) {
		if ((worldtime() - odi->lastdetection) < 5) {
			vsub(&odi->lastpos, &perception->botent->v.origin, &dir);
			odi->distance = vlen(&dir);
			if ((odi->pclass <= PI_HOSTAGE) && (odi->entity->v.health < 1)) {
				odi++;	// only living persons
				continue;
			} else if ((odi->pclass == PI_HALO || odi->pclass == PI_WEAPONBOX) && (odi->distance < 50)) {
				odi++;	// no items that have already been picked up
				continue;
			} else if (percept_isvisible(&(*odi))) { // set disappeared and visible flags...
				odi->state &= ~PI_VISIBLE;	
				odi->flags |= PI_DISAPPEARED;
			} else {
				odi->flags &= ~PI_DISAPPEARED;
			}
			perception->detections[perception->cdet].push_back(*odi);
		}
		odi++;
	}

	// add tactile perceptions
	tPerceptionList::iterator tdi = perception->tactiledetections.begin();
	while (tdi != perception->tactiledetections.end()) {
		if (perception_isnewtactileperception(perception, perception->detections[perception->cdet], &(*tdi)))
			perception_checkinflictorfor(perception, &(*tdi));

		//tdi->update = worldtime();	// no delay -> accounted for in getReactionTime() !
		// DEBUG_MSG("New tactile perception!\n");
		perception->detections[perception->cdet].push_back(*tdi);
		tdi++;
	}
	perception->tactiledetections.clear();

	// count number of overall enemies and unidentified
	perception->numenemies = 0;
	perception->numunidentified = 0;
	cdi = perception->detections[perception->cdet].begin();
	while (cdi != perception->detections[perception->cdet].end()) {
		if (cdi->pclass == PI_FOE) perception->numenemies++;
		else if (cdi->pclass == PI_PLAYER) perception->numunidentified++;
		cdi++;
	}
	// DEBUG_MSG( "E=%i  U=%i\n", perception->numenemies, perception->numunidentified );
}

void
perception_resetplayerclassifications(PERCEPTION *perception)
{
	DEBUG_MSG("Resetting classifications...\n");
	tPerceptionList::iterator di;
	for (int list = 0; list < 2; list++) {
		di = perception->detections[list].begin();
		while (di != perception->detections[list].end()) {
			if (di->pclass <= PI_FOE) di->pclass = PI_PLAYER;
			di++;
		}
	}
}

EDICT *
perception_getnearesttripmine(PERCEPTION *perception)
// returns nearest tripmine or 0 if no tripmine seen
{
	Vec3D dir;
	EDICT *mine = 0;
	float closest = 8000;

	tPerceptionList::iterator cdi = perception->detections[perception->cdet].begin();
	while (cdi != perception->detections[perception->cdet].end()) {
		if (cdi->pclass == PI_TRIPMINE) {
			vsub(&perception->botent->v.origin, &cdi->lastpos, &dir);
			float dist = vlen(&dir);
			if (dist < closest) {
				mine = cdi->entity;
				closest = dist;
			}
		}
		cdi++;
	}
	return mine;
}

bool
perception_underfire(PERCEPTION *perception)
// returns true if last damage less than 1 second ago
{
	static bool lastReturn = false;
	static float lastCall = 0;

	if (worldtime() < (lastCall + 0.1f)) return lastReturn;	//	10 updates/sec. ok
	
	bool attacked = false;
	tPerceptionList::iterator cdi = perception->detections[perception->cdet].begin();
	while (cdi != perception->detections[perception->cdet].end()) {
		if ((cdi->pclass == PI_DAMAGE) && ((cdi->update + 1.0f) > worldtime())) {
			attacked = true;
			break;
		}
		cdi++;
	}

	lastReturn = attacked;
	lastCall = worldtime();
	return attacked;
}

tPerceptionList *
perception_list(PERCEPTION *perception)
{
	return &(perception->detections[perception->cdet]);
}
