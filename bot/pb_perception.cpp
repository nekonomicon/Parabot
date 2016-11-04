#include "pb_perception.h"
#include "bot.h"
#include "pb_weapon.h"
#include "pb_configuration.h"
#include "sounds.h"
#include "pb_mapcells.h"


extern int mod_id;
extern int clientWeapon[32];
extern bool haloOnBase;
extern bool valveTeamPlayMode;
extern PB_Configuration pbConfig;	// from configfiles.cpp
extern Sounds playerSounds;
extern PB_MapCells map;


int globalPerceptionId = 0;


// maximum perception distances for different items
static float perceptionDist[MAX_PERCEPTION+1] = { 0,
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
0,				// damage
MAX_DIST_VI		// snark
};


float PB_Percept::getReactionTime( edict_t *ent, short state, short realClass, float dist )
// returns the time needed to react to the item (time to enter goal-queue)
{
	float time;

	if (realClass <= PI_HOSTAGE) {
		// players
		time = (0.4 / botSensitivity) - 0.2;
		if (state==PI_VISIBLE && isInvisible( ent )) time *= 15;	// DMC
	}
	else if (realClass == PI_DAMAGE) {
		// damage
		time = (0.2 / botSensitivity) - 0.1;
	}
	else if (realClass == PI_TRIPMINE) {
		// bots run into them too often anyway...
		time = 0.0;
	}
	else {
		// items
		time = (0.4 / botSensitivity) - 0.2;
	}
		
	return time;
}


PB_Percept::PB_Percept( float botSens, edict_t *ent, short state, short realClass, float dist )
{
	id = globalPerceptionId++;
	botSensitivity = botSens;
	distance = dist;
	entity = ent;
	firstDetection = worldTime();
	flags = 0;
	lastDetection = worldTime();
	lastPos = UNKNOWN_POS;
	lastSeenPos = UNKNOWN_POS;
	lastSeenTime = -100;
	lastSeenVelocity = UNKNOWN_POS;	

	predTarget = UNKNOWN_POS;	
	lastCalcTarget = 0;
	predAppearance = UNKNOWN_POS;	
	lastCalcAppearance = 0;

	model = PI_UNKNOWN;					// else mark as unknown
	orientation = 0;
	pClass = realClass;
	pState = state;
	rating = 0;
	update = worldTime() + getReactionTime( ent, state, realClass, dist );	
	if (ent) {
		lastPos = ent->v.origin;
		if (state==PI_VISIBLE) {
			lastSeenPos = ent->v.origin;		// if visible, store position
			lastSeenTime = worldTime();			// ...time
			lastSeenVelocity = ent->v.velocity;	// ...velocity
			model = ent->v.modelindex;			// ...and model
		}
	}
}


Vector PB_Percept::predictedPosition( Vector &botPos )
// returns a predicted position for an enemy that is not perceived anymore
{
	// not implemented yet!
	return lastPos;
}


Vector PB_Percept::predictedAppearance( Vector &botPos )
// returns the position where a not visible enemy is most likely to get into line-of-sight
{
	if (isVisible() || entity==0 ) return lastPos;
	
	// update prediction every second:
	if (worldTime()-lastCalcAppearance > 1) predAppearance = UNKNOWN_POS;

	if (predAppearance == UNKNOWN_POS) {
		short predictedPath[128];	// contains cell indices to target
		short origin = map.getCellId( lastSeenPos );
		short start = map.getCellId( lastPos );
		short target = map.getCellId( botPos );
		if ( start==NO_CELL_FOUND || target==NO_CELL_FOUND ) return predAppearance;

		if (hasBeenVisible() && map.lineOfSight( origin, target )) {
			// bot is maintaining LOS -> probably enemy is searching cover!
			// first check if he might run through...
			if ( map.getDirectedPathToAttack( start, target, lastSeenVelocity, predictedPath ) > 0 )
				predAppearance = map.cell( predictedPath[0] ).pos();
			// if not, just pick shortest path
			else if ( map.getPathToAttack( start, target, predictedPath ) > 0 )
				predAppearance = map.cell( predictedPath[0] ).pos();
			else predAppearance = lastPos;
		}
		else {
			// either enemy was never visible or bot has moved and lost LOS...
			// assume enemy is heading towards bot:
			if ( map.getPathToAttack( start, target, predictedPath ) > 0 )
				predAppearance = map.cell( predictedPath[0] ).pos();
			else predAppearance = lastPos;
		}
		
		lastCalcAppearance = worldTime();
	}
	
	return predAppearance;
}


