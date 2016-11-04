//
// HPB bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// bot.cpp
//

#include "extdll.h"
#include "parabot.h"
#include "pb_global.h"

#include "bot.h"
#include "bot_func.h"
#include "bot_weapons.h"
#include "pb_goals.h"
#include "pb_chat.h"
#include "pb_configuration.h"


extern HINSTANCE h_Library;
extern int mod_id;
extern float roundStartTime;
extern PB_Configuration pbConfig;
extern PB_Chat chat;
//extern int maxPers;
//extern PB_Personality personality[MAX_PERS];	// stores different bot personalities
//extern bool personalityUsed[MAX_PERS];			// true if bot exists using this personality


bot_t bots[32];   // max of 32 bots in a game

int   need_init = 1;

static FILE *fp;


void adjustAimSkills();


inline edict_t *CREATE_FAKE_CLIENT( const char *netname )
{
   return (*g_engfuncs.pfnCreateFakeClient)( netname );
}

inline char *GET_INFOBUFFER( edict_t *e )
{
   return (*g_engfuncs.pfnGetInfoKeyBuffer)( e );
}

inline char *GET_INFO_KEY_VALUE( char *infobuffer, char *key )
{
   return (g_engfuncs.pfnInfoKeyValue( infobuffer, key ));
}

inline void SET_CLIENT_KEY_VALUE( int clientIndex, char *infobuffer,
                                  char *key, char *value )
{
   (*g_engfuncs.pfnSetClientKeyValue)( clientIndex, infobuffer, key, value );
}


// this is the LINK_ENTITY_TO_CLASS function that creates a player (bot)
void player( entvars_t *pev )
{
	static LINK_ENTITY_FUNC otherClassName = NULL;
	if (otherClassName == NULL)
		otherClassName = (LINK_ENTITY_FUNC)GetProcAddress(h_Library, "player");
	if (otherClassName != NULL) {
		(*otherClassName)( pev );
	}
	else {
		errorMsg( "Can't get player() function from MOD!" );
		printf("Parabot - Can't get player() function from MOD!\n" );
		Sleep(5000);
		exit(0);
	}
}


// init variables for spawning here!
void BotSpawnInit( bot_t *pBot )
{
	assert( pBot != 0 );
   pBot->v_prev_origin = Vector(9999.0, 9999.0, 9999.0);

   pBot->msecnum = 0;
   pBot->msecdel = 0.0;
   pBot->msecval = 0.0;

   pBot->bot_health = 0;
   pBot->bot_armor = 0;
   pBot->bot_weapons = 0;
   pBot->bot_money = 0;

   pBot->f_max_speed = CVAR_GET_FLOAT("sv_maxspeed");

   pBot->prev_speed = 0.0;  // fake "paused" since bot is NOT stuck

   //pBot->pBotEnemy = NULL;  // no enemy yet

   memset(&(pBot->current_weapon), 0, sizeof(pBot->current_weapon));
   memset(&(pBot->m_rgAmmo), 0, sizeof(pBot->m_rgAmmo));
   debugMsg( "Clearing ammo\n" );

   pBot->need_to_initialize = FALSE;
   
}


