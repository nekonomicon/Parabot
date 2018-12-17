#include "parabot.h"
#include "pb_perception.h"
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

int globalPerceptionId = 0;


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


float PB_Percept::getReactionTime( EDICT *ent, short state, short realClass, float dist )
// returns the time needed to react to the item (time to enter goal-queue)
{
	float time;

	if (realClass <= PI_HOSTAGE) {
		// players
		time = (0.4f / botSensitivity) - 0.2f;
		if (state == PI_VISIBLE && is_invisible( ent ))
			time *= 15.0f;	// DMC
	} else if (realClass == PI_DAMAGE) {
		// damage
		time = (0.2f / botSensitivity) - 0.1f;
	} else if (realClass == PI_TRIPMINE) {
		// bots run into them too often anyway...
		time = 0.0f;
	} else {
		// items
		time = (0.4f / botSensitivity) - 0.2f;
	}
		
	return time;
}


PB_Percept::PB_Percept( float botSens, EDICT *ent, short state, short realClass, float dist )
{
	id = globalPerceptionId++;
	botSensitivity = botSens;
	distance = dist;
	entity = ent;
	firstDetection = worldtime();
	flags = 0;
	lastDetection = worldtime();
	vcopy(UNKNOWN_POS, &lastPos);
	vcopy(UNKNOWN_POS, &lastSeenPos);
	lastSeenTime = -100;
	vcopy(UNKNOWN_POS, &lastSeenVelocity);
	
	lastCalcTarget = 0;
	vcopy(UNKNOWN_POS, &predTarget);
	vcopy(UNKNOWN_POS, &predAppearance);
	lastCalcAppearance = 0;

	model = PI_UNKNOWN;					// else mark as unknown
	orientation = 0;
	pClass = realClass;
	pState = state;
	rating = 0;
	update = worldtime() + getReactionTime(ent, state, realClass, dist);	
	if (ent) {
		lastPos = ent->v.origin;
		if (state == PI_VISIBLE) {
			vcopy(&ent->v.origin, &lastSeenPos);	// if visible, store position
			lastSeenTime = worldtime();			// ...time
			vcopy(&ent->v.velocity, &lastSeenVelocity);	// ...velocity
			model = ent->v.modelindex;			// ...and model
		}
	}
}


void PB_Percept::predictedPosition(const Vec3D *botPos, Vec3D *lastPos)
// returns a predicted position for an enemy that is not perceived anymore
{
	// not implemented yet!
	vcopy(botPos, lastPos);
}


Vec3D *PB_Percept::predictedAppearance( const Vec3D *botPos )
// returns the position where a not visible enemy is most likely to get into line-of-sight
{
	if (isVisible() || entity == 0 ) return &lastPos;

	// update prediction every second:
	if (worldtime() - lastCalcAppearance > 1)
		vcopy(UNKNOWN_POS, &predAppearance);

	if (vcomp(&predAppearance, UNKNOWN_POS)) {
		short predictedPath[128];	// contains cell indices to target
		short origin = mapcells_getcellid(&lastSeenPos);
		short start = mapcells_getcellid(&lastPos);
		short target = mapcells_getcellid(botPos);
		if (start == NO_CELL_FOUND || target == NO_CELL_FOUND) return &predAppearance;

		if (hasBeenVisible() && mapcells_lineofsight( origin, target )) {
			// bot is maintaining LOS -> probably enemy is searching cover!
			// first check if he might run through...
			if (mapcells_getdirectedpathtoattack(start, target, &lastSeenVelocity, predictedPath) > 0)
				cell_pos(mapcells_getcell(predictedPath[0]), &predAppearance);
			// if not, just pick shortest path
			else if (mapcells_getpathtoattack( start, target, predictedPath) > 0)
				cell_pos(mapcells_getcell(predictedPath[0]), &predAppearance);
			else
				vcopy(&lastPos, &predAppearance);
		} else {
			// either enemy was never visible or bot has moved and lost LOS...
			// assume enemy is heading towards bot:
			if (mapcells_getpathtoattack(start, target, predictedPath) > 0 )
				cell_pos(mapcells_getcell(predictedPath[0]), &predAppearance);
			else
				vcopy(&lastPos, &predAppearance);
		}

		lastCalcAppearance = worldtime();
	}

	return &predAppearance;
}