float PB_Percept::targetAccuracy()
{
	if ( orientation < 0.5 ) return 0;
	float x = distance * (1-orientation);
	if (x >= 5.48) return 0.2;
	if (x >= 0.77) return 0.4;
	if (x >= 0.152) return 0.6;
	if (x >= 0.021) return 0.8;
	return 1.0;
}


bool PB_Percept::isAimingAtBot()
{ 
	// only notices aiming after a certain time according to distance (dist->time):
	// 200->0	400->1	600->2	800->3
	float recTime = (distance - 200.0) / (botSensitivity*200);
		
	if (worldTime() >= (firstDetection+recTime)) return (orientation > 0.95);
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


void PB_Perception::init( edict_t *ent )
// initializes all necessary variables
{
	botEnt = ent;
	maxSpeed = serverMaxSpeed();
	detections[0].clear();
	detections[1].clear();
	tactileDetections.clear();
	cdet = 0;
	odet = 1;
	numEnemies = 0;
}


void PB_Perception::setSensitivity( int skill )
{
	//sensitivity = ((float)skill+3.5)/9.0;
	sensitivity =  30.0 / ((float)(20-skill)) - 1.0;	// worst=0.58 .. best=2.0
}


void PB_Perception::addAttack( edict_t *inflictor, int dmg )
{
	// distance means amount of damage done!
	tactileDetections.push_back( PB_Percept( sensitivity, inflictor, PI_TACTILE, PI_DAMAGE, (float)dmg ) );
	if (inflictor) {
		// hack for raising skill:
		float dist = (inflictor->v.origin - botEnt->v.origin).Length();
		detections[cdet].push_back( PB_Percept( sensitivity, inflictor, PI_VISIBLE, PI_FOE, dist) );
	}
}


void PB_Perception::addNewArea( Vector viewDir )
{
	PB_Percept newArea( sensitivity, 0, PI_PREDICTED, PI_NEWAREA, 0);
	newArea.lastPos = viewDir;
	detections[cdet].push_back( newArea );
}



bool PB_Perception::classify( PB_Percept &perc )
// identifies a PI_PLAYER perception as friend or foe if possible
{
	if (pbConfig.onPeaceMode()) {	
		perc.pClass = PI_FRIEND;	// yeah, we love everybody (until he hurts us)
		return true;
	}

	if ((mod_id==VALVE_DLL || mod_id==GEARBOX_DLL || mod_id==DMC_DLL) && !valveTeamPlayMode) {
		perc.pClass = PI_FOE;	// no friends in deathmatch...
		return true;
	}
	else if (mod_id==HOLYWARS_DLL) {
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
		else if (isHeretic( perc.entity )) perc.pClass = PI_FOE;
		else perc.pClass = PI_FRIEND;
		return true;*/
	}
	else {
		if (!(perc.pState & PI_VISIBLE)) return false;	
		// calculate time needed for identification:
		float distance = (botEnt->v.origin - perc.entity->v.origin).Length();
		assert( sensitivity != 0 );
		float recTime = (distance - MAX_DIST_VPR) / (sensitivity*1000);
		
		if (worldTime() >= (perc.firstDetection+recTime)) {	
			if (UTIL_GetTeam( perc.entity ) != UTIL_GetTeam( botEnt )) perc.pClass = PI_FOE;
			else perc.pClass = PI_FRIEND;
			return true;
		}
	}
	return false;
}


bool PB_Perception::isNewPerception( tPerceptionList &oldList, PB_Percept &perc )
// if perc exists in old perception list, delete it there, update perc and return false
{
	// new damages are in tactile list -> no check necessary:
	if (perc.pClass == PI_DAMAGE) return false;

	tPerceptionList::iterator pli = oldList.begin();
	tPerceptionList::iterator match = oldList.end();

	// special check for areas:
	if (perc.pClass == PI_NEWAREA) {
		while ( pli != oldList.end() ) {
			// only check for other areas:
			if (pli->pClass != PI_NEWAREA) {	
				pli++;  continue; 
			}
			if ((perc.lastPos - pli->lastPos).Length() < 16.0) {
				perc.id = pli->id;
				perc.lastPos = pli->lastPos;
				return false;
			}
		}
		return true;
	}


	// normal case:
	float bestDist = 10000;
	while ( pli != oldList.end() ) {
		// don't confuse different models!
		if (perc.model!=PI_UNKNOWN && pli->model!=PI_UNKNOWN && perc.model!=pli->model) {
			pli++;  continue; 
		}

		// memorize every new damage:
		if (pli->pClass == PI_DAMAGE || pli->pClass == PI_NEWAREA) {	
			pli++;  continue; 
		}
		// check directions for area:
		
		// predicted position ok?
		float passedTime = perc.lastDetection - pli->lastDetection;
		if ( (perc.lastPos - pli->lastPos).Length() > ((passedTime+0.2) * maxSpeed) ) {
			pli++;  continue;
		}
		// calculate deviation from predicted position
		float dev = ( pli->lastSeenPos - perc.lastPos ).Length();
		if (dev < bestDist) {
			match = pli;
			bestDist = dev;
		}
		pli++;
	}

	if ( match != oldList.end() ) {	// match found?
		perc.id = match->id;
		// copy timings:
		perc.update = match->update;
		perc.firstDetection = match->firstDetection;
		perc.flags = match->flags & ~(PI_FOCUS1|PI_FOCUS2|PI_DISAPPEARED);
		if (match->flags & PI_FOCUS1) perc.flags |= PI_FOCUS2;	// remember focus once
		
		if ( perc.isVisible() ) {
			perc.pState |= PI_PREDICTED;	// add PREDICTED-flag
			perc.flags &= ~PI_PREEMPTIVE;	// stop preemptive fire
		}
		else {	// not visible:
			perc.lastSeenTime = match->lastSeenTime;
			perc.lastSeenPos = match->lastSeenPos;
			perc.lastSeenVelocity = match->lastSeenVelocity;
			perc.predTarget = match->predTarget;
			perc.lastCalcTarget = match->lastCalcTarget;
			perc.predAppearance = match->predAppearance;
			perc.lastCalcAppearance = match->lastCalcAppearance;
			if (match->pState & PI_VISIBLE) {	// check if just disappeared
				perc.flags |= PI_DISAPPEARED;
			}
			else if (worldTime() > (perc.lastSeenTime+2.0)) {
				perc.flags &= ~PI_PREEMPTIVE;	// stop preemptive fire (only 2 secs!)
			}
		}

		// remember attacks and visual contact:
		perc.pState |= (match->pState & (PI_TACTILE|PI_PREDICTED)); 

		if (perc.model==PI_UNKNOWN) perc.model = match->model;	// remember model 
		if (perc.pClass==PI_PLAYER) {					// unidentified player:
			if (match->pClass!=PI_PLAYER) {				// if formerly known
				perc.pClass = match->pClass;			// ...remember class
			}
		}
		oldList.erase( match );
		return false;
	}

	//debugMsg( "New perception: %i!\n", perc.pClass );
	return true;
}


bool PB_Perception::isNewTactilePerception( tPerceptionList &pList, PB_Percept &perc )
// if perc exists in perception list, delete it there, update perc and return false
{
	tPerceptionList::iterator pli = pList.begin();
	tPerceptionList::iterator match = pList.end();
	while ( pli != pList.end() ) {
		
		// only damages!
		if (pli->pClass != PI_DAMAGE) {
			pli++;  continue; 
		}
		
		// same entity?
		if (perc.entity != pli->entity) {
			pli++;  continue;
		}
		match = pli;
		break;	
	}

	if ( match != pList.end() ) {	// match found?
		perc.id = match->id;
		// copy timings:	-> don't copy update since that is used for underFire() !
		perc.firstDetection = match->firstDetection;
		perc.distance += match->distance;	// add damages
		perc.pState = match->pState;	// origin known flag!
		pList.erase( match );
		//debugFile( "INFLICTOR KNOWN.\n" );
		return false;
	}

	//debugMsg( "New damage-perception!" );
	return true;
}


bool PB_Perception::addIfVisible( Vector pos, edict_t *ent, int pClass )
{
	TraceResult tr;

	// check if in visible distance
	Vector botpos = botEnt->v.origin + botEnt->v.view_ofs;
	float dist = (pos - botpos).Length();
	if (dist > (sensitivity*perceptionDist[pClass])) return false;

	// check if in viewcone
	Vector dir = (pos - botpos).Normalize();
	float dot = DotProduct( gpGlobals->v_forward, dir );
	if (  dot > 0.6 ) {
		UTIL_TraceLine( pos, botpos, dont_ignore_monsters, ignore_glass, botEnt, &tr);	
		if ( tr.flFraction == 1.0 ) {
			detections[cdet].push_back( PB_Percept( sensitivity, ent, PI_PREDICTED, pClass, dist) );
			// entity itself is not supposed to be visible, therefore PI_PREDICTED
			return true;
		}
	}
	return false;
}


bool PB_Perception::addIfVisible( edict_t *ent, int pClass )
{
	TraceResult tr;

	// check if in visible distance
	Vector botpos = botEnt->v.origin + botEnt->v.view_ofs;
	Vector pos = ent->v.origin; //+ ent->v.view_ofs;
	float dist = (pos - botpos).Length();
	if (dist > (sensitivity*perceptionDist[pClass])) return false;

	// check if in viewcone
	Vector dir = (pos - botpos).Normalize();
	float dot = DotProduct( gpGlobals->v_forward, dir );
	if (  dot > 0.6 ) {
		UTIL_TraceLine( botpos, pos, dont_ignore_monsters, ignore_glass, botEnt, &tr);	
		if ( (tr.flFraction == 1.0) || (tr.pHit == ent) ) {
			detections[cdet].push_back( PB_Percept( sensitivity, ent, PI_VISIBLE, pClass, dist) );
			return true;
		}
	}
	return false;
}


void PB_Perception::checkDamageFor( PB_Percept &player ) 
{
	if (player.pClass >= PI_HOSTAGE) return;	// only players cause damage
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
		if ((cdi->pClass < PI_HOSTAGE) && 
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
	CBaseEntity *ent = 0;
	bool detected;

	UTIL_MakeVectors( botEnt->v.v_angle );
	// for (not) detecting own laserspot:
	Vector vecSrc = botEnt->v.origin + botEnt->v.view_ofs;
	Vector vecAiming = gpGlobals->v_forward;
	TraceResult tr;
	UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, botEnt, &tr );
	Vector aimingPos = tr.vecEndPos;

	int h=cdet;  cdet=odet;  odet=h;	// switch lists
	detections[cdet].clear();

	while (ent = UTIL_FindEntityInSphere( ent, botEnt->v.origin, sensitivity*MAX_DIST_VP )) 
	{
		char *pClassname = (char *)STRING(ent->pev->classname);
		
		// detect players
		if ( strcmp( pClassname, "player" ) == 0 ) {
			// check if valid
			if (ent->pev == &(botEnt->v)) continue;		// skip self
			if (!isAlive( ENT(ent->pev) )) continue;	// skip player if not alive
			if (ent->pev->solid == SOLID_NOT) continue;	// skip player if observer
			
			detected = addIfVisible( ent->edict(), PI_PLAYER );
			if (!detected) detected = addIfVisible( ent->pev->absmin, ent->edict(), PI_PLAYER );
			if (!detected) detected = addIfVisible( ent->pev->absmax, ent->edict(), PI_PLAYER );

			// if not detected check if audible by shooting
			if ( !detected ) {
				int clientIndex = ENTINDEX( ent->edict() );
				float dist = (ent->pev->origin - botEnt->v.origin).Length();
				float audibleDist = playerSounds.getSensableDist( clientIndex );
				if (dist < audibleDist*sensitivity) {
					detected = true;
					float trackableDist = playerSounds.getTrackableDist( clientIndex );
					if (dist < trackableDist*sensitivity) {
						//debugMsg( "Player trackable\n" );
						detections[cdet].push_back( PB_Percept( sensitivity, ent->edict(), PI_TRACKABLE, PI_PLAYER, dist) );
					}
					else {
						//debugMsg( "Player audible\n" );
						detections[cdet].push_back( PB_Percept( sensitivity, ent->edict(), PI_AUDIBLE, PI_PLAYER, dist) );
					}
				}
			}	
   		}
		else if ( ( mod_id != DMC_DLL && strcmp( pClassname, "weaponbox" ) == 0 ) ||
				  ( mod_id == DMC_DLL && strcmp( pClassname, "item_backpack" ) == 0 ) )	{
			addIfVisible( ent->edict(), PI_WEAPONBOX );
		}
		else if ( strcmp( pClassname, "halo" ) == 0 ) {
			addIfVisible( ent->edict(), PI_HALO );
		}
		else if ( strcmp( pClassname, "hostage_entity" ) == 0 ) {
			addIfVisible( ent->edict(), PI_HOSTAGE );
		}
		else if ( strcmp( pClassname, "weapon_c4" ) == 0 ) {
			addIfVisible( ent->edict(), PI_BOMB );
		}
		else if ( strcmp( pClassname, "laser_spot" ) == 0 ) {
			if ((ent->pev->origin != aimingPos) && !(ent->pev->effects & EF_NODRAW)) {
				addIfVisible( ent->edict(), PI_LASERDOT );
			}
		}
		else if ( strcmp( pClassname, "monster_satchel" ) == 0 ) {
			addIfVisible( ent->edict(), PI_EXPLOSIVE );
		}
		else if ( strcmp( pClassname, "grenade" ) == 0 ) {
			if (!addIfVisible( ent->edict(), PI_EXPLOSIVE )) {
				float dist = (ent->pev->origin - botEnt->v.origin).Length();
				if (dist < 200*sensitivity) {
					detections[cdet].push_back( PB_Percept( sensitivity, ent->edict(), PI_AUDIBLE, PI_EXPLOSIVE, dist) );
				}
			}
		}
		else if ( strcmp( pClassname, "monster_snark" ) == 0 ) {
			addIfVisible( ent->edict(), PI_SNARK );
		}
		else if ( strcmp( pClassname, "monster_tripmine" ) == 0 ) {
			if (tripmineOwner( ent ) == botEnt) {
				// remember own tripmines even without seeing them:
				float dist = (ent->pev->origin - botEnt->v.origin).Length();
				detections[cdet].push_back( PB_Percept( sensitivity, ent->edict(), PI_TRACKABLE, PI_TRIPMINE, dist) );
			}
			else if ( !addIfVisible( ent->edict(), PI_TRIPMINE ) ) {		// check beamstart
				char *mineClass = (char*)ent;
				Vector *mine_vecDir = (Vector*) (mineClass+616);
				float *mine_flBeamLength = (float*) (mineClass+640);
				Vector beamEnd = ent->pev->origin + (*mine_vecDir) * 2048 * (*mine_flBeamLength);
				Vector beamCenter = (ent->pev->origin + beamEnd) / 2;
				if ( !addIfVisible( beamEnd, ent->edict(), PI_TRIPMINE ) )	// check beamend
					addIfVisible( beamCenter, ent->edict(), PI_TRIPMINE );	// check beamcenter
			}
		}
		
	}

	// determine new perceptions
	tPerceptionList::iterator cdi = detections[cdet].begin();
	while ( cdi != detections[cdet].end() ) {
		// update values if perceipt is known:
		if ( isNewPerception( detections[odet], *cdi ) ) checkDamageFor( *cdi );
		// try to identify as friend or foe if not sure:
		if (cdi->pClass==PI_PLAYER) classify( *cdi );
		cdi++;
	}

	// if we lost some perceptions, don't forget these for a while
	tPerceptionList::iterator odi = detections[odet].begin();
	while ( odi != detections[odet].end() ) {
		if ( (worldTime()-odi->lastDetection) < 5 ) {
			odi->distance = ((Vector)(odi->lastPos-botEnt->v.origin)).Length();
			if ( (odi->pClass <= PI_HOSTAGE) && (odi->entity->v.health < 1) ) {
				odi++;	continue; }	// only living persons
			if ( (odi->pClass==PI_HALO || odi->pClass==PI_WEAPONBOX) && (odi->distance < 50) ) {
				odi++;	continue; }	// no items that have already been picked up

			// set disappeared and visible flags...
			if ( odi->isVisible() ) {
				odi->pState &= ~PI_VISIBLE;	
				odi->flags |= PI_DISAPPEARED;
			}
			else {
				odi->flags &= ~PI_DISAPPEARED;
			}
			detections[cdet].push_back( *odi );
		}
		odi++;
	}

	// add tactile perceptions
	tPerceptionList::iterator tdi = tactileDetections.begin();
	while ( tdi != tactileDetections.end() ) {
		if ( isNewTactilePerception( detections[cdet], *tdi ) ) 
			checkInflictorFor( *tdi );
		//tdi->update = worldTime();	// no delay -> accounted for in getReactionTime() !
		//debugMsg( "New tactile perception!\n" );
		detections[cdet].push_back( *tdi );
		tdi++;
	}
	tactileDetections.clear();

	// count number of overall enemies and unidentified
	numEnemies = 0;
	numUnidentified = 0;
	cdi = detections[cdet].begin();
	while ( cdi != detections[cdet].end() ) {
		if (cdi->pClass == PI_FOE) numEnemies++;
		else if (cdi->pClass == PI_PLAYER) numUnidentified++;
		cdi++;
	}
	char buf[32];
	sprintf( buf, "E=%i  U=%i\n", numEnemies, numUnidentified );
	//debugMsg( buf );
}


void PB_Perception::resetPlayerClassifications()
{
	debugMsg( "Resetting classifications...\n" );
	tPerceptionList::iterator di;
	for (int list=0; list<2; list++) {
		di = detections[list].begin();
		while ( di != detections[list].end() ) {
			if (di->pClass <= PI_FOE) di->pClass = PI_PLAYER;
			di++;
		}
	}
}


edict_t* PB_Perception::getNearestTripmine()
// returns nearest tripmine or 0 if no tripmine seen
{
	edict_t *mine = 0;
	float closest = 8000;

	tPerceptionList::iterator cdi = detections[cdet].begin();
	while ( cdi != detections[cdet].end() ) {
		if (cdi->pClass == PI_TRIPMINE) {
			float dist = ((Vector)(botEnt->v.origin - cdi->lastPos)).Length();
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

	if ( worldTime() < (lastCall+0.1) ) return lastReturn;	//	10 updates/sec. ok
	
	bool attacked = false;
	tPerceptionList::iterator cdi = detections[cdet].begin();
	while ( cdi != detections[cdet].end() ) {
		if ((cdi->pClass == PI_DAMAGE) && ((cdi->update+1.0) > worldTime())) {
			attacked = true;
			break;
		}	
		cdi++;
	}

	lastReturn = attacked;
	lastCall = worldTime();
	return attacked;
}