//void BotCreate( edict_t *pPlayer, const char *botTeam, const char *botClass,
//			   const char *arg3, const char *arg4)
void BotCreate( int fixedPersNr )
// arg1 = team, arg2 = class
{
	edict_t *botEnt;
	bot_t *pBot;
	
	// search free slot for entity
	int numBots = 0;
	int slot = 32;
	for (int i=31; i>=0; i--) {
		if (bots[i].is_used) numBots++;
		else slot = i;
	}
	
	if (slot == 32) {
		infoMsg( "32 bots in game - can't create another!\n" );
		return;
	}

	int persNr;
	if (fixedPersNr >= 0) {
		persNr = fixedPersNr;
	}
	else {
		// search for personality
		int maxPers = pbConfig.numberOfPersonalities();
		int count = 0;
		do {
			count++;
			if (count>500) {
				persNr = 0;
			}
			persNr = RANDOM_LONG( 0, maxPers-1 );
		} while (pbConfig.personality( persNr ).inUse && (numBots<maxPers) && (count<1000));
		
		if (count>=1000) {
			FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
			fprintf( dfp, "Could not get free character in BotCreate:\n" ); 
			fprintf( dfp, "maxPers = %i, numBots = %i\n\n", maxPers, numBots );
			fclose( dfp );
		}
	}
	
	// try to create entity
	const char *botName = pbConfig.personality( persNr ).name;
	botEnt = CREATE_FAKE_CLIENT( botName );
	
	if (FNullEnt( botEnt )) {	// if NULL entity return
		infoMsg( "Max. Players reached. Can't create bot!\n");
		return;
	}

	char dbgBuffer[256];
	sprintf( dbgBuffer, "%.f: BotCreate() fixedPersNr = %i, persNr = %i, botname = %s\n", worldTime(), fixedPersNr, persNr, botName );
	debugFile( dbgBuffer );
	pbConfig.personalityJoins( persNr, worldTime() );	// now we know the bot can be created

	char ptr[128];  // allocate space for message from ClientConnect
	char *infobuffer;
	int clientIndex;

	// Fix from Lean Hartwig at Topica
    //FREE_PRIVATE( botEnt );
                 
	// create the player entity by calling MOD's player function
	// (from LINK_ENTITY_TO_CLASS for player object)
	player( VARS(botEnt) );

	infobuffer = GET_INFOBUFFER( botEnt );
	clientIndex = ENTINDEX( botEnt );
		
	SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "model", pbConfig.personality( persNr ).model );
	
	if (mod_id == VALVE_DLL || mod_id == DMC_DLL || mod_id == GEARBOX_DLL) {	// set colors
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "topcolor", pbConfig.getColor( persNr, 371 ) );
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "bottomcolor", pbConfig.getColor( persNr, 97 ) );
	}
	else if (mod_id == CSTRIKE_DLL)	{
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "rate", "3500.000000");
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "cl_updaterate", "20");
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "cl_lw", "1");
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "cl_lc", "1");
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "tracker", "0");
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "cl_dlmax", "128");
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "lefthand", "1");
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "friends", "0");
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "dm", "0");
		SET_CLIENT_KEY_VALUE( clientIndex, infobuffer, "ah", "1");
	}
	
	if (!ClientConnect( botEnt, pbConfig.personality( persNr ).name, "127.0.0.1", ptr )) {
		FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
		fprintf( dfp, "BotCreate: ClientConnect() returned false!\n" ); 
		fclose( dfp );
	}
	
	// Pieter van Dijk - use instead of DispatchSpawn() - Hip Hip Hurray!
	ClientPutInServer( botEnt );

	assert( botEnt != 0 );
	botEnt->v.flags |= FL_FAKECLIENT;
	
	// initialize all the variables for this bot...
	
	pBot = &bots[slot];
	pBot->name[0] = 0;  // name not set by server yet
	pBot->pEdict = botEnt;
	pBot->not_started = 1;  // hasn't joined game yet
	
	if (mod_id == TFC_DLL)
		pBot->start_action = MSG_TFC_IDLE;
	else if (mod_id == CSTRIKE_DLL)
		pBot->start_action = MSG_CS_IDLE;
	else
		pBot->start_action = 0;  // not needed for non-team MODs

	// make instance of parabot
	if (pBot->parabot != 0) delete pBot->parabot;
	pBot->parabot = ::new CParabot( botEnt, slot );
	pBot->parabot->action.setAimSkill( pbConfig.personality( persNr ).aimSkill );
	pBot->parabot->setAggression( pbConfig.personality( persNr ).aggression );
	pBot->parabot->senses.setSensitivity( pbConfig.personality( persNr ).sensitivity );
	pBot->parabot->setCommunication( pbConfig.personality( persNr ).communication );
	
	pBot->personality = persNr;

	// add goals:

	pBot->parabot->goalFinder.addGoal( G_ACTION,
		GOAL_UNCONDITIONAL, goalArmBestWeapon, weightArmBestWeapon );
	pBot->parabot->goalFinder.addGoal( G_MOVE,
		GOAL_UNCONDITIONAL, goalCollectItems, weightCollectItems );
	pBot->parabot->goalFinder.addGoal( G_MOVE,
		GOAL_UNCONDITIONAL, goalCamp, weightCamp );
	pBot->parabot->goalFinder.addGoal( G_MOVE,
		GOAL_UNCONDITIONAL, goalUseTank, weightUseTank );
	pBot->parabot->goalFinder.addGoal( G_MOVE | G_VIEW,
		GOAL_UNCONDITIONAL, goalLoadHealthOrArmor, weightLoadHealthOrArmor );
	//pBot->parabot->goalFinder.addGoal( G_MOVE,
	//	GOAL_UNCONDITIONAL, goalPause, weightPause );
	if (mod_id == VALVE_DLL || mod_id == GEARBOX_DLL) {
		pBot->parabot->goalFinder.addGoal( G_MOVE | G_VIEW,
			GOAL_UNCONDITIONAL, goalLayTripmine, weightLayTripmine );
	}
	else if (mod_id == HOLYWARS_DLL) {
		pBot->parabot->goalFinder.addGoal( G_MOVE,
			GOAL_UNCONDITIONAL, goalWaitAtNavpoint, weightWaitForHalo );
	}

	pBot->parabot->goalFinder.addGoal( G_VIEW,
		PI_DAMAGE, goalLookAround, weightLookAroundDamage );
	pBot->parabot->goalFinder.addGoal( G_ACTION,
		PI_DAMAGE, goalBunnyHop, weightBunnyHop );

	pBot->parabot->goalFinder.addGoal( G_VIEW,
		PI_PLAYER, goalReactToUnidentified, weightReactToUnidentified );
	
	pBot->parabot->goalFinder.addGoal( G_VIEW,
		PI_NEWAREA, goalLookAtNewArea, weightLookAtNewArea );

	pBot->parabot->goalFinder.addGoal( G_MOVE,
		PI_FRIEND, goalMakeRoom, weightMakeRoom );

	if ( (mod_id == CSTRIKE_DLL) || (mod_id == TFC_DLL) ) {
		pBot->parabot->goalFinder.addGoal( G_ACTION,
			PI_FRIEND, goalAssistFire, weightAssistFire );
		pBot->parabot->goalFinder.addGoal( G_MOVE,
			PI_FRIEND, goalFollow, weightFollowLeader );
	}

	pBot->parabot->goalFinder.addGoal( G_VIEW,
		PI_FOE, goalLookAround, weightLookAroundPlayerSound );
	pBot->parabot->goalFinder.addGoal( G_ACTION,
		PI_FOE, goalArmBestWeapon, weightArmBestWeapon );
	pBot->parabot->goalFinder.addGoal( G_VIEW,
		PI_FOE, goalShootAtEnemy, weightShootAtEnemy );
	pBot->parabot->goalFinder.addGoal( G_MOVE | G_VIEW,
		PI_FOE, goalCloseCombat, weightCloseCombat );
	pBot->parabot->goalFinder.addGoal( G_MOVE | G_VIEW,
		PI_FOE, goalSilentAttack, weightSilentAttack );
	pBot->parabot->goalFinder.addGoal( G_MOVE | G_VIEW,
		PI_FOE, goalUseTank, weightUseTank );
