#include "pb_goals.h"
#include "parabot.h"
#include "pb_observer.h"
#include "sectors.h"
#include "pb_mapcells.h"

extern int mod_id;
extern PB_MapGraph mapGraph;
extern PB_MapCells map;
extern PB_Observer observer;
extern int botNr;
extern EDICT *camPlayer;
extern bool haloOnBase;

void startBotCam( EDICT *pEntity );

extern float sineTable[256];

#define DEFAULT_LOOKAROUND_ANGLE 140
float lookAroundAngle = DEFAULT_LOOKAROUND_ANGLE;


#if 0
void goalDoNothing( CParabot *pb, PB_Percept*item )
{
	// DEBUG_MSG( "DoNothing\n" );
}
#endif


//---------------------------------------------------------------------------------------
//  LOOK GOALS
//---------------------------------------------------------------------------------------

void goalReactToUnidentified( CParabot *pb, PB_Percept*item )
{
	assert(item != 0);
	//DEBUG_MSG( "ReactToUnidentified\n" );
	if (item->isTrackable()) {
		action_setviewdir(&pb->action,item->predictedAppearance( pb->botPos() ), 1 );
		pb->setGoalViewDescr( "ReactToUnidentified (Trackable)" );
	} else {
		goalLookAround( pb, item );
		pb->setGoalViewDescr( "ReactToUnidentified (Unknown Pos)" );
	}
}

float weightReactToUnidentified( CParabot *pb, PB_Percept*item )
{
	float weight = 0;
	float time = worldtime() - item->firstDetection;	
	if ( time < 2 ) weight = 2 - time;		// look 2 seconds at new sounds
	return weight;
}

void goalLookAtNewArea( CParabot *pb, PB_Percept*item )
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
		vectoangles(&item->lastPos, &view); 
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

float weightLookAtNewArea( CParabot *pb, PB_Percept*item )
{
	float weight = 0;
	float time = worldtime() - item->firstDetection;	
	if ( time < 2 ) weight = 2 - time;		// look 2 seconds at new areas
	return weight;
}

void goalLookAround( CParabot *pb, PB_Percept*item )
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
		switch( item->pClass ) {
		case PI_LASERDOT:	pb->setGoalViewDescr( "LookAround (Laserdot)" );		break;
		case PI_DAMAGE:		pb->setGoalViewDescr( "LookAround (Damage)" );			break;
		case PI_EXPLOSIVE:	pb->setGoalViewDescr( "LookAround (ExplosiveSound)" );	break;
		case PI_FOE:		pb->setGoalViewDescr( "LookAround (PlayerSound)" );		break;
		case PI_NEWAREA:	pb->setGoalViewDescr( "LookAround (NewArea)" );			break;
		default:			pb->setGoalViewDescr( "LookAround (?)" );				break;
		}
	}
	else pb->setGoalViewDescr( "LookAround" );
}

float weightLookAroundLaserdot( CParabot *pb, PB_Percept*item )
{
	Vec3D dir;

	assert( item != 0 );

	vsub(pb->botPos(), &item->entity->v.origin, &dir);
	float dist = vlen(&dir);
	float weight = (2000 - dist) / 1000;
	if (weight < 0.5f) weight = 0.5f;

	return weight;
}

float weightLookAroundDamage( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	if ( !(item->inflictorKnown()) ) return 4;

	return 0;
}

float weightLookAroundDangerousSound( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	if ( !(item->isTrackable()) ) return 3;

	return 0;
}

float weightLookAroundPlayerSound( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	if ( !(item->isTrackable()) )
		return weightReactToUnidentified( pb, item );

	return 0;
}

void goalAssistFire(CParabot *pb, PB_Percept*item)
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


float weightAssistFire( CParabot *pb, PB_Percept*item )
{
	assert( item->entity != 0 );

	if ( !pb->combat.weapon.bestWeaponUsable() ) return 0;
	if (item->entity->v.button & ACTION_ATTACK1) return 2;
	else return 0;
}


