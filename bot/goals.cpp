#include "goals.h"
#include "parabot.h"
#include "observer.h"
#include "sectors.h"
#include "vistable.h"
#include "cell.h"
#include "mapcells.h"

extern int mod_id;
extern int botNr;
extern bool haloOnBase;

extern float sineTable[256];

#define DEFAULT_LOOKAROUND_ANGLE 140
float lookAroundAngle = DEFAULT_LOOKAROUND_ANGLE;


#if 0
void
goal_donothing(CParabot *pb, PERCEPT *item)
{
	// DEBUG_MSG( "DoNothing\n" );
}
#endif

//---------------------------------------------------------------------------------------
//  LOOK GOALS
//---------------------------------------------------------------------------------------

void
goal_reacttounidentified(CParabot *pb, PERCEPT *item)
{
	assert(item != 0);
	//DEBUG_MSG( "ReactToUnidentified\n" );
	if (percept_istrackable(item)) {
		action_setviewdir(&pb->action, percept_predictedappearance(item, pb->botPos()), 1);
		pb->setGoalViewDescr( "ReactToUnidentified (Trackable)" );
	} else {
		goal_lookaround( pb, item );
		pb->setGoalViewDescr( "ReactToUnidentified (Unknown Pos)" );
	}
}

float
weight_reacttounidentified(CParabot *pb, PERCEPT *item)
{
	float weight = 0;
	float time = worldtime() - item->firstdetection;	
	if ( time < 2 ) weight = 2 - time;		// look 2 seconds at new sounds
	return weight;
}

void
goal_lookatnewarea(CParabot *pb, PERCEPT *item)
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };
	static int lastItem[32];
	static float startLook[32];
	static float startAngle[32];

	int b = pb->slot;
	Vec3D view;
	action_getviewangle(&pb->action, &view);
	if ((lastCall[b] + 0.5f) < worldtime() || item->id!=lastItem[b] ) {	// in first call init viewangle
		startLook[b] = worldtime();
		vectoangles(&item->lastpos, &view); 
		startAngle[b] = view.y;
	}
	lastCall[b] = worldtime();
	lastItem[b] = item->id;
		
	int cycle = (int)((worldtime() - startLook[b]) * 128);
	cycle &= 255;
	++cycle;
	view.y = startAngle[b] + 30.0f * sinf(2.0f * M_PI / ((float)cycle));
	action_setviewangle(&pb->action, &view, 1);
	pb->setGoalViewDescr( "LookAround (NewArea)" );
}

float
weight_lookatnewarea(CParabot *pb, PERCEPT *item)
{
	float weight = 0;
	float time = worldtime() - item->firstdetection;	
	if ( time < 2 ) weight = 2 - time;		// look 2 seconds at new areas
	return weight;
}

void
goal_lookaround(CParabot *pb, PERCEPT *item)
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };
	static float startLook[32];
	static float startAngle[32];

	int b = pb->slot;
	// DEBUG_MSG( "LookAround\n" );
	Vec3D view;
	action_getviewangle(&pb->action, &view);
	if ( (lastCall[b] + 0.5f) < worldtime() ) {
		startLook[b] = worldtime();
		startAngle[b] = view.y;
	}
	lastCall[b] = worldtime();
	int cycle = (int) ((worldtime() - startLook[b]) * 128);
	cycle &= 255;
	++cycle;
	view.y = startAngle[b] + lookAroundAngle * sinf(2.0f * M_PI / ((float)cycle));	// 60*...
	action_setviewangle(&pb->action, &view, 1);
	if (item) {
		switch (item->pclass) {
		case PI_LASERDOT:	pb->setGoalViewDescr( "LookAround (Laserdot)" );		break;
		case PI_DAMAGE:		pb->setGoalViewDescr( "LookAround (Damage)" );			break;
		case PI_EXPLOSIVE:	pb->setGoalViewDescr( "LookAround (ExplosiveSound)" );	break;
		case PI_FOE:		pb->setGoalViewDescr( "LookAround (PlayerSound)" );		break;
		case PI_NEWAREA:	pb->setGoalViewDescr( "LookAround (NewArea)" );			break;
		default:			pb->setGoalViewDescr( "LookAround (?)" );				break;
		}
	}
	else
		pb->setGoalViewDescr( "LookAround" );
}