//	pBot->parabot->goalFinder.addGoal( G_MOVE | G_VIEW,
//		PI_FOE, goalRangeAttack, weightRangeAttack );
//	pBot->parabot->goalFinder.addGoal( G_MOVE | G_VIEW,
//		PI_FOE, goalPrepareAmbush, weightPrepareAmbush );
	pBot->parabot->goalFinder.addGoal( G_MOVE,
		PI_FOE, goalHuntEnemy, weightHuntEnemy );
	pBot->parabot->goalFinder.addGoal( G_MOVE,
		PI_FOE, goalFleeEnemy, weightFleeEnemy );
	pBot->parabot->goalFinder.addGoal( G_MOVE,
		PI_FOE, goalTakeCover, weightTakeCover );

	
	if (mod_id == VALVE_DLL || mod_id == GEARBOX_DLL) {
		pBot->parabot->goalFinder.addGoal( G_VIEW,
			PI_LASERDOT, goalLookAround, weightLookAroundLaserdot );
		pBot->parabot->goalFinder.addGoal( G_MOVE,
			PI_LASERDOT, goalGetAway, weightGetAwayLaserdot );
		pBot->parabot->goalFinder.addGoal( G_VIEW,
			PI_SNARK, goalShootAtEnemy, weightShootAtSnark );
		pBot->parabot->goalFinder.addGoal( G_ACTION,
			PI_SNARK, goalArmBestWeapon, weightArmBestWeapon );
	}

	pBot->parabot->goalFinder.addGoal( G_MOVE,
		PI_WEAPONBOX, goalGetItem, weightGetWeaponbox );
	
	pBot->parabot->goalFinder.addGoal( G_VIEW,
		PI_EXPLOSIVE, goalLookAround, weightLookAroundDangerousSound );
	pBot->parabot->goalFinder.addGoal( G_MOVE,
		PI_EXPLOSIVE, goalGetAway, weightGetAwayExplosive );
	
	if (mod_id == HOLYWARS_DLL) {
		pBot->parabot->goalFinder.addGoal( G_MOVE,
			PI_HALO, goalGetItem, weightGetHalo );
	}

	debugMsg( "BOT CREATE.\n" );
	
	BotSpawnInit(pBot);	// init variables
	
	pBot->is_used = TRUE;
	pBot->respawn_state = RESPAWN_IDLE;

	botEnt->v.idealpitch = botEnt->v.v_angle.x;
	botEnt->v.ideal_yaw = botEnt->v.v_angle.y;
	botEnt->v.pitch_speed = BOT_PITCH_SPEED;
	botEnt->v.yaw_speed = BOT_YAW_SPEED;
	
	pBot->bot_team = -1;  // don't know what these are yet, server can change them
	pBot->bot_class = -1;
	
	pBot->bot_team = 2;

	adjustAimSkills();	// take care of min-/maxskill
}