//---------------------------------------------------------------------------------------
//  MOVE GOALS
//---------------------------------------------------------------------------------------

void goalCollectItems( CParabot *pb, PB_Percept*item )
{
	if (pb->actualPath)	{				// on path?
		char buffer[256];
		strcpy( buffer, "CollectItems (" );
		strcat( buffer, pb->actualPath->endNav().classname() );
		if (pb->actualJourney.continues()) {
			strcat( buffer, ", " );
			int pathId = pb->actualJourney.pathList.back();
			strcat( buffer, getPath( pathId )->endNav().classname() );
		}
		strcat( buffer, ")" );
		pb->setGoalMoveDescr( buffer );
		pb->followActualPath();
	}
	else if (pb->roamingTarget)	{		// target chosen?
		pb->setGoalMoveDescr( "CollectItems (Roaming)" );
		pb->approachRoamingTarget();
	}
	else {
		pb->setGoalMoveDescr( "CollectItems (GetTarget)" );
		if (mapGraph.linkedNavpointsFrom( pb->actualNavpoint )==0) {
			// bot is not at navpoint or navpoint not linked:
			pb->getRoamingTarget();		
		}
		else {
			// bot is at linked navpoint:
			if ( !pb->getJourneyTarget() )  // try to get a tour,
				pb->getRoamingTarget();		// if not possible roaming			
		}
	}
}


float weightCollectItems( CParabot *pb, PB_Percept*item )
{
	float wish = pb->needs.needForItems();
	if (pb->actualNavpoint) {	// when getting to a navpoint is all we want...
		if ( pb->actualNavpoint->offersHealth() &&
			 (pb->needs.needForHealth() > 0) ) wish = 0;
		else if ( pb->actualNavpoint->offersArmor() &&
			 (pb->needs.needForArmor() > 0) ) wish = 0;
		else if ( pb->actualNavpoint->offersCamping() &&
			 (pb->needs.desireFor( NAV_S_CAMPING ) == wish) ) wish = 0;
		else if ( pb->actualNavpoint->type()==NAV_F_TANKCONTROLS &&
			 (pb->needs.desireFor( NAV_F_TANKCONTROLS ) == wish) ) wish = 0;
		else if ( pb->actualNavpoint->type()==NAV_S_USE_TRIPMINE &&
			 (pb->needs.desireFor( NAV_S_USE_TRIPMINE ) > 0) ) wish *= 0.5f;
	}
	// DEBUG_MSG( "Items = %.1f\n", wish );
	return wish;
}




void goalGetItem( CParabot *pb, PB_Percept*item )
{
	assert(item != 0);
	item->update = worldtime();

	pb->pathfinder.checkWay(&item->lastSeenPos);
	if (action_gotstuck(&pb->action))
		item->flags |= PI_UNREACHABLE;
	switch( item->pClass ) {
	case PI_WEAPONBOX:	pb->setGoalMoveDescr( "GetItem (Weaponbox)" );		break;
	case PI_HALO:		pb->setGoalMoveDescr( "GetItem (Halo)" );			break;
	default:			pb->setGoalMoveDescr( "GetItem (?)" );			break;
	}
}


float weightGetWeaponbox( CParabot *pb, PB_Percept*item )
{
	assert(item != 0);
	assert(item->distance > 0);

	if (!item->isReachable()) return 0;
	float weight = 300 * pb->needs.needForWeapons() / (400 + item->distance);
	//if (weight > 9) weight = 9;
	item->update = worldtime() + (item->distance / 1000);
	// DEBUG_MSG( "wbw=%.2f\n", weight );
	return weight;
}


float weightGetHalo( CParabot *pb, PB_Percept*item )
{
	assert(item != 0);
	assert(item->distance > 0);

	if (!item->isReachable()) return 0;
	float weight = 4000 / item->distance;
	if (weight > 15) weight = 15;
	item->update = worldtime() + (item->distance / 2000);
	
	return weight;
	
}