float
weight_lookaroundlaserdot(CParabot *pb, PERCEPT *item)
{
	Vec3D dir;

	assert( item != 0 );

	vsub(pb->botPos(), &item->entity->v.origin, &dir);
	float dist = vlen(&dir);
	float weight = (2000 - dist) / 1000;
	if (weight < 0.5f) weight = 0.5f;

	return weight;
}

float
weight_lookarounddamage(CParabot *pb, PERCEPT *item)
{
	assert(item != 0);
	if (!(percept_inflictorknown(item)))
		return 4;

	return 0;
}

float
weight_lookarounddangeroussound(CParabot *pb, PERCEPT *item)
{
	assert( item != 0 );
	if ( !(percept_istrackable(item)) ) return 3;

	return 0;
}

float
weight_lookaroundplayersound(CParabot *pb, PERCEPT *item)
{
	assert( item != 0 );
	if ( !(percept_istrackable(item)) )
		return weight_reacttounidentified( pb, item );

	return 0;
}

void
goal_assistfire(CParabot *pb, PERCEPT *item)
{
	TRACERESULT tr;
	Vec3D start, target;

	makevectors(&item->entity->v.v_angle);
	eyepos(item->entity, &start);
	vma(&start, 4096, &com.globals->fwd, &target);
	trace_line(&start, &target, false, false, item->entity, &tr);	
	// target = tr.vecEndPos;
	// TODO: search for entity in this area and fire
	item->flags |= PI_FOCUS1;
	pb->setGoalViewDescr( "AssistFire" );
}

float
weight_assistfire(CParabot *pb, PERCEPT *item)
{
	assert( item->entity != 0 );

	if ( !weaponhandling_bestweaponusable(&pb->combat.weapon) ) return 0;
	if (item->entity->v.button & ACTION_ATTACK1) return 2;
	return 0;
}

//---------------------------------------------------------------------------------------
//  MOVE GOALS
//---------------------------------------------------------------------------------------

void
goal_collectitems(CParabot *pb, PERCEPT *item)
{
	if (pb->actualPath)	{				// on path?
		char buffer[256];
		strcpy( buffer, "CollectItems (" );
		strcat( buffer, navpoint_classname(pb->actualPath->endNav()));
		if (journey_continues(&pb->actualJourney)) {
			strcat( buffer, ", " );
			int pathId = pb->actualJourney.pathlist.back();
			strcat( buffer, navpoint_classname(getPath(pathId)->endNav()));
		}
		strcat( buffer, ")" );
		pb->setGoalMoveDescr( buffer );
		pb->followActualPath();
	} else if (pb->roamingTarget) {		// target chosen?
		pb->setGoalMoveDescr( "CollectItems (Roaming)" );
		pb->approachRoamingTarget();
	} else {
		pb->setGoalMoveDescr( "CollectItems (GetTarget)" );
		if (mapgraph_linkednavpointsfrom( pb->actualNavpoint )==0) {
			// bot is not at navpoint or navpoint not linked:
			pb->getRoamingTarget();		
		} else {
			// bot is at linked navpoint:
			if ( !pb->getJourneyTarget() )  // try to get a tour,
				pb->getRoamingTarget();		// if not possible roaming			
		}
	}
}

float
weight_collectitems(CParabot *pb, PERCEPT *item)
{
	float wish = needs_wishforitems(&pb->needs);
	if (pb->actualNavpoint) {	// when getting to a navpoint is all we want...
		if (navpoint_offershealth(pb->actualNavpoint) &&
			 (needs_wishforhealth(&pb->needs) > 0) ) wish = 0;
		else if (navpoint_offersarmor(pb->actualNavpoint) &&
			 (needs_wishforarmor(&pb->needs) > 0) ) wish = 0;
		else if ( navpoint_offerscamping(pb->actualNavpoint) &&
			 (needs_desirefor(&pb->needs, NAV_S_CAMPING) == wish) ) wish = 0;
		else if (navpoint_type(pb->actualNavpoint)==NAV_F_TANKCONTROLS &&
			 (needs_desirefor(&pb->needs, NAV_F_TANKCONTROLS) == wish) ) wish = 0;
		else if (navpoint_type(pb->actualNavpoint)==NAV_S_USE_TRIPMINE &&
			 (needs_desirefor(&pb->needs, NAV_S_USE_TRIPMINE) > 0) ) wish *= 0.5f;
	}
	// DEBUG_MSG( "Items = %.1f\n", wish );
	return wish;
}