// Checks bot->start_action:
// if ==TEAM_SELECT selects team bot->bot_team
// if ==CLASS_SELECT selects class bot->bot_class and sets bot->not_started = 0
void BotStartGame( bot_t *pBot )
{
   char c_team[32];
   char c_class[32];

	assert( pBot != 0 );
   edict_t *pEdict = pBot->pEdict;

   chat.registerJoin( pEdict );

   if (mod_id == TFC_DLL)
   {
      // handle Team Fortress Classic stuff here...

      if (pBot->start_action == MSG_TFC_TEAM_SELECT)
      {
		  if (pBot->menuSelectTime > worldTime()) return;

         pBot->start_action = MSG_TFC_IDLE;  // switch back to idle

         if ((pBot->bot_team != 1) && (pBot->bot_team != 2) && (pBot->bot_team != 5))
            pBot->bot_team = -1;

         if (pBot->bot_team == -1) pBot->bot_team = RANDOM_LONG(1, 2);

         // select the team the bot wishes to join...
         if (pBot->bot_team == 1)      strcpy(c_team, "1");
         else if (pBot->bot_team == 2) strcpy(c_team, "2");
         else						   strcpy(c_team, "5");

         FakeClientCommand(pEdict, "jointeam", c_team, NULL);
         return;
      }

      if (pBot->start_action == MSG_TFC_CLASS_SELECT)
      {
		  if (pBot->menuSelectTime > worldTime()) return;
         
		  pBot->start_action = MSG_TFC_IDLE;  // switch back to idle
		/* if (pBot->not_started == 0) {
			 debugMsg( "BOT ALREADY STARTED IN TEAMSELECT\n" );
			 return;
		 }*/
		 
         if ((pBot->bot_class < 0) || (pBot->bot_class > 10))
            pBot->bot_class = -1;

         if (pBot->bot_class == -1) pBot->bot_class = RANDOM_LONG(1, 10);
		 pBot->bot_class = 3;

         // select the class the bot wishes to use...
         if (pBot->bot_class == 0)      strcpy(c_class, "civilian");
         else if (pBot->bot_class == 1) strcpy(c_class, "scout");
         else if (pBot->bot_class == 2) strcpy(c_class, "sniper");
         else if (pBot->bot_class == 3) strcpy(c_class, "soldier");
         else if (pBot->bot_class == 4) strcpy(c_class, "demoman");
         else if (pBot->bot_class == 5) strcpy(c_class, "medic");
         else if (pBot->bot_class == 6) strcpy(c_class, "hwguy");
         else if (pBot->bot_class == 7) strcpy(c_class, "pyro");
         else if (pBot->bot_class == 8) strcpy(c_class, "spy");
         else if (pBot->bot_class == 9) strcpy(c_class, "engineer");
         else							strcpy(c_class, "randompc");

		 debugMsg( "Choosen Class %i: ", pBot->bot_class );
		 debugMsg( c_class );  debugMsg( "\n" );
		 FakeClientCommand(pEdict, c_class, NULL, NULL);
		 debugMsg( "Started!\n" );
		 pBot->not_started = 0;	// bot has now joined the game (doesn't need to be started)
		 return;
      }
   }

   else if (mod_id == CSTRIKE_DLL)
   {
      // handle Counter-Strike stuff here...

      if (pBot->start_action == MSG_CS_TEAM_SELECT)
      {
         pBot->start_action = MSG_CS_IDLE;  // switch back to idle

         if ((pBot->bot_team != 1) && (pBot->bot_team != 2) && (pBot->bot_team != 5))
            pBot->bot_team = -1;

         if (pBot->bot_team == -1) pBot->bot_team = RANDOM_LONG(1, 2);

         // select the team the bot wishes to join...
         if (pBot->bot_team == 1)      strcpy(c_team, "1");
         else if (pBot->bot_team == 2) strcpy(c_team, "2");
         else						   strcpy(c_team, "5");

         FakeClientCommand(pEdict, "menuselect", c_team, NULL);
         return;
      }

      if (pBot->start_action == MSG_CS_CT_SELECT)  // counter terrorist
      {
         pBot->start_action = MSG_CS_IDLE;  // switch back to idle

         if ((pBot->bot_class < 1) || (pBot->bot_class > 4)) 
			 pBot->bot_class = -1;

         if (pBot->bot_class == -1) pBot->bot_class = RANDOM_LONG(1, 4);

         // select the class the bot wishes to use...
         if (pBot->bot_class == 1)      strcpy(c_class, "1");
         else if (pBot->bot_class == 2) strcpy(c_class, "2");
         else if (pBot->bot_class == 3) strcpy(c_class, "3");
         else if (pBot->bot_class == 4) strcpy(c_class, "4");
         else							strcpy(c_class, "5");  // random

         FakeClientCommand(pEdict, "menuselect", c_class, NULL);
         pBot->not_started = 0;	// bot has now joined the game (doesn't need to be started)
         return;
      }

      if (pBot->start_action == MSG_CS_T_SELECT)  // terrorist select
      {
         pBot->start_action = MSG_CS_IDLE;  // switch back to idle

         if ((pBot->bot_class < 1) || (pBot->bot_class > 4))
            pBot->bot_class = -1;  // use random if invalid

         if (pBot->bot_class == -1) pBot->bot_class = RANDOM_LONG(1, 4);

         // select the class the bot wishes to use...
         if (pBot->bot_class == 1)      strcpy(c_class, "1");
         else if (pBot->bot_class == 2) strcpy(c_class, "2");
         else if (pBot->bot_class == 3) strcpy(c_class, "3");
         else if (pBot->bot_class == 4) strcpy(c_class, "4");
         else							strcpy(c_class, "5");  // random

         FakeClientCommand(pEdict, "menuselect", c_class, NULL);
         pBot->not_started = 0;	// bot has now joined the game (doesn't need to be started)
         return;
      }
   }
   else if (mod_id == DMC_DLL)
   {
	   FakeClientCommand(pEdict, "_firstspawn", NULL, NULL);
	   pBot->not_started = 0;
   }
   else
   {
      // otherwise, don't need to do anything to start game...
      pBot->not_started = 0;
   }
}