float PB_Percept::targetAccuracy()
{
	if (orientation < 0.5f)
		return 0.0f;
	float x = distance * (1.0f - orientation);
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


bool PB_Percept::isAimingAtBot()
{
	// only notices aiming after a certain time according to distance (dist->time):
	// 200->0	400->1	600->2	800->3
	float recTime = (distance - 200.0) / (botSensitivity*200);

	if (worldtime() >= (firstDetection+recTime)) return (orientation > 0.95);
	else return false;
}



//****************************************************************************************



PB_Perception::PB_Perception() 
{
	setSensitivity( 3 );
}


PB_Perception::~PB_Perception() 
{
	detections[0].clear();
	detections[1].clear();
	tactileDetections.clear();
}


void PB_Perception::init( EDICT *ent )
// initializes all necessary variables
{
	botEnt = ent;
	maxSpeed = servermaxspeed();
	detections[0].clear();
	detections[1].clear();
	tactileDetections.clear();
	cdet = 0;
	odet = 1;
	numEnemies = 0;
}

void PB_Perception::setSensitivity( int skill )
{
	//sensitivity = ((float)skill + 3.5f) / 9.0f ;
	sensitivity =  30.0f / ((float)(20 - skill)) - 1.0f;	// worst=0.58 .. best=2.0
}

void PB_Perception::addAttack( EDICT *inflictor, int dmg )
{
	Vec3D dir;

	// distance means amount of damage done!
	tactileDetections.push_back(PB_Percept(sensitivity, inflictor, PI_TACTILE, PI_DAMAGE, (float)dmg));
	if (inflictor) {
		// hack for raising skill:
		vsub(&inflictor->v.origin, &botEnt->v.origin, &dir);
		float dist = vlen(&dir);
		detections[cdet].push_back(PB_Percept(sensitivity, inflictor, PI_VISIBLE, PI_FOE, dist));
	}
}


void PB_Perception::addNewArea( Vec3D *viewDir )
{
	PB_Percept newArea(sensitivity, 0, PI_PREDICTED, PI_NEWAREA, 0);
	vcopy(viewDir, &newArea.lastPos);
	detections[cdet].push_back( newArea );
}



bool PB_Perception::classify( PB_Percept &perc )
// identifies a PI_PLAYER perception as friend or foe if possible
{
	Vec3D dir;

	if (configuration_onpeacemode()) {	
		perc.pClass = PI_FRIEND;	// yeah, we love everybody (until he hurts us)
		return true;
	}

	if ((mod_id==VALVE_DLL || mod_id==HUNGER_DLL || mod_id==GEARBOX_DLL || mod_id==DMC_DLL) && !(com.gamedll_flags & GAMEDLL_TEAMPLAY)) {
		perc.pClass = PI_FOE;	// no friends in deathmatch...
		return true;
	} else if (mod_id==HOLYWARS_DLL) {
		perc.pClass = PI_FOE;	// no friends in deathmatch...
		return true;
/*		if (isTheSaint( botEnt ) || haloOnBase) {
			perc.pClass = PI_FOE;
			return true;
		}
		if (!(perc.pState & PI_VISIBLE)) return false;	
		// only classify visible players:
		if (isTheSaint( perc.entity )) {
			perc.pClass = PI_FOE;
			perc.flags |= PI_HIGH_PRIORITY;
		}
		else if (isHeretic(perc.entity)) perc.pClass = PI_FOE;
		else perc.pClass = PI_FRIEND;
		return true;*/
	} else {
		if (!(perc.pState & PI_VISIBLE)) return false;	
		// calculate time needed for identification:
		vsub(&botEnt->v.origin, &perc.entity->v.origin, &dir);
		float distance = vlen(&dir);
		assert( sensitivity != 0 );
		float recTime = (distance - MAX_DIST_VPR) / (sensitivity * 1000);

		if (worldtime() >= (perc.firstDetection + recTime)) {
			if (getteam(perc.entity) != getteam(botEnt))
				perc.pClass = PI_FOE;
			else
				perc.pClass = PI_FRIEND;

			return true;
		}
	}
	return false;
}


bool PB_Perception::isNewPerception( tPerceptionList &oldList, PB_Percept &perc )
// if perc exists in old perception list, delete it there, update perc and return false
{
	Vec3D dir;

	// new damages are in tactile list -> no check necessary:
	if (perc.pClass == PI_DAMAGE) return false;

	tPerceptionList::iterator pli = oldList.begin();
	tPerceptionList::iterator match = oldList.end();

	// special check for areas:
	if (perc.pClass == PI_NEWAREA) {
		while (pli != oldList.end()) {
			// only check for other areas:
			if (pli->pClass != PI_NEWAREA) {	
				pli++;
				continue; 
			}
			vsub(&perc.lastPos, &pli->lastPos, &dir);
			if (vlen(&dir) < 16.0f) {
				perc.id = pli->id;
				perc.lastPos = pli->lastPos;
				return false;
			}
		}
		return true;
	}


	// normal case:
	float bestDist = 10000;
	while (pli != oldList.end()) {
		// don't confuse different models!
		if (perc.model != PI_UNKNOWN && pli->model != PI_UNKNOWN && perc.model != pli->model) {
			pli++;
			continue;
		}

		// memorize every new damage:
		if (pli->pClass == PI_DAMAGE || pli->pClass == PI_NEWAREA) {	
			pli++;  continue; 
		}
		// check directions for area:
		
		// predicted position ok?
		float passedTime = perc.lastDetection - pli->lastDetection;
		vsub(&perc.lastPos, &pli->lastPos, &dir);
		if (vlen(&dir) > ((passedTime + 0.2f) * maxSpeed)) {
			pli++;
			continue;
		}
		// calculate deviation from predicted position
		vsub(&pli->lastSeenPos, &perc.lastPos, &dir);
		float dev = vlen(&dir);
		if (dev < bestDist) {
			match = pli;
			bestDist = dev;
		}
		pli++;
	}

	if (match != oldList.end()) {	// match found?
		perc.id = match->id;
		// copy timings:
		perc.update = match->update;
		perc.firstDetection = match->firstDetection;
		perc.flags = match->flags & ~(PI_FOCUS1 | PI_FOCUS2 | PI_DISAPPEARED);
		if (match->flags & PI_FOCUS1) perc.flags |= PI_FOCUS2;	// remember focus once
		
		if (perc.isVisible()) {
			perc.pState |= PI_PREDICTED;	// add PREDICTED-flag
			perc.flags &= ~PI_PREEMPTIVE;	// stop preemptive fire
		} else {	// not visible:
			perc.lastSeenTime = match->lastSeenTime;
			perc.lastSeenPos = match->lastSeenPos;
			perc.lastSeenVelocity = match->lastSeenVelocity;
			perc.predTarget = match->predTarget;
			perc.lastCalcTarget = match->lastCalcTarget;
			perc.predAppearance = match->predAppearance;
			perc.lastCalcAppearance = match->lastCalcAppearance;
			if (match->pState & PI_VISIBLE) {	// check if just disappeared
				perc.flags |= PI_DISAPPEARED;
			} else if (worldtime() > (perc.lastSeenTime+2.0)) {
				perc.flags &= ~PI_PREEMPTIVE;	// stop preemptive fire (only 2 secs!)
			}
		}

		// remember attacks and visual contact:
		perc.pState |= (match->pState & (PI_TACTILE | PI_PREDICTED)); 

		if (perc.model == PI_UNKNOWN) perc.model = match->model;	// remember model 
		if (perc.pClass == PI_PLAYER) {					// unidentified player:
			classify( perc );
			/*if (match->pClass!=PI_PLAYER) {				// if formerly known
				perc.pClass = match->pClass;			// ...remember class
			}*/
		}
		oldList.erase( match );
		return false;
	}

	// DEBUG_MSG( "New perception: %i!\n", perc.pClass );
	return true;
}


bool PB_Perception::isNewTactilePerception( tPerceptionList &pList, PB_Percept &perc )
// if perc exists in perception list, delete it there, update perc and return false
{
	tPerceptionList::iterator pli = pList.begin();
	tPerceptionList::iterator match = pList.end();
	while (pli != pList.end()) {
		// only damages!
		if (pli->pClass != PI_DAMAGE) {
			pli++;
			continue;
		}
		
		// same entity?
		if (perc.entity != pli->entity) {
			pli++;
			continue;
		}
		match = pli;
		break;	
	}

	if (match != pList.end()) {	// match found?
		perc.id = match->id;
		// copy timings:	-> don't copy update since that is used for underFire() !
		perc.firstDetection = match->firstDetection;
		perc.distance += match->distance;	// add damages
		perc.pState = match->pState;	// origin known flag!
		pList.erase(match);
		//debugFile( "INFLICTOR KNOWN.\n" );
		return false;
	}

	// DEBUG_MSG( "New damage-perception!" );
	return true;
}

bool PB_Perception::addIfVisible( Vec3D *pos, EDICT *ent, int pClass )
{
	Vec3D botpos, dir;
	TRACERESULT tr;
	bool isvisible = false;

	if(!pos) {
                isvisible = true;
                pos = &ent->v.origin; //+ ent->v.view_ofs;
        }

	// check if in visible distance
	vadd(&botEnt->v.origin, &botEnt->v.view_ofs, &botpos);
	vsub(pos, &botpos, &dir);
	float dist = vlen(&dir);
	if (dist > (sensitivity * perceptionDist[pClass])) return false;

	// check if in viewcone
	normalize(&dir);
	float dot = dotproduct(&com.globals->fwd, &dir);
	if (dot > 0.6) {
		trace_line(pos, &botpos, false, true, botEnt, &tr);
		if ((tr.fraction == 1.0f) || (isvisible && tr.hit == ent)) {
			detections[cdet].push_back(PB_Percept(sensitivity, ent, isvisible ? PI_VISIBLE : PI_PREDICTED, pClass, dist));
			// entity itself is not supposed to be visible, therefore PI_PREDICTED
			return true;
		}
	}
	return false;
}

void PB_Perception::checkDamageFor( PB_Percept &player ) 
{
	if( player.pClass >= PI_HOSTAGE || player.pClass == PI_FRIEND ) return;	// only players cause damage
	tPerceptionList::iterator cdi = detections[cdet].begin();
	while ( cdi != detections[cdet].end() ) {
		if ((cdi->pClass == PI_DAMAGE) && 
			(cdi->entity == player.entity || (cdi->entity == 0 && player.pClass == PI_FOE) )) {
			player.pState |= PI_TACTILE;
			player.pClass = PI_FOE;
			cdi->pState |= PI_ORIG_KNOWN;
			if (cdi->entity == 0) cdi->entity = player.entity;	// assume player is responsible
		}
		cdi++;
	}
}


void PB_Perception::checkInflictorFor( PB_Percept &dmg ) 
{
	tPerceptionList::iterator cdi = detections[cdet].begin();
	while ( cdi != detections[cdet].end() ) {
		if( ( cdi->pClass < PI_HOSTAGE && cdi->pClass != PI_FRIEND ) && 
			(cdi->entity == dmg.entity || (dmg.entity == 0 && cdi->pClass == PI_FOE) )) {
			cdi->pState |= PI_TACTILE;
			cdi->pClass = PI_FOE;
			dmg.pState |= PI_ORIG_KNOWN;
			if (dmg.entity == 0) dmg.entity = cdi->entity;
		}
		cdi++;
	}
}


void PB_Perception::collectData()
{
	Vec3D dir;
	EDICT *ent = 0;
	TRACERESULT tr;
	bool detected;

	makevectors(&botEnt->v.v_angle);

	int h = cdet;
	cdet = odet;
	odet = h;	// switch lists
	detections[cdet].clear();

	while((ent = find_entityinsphere(ent, &botEnt->v.origin, sensitivity * MAX_DIST_VP))) {
		const char *pClassname = STRING(ent->v.classname);

		// detect players
		if (Q_STREQ(pClassname, "player")) {
			// check if valid
			if (!is_alive(ent)		// skip player if not alive
			    || ent == botEnt			// skip self
			    || ent->v.solid == SOLID_NOT)
				continue;	// skip player if observer
			
			detected = addIfVisible(NULL, ent, PI_PLAYER);
			if (!detected)
				detected = addIfVisible(&ent->v.absmin, ent, PI_PLAYER);

			if (!detected)
				detected = addIfVisible(&ent->v.absmax, ent, PI_PLAYER);

			// if not detected check if audible by shooting
			if (!detected) {
				int clientIndex = indexofedict(ent);
				vsub(&ent->v.origin, &botEnt->v.origin, &dir);
				float dist = vlen(&dir);
				float audibleDist = sounds_getSensableDist( clientIndex );
				if (dist < audibleDist * sensitivity) {
					detected = true;
					float trackableDist = sounds_getTrackableDist( clientIndex );
					if (dist < trackableDist * sensitivity) {
						// DEBUG_MSG("Player trackable\n");
						detections[cdet].push_back( PB_Percept( sensitivity, ent, PI_TRACKABLE, PI_PLAYER, dist) );
					} else {
						// DEBUG_MSG( "Player audible\n" );
						detections[cdet].push_back( PB_Percept( sensitivity, ent, PI_AUDIBLE, PI_PLAYER, dist) );
					}
				}
			}	
   		} else if ((mod_id != DMC_DLL && Q_STREQ( pClassname, "weaponbox")) ||
				  (mod_id == DMC_DLL && Q_STREQ( pClassname, "item_backpack")))	{
			addIfVisible(NULL, ent, PI_WEAPONBOX);
		} else if (Q_STREQ(pClassname, "halo")) {
			addIfVisible(NULL, ent, PI_HALO);
		} else if (Q_STREQ(pClassname, "hostage_entity")) {
			addIfVisible(NULL, ent, PI_HOSTAGE);
		} else if (Q_STREQ(pClassname, "weapon_c4")) {
			addIfVisible(NULL, ent, PI_BOMB);
		} else if (Q_STREQ(pClassname, "laser_spot")) {
			// for (not) detecting own laserspot:
			Vec3D vecSrc;

			eyepos(botEnt, &vecSrc);
			vma(&vecSrc, 8192.0f, &com.globals->fwd, &dir);
			trace_line(&vecSrc, &dir, false, false, botEnt, &tr );
			if (!vcomp(&ent->v.origin, &tr.endpos) && !(ent->v.effects & EF_NODRAW)) {
				addIfVisible( NULL, ent, PI_LASERDOT );
			}
		} else if (Q_STREQ(pClassname, "monster_satchel")) {
			addIfVisible( NULL, ent, PI_EXPLOSIVE );
		} else if (Q_STREQ(pClassname, "grenade")) {
			if (!addIfVisible( NULL, ent, PI_EXPLOSIVE )) {
				vsub(&ent->v.origin, &botEnt->v.origin, &dir);
				float dist = vlen(&dir);
				if (dist < 200.0f * sensitivity) {
					detections[cdet].push_back( PB_Percept( sensitivity, ent, PI_AUDIBLE, PI_EXPLOSIVE, dist) );
				}
			}
		} else if (Q_STREQ(pClassname, "monster_snark")) {
			addIfVisible(NULL, ent, PI_SNARK);
		} else if (Q_STREQ(pClassname, "monster_tripmine") || Q_STREQ(pClassname, "monster_tripsnark")) {
			if (ent->v.owner == botEnt) {
				// remember own tripmines even without seeing them:
				vsub(&ent->v.origin, &botEnt->v.origin, &dir);
				float dist = vlen(&dir);
				detections[cdet].push_back( PB_Percept( sensitivity, ent, PI_TRACKABLE, PI_TRIPMINE, dist) );
			} else if (!addIfVisible(NULL, ent, PI_TRIPMINE)) {		// check beamstart
				Vec3D mine_vecEnd, beamEnd, beamCenter;

				vma(&ent->v.origin, 2048.0f, &com.globals->fwd, &mine_vecEnd);
				trace_line(&ent->v.origin, &mine_vecEnd, false, false, ent, &tr );
				float mine_flBeamLength = tr.fraction;
				vscale(&mine_vecEnd, mine_flBeamLength, &beamEnd);
				if (!addIfVisible(&beamEnd, ent, PI_TRIPMINE)) {	// check beamend
					vadd(&ent->v.origin, &beamEnd, &beamCenter);
					addIfVisible(&beamCenter, ent, PI_TRIPMINE);	// check beamcenter
				}
			}
		}
	}

	// determine new perceptions
	tPerceptionList::iterator cdi = detections[cdet].begin();
	while (cdi != detections[cdet].end()) {
		// update values if perceipt is known:
		if (isNewPerception(detections[odet], *cdi)) checkDamageFor(*cdi);
		// try to identify as friend or foe if not sure:
		if (cdi->pClass == PI_PLAYER) classify(*cdi);
		cdi++;
	}

	// if we lost some perceptions, don't forget these for a while
	tPerceptionList::iterator odi = detections[odet].begin();
	while (odi != detections[odet].end()) {
		if ((worldtime() - odi->lastDetection) < 5) {
			vsub(&odi->lastPos, &botEnt->v.origin, &dir);
			odi->distance = vlen(&dir);
			if ((odi->pClass <= PI_HOSTAGE) && (odi->entity->v.health < 1)) {
				odi++;	// only living persons
				continue;
			} else if ((odi->pClass == PI_HALO || odi->pClass == PI_WEAPONBOX) && (odi->distance < 50)) {
				odi++;	// no items that have already been picked up
				continue;
			} else if (odi->isVisible()) { // set disappeared and visible flags...
				odi->pState &= ~PI_VISIBLE;	
				odi->flags |= PI_DISAPPEARED;
			} else {
				odi->flags &= ~PI_DISAPPEARED;
			}
			detections[cdet].push_back(*odi);
		}
		odi++;
	}

	// add tactile perceptions
	tPerceptionList::iterator tdi = tactileDetections.begin();
	while (tdi != tactileDetections.end()) {
		if (isNewTactilePerception(detections[cdet], *tdi)) 
			checkInflictorFor(*tdi);

		//tdi->update = worldtime();	// no delay -> accounted for in getReactionTime() !
		// DEBUG_MSG("New tactile perception!\n");
		detections[cdet].push_back(*tdi);
		tdi++;
	}
	tactileDetections.clear();

	// count number of overall enemies and unidentified
	numEnemies = 0;
	numUnidentified = 0;
	cdi = detections[cdet].begin();
	while (cdi != detections[cdet].end()) {
		if (cdi->pClass == PI_FOE) numEnemies++;
		else if (cdi->pClass == PI_PLAYER) numUnidentified++;
		cdi++;
	}
	// DEBUG_MSG( "E=%i  U=%i\n", numEnemies, numUnidentified );
}


void PB_Perception::resetPlayerClassifications()
{
	DEBUG_MSG("Resetting classifications...\n");
	tPerceptionList::iterator di;
	for (int list = 0; list < 2; list++) {
		di = detections[list].begin();
		while (di != detections[list].end()) {
			if (di->pClass <= PI_FOE) di->pClass = PI_PLAYER;
			di++;
		}
	}
}


EDICT* PB_Perception::getNearestTripmine()
// returns nearest tripmine or 0 if no tripmine seen
{
	Vec3D dir;
	EDICT *mine = 0;
	float closest = 8000;

	tPerceptionList::iterator cdi = detections[cdet].begin();
	while (cdi != detections[cdet].end()) {
		if (cdi->pClass == PI_TRIPMINE) {
			vsub(&botEnt->v.origin, &cdi->lastPos, &dir);
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


bool PB_Perception::underFire()
// returns true if last damage less than 1 second ago
{
	static bool lastReturn = false;
	static float lastCall = 0;

	if (worldtime() < (lastCall + 0.1f)) return lastReturn;	//	10 updates/sec. ok
	
	bool attacked = false;
	tPerceptionList::iterator cdi = detections[cdet].begin();
	while (cdi != detections[cdet].end()) {
		if ((cdi->pClass == PI_DAMAGE) && ((cdi->update + 1.0f) > worldtime())) {
			attacked = true;
			break;
		}
		cdi++;
	}

	lastReturn = attacked;
	lastCall = worldtime();
	return attacked;
}