void
goal_getitem(CParabot *pb, PERCEPT *item)
{
	assert(item != 0);
	item->update = worldtime();

	roaming_checkway(&pb->pathfinder, &item->lastseenpos);
	if (action_gotstuck(&pb->action))
		item->flags |= PI_UNREACHABLE;
	switch( item->pclass ) {
	case PI_WEAPONBOX:	pb->setGoalMoveDescr( "GetItem (Weaponbox)" );		break;
	case PI_HALO:		pb->setGoalMoveDescr( "GetItem (Halo)" );			break;
	default:			pb->setGoalMoveDescr( "GetItem (?)" );			break;
	}
}

float
weight_getweaponbox(CParabot *pb, PERCEPT *item)
{
	assert(item != 0);
	assert(item->distance > 0);

	if (!percept_isreachable(item)) return 0;
	float weight = 300 * needs_wishforweapons(&pb->needs) / (400 + item->distance);
	//if (weight > 9) weight = 9;
	item->update = worldtime() + (item->distance / 1000);
	// DEBUG_MSG( "wbw=%.2f\n", weight );
	return weight;
}

float
weight_gethalo(CParabot *pb, PERCEPT *item)
{
	assert(item != 0);
	assert(item->distance > 0);

	if (!percept_isreachable(item)) return 0;
	float weight = 4000 / item->distance;
	if (weight > 15) weight = 15;
	item->update = worldtime() + (item->distance / 2000);
	
	return weight;
	
}

void
goal_getaway(CParabot *pb, PERCEPT *item)
{
	Vec3D tDir, botPos;

	assert(item != 0);
	item->update = worldtime();

	vcopy(pb->botPos(), &botPos);
	percept_predictedposition(item, &botPos, &tDir);
	vsub(&tDir, &botPos, &tDir);
	vsub(&botPos, &tDir, &tDir);
	roaming_checkway(&pb->pathfinder, &tDir);
	switch( item->pclass ) {
	case PI_LASERDOT:	pb->setGoalMoveDescr( "GetAway (Laserdot)" );		break;
	case PI_EXPLOSIVE:	pb->setGoalMoveDescr( "GetAway (Explosive)" );		break;
	case PI_FOE:		pb->setGoalMoveDescr( "GetAway (Enemy)" );			break;
	default:			pb->setGoalMoveDescr( "GetAway (?)" );			break;
	}
}

float
weight_getawaylaserdot(CParabot *pb, PERCEPT *item)
{
	Vec3D dir;

	assert( item != 0 );
	vsub(pb->botPos(), &item->entity->v.origin, &dir);
	float dist = vlen(&dir);
	float weight = (500 - dist) / 40;
	if (weight < 0) weight = 0;
	return weight;
}

float
weight_getawayexplosive(CParabot *pb, PERCEPT *item)
{
	Vec3D dir;

	assert( item != 0 );
	vsub(pb->botPos(), &item->entity->v.origin, &dir);
	float dist = vlen(&dir);
	float weight = (600 - dist) / 20;
	if (weight < 0) weight = 0;
	return weight;
}

float
weight_getawayenemy(CParabot *pb, PERCEPT *item)
{
	assert( item != 0 );
	if (percept_istrackable(item)) return ( 4 - item->rating - needs_wishforcombat(&pb->needs) );
	return 0;
}

void
goal_loadhealthorarmor(CParabot *pb, PERCEPT *item)
{
	// DEBUG_MSG( "LoadHealthOrArmor\n" );
	action_setspeed(&pb->action, 0);
	action_setviewdir(&pb->action, navpoint_pos(pb->actualNavpoint), 2);
	action_add(&pb->action, BOT_USE, NULL);
	pb->setGoalMoveDescr( "LoadHealthOrArmor" );
}