int BotInFieldOfView(bot_t *pBot, Vector dest)
{
	assert( pBot != 0 );
   // find angles from source to destination...
   Vector entity_angles = UTIL_VecToAngles( dest );

   // make yaw angle 0 to 360 degrees if negative...
   if (entity_angles.y < 0)
      entity_angles.y += 360;

   // get bot's current view angle...
   float view_angle = pBot->pEdict->v.v_angle.y;

   // make view angle 0 to 360 degrees if negative...
   if (view_angle < 0)
      view_angle += 360;

   // return the absolute value of angle to destination entity
   // zero degrees means straight ahead,  45 degrees to the left or
   // 45 degrees to the right is the limit of the normal view angle

   // rsm - START angle bug fix
   int angle = abs((int)view_angle - (int)entity_angles.y);

   if (angle > 180)
      angle = 360 - angle;

   return angle;
   // rsm - END
}


bool BotEntityIsVisible( bot_t *pBot, Vector dest )
{
   TraceResult tr;

   assert( pBot != 0 );
   // trace a line from bot's eyes to destination...
   UTIL_TraceLine( pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs,
                   dest, ignore_monsters,
                   pBot->pEdict->v.pContainingEntity, &tr );

   // check if line of sight to object is not blocked (i.e. visible)
   if (tr.flFraction >= 1.0)
      return TRUE;
   else
      return FALSE;
}