void goalGetAway( CParabot *pb, PB_Percept*item )
{
	Vec3D tDir, botPos;

	assert(item != 0);
	item->update = worldtime();

	vcopy(pb->botPos(), &botPos);
	item->predictedPosition(&botPos, &tDir);
	vsub(&tDir, &botPos, &tDir);
	vsub(&botPos, &tDir, &tDir);
	pb->pathfinder.checkWay(&tDir);
	switch( item->pClass ) {
	case PI_LASERDOT:	pb->setGoalMoveDescr( "GetAway (Laserdot)" );		break;
	case PI_EXPLOSIVE:	pb->setGoalMoveDescr( "GetAway (Explosive)" );		break;
	case PI_FOE:		pb->setGoalMoveDescr( "GetAway (Enemy)" );			break;
	default:			pb->setGoalMoveDescr( "GetAway (?)" );			break;
	}
}


float weightGetAwayLaserdot( CParabot *pb, PB_Percept*item )
{
	Vec3D dir;

	assert( item != 0 );
	vsub(pb->botPos(), &item->entity->v.origin, &dir);
	float dist = vlen(&dir);
	float weight = (500 - dist) / 40;
	if (weight < 0) weight = 0;
	return weight;
}


float weightGetAwayExplosive( CParabot *pb, PB_Percept*item )
{
	Vec3D dir;

	assert( item != 0 );
	vsub(pb->botPos(), &item->entity->v.origin, &dir);
	float dist = vlen(&dir);
	float weight = (600 - dist) / 20;
	if (weight < 0) weight = 0;
	return weight;
}


float weightGetAwayEnemy( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	if (item->isTrackable()) return ( 4 - item->rating - pb->needs.wishForCombat() );
	return 0;
}



void goalLoadHealthOrArmor( CParabot *pb, PB_Percept*item )
{
	// DEBUG_MSG( "LoadHealthOrArmor\n" );
	action_setspeed(&pb->action, 0);
	action_setviewdir(&pb->action, pb->actualNavpoint->pos(), 2);
	action_add(&pb->action, BOT_USE, NULL);
	pb->setGoalMoveDescr( "LoadHealthOrArmor" );
}


float weightLoadHealthOrArmor( CParabot *pb, PB_Percept*item )
{
	float weight = 0;

	if ( pb->actualNavpoint && !pb->senses.underFire() ) {
		if (pb->actualNavpoint->offersHealth()) weight = pb->needs.needForHealth();
		if (pb->actualNavpoint->offersArmor())  weight = pb->needs.needForArmor();
		if ((weight > 0) && (pb->senses.numEnemies > 0)) {
			weight = 0;
			DEBUG_MSG( "No Loading - enemy around!\n" );
		}
	}
	// DEBUG_MSG( "Health/Armor = %f\n", weight );
	return weight;
}



void goalLayTripmine( CParabot *pb, PB_Percept*item )
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };

	int b = pb->slot;
	if ( (lastCall[b] + 0.5f) < worldtime() ) {	// in first call arm tripmine
		pb->combat.weapon.setPreferredWeapon( VALVE_WEAPON_TRIPMINE );
		pb->combat.weapon.armBestWeapon( 50, 0.4f, 0);
	}

	assert( pb->actualNavpoint != 0 );

	// duck if necessary
	if ( pb->actualNavpoint->pos()->z < (pb->ent->v.absmin.z + 40.0f) ) 
		action_add(&pb->action, BOT_DUCK, NULL);
	// keep tripmine
	pb->combat.weapon.setPreferredWeapon( VALVE_WEAPON_TRIPMINE );
		
	pb->combat.weapon.attack(pb->actualNavpoint->pos(), 0.4f, NULL);
	
	lastCall[b] = worldtime();
	pb->setGoalMoveDescr( "LayTripmine" );
}