float
weight_loadhealthorarmor(CParabot *pb, PERCEPT *item)
{
	float weight = 0;

	if ( pb->actualNavpoint && !perception_underfire(&pb->senses) ) {
		if (navpoint_offershealth(pb->actualNavpoint)) weight = needs_wishforhealth(&pb->needs);
		if (navpoint_offersarmor(pb->actualNavpoint))  weight = needs_wishforarmor(&pb->needs);
		if ((weight > 0) && (pb->senses.numenemies > 0)) {
			weight = 0;
			DEBUG_MSG( "No Loading - enemy around!\n" );
		}
	}
	// DEBUG_MSG( "Health/Armor = %f\n", weight );
	return weight;
}

void
goal_laytripmine(CParabot *pb, PERCEPT *item)
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };

	int b = pb->slot;
	if ( (lastCall[b] + 0.5f) < worldtime() ) {	// in first call arm tripmine
		weaponhandling_setpreferredweapon(&pb->combat.weapon, VALVE_WEAPON_TRIPMINE, 1);
		weaponhandling_armbestweapon(&pb->combat.weapon, 50, 0.4f, 0);
	}

	assert( pb->actualNavpoint != 0 );

	// duck if necessary
	if (navpoint_pos(pb->actualNavpoint)->z < (pb->ent->v.absmin.z + 40.0f) ) 
		action_add(&pb->action, BOT_DUCK, NULL);
	// keep tripmine
	weaponhandling_setpreferredweapon(&pb->combat.weapon, VALVE_WEAPON_TRIPMINE, 1);

	weaponhandling_attack(&pb->combat.weapon, navpoint_pos(pb->actualNavpoint), 0.4f, NULL);

	lastCall[b] = worldtime();
	pb->setGoalMoveDescr( "LayTripmine" );
}

float
weight_laytripmine(CParabot *pb, PERCEPT *item)
{
	Vec3D dir;

	if (!weaponhandling_available(&pb->combat.weapon, VALVE_WEAPON_TRIPMINE )) return 0;

	if ( pb->actualNavpoint && !perception_underfire(&pb->senses) ) {
		float nextMine = 10000;
		EDICT *mine = perception_getnearesttripmine(&pb->senses);
		if (mine) {
			vsub(&mine->v.origin, navpoint_pos(pb->actualNavpoint), &dir);
			nextMine = vlen(&dir);
		}
		if ( (navpoint_type(pb->actualNavpoint)==NAV_S_USE_TRIPMINE) && (nextMine > 50) )
			return 5;
	}
	return 0;
}

void
goal_camp(CParabot *pb, PERCEPT *item)
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };

	int b = pb->slot;
	assert( pb->actualNavpoint != 0 );

	if ((lastCall[b] + 0.5f) < worldtime()) {	// in first call init viewangle
		pb->campTime = 0;
		int angleX = navpoint_special(pb->actualNavpoint);
		int angleY = navpoint_special(pb->actualNavpoint);
		angleX &= 0xFFFF;
		angleY >>= 16;
		Vec3D campAngle;
		campAngle.x = (float) (angleX - 360);
		campAngle.y = (float) (angleY - 360);
		campAngle.z = 0;
		action_setviewangle(&pb->action, &campAngle, 1);
	} else {
		pb->campTime += (worldtime() - lastCall[b]);
	}
	pb->lastCamp = worldtime();
	lastCall[b] = worldtime();
		
	action_setspeed(&pb->action, 0);
	lookAroundAngle = 45;
	goal_lookaround( pb, item );
	lookAroundAngle = DEFAULT_LOOKAROUND_ANGLE;
	pb->setGoalMoveDescr( "Camp" );
}

float
weight_camp(CParabot *pb, PERCEPT *item)
{
	float weight = 0;

	if (pb->actualNavpoint && !perception_underfire(&pb->senses)) {
		if (navpoint_offerscamping(pb->actualNavpoint) &&
		    weaponhandling_bestweaponusable(&pb->combat.weapon))
			weight = needs_wishforsniping(&pb->needs, true);
	}
	return weight;
}

