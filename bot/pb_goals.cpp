#include "pb_goals.h"
#include "parabot.h"
#include "pb_observer.h"
#include "pb_mapcells.h"

extern int mod_id;
extern PB_MapGraph mapGraph;
extern PB_MapCells map;
extern PB_Observer observer;
extern int botNr;
extern edict_t *camPlayer;
extern bool haloOnBase;

void debugFile( char *msg );
void startBotCam( edict_t *pEntity );

extern float sineTable[256];

#define DEFAULT_LOOKAROUND_ANGLE 140
float lookAroundAngle = DEFAULT_LOOKAROUND_ANGLE;



void goalDoNothing( CParabot *pb, PB_Percept*item )
{
	//debugMsg( "DoNothing\n" );
}



//---------------------------------------------------------------------------------------
//  LOOK GOALS
//---------------------------------------------------------------------------------------



void goalReactToUnidentified( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	//debugMsg( "ReactToUnidentified\n" );
	if (item->isTrackable()) {
		pb->action.setViewDir( item->predictedAppearance( pb->botPos() ), 1 );
		pb->setGoalViewDescr( "ReactToUnidentified (Trackable)" );
	}
	else {
		goalLookAround( pb, item );
		pb->setGoalViewDescr( "ReactToUnidentified (Unknown Pos)" );
	}
}


float weightReactToUnidentified( CParabot *pb, PB_Percept*item )
{
	float weight = 0;
	float time = worldTime() - item->firstDetection;	
	if ( time < 2 ) weight = 2 - time;		// look 2 seconds at new sounds
	return weight;
}



void goalLookAtNewArea( CParabot *pb, PB_Percept*item )
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };
	static int lastItem[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	static float startLook[32];
	static float startAngle[32];

	int b = pb->slot;
	Vector view = pb->action.getViewAngle();
	if ( (lastCall[b]+0.5) < worldTime() || item->id!=lastItem[b] ) {	// in first call init viewangle
		startLook[b] = worldTime();
		view = UTIL_VecToAngles( item->lastPos ); 
		startAngle[b] = view.y;
	}
	lastCall[b] = worldTime();
	lastItem[b] = item->id;
		
	int cycle = (int) ( (worldTime()-startLook[b]) * 128 );
	cycle &= 255;
	view.y = startAngle[b] + 30*sineTable[cycle];
	pb->action.setViewAngle( view, 1 );
	pb->setGoalViewDescr( "LookAround (NewArea)" );
}


float weightLookAtNewArea( CParabot *pb, PB_Percept*item )
{
	float weight = 0;
	float time = worldTime() - item->firstDetection;	
	if ( time < 2 ) weight = 2 - time;		// look 2 seconds at new areas
	return weight;
}



void goalLookAround( CParabot *pb, PB_Percept*item )
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };
	static float startLook[32];
	static float startAngle[32];

	int b = pb->slot;
	//debugMsg( "LookAround\n" );
	Vector view = pb->action.getViewAngle();
	if ( (lastCall[b]+0.5) < worldTime() ) {
		startLook[b] = worldTime();
		startAngle[b]= view.y;
	}
	lastCall[b] = worldTime();
	int cycle = (int) ( (worldTime()-startLook[b]) * 128 );
	cycle &= 255;
	view.y = startAngle[b] + lookAroundAngle*sineTable[cycle];	// 60*...
	pb->action.setViewAngle( view, 1 );
	if (item) {
		switch( item->pClass ) {
		case PI_LASERDOT:	pb->setGoalViewDescr( "LookAround (Laserdot)" );		break;
		case PI_DAMAGE:		pb->setGoalViewDescr( "LookAround (Damage)" );			break;
		case PI_EXPLOSIVE:	pb->setGoalViewDescr( "LookAround (ExplosiveSound)" );	break;
		case PI_FOE:		pb->setGoalViewDescr( "LookAround (PlayerSound)" );		break;
		case PI_NEWAREA:	pb->setGoalViewDescr( "LookAround (NewArea)" );			break;
		default:			pb->setGoalViewDescr( "LookAround (???)" );				break;
		}
	}
	else pb->setGoalViewDescr( "LookAround" );
}


float weightLookAroundLaserdot( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	float dist = (pb->botPos() - item->entity->v.origin).Length();
	float weight = (2000-dist) / 1000;
	if (weight<0.5) weight = 0.5;
	return weight;
}