float weightLayTripmine( CParabot *pb, PB_Percept*item )
{
	Vec3D dir;

	if (!pb->combat.weapon.available( VALVE_WEAPON_TRIPMINE )) return 0;

	if ( pb->actualNavpoint && !pb->senses.underFire() ) {
		float nextMine = 10000;
		EDICT *mine = pb->senses.getNearestTripmine();
		if (mine) {
			vsub(&mine->v.origin, pb->actualNavpoint->pos(), &dir);
			nextMine = vlen(&dir);
		}
		if ( (pb->actualNavpoint->type()==NAV_S_USE_TRIPMINE) && (nextMine > 50) )
			return 5;
	}
	return 0;
}



void goalCamp( CParabot *pb, PB_Percept *item )
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };

	int b = pb->slot;
	assert( pb->actualNavpoint != 0 );

	if ((lastCall[b] + 0.5f) < worldtime()) {	// in first call init viewangle
		pb->campTime = 0;
		int angleX = pb->actualNavpoint->special();
		int angleY = pb->actualNavpoint->special();
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
	goalLookAround( pb, item );
	lookAroundAngle = DEFAULT_LOOKAROUND_ANGLE;
	pb->setGoalMoveDescr( "Camp" );
}


float weightCamp( CParabot *pb, PB_Percept*item )
{
	float weight = 0;

	if (pb->actualNavpoint && !pb->senses.underFire()) {
		if (pb->actualNavpoint->offersCamping() &&
		    pb->combat.weapon.bestWeaponUsable())
			weight = pb->needs.wishForSniping();
	}
	return weight;
}



void goalUseTank( CParabot *pb, PB_Percept*item )
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
		vadd(pb->actualNavpoint->pos(), &pb->ent->v.view_ofs, &lookPos);
		action_add(&pb->action, BOT_USE, &lookPos );
		DEBUG_MSG( "Trying to use tank\n" );
	} else {
		pb->campTime += (worldtime() - lastCall[b]);
	}
	pb->lastCamp = worldtime();
	lastCall[b] = worldtime();

	if (!pb->ent->v.viewmodel) {		// use tank!
		if (item) {
			if ( item->isVisible() ) {
				pb->combat.shootAtEnemy(&item->lastPos, 0.1f);
				pb->setGoalMoveDescr( "UseTank (Shoot)" );
			} else {
				action_setaimdir(&pb->action, item->predictedAppearance(pb->botPos()), NULL);
				pb->setGoalMoveDescr( "UseTank (Tracking)" );
			}
		} else {
			lookAroundAngle = 45;
			goalLookAround( pb, item );
			lookAroundAngle = DEFAULT_LOOKAROUND_ANGLE;
			pb->setGoalMoveDescr( "UseTank (LookAround)" );
		}
	} else {
		pb->setGoalMoveDescr( "UseTank (StartUse)" );
	}
}


float weightUseTank( CParabot *pb, PB_Percept*item )
{
	float weight = 0;

	if ( pb->actualNavpoint && !pb->senses.underFire() ) {
		if ( pb->actualNavpoint->type() == NAV_F_TANKCONTROLS ) {
			weight = pb->needs.wishForSniping( false );
			if (item && item->isVisible() && item->distance > 300) weight += 10;
		}
	}
	return weight;
}



void goalWaitAtNavpoint( CParabot *pb, PB_Percept*item )
{
	// don't do anything here...
	pb->setGoalMoveDescr( "WaitAtNavpoint" );
	DEBUG_MSG( "Waiting for halo!\n" );
}


float weightWaitForHalo( CParabot *pb, PB_Percept*item )
{
	float weight = 0;
	if ( pb->actualNavpoint ) {
		if ( pb->actualNavpoint->type()==NAV_HW_HALOBASE && haloOnBase ) 
			weight = pb->needs.needForItems() + 0.1;	// just a bit more than running to it...
	}
	return weight;
}