void BotFixIdealYaw(edict_t *pEdict)
{
	assert( pEdict != 0 );
   // check for wrap around of angle...
   if (pEdict->v.ideal_yaw > 180)
      pEdict->v.ideal_yaw -= 360;

   if (pEdict->v.ideal_yaw < -180)
      pEdict->v.ideal_yaw += 360;
}


float BotChangeYaw( bot_t *pBot, float speed )
{
	assert( pBot != 0 );
   edict_t *pEdict = pBot->pEdict;
	assert( pEdict != 0 );
   float ideal;
   float current;
   float current_180;  // current +/- 180 degrees
   float diff;

   // turn from the current v_angle yaw to the ideal_yaw by selecting
   // the quickest way to turn to face that direction
   
   current = pEdict->v.v_angle.y;

   ideal = pEdict->v.ideal_yaw;

   // find the difference in the current and ideal angle
   diff = abs(current - ideal);

   // check if the bot is already facing the ideal_yaw direction...
   if (diff <= 1)
      return diff;  // return number of degrees turned

   // check if difference is less than the max degrees per turn
   if (diff < speed)
      speed = diff;  // just need to turn a little bit (less than max)

   // here we have four cases, both angle positive, one positive and
   // the other negative, one negative and the other positive, or
   // both negative.  handle each case separately...

   if ((current >= 0) && (ideal >= 0))  // both positive
   {
      if (current > ideal)
         current -= speed;
      else
         current += speed;
   }
   else if ((current >= 0) && (ideal < 0))
   {
      current_180 = current - 180;

      if (current_180 > ideal)
         current += speed;
      else
         current -= speed;
   }
   else if ((current < 0) && (ideal >= 0))
   {
      current_180 = current + 180;
      if (current_180 > ideal)
         current += speed;
      else
         current -= speed;
   }
   else  // (current < 0) && (ideal < 0)  both negative
   {
      if (current > ideal)
         current -= speed;
      else
         current += speed;
   }

   // check for wrap around of angle...
   if (current > 180)
      current -= 360;
   if (current < -180)
      current += 360;

   pEdict->v.v_angle.y = current;

   return speed;  // return number of degrees turned
}