float weightLookAroundDamage( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	if ( !(item->inflictorKnown()) ) return 4;
	else return 0;
}


float weightLookAroundDangerousSound( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	if ( !(item->isTrackable()) ) return 3;
	else return 0;
}


float weightLookAroundPlayerSound( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	if ( !(item->isTrackable()) ) return weightReactToUnidentified( pb, item );
	else return 0;
}




void goalAssistFire( CParabot *pb, PB_Percept*item )
{
	TraceResult tr;
	UTIL_MakeVectors( item->entity->v.v_angle );
	Vector start = item->entity->v.origin + item->entity->v.view_ofs;
	Vector target = start + gpGlobals->v_forward * 4096;
	UTIL_TraceLine( start, target, dont_ignore_monsters, ENT(item->entity), &tr);	
	target = tr.vecEndPos;
	// TODO: search for entity in this area and fire
	item->flags |= PI_FOCUS1;
	pb->setGoalViewDescr( "AssistFire" );
}


float weightAssistFire( CParabot *pb, PB_Percept*item )
{
	assert( item->entity != 0 );

	if ( !pb->combat.weapon.bestWeaponUsable() ) return 0;
	if (item->entity->v.button & IN_ATTACK) return 2;
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
			 (pb->needs.desireFor( NAV_S_USE_TRIPMINE ) > 0) ) wish /= 2;
	}
	//debugMsgF( "Items = %.1f\n", wish );
	return wish;
}




void goalGetItem( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	item->update = worldTime();

	pb->pathfinder.checkWay( item->lastSeenPos );
	if (pb->action.gotStuck()) item->flags |= PI_UNREACHABLE;
	switch( item->pClass ) {
	case PI_WEAPONBOX:	pb->setGoalMoveDescr( "GetItem (Weaponbox)" );		break;
	case PI_HALO:		pb->setGoalMoveDescr( "GetItem (Halo)" );			break;
	default:			pb->setGoalMoveDescr( "GetItem (???)" );			break;
	}
}


float weightGetWeaponbox( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	assert( item->distance > 0 );

	if (!item->isReachable()) return 0;
	float weight = 300*pb->needs.needForWeapons() / (400+item->distance);
	//if (weight > 9) weight = 9;
	item->update = worldTime() + (item->distance / 1000);
	//debugMsgF( "wbw=%.2f\n", weight );
	return weight;
}


float weightGetHalo( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	assert( item->distance > 0 );

	if (!item->isReachable()) return 0;
	float weight = 4000 / item->distance;
	if (weight > 15) weight = 15;
	item->update = worldTime() + (item->distance / 2000);
	
	return weight;
	
}


void goalGetAway( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	item->update = worldTime();

	Vector tDir = pb->botPos() - (item->predictedPosition( pb->botPos() ) - pb->botPos());
	pb->pathfinder.checkWay( tDir );
	switch( item->pClass ) {
	case PI_LASERDOT:	pb->setGoalMoveDescr( "GetAway (Laserdot)" );		break;
	case PI_EXPLOSIVE:	pb->setGoalMoveDescr( "GetAway (Explosive)" );		break;
	case PI_FOE:		pb->setGoalMoveDescr( "GetAway (Enemy)" );			break;
	default:			pb->setGoalMoveDescr( "GetAway (???)" );			break;
	}
}


float weightGetAwayLaserdot( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	float dist = (pb->botPos() - item->entity->v.origin).Length();
	float weight = (500-dist) / 40;
	if (weight<0) weight = 0;
	return weight;
}


float weightGetAwayExplosive( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	float dist = (pb->botPos() - item->entity->v.origin).Length();
	float weight = (600-dist) / 20;
	if (weight<0) weight = 0;
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
	//debugMsg( "LoadHealthOrArmor\n" );
	pb->action.setSpeed( 0 );
	pb->action.setViewDir( pb->actualNavpoint->pos(), 2 );
	pb->action.add( BOT_USE );
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
			debugMsg( "No Loading - enemy around!\n" );
		}
	}
	//debugMsgF( "Health/Armor = %f\n", weight );
	return weight;
}