void goal_usetank(CParabot *pb, PERCEPT *item)
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };

	//botNr = pb->slot;

	int b = pb->slot;
	assert( pb->actualNavpoint != 0 );

	action_setspeed(&pb->action, 0);

	if ( (lastCall[b]+0.5) < worldtime() ||
		 (pb->campTime>0 && pb->ent->v.viewmodel) ) {	// in first call press button
		pb->campTime = -2;
		Vec3D lookPos;
		vadd(navpoint_pos(pb->actualNavpoint), &pb->ent->v.view_ofs, &lookPos);
		action_add(&pb->action, BOT_USE, &lookPos );
		DEBUG_MSG( "Trying to use tank\n" );
	} else {
		pb->campTime += (worldtime() - lastCall[b]);
	}
	pb->lastCamp = worldtime();
	lastCall[b] = worldtime();

	if (!pb->ent->v.viewmodel) {		// use tank!
		if (item) {
			if (percept_isvisible(item) ) {
				combat_shootatenemy(&pb->combat,&item->lastpos, 0.1f);
				pb->setGoalMoveDescr( "UseTank (Shoot)" );
			} else {
				action_setaimdir(&pb->action, percept_predictedappearance(item, pb->botPos()), NULL);
				pb->setGoalMoveDescr( "UseTank (Tracking)" );
			}
		} else {
			lookAroundAngle = 45;
			goal_lookaround( pb, item );
			lookAroundAngle = DEFAULT_LOOKAROUND_ANGLE;
			pb->setGoalMoveDescr( "UseTank (LookAround)" );
		}
	} else {
		pb->setGoalMoveDescr( "UseTank (StartUse)" );
	}
}

float
weight_usetank(CParabot *pb, PERCEPT *item)
{
	float weight = 0;

	if ( pb->actualNavpoint && !perception_underfire(&pb->senses) ) {
		if (navpoint_type(pb->actualNavpoint) == NAV_F_TANKCONTROLS ) {
			weight = needs_wishforsniping(&pb->needs, false);
			if (item && percept_isvisible(item) && item->distance > 300) weight += 10;
		}
	}
	return weight;
}

void
goal_waitatnavpoint(CParabot *pb, PERCEPT *item)
{
	// don't do anything here...
	pb->setGoalMoveDescr( "WaitAtNavpoint" );
	DEBUG_MSG( "Waiting for halo!\n" );
}

float
weight_waitforhalo(CParabot *pb, PERCEPT *item)
{
	float weight = 0;
	if ( pb->actualNavpoint ) {
		if (navpoint_type(pb->actualNavpoint)==NAV_HW_HALOBASE && haloOnBase ) 
			weight = needs_wishforitems(&pb->needs) + 0.1;	// just a bit more than running to it...
	}
	return weight;
}

void
goal_pause(CParabot *pb, PERCEPT *item)
{
	action_setspeed(&pb->action, 0);
	pb->setGoalMoveDescr( "Pause" );
	//pb->lastPause = worldtime();
}

float
weight_pause(CParabot *pb, PERCEPT *item)
{
	float weight = 0;//exp( pb->pauseFactor * (worldtime() - pb->lastPause) );
	if (weight > 4) weight = 4;
	return weight;
}