bool pb_pause = false;

void fixAngle( Vector &angle );


void BotThink( bot_t *pBot )
{
   int index = 0;
   Vector v_diff;             // vector from previous to current location
   TraceResult tr;
   
   assert( pBot != 0 );
   edict_t *pEdict = pBot->pEdict;
   assert( pEdict != 0 );
   
   pEdict->v.flags |= FL_FAKECLIENT;

   if (pBot->name[0] == 0)  // name filled in yet?
      strcpy(pBot->name, STRING(pBot->pEdict->v.netname));


// TheFatal - START from Advanced Bot Framework (Thanks Rich!)

   // adjust the millisecond delay based on the frame rate interval...
   if (pBot->msecdel <= gpGlobals->time)
   {
      pBot->msecdel = gpGlobals->time + 0.5;
      if (pBot->msecnum > 0)
         pBot->msecval = 450.0/pBot->msecnum;
      pBot->msecnum = 0;
   }
   else
      pBot->msecnum++;

   if (pBot->msecval < 5)    // don't allow msec to be less than 5...
      pBot->msecval = 5;

   if (pBot->msecval > 100)  // ...or greater than 100
      pBot->msecval = 100;

// TheFatal - END


   pEdict->v.button = 0;
   pBot->f_move_speed = 0.0;

   // if the bot hasn't selected stuff to start the game yet, go do that...
   if (pBot->not_started) {
      BotStartGame( pBot );
	  fixAngle( pEdict->v.v_angle );
      g_engfuncs.pfnRunPlayerMove( pEdict, pEdict->v.v_angle, 0.0,
                                   0, 0, pEdict->v.button, 0, pBot->msecval);
	  pEdict->v.flags |= FL_FAKECLIENT;
      return;
   }

   // if the bot is dead, randomly press fire to respawn...
   if ((pEdict->v.health < 1) || (pEdict->v.deadflag != DEAD_NO))
   {
	   debugFile( "X" );
	  if (pBot->need_to_initialize)
         BotSpawnInit(pBot);

      if (RANDOM_LONG(1, 100) > 50)
         pEdict->v.button = IN_ATTACK;

	  fixAngle( pEdict->v.v_angle );
      g_engfuncs.pfnRunPlayerMove( pEdict, pEdict->v.v_angle, 0.0,
                                   0, 0, pEdict->v.button, 0, pBot->msecval);
	  pEdict->v.flags |= FL_FAKECLIENT;
	  return;
   }

   // set this for the next time the bot dies so it will initialize stuff
   if (!pBot->need_to_initialize) {
		pBot->need_to_initialize = TRUE;
		pBot->parabot->initAfterRespawn();
   }
  
   if ((worldTime() > roundStartTime) && !pb_pause) {	// don't call this before round has
	   pBot->parabot->botThink();						// started
   }
   else {	// prevent crashes...???
	   g_engfuncs.pfnRunPlayerMove( pEdict, Vector(0,0,0), 0,
		   0, 0, 0, 0, pBot->msecval);
   }

   pEdict->v.flags |= FL_FAKECLIENT;
   return;
}