void goalPause( CParabot *pb, PB_Percept*item )
{
	action_setspeed(&pb->action, 0);
	pb->setGoalMoveDescr( "Pause" );
	//pb->lastPause = worldtime();
}


float weightPause( CParabot *pb, PB_Percept*item )
{
	float weight = 0;//exp( pb->pauseFactor * (worldtime() - pb->lastPause) );
	if (weight > 4) weight = 4;
	return weight;
}



void goalFollow( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	// DEBUG_MSG( "FollowAndAssistLeader\n" );
	if (pb->partner==0) DEBUG_MSG( "WARNING: partner == 0!\n" );
	if ( (pb->partner == 0 ) || (pb->partner != item->entity ) ) {
		int partnerId = observer.playerId( item->entity );
		if (partnerId >= 0) {
			observer.reportPartner( pb->slot, partnerId );
			pb->partner = item->entity;
			pb->actualPath = 0;
		}
	}
	
	if ( !observer.partnerValid( pb->slot ) ) {
		pb->partner = 0;
		return;
	}
	
	if ( observer.shouldFollow( pb->slot, pb->ent ) ) {
		PB_Path_Waypoint wp = observer.getNextWaypoint( pb->slot );
		if (wp.reached( pb->ent )) {
			// DEBUG_MSG( "WP reached, act=%i\n", wp.action() );
			action_add(&pb->action, wp.action(), wp.pos() );	// if there's something to do...
			observer.reportWaypointReached( pb->slot );		// confirm waypoint
		}
		
		action_setviewdir(&pb->action, &pb->partner->v.origin, 0);		// set viewAngle
		action_setmovedir(&pb->action, wp.pos(pb->ent), 0);				// set moveAngle and speed
		action_setmaxspeed(&pb->action);
		pb->pathCheckWay();	
		if (observer.canNotFollow( pb->slot )) {
			DEBUG_MSG( "Cannot follow\n" );
			pb->partner = 0;
		}
	}
/*	if ( observer.partnerInCombat( pb->slot ) ) {	// partner involved in combat
		DEBUG_MSG( "I assist!\n" );
		//botState = PB_IN_COMBAT;
	}*/
	pb->setGoalMoveDescr( "Follow" );
}


float weightFollowLeader( CParabot *pb, PB_Percept*item )
{
	Vec3D dir;

	if (pb->partner) {
		if (pb->partner == item->entity ) return 4;	// keep on following!
	}

	if (item->isVisible()	// leader visible
	    && (item->lastSeenPos.z < (pb->botPos()->z + 16))) {	// not higher than bot
		vsub(&item->lastSeenPos, pb->botPos(), &dir);
		if(500.0f > vlen(&dir))	// not too far away
			return 2;
	}
	return 0;
}

float weightFollowEnemy( CParabot *pb, PB_Percept*item )
{
	return 0;
	assert( item != 0 );
	if ( !pb->combat.weapon.bestWeaponUsable() ) return 0;
	if (item->pState & PI_TACTILE) return (5 + item->rating + pb->needs.wishForCombat());
	else if (item->rating < -3) return 0;	// too bad
	else return (item->rating + pb->needs.wishForCombat());
}

void goalMakeRoom( CParabot *pb, PB_Percept*item )
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
		goalGetAway( pb, item );
		DEBUG_MSG( "Making room - getAway\n" );
	} else {
		//pb->action.setMoveDir(newWp);
		pb->pathfinder.checkWay(&newWp[b]);
		DEBUG_MSG( "Making room - newWp \n" );
	}
	pb->makeRoomTime = worldtime() + 0.5f;	// maintain goal for 0.5 sec 
	lastCall[b] = worldtime();
	pb->setGoalMoveDescr("MakeRoom");
}

float weightMakeRoom( CParabot *pb, PB_Percept *item )
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
