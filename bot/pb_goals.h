class CParabot;
class PB_Percept;



void goalDoNothing( CParabot *pb, PB_Percept*item );



//***************************************************************************
//							COMBAT GOALS
//***************************************************************************

// hunt enemy that is trackable but currently not seen
void    goalHuntEnemy( CParabot *pb, PB_Percept*item );
float weightHuntEnemy( CParabot *pb, PB_Percept*item );

// flee taking cover
void    goalFleeEnemy( CParabot *pb, PB_Percept*item );
float weightFleeEnemy( CParabot *pb, PB_Percept*item );

// move to next cover from enemy position
void    goalTakeCover( CParabot *pb, PB_Percept*item );
float weightTakeCover( CParabot *pb, PB_Percept*item );

// combat movements for short range
void    goalCloseCombat( CParabot *pb, PB_Percept*item );
float weightCloseCombat( CParabot *pb, PB_Percept*item );

// combat movements for medium to long range
void    goalRangeAttack( CParabot *pb, PB_Percept*item );
float weightRangeAttack( CParabot *pb, PB_Percept*item );

// prepared fatal attack
void    goalSilentAttack( CParabot *pb, PB_Percept*item );
float weightSilentAttack( CParabot *pb, PB_Percept*item );

// move to ambush position and wait
void    goalPrepareAmbush( CParabot *pb, PB_Percept*item );
float weightPrepareAmbush( CParabot *pb, PB_Percept*item );

// bunny hopping
void    goalBunnyHop( CParabot *pb, PB_Percept*item );
float weightBunnyHop( CParabot *pb, PB_Percept*item );

// shooting
void    goalShootAtEnemy( CParabot *pb, PB_Percept*item );
float weightShootAtEnemy( CParabot *pb, PB_Percept*item );
float weightShootAtSnark( CParabot *pb, PB_Percept*item );

// chose best weapon
void    goalArmBestWeapon( CParabot *pb, PB_Percept*item );
float weightArmBestWeapon( CParabot *pb, PB_Percept*item );



//***************************************************************************
//							COLLECT GOALS
//***************************************************************************

// chose route to collect static items
void    goalCollectItems( CParabot *pb, PB_Percept*item );
float weightCollectItems( CParabot *pb, PB_Percept*item );

// get dynamic items
void    goalGetItem( CParabot *pb, PB_Percept*item );
float weightGetWeaponbox( CParabot *pb, PB_Percept*item );
float weightGetHalo( CParabot *pb, PB_Percept*item );



//***************************************************************************
//							MISC MOVE GOALS
//***************************************************************************

// evade an item
void    goalGetAway( CParabot *pb, PB_Percept*item );
float weightGetAwayEnemy( CParabot *pb, PB_Percept*item );
float weightGetAwayLaserdot( CParabot *pb, PB_Percept*item );
float weightGetAwayExplosive( CParabot *pb, PB_Percept*item );

// wait for an item
void    goalWaitAtNavpoint( CParabot *pb, PB_Percept*item );
float weightWaitForHalo( CParabot *pb, PB_Percept*item );

// pause
void    goalPause( CParabot *pb, PB_Percept*item );
float weightPause( CParabot *pb, PB_Percept*item );



//***************************************************************************
//							LOOK GOALS
//***************************************************************************

// look around to discover enemy
void    goalLookAround( CParabot *pb, PB_Percept*item );
float weightLookAroundLaserdot( CParabot *pb, PB_Percept*item );
float weightLookAroundDamage( CParabot *pb, PB_Percept*item );
float weightLookAroundDangerousSound( CParabot *pb, PB_Percept*item );
float weightLookAroundPlayerSound( CParabot *pb, PB_Percept*item );

// look around to check new area
void    goalLookAtNewArea( CParabot *pb, PB_Percept*item );
float weightLookAtNewArea( CParabot *pb, PB_Percept*item );

// look closer at unidentified player
void    goalReactToUnidentified( CParabot *pb, PB_Percept*item );
float weightReactToUnidentified( CParabot *pb, PB_Percept*item );



//***************************************************************************
//							TACTICAL GOALS
//***************************************************************************

// go sniping
void    goalCamp( CParabot *pb, PB_Percept*item );
float weightCamp( CParabot *pb, PB_Percept*item );

// wait at a tank
void    goalUseTank( CParabot *pb, PB_Percept*item );
float weightUseTank( CParabot *pb, PB_Percept*item );

// lay tripmines
void    goalLayTripmine( CParabot *pb, PB_Percept*item );
float weightLayTripmine( CParabot *pb, PB_Percept*item );

// load health or armor
void    goalLoadHealthOrArmor( CParabot *pb, PB_Percept*item );
float weightLoadHealthOrArmor( CParabot *pb, PB_Percept*item );



//***************************************************************************
//							TEAM GOALS
//***************************************************************************

// make room for a team-mamber to pass
void    goalMakeRoom( CParabot *pb, PB_Percept*item );
float weightMakeRoom( CParabot *pb, PB_Percept*item );

// follow team-member
void    goalFollow( CParabot *pb, PB_Percept*item );
float weightFollowLeader( CParabot *pb, PB_Percept*item );
float weightFollowEnemy( CParabot *pb, PB_Percept*item );

// assist team-member in combat
void    goalAssistFire( CParabot *pb, PB_Percept*item );
float weightAssistFire( CParabot *pb, PB_Percept*item );