void
goal_follow(CParabot *pb, PERCEPT *item)
{
	assert( item != 0 );
	// DEBUG_MSG( "FollowAndAssistLeader\n" );
	if (pb->partner==0) DEBUG_MSG( "WARNING: partner == 0!\n" );
	if ( (pb->partner == 0 ) || (pb->partner != item->entity ) ) {
		int partnerId = observer_playerid( item->entity );
		if (partnerId >= 0) {
			observer_reportpartner( pb->slot, partnerId );
			pb->partner = item->entity;
			pb->actualPath = 0;
		}
	}
	
	if ( !observer_partnervalid( pb->slot ) ) {
		pb->partner = 0;
		return;
	}
	
	if ( observer_shouldfollow( pb->slot, pb->ent ) ) {
		PB_Path_Waypoint wp = observer_getnextwaypoint( pb->slot );
		if (wp.reached( pb->ent )) {
			// DEBUG_MSG( "WP reached, act=%i\n", wp.action() );
			action_add(&pb->action, wp.action(), wp.pos() );	// if there's something to do...
			observer_reportwaypointreached( pb->slot );		// confirm waypoint
		}
		
		action_setviewdir(&pb->action, &pb->partner->v.origin, 0);		// set viewAngle
		action_setmovedir(&pb->action, wp.pos(pb->ent), 0);				// set moveAngle and speed
		action_setmaxspeed(&pb->action);
		pb->pathCheckWay();	
		if (observer_cannotfollow( pb->slot )) {
			DEBUG_MSG( "Cannot follow\n" );
			pb->partner = 0;
		}
	}
/*	if ( observer_partnerincombat( pb->slot ) ) {	// partner involved in combat
		DEBUG_MSG( "I assist!\n" );
		//botState = PB_IN_COMBAT;
	}*/
	pb->setGoalMoveDescr( "Follow" );
}

float
weight_followleader(CParabot *pb, PERCEPT *item)
{
	Vec3D dir;

	if (pb->partner) {
		if (pb->partner == item->entity ) return 4;	// keep on following!
	}

	if (percept_isvisible(item)	// leader visible
	    && (item->lastseenpos.z < (pb->botPos()->z + 16))) {	// not higher than bot
		vsub(&item->lastseenpos, pb->botPos(), &dir);
		if(500.0f > vlen(&dir))	// not too far away
			return 2;
	}
	return 0;
}

float
weight_followenemy(CParabot *pb, PERCEPT *item)
{
	return 0;
	assert( item != 0 );
	if ( !weaponhandling_bestweaponusable(&pb->combat.weapon) ) return 0;
	if (item->state & PI_TACTILE) return (5 + item->rating + needs_wishforcombat(&pb->needs));
	else if (item->rating < -3) return 0;	// too bad
	return (item->rating + needs_wishforcombat(&pb->needs));
}

void
goal_makeroom(CParabot *pb, PERCEPT *item)
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };
	static Vec3D newWp[32];

	int b = pb->slot;

	if ((lastCall[b] + 0.5f) < worldtime()) {	// in first call init newWp
		Vec3D forward, right;

		if (pb->actualPath)
			vsub(pb->waypoint.pos(pb->ent), pb->botPos(), &forward);
		else
			action_getmovedir(&pb->action, &forward);
		normalize(&forward);
		getright(&forward, &right);
		vadd(&forward, &right, &right);
		vma(pb->botPos(), 50.0f, &right, &newWp[b]);
	}

	if (!(pb->actualPath)) {		// if not on path: no problem
		goal_getaway( pb, item );
		DEBUG_MSG( "Making room - getAway\n" );
	} else {
		//pb->action.setMoveDir(newWp);
		roaming_checkway(&pb->pathfinder, &newWp[b]);
		DEBUG_MSG( "Making room - newWp \n" );
	}
	pb->makeRoomTime = worldtime() + 0.5f;	// maintain goal for 0.5 sec 
	lastCall[b] = worldtime();
	pb->setGoalMoveDescr("MakeRoom");
}

float
weight_makeroom(CParabot *pb, PERCEPT *item)
{
	Vec3D dir;

	assert(item != 0);

	vsub(pb->botPos(), &item->entity->v.origin, &dir);
	float dist = vlen(&dir);
	if (dist > 100)
		return 0;
	
	vma(pb->botPos(), 0.05f, &pb->ent->v.velocity, &dir);
	vsub(&dir, &item->entity->v.origin, &dir);
	vsub(&dir, &item->entity->v.velocity, &dir);
	float newDist = vlen(&dir);
	if (newDist >= dist && (worldtime() > pb->makeRoomTime))
		return 0;
	float weight = 10.0f + (100.0f - newDist) * 0.2f;
	if (weight > 20.0f) weight = 20.0f;
	return weight;
}

//---------------------------------------------------------------------------------------
//  ACTION GOALS
//---------------------------------------------------------------------------------------