void goalLayTripmine( CParabot *pb, PB_Percept*item )
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };

	int b = pb->slot;
	if ( (lastCall[b]+0.5) < worldTime() ) {	// in first call arm tripmine
		pb->combat.weapon.setPreferredWeapon( VALVE_WEAPON_TRIPMINE );
		pb->combat.weapon.armBestWeapon( 50, 0.4, 0);
	}

	assert( pb->actualNavpoint != 0 );

	// duck if necessary
	if ( pb->actualNavpoint->pos().z < (pb->ent->v.absmin.z+40) ) 
		pb->action.add( BOT_DUCK );
	// keep tripmine
	pb->combat.weapon.setPreferredWeapon( VALVE_WEAPON_TRIPMINE );
		
	pb->combat.weapon.attack( pb->actualNavpoint->pos(), 0.4 );
	
	lastCall[b] = worldTime();
	pb->setGoalMoveDescr( "LayTripmine" );
}


float weightLayTripmine( CParabot *pb, PB_Percept*item )
{
	if (!pb->combat.weapon.available( VALVE_WEAPON_TRIPMINE )) return 0;

	if ( pb->actualNavpoint && !pb->senses.underFire() ) {
		float nextMine = 10000;
		edict_t *mine = pb->senses.getNearestTripmine();
		if (mine) nextMine = (mine->v.origin - pb->actualNavpoint->pos()).Length();
		if ( (pb->actualNavpoint->type()==NAV_S_USE_TRIPMINE) && (nextMine > 50) )
			return 5;
	}
	return 0;
}



void goalCamp( CParabot *pb, PB_Percept*item )
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };

	int b = pb->slot;
	assert( pb->actualNavpoint != 0 );

	if ( (lastCall[b]+0.5) < worldTime() ) {	// in first call init viewangle
		pb->campTime = 0;
		int angleX = pb->actualNavpoint->special();
		int angleY = pb->actualNavpoint->special();
		angleX &= 0xFFFF;
		angleY >>= 16;
		Vector campAngle;
		campAngle.x = (float) (angleX - 360);
		campAngle.y = (float) (angleY - 360);
		campAngle.z = 0;
		pb->action.setViewAngle( campAngle, 1 );
	}
	else {
		pb->campTime += (worldTime() - lastCall[b]);
	}
	pb->lastCamp = worldTime();
	lastCall[b] = worldTime();
		
	pb->action.setSpeed( 0 );
	lookAroundAngle = 45;
	goalLookAround( pb, item );
	lookAroundAngle = DEFAULT_LOOKAROUND_ANGLE;
	pb->setGoalMoveDescr( "Camp" );
}


float weightCamp( CParabot *pb, PB_Percept*item )
{
	float weight = 0;

	if ( pb->actualNavpoint && !pb->senses.underFire() ) {
		if ( pb->actualNavpoint->offersCamping() &&
			 pb->combat.weapon.bestWeaponUsable()				) weight = pb->needs.wishForSniping();
	}
	return weight;
}



void goalUseTank( CParabot *pb, PB_Percept*item )
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };

	//botNr = pb->slot;

	int b = pb->slot;
	assert( pb->actualNavpoint != 0 );

	pb->action.setSpeed( 0 );

	if ( (lastCall[b]+0.5) < worldTime() ||
		 (pb->campTime>0 && pb->ent->v.viewmodel) ) {	// in first call press button
		pb->campTime = -2;
		Vector lookPos = pb->actualNavpoint->pos() + pb->ent->v.view_ofs;
		pb->action.add( BOT_USE, &lookPos );
		debugMsg( "Trying to use tank\n" );
	}
	else {
		pb->campTime += (worldTime() - lastCall[b]);
	}
	pb->lastCamp = worldTime();
	lastCall[b] = worldTime();

	if (!pb->ent->v.viewmodel) {		// use tank!
		if (item) {
			if ( item->isVisible() ) {
				pb->combat.shootAtEnemy( item->lastPos, 0.1 );
				pb->setGoalMoveDescr( "UseTank (Shoot)" );
			}
			else {
				pb->action.setAimDir( item->predictedAppearance( pb->botPos() ) );
				pb->setGoalMoveDescr( "UseTank (Tracking)" );
			}
		}
		else {
			lookAroundAngle = 45;
			goalLookAround( pb, item );
			lookAroundAngle = DEFAULT_LOOKAROUND_ANGLE;
			pb->setGoalMoveDescr( "UseTank (LookAround)" );
		}
	}
	else {
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
	debugMsg( "Waiting for halo!\n" );
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
	pb->action.setSpeed( 0 );
	pb->setGoalMoveDescr( "Pause" );
	//pb->lastPause = worldTime();
}


float weightPause( CParabot *pb, PB_Percept*item )
{
	float weight = 0;//exp( pb->pauseFactor * (worldTime() - pb->lastPause) );
	if (weight > 4) weight = 4;
	return weight;
}



void goalFollow( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	//debugMsg( "FollowAndAssistLeader\n" );
	if (pb->partner==0) debugMsg( "WARNING: partner == 0!\n" );
	if ( (pb->partner == 0 ) || (pb->partner->edict() != item->entity ) ) {
		int partnerId = observer.playerId( item->entity );
		if (partnerId >= 0) {
			observer.reportPartner( pb->slot, partnerId );
			pb->partner = (CBaseEntity*) GET_PRIVATE( item->entity );
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
			//debugMsg( "WP reached, act=%i\n", wp.action() );
			pb->action.add( wp.action(), &(wp.pos()) );	// if there's something to do...
			observer.reportWaypointReached( pb->slot );		// confirm waypoint
		}
		
		pb->action.setViewDir( pb->partner->pev->origin );		// set viewAngle
		pb->action.setMoveDir( wp.pos(pb->ent) );				// set moveAngle and speed
		pb->action.setMaxSpeed();
		pb->pathCheckWay();	
		if (observer.canNotFollow( pb->slot )) {
			debugMsg( "Cannot follow\n" );
			pb->partner = 0;
		}
	}
/*	if ( observer.partnerInCombat( pb->slot ) ) {	// partner involved in combat
		debugMsg( "I assist!\n" );
		//botState = PB_IN_COMBAT;
	}*/
	pb->setGoalMoveDescr( "Follow" );
}


float weightFollowLeader( CParabot *pb, PB_Percept*item )
{
	if (pb->partner) {
		if (pb->partner->edict() == item->entity ) return 4;	// keep on following!
	}

	if ( (item->isVisible()) &&						// leader visible
		 (item->lastSeenPos.z < (pb->botPos().z+16)) &&			// not higher than bot
		 ((item->lastSeenPos-pb->botPos()).Length() < 500) )	// not too far away
		return 2;
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


Vector UTIL_GetRight( const Vector &vec );

void goalMakeRoom( CParabot *pb, PB_Percept*item )
{
	static float lastCall[32] = { -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10 };
	static Vector newWp[32];

	int b = pb->slot;

	if ( (lastCall[b]+0.5) < worldTime() ) {	// in first call init newWp
		Vector forward;
		if (pb->actualPath) forward = (pb->waypoint.pos( pb->ent ) - pb->botPos());
		else forward = pb->action.getMoveDir();
		forward.Normalize();
		Vector right = UTIL_GetRight( forward );
		newWp[b] = pb->botPos()+ 50*(forward + right);
	}
	if (!(pb->actualPath)) {		// if not on path: no problem
		goalGetAway( pb, item );
		debugMsg( "Making room - getAway\n" );
	}
	else {
		//pb->action.setMoveDir( newWp );
		pb->pathfinder.checkWay( newWp[b] );
		debugMsg( "Making room - newWp \n" );
	}
	pb->makeRoomTime = worldTime() + 0.5;	// maintain goal for 0.5 sec 
	lastCall[b] = worldTime();
	pb->setGoalMoveDescr( "MakeRoom" );
}


float weightMakeRoom( CParabot *pb, PB_Percept*item )
{
	assert( item != 0 );
	float dist = (pb->botPos() - item->entity->v.origin).Length();
	if (dist > 100) return 0;
	float newDist = ( (pb->botPos() + pb->ent->v.velocity/20) - 
		              (item->entity->v.origin + item->entity->v.velocity) ).Length();
	if (newDist>=dist && (worldTime()>pb->makeRoomTime)) return 0;
	float weight = 10 + (100-newDist)/5;
	if (weight>20) weight= 20;
	return weight;
}



//---------------------------------------------------------------------------------------
//  ACTION GOALS
//---------------------------------------------------------------------------------------


