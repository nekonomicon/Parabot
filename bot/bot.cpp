//
// HPB bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// bot.cpp
//

#include "parabot.h"
#include "bot.h"
#include "bot_func.h"
#include "bot_weapons.h"
#include "pb_goals.h"
#include "chat.h"
#include "observer.h"
#include "configuration.h"
#include "personalities.h"
#include "sounds.h"
#include "dllwrap.h"

extern int mod_id;
extern float roundStartTime;
//extern int maxPers;
//extern PB_Personality personality[MAX_PERS];	// stores different bot personalities
//extern bool personalityUsed[MAX_PERS];			// true if bot exists using this personality
extern int numberOfClients;
class PB_MapCells;
extern PB_MapGraph mapGraph;	// mapgraph for waypoints
extern PB_MapCells map;
// extern Sounds playerSounds;
float observerUpdate;
bool fatalParabotError = false;
extern bool g_meta_init;

extern char valveTeamList[MAX_TEAMS][32];
extern int valveTeamNumber;
extern bool headToBunker;
extern float airStrikeTime;
int mod_id;			// the MOD in which the bot runs
float roundStartTime;
extern float nextAirstrikeTime;
extern bot_t			bots[32];
extern EDICT			*clients[32];
extern int				welcome_index;			// client to welcome
extern float			bot_check_time;			// for checking if new bots should be created
extern bool				g_GameRules;
//extern int				min_bots;
extern bool				pb_pause;
extern int				numberOfClients;
extern float airStrikeTime;
void saveLevelData();
bool loadLevelData();

extern int debug_engine;
static FILE *fp;

static char		cmd_line[80];
static float	respawn_time = 0.0;
static float	client_update_time = 0.0;
//extern int max_bots;

bot_t bots[32];   // max of 32 bots in a game

void adjustAimSkills();

float welcome_time;
#if _DEBUG
	const char *welcome_msg = "You are playing a debug version of Parabot 0.92.1\n";
#else
	const char *welcome_msg = "Welcome to Parabot 0.92.1\n";
#endif

// this is the LINK_ENTITY_TO_CLASS function that creates a player (bot)
void player( ENTVARS *pev )
{
	static LINK_ENTITY_FUNC otherClassName = NULL;
	if (otherClassName == NULL)
		otherClassName = (LINK_ENTITY_FUNC)GetProcAddress(com.gamedll_handle, "player");
	if (otherClassName != NULL){
		(*otherClassName)( pev );
	} else {
		ERROR_MSG( "Can't get player() function from MOD!" );
		printf("Parabot - Can't get player() function from MOD!\n" );
		Sleep(5000);
		exit(0);
	}
}
// init variables for spawning here!
void BotSpawnInit( bot_t *pBot )
{
	assert( pBot != 0 );
   pBot->v_prev_origin.x = pBot->v_prev_origin.y = pBot->v_prev_origin.z = 9999.0f;

   pBot->prev_speed = 0.0;  // fake "paused" since bot is NOT stuck

   //pBot->pBotEnemy = NULL;  // no enemy yet

   memset(&(pBot->current_weapon), 0, sizeof(pBot->current_weapon));
   memset(&(pBot->m_rgAmmo), 0, sizeof(pBot->m_rgAmmo));
   DEBUG_MSG( "Clearing ammo\n" );

   pBot->need_to_initialize = false;
   
}

//void BotCreate( EDICT *pPlayer, const char *botTeam, const char *botClass,
//			   const char *arg3, const char *arg4)
void BotCreate( int fixedPersNr )
// arg1 = team, arg2 = class
{
	EDICT *botEnt;
	bot_t *pBot;
	
	// search free slot for entity
	int numBots = 0;
	int slot = 32;
	for (int i = 31; i >= 0; i--) {
		if (bots[i].is_used) numBots++;
		else slot = i;
	}
	
	if (slot == 32) {
		INFO_MSG( "32 bots in game - can't create another!\n" );
		return;
	}

	int persNr;
	if (fixedPersNr >= 0) {
		persNr = fixedPersNr;
	} else {
		// search for personality
		int maxPers = personalities_count();
		int count = 0;
		do {
			count++;
			/*if (count>500) {
				persNr = 0;
			}*/
			persNr = randomint(0, maxPers - 1);
		} while (personalities_get( persNr ).inuse && (numBots < maxPers) && (count < 1000));

		/*if (count>=1000) {
			FILE *dfp=fopen( "parabot/crashlog.txt", "a" ); 
			fprintf( dfp, "Could not get free character in BotCreate:\n" ); 
			fprintf( dfp, "maxPers = %i, numBots = %i\n\n", maxPers, numBots );
			fclose( dfp );
		}*/
	}
	
	// try to create entity
	botEnt = createfakeclient(personalities_get(persNr).name);

	if (!playerexists(botEnt)) {	// if NULL entity return
		INFO_MSG("Max. Players reached. Can't create bot!\n");
		return;
	}

	debugFile( "%.f: BotCreate() fixedPersNr = %i, persNr = %i, botname = %s\n", worldtime(), fixedPersNr, persNr, botName );

	personalities_joins(persNr);	// now we know the bot can be created

	char ptr[128];  // allocate space for message from ClientConnect
	const char *infobuffer;
	int clientIndex;

	// Fix from Lean Hartwig at Topica
	if (botEnt->pvPrivateData)
		free_privatedata(botEnt);
	botEnt->pvPrivateData = NULL;
	botEnt->v.frags = 0;

	// create the player entity by calling MOD's player function
	// (from LINK_ENTITY_TO_CLASS for player object)
	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
		player( &botEnt->v );
	else
		CALL_GAME_ENTITY("player", &botEnt->v);

	infobuffer = getinfokeybuffer( botEnt );

	clientIndex = indexofedict( botEnt );

	setclientkeyvalue(clientIndex, infobuffer, "model", personalities_get( persNr ).model);
	
	if (mod_id == VALVE_DLL || mod_id == AG_DLL || mod_id == HUNGER_DLL || mod_id == DMC_DLL || mod_id == GEARBOX_DLL) {	// set colors
		setclientkeyvalue( clientIndex, infobuffer, "topcolor", personalities_gettopcolor( persNr ) );
		setclientkeyvalue( clientIndex, infobuffer, "bottomcolor", personalities_getbottomcolor( persNr )  );
	} else if (mod_id == CSTRIKE_DLL) {
		setclientkeyvalue( clientIndex, infobuffer, "rate", "3500.000000");
		setclientkeyvalue( clientIndex, infobuffer, "cl_updaterate", "20");
		setclientkeyvalue( clientIndex, infobuffer, "cl_lw", "1");
		setclientkeyvalue( clientIndex, infobuffer, "cl_lc", "1");
		setclientkeyvalue( clientIndex, infobuffer, "tracker", "0");
		setclientkeyvalue( clientIndex, infobuffer, "cl_dlmax", "128");
		setclientkeyvalue( clientIndex, infobuffer, "lefthand", "1");
		setclientkeyvalue( clientIndex, infobuffer, "friends", "0");
		setclientkeyvalue( clientIndex, infobuffer, "dm", "0");
		setclientkeyvalue( clientIndex, infobuffer, "ah", "1");
	}

	clientconnect( botEnt, personalities_get( persNr ).name, "127.0.0.1", ptr );

	numberOfClients++;

	// Pieter van Dijk - use instead of DispatchSpawn() - Hip Hip Hurray!
	clientputinserver( botEnt );

	assert( botEnt != 0 );
	botEnt->v.flags |= FL_FAKECLIENT;
	
	// initialize all the variables for this bot...
	
	pBot = &bots[slot];
	pBot->name[0] = 0;  // name not set by server yet
	pBot->e = botEnt;
	pBot->not_started = 1;  // hasn't joined game yet
	
	if (mod_id == TFC_DLL)
		pBot->start_action = MSG_TFC_IDLE;
	else if (mod_id == CSTRIKE_DLL)
		pBot->start_action = MSG_CS_IDLE;
	else if ((mod_id == GEARBOX_DLL) && (com.gamedll_flags & GAMEDLL_CTF))
		pBot->start_action = MSG_OPFOR_IDLE;
	/*else if (mod_id == FRONTLINE_DLL)
		pBot->start_action = MSG_FLF_IDLE;*/
	else
		pBot->start_action = 0;  // not needed for non-team MODs

	// make instance of parabot
	if (pBot->parabot != 0) delete pBot->parabot;
	pBot->parabot = ::new CParabot( botEnt, slot );
	action_setaimskill(&pBot->parabot->action, personalities_get( persNr ).aimskill );
	pBot->parabot->setAggression( personalities_get( persNr ).aggression );
	pBot->parabot->senses.setSensitivity( personalities_get( persNr ).sensitivity );
	pBot->parabot->setCommunication( personalities_get( persNr ).communication );
	
	pBot->personality = persNr;

	// add goals:
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_ACTION, GOAL_UNCONDITIONAL, goalArmBestWeapon, weightArmBestWeapon);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, GOAL_UNCONDITIONAL, goalCollectItems, weightCollectItems);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, GOAL_UNCONDITIONAL, goalCamp, weightCamp);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, GOAL_UNCONDITIONAL, goalUseTank, weightUseTank);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE | G_VIEW, GOAL_UNCONDITIONAL, goalLoadHealthOrArmor, weightLoadHealthOrArmor);
	// goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, GOAL_UNCONDITIONAL, goalPause, weightPause);

	if (mod_id == VALVE_DLL || mod_id == AG_DLL || mod_id == HUNGER_DLL || mod_id == GEARBOX_DLL) {
		goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE | G_VIEW, GOAL_UNCONDITIONAL, goalLayTripmine, weightLayTripmine);
	} else if (mod_id == HOLYWARS_DLL) {
		goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, GOAL_UNCONDITIONAL, goalWaitAtNavpoint, weightWaitForHalo);
	}

	goalfinder_addgoal(&pBot->parabot->goalFinder, G_VIEW, PI_DAMAGE, goalLookAround, weightLookAroundDamage);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_ACTION, PI_DAMAGE, goalBunnyHop, weightBunnyHop);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_VIEW, PI_PLAYER, goalReactToUnidentified, weightReactToUnidentified);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_VIEW, PI_NEWAREA, goalLookAtNewArea, weightLookAtNewArea);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, PI_FRIEND, goalMakeRoom, weightMakeRoom);

	if (com.gamedll_flags & GAMEDLL_TEAMPLAY) {
		goalfinder_addgoal(&pBot->parabot->goalFinder, G_ACTION, PI_FRIEND, goalAssistFire, weightAssistFire);
		goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, PI_FRIEND, goalFollow, weightFollowLeader);
	}

	goalfinder_addgoal(&pBot->parabot->goalFinder, G_VIEW, PI_FOE, goalLookAround, weightLookAroundPlayerSound);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_ACTION, PI_FOE, goalArmBestWeapon, weightArmBestWeapon);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_VIEW, PI_FOE, goalShootAtEnemy, weightShootAtEnemy);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE | G_VIEW, PI_FOE, goalCloseCombat, weightCloseCombat);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE | G_VIEW, PI_FOE, goalSilentAttack, weightSilentAttack);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE | G_VIEW, PI_FOE, goalUseTank, weightUseTank);
//	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE | G_VIEW, PI_FOE, goalRangeAttack, weightRangeAttack);
//	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE | G_VIEW, PI_FOE, goalPrepareAmbush, weightPrepareAmbush);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, PI_FOE, goalHuntEnemy, weightHuntEnemy);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, PI_FOE, goalFleeEnemy, weightFleeEnemy);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, PI_FOE, goalTakeCover, weightTakeCover);

	if (mod_id == VALVE_DLL || mod_id == AG_DLL || mod_id == HUNGER_DLL || mod_id == GEARBOX_DLL) {
		goalfinder_addgoal(&pBot->parabot->goalFinder, G_VIEW, PI_LASERDOT, goalLookAround, weightLookAroundLaserdot);
		goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, PI_LASERDOT, goalGetAway, weightGetAwayLaserdot);
		goalfinder_addgoal(&pBot->parabot->goalFinder, G_VIEW, PI_SNARK, goalShootAtEnemy, weightShootAtSnark);
		goalfinder_addgoal(&pBot->parabot->goalFinder, G_ACTION, PI_SNARK, goalArmBestWeapon, weightArmBestWeapon);
	}

	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, PI_WEAPONBOX, goalGetItem, weightGetWeaponbox);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_VIEW, PI_EXPLOSIVE, goalLookAround, weightLookAroundDangerousSound);
	goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, PI_EXPLOSIVE, goalGetAway, weightGetAwayExplosive);

	if (mod_id == HOLYWARS_DLL) {
		goalfinder_addgoal(&pBot->parabot->goalFinder, G_MOVE, PI_HALO, goalGetItem, weightGetHalo);
	}

	DEBUG_MSG( "BOT CREATE.\n" );

	BotSpawnInit(pBot);	// init variables

	pBot->is_used = true;
	pBot->respawn_state = RESPAWN_IDLE;

	botEnt->v.idealpitch = botEnt->v.v_angle.x;
	botEnt->v.ideal_yaw = botEnt->v.v_angle.y;
	botEnt->v.pitch_speed = BOT_PITCH_SPEED;
	botEnt->v.yaw_speed = BOT_YAW_SPEED;

	pBot->bot_team = -1;  // don't know what these are yet, server can change them
	pBot->bot_class = -1;

	adjustAimSkills();	// take care of min-/maxskill
}

// Checks bot->start_action:
// if ==TEAM_SELECT selects team bot->bot_team
// if ==CLASS_SELECT selects class bot->bot_class and sets bot->not_started = 0
void BotStartGame( bot_t *pBot )
{
   const char *c_team;
   const char *c_class;

	assert( pBot != 0 );
   EDICT *pEdict = pBot->e;

   chat_registerjoin( pEdict );

   if (mod_id == TFC_DLL)
   {
      // handle Team Fortress Classic stuff here...

      if (pBot->start_action == MSG_TFC_TEAM_SELECT)
      {
		  if (pBot->menuSelectTime > worldtime()) return;

         pBot->start_action = MSG_TFC_IDLE;  // switch back to idle

         if ((pBot->bot_team != 1) && (pBot->bot_team != 2) && (pBot->bot_team != 5))
            pBot->bot_team = -1;

         if (pBot->bot_team == -1) pBot->bot_team = randomint(1, 2);

         // select the team the bot wishes to join...
	switch( pBot->bot_team )
	{
	case 1:
		c_team = "1";
		break;
	case 2:
		c_team = "2";
		break;
	default:
		c_team = "5";
		break;
	}

         FakeClientCommand(pEdict, "jointeam", c_team, NULL);
         return;
      }

      if (pBot->start_action == MSG_TFC_CLASS_SELECT)
      {
		  if (pBot->menuSelectTime > worldtime()) return;
         
		  pBot->start_action = MSG_TFC_IDLE;  // switch back to idle
		/* if (pBot->not_started == 0) {
			 DEBUG_MSG( "BOT ALREADY STARTED IN TEAMSELECT\n" );
			 return;
		 }*/
		 
         if ((pBot->bot_class < 0) || (pBot->bot_class > 10))
            pBot->bot_class = -1;

         if (pBot->bot_class == -1) pBot->bot_class = randomint(1, 10);
		// pBot->bot_class = 3;

         // select the class the bot wishes to use...
	switch( pBot->bot_class )
	{
		case 0:
			c_class = "civilian";
			break;
		case 1:
			c_class = "scout";
			break;
		case 2:
			c_class = "sniper";
			break;
		case 3:
			c_class = "soldier";
			break;
		case 4:
			c_class = "demoman";
			break;
		case 5:
			c_class = "medic";
			break;
		case 6:
			c_class = "hwguy";
			break;
		case 7:
			c_class = "pyro";
			break;
		case 8:
			c_class = "spy";
			break;
		case 9:
			c_class = "engineer";
			break;
		default:
			c_class = "randompc";
			break;
	}

		 DEBUG_MSG( "Choosen Class %i: %s\n", pBot->bot_class, c_class );
		 FakeClientCommand(pEdict, c_class, NULL, NULL);
		 DEBUG_MSG( "Started!\n" );
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

         if (pBot->bot_team == -1) pBot->bot_team = randomint(1, 2);

         // select the team the bot wishes to join...
	switch( pBot->bot_team )
        {
        case 1:
                c_team = "1";
                break;
        case 2:
                c_team = "2";
                break;
        default:
                c_team = "5";
                break;
        }

         FakeClientCommand(pEdict, "menuselect", c_team, NULL);
         return;
      }

      if (pBot->start_action == MSG_CS_CT_SELECT // counter terrorist
	|| pBot->start_action == MSG_CS_T_SELECT )  // terrorist select
      {
         pBot->start_action = MSG_CS_IDLE;  // switch back to idle

         if ((pBot->bot_class < 1) || (pBot->bot_class > 4)) 
			 pBot->bot_class = -1;

         if (pBot->bot_class == -1) pBot->bot_class = randomint(1, 4);

         // select the class the bot wishes to use...
	switch( pBot->bot_class )
	{
	case 1:
		c_class = "1";
		break;
	case 2:
		c_class = "2";
		break;
	case 3:
		c_class = "3";
		break;
	case 4:
		c_class = "4";
		break;
	default:
		c_class = "5";
		break;
	}

         FakeClientCommand(pEdict, "menuselect", c_class, NULL);
         pBot->not_started = 0;	// bot has now joined the game (doesn't need to be started)
         return;
      }
   }
   else if (mod_id == GEARBOX_DLL && (com.gamedll_flags & GAMEDLL_CTF))
   {
      // handle Opposing Force CTF stuff here...

      if (pBot->start_action == MSG_OPFOR_TEAM_SELECT)
      {
         pBot->start_action = MSG_OPFOR_IDLE;  // switch back to idle

         if ((pBot->bot_team != 1) && (pBot->bot_team != 2) &&
             (pBot->bot_team != 3))
            pBot->bot_team = -1;

         if (pBot->bot_team == -1)
            pBot->bot_team = randomint(1, 2);

         // select the team the bot wishes to join...
	switch( pBot->bot_team )
        {
        case 1:
                c_team = "1";
                break;
        case 2:
                c_team = "2";
                break;
        default:
                c_team = "3";
                break;
        }

         FakeClientCommand(pEdict, "jointeam", c_team, NULL);

         return;
      }
    
      if (pBot->start_action == MSG_OPFOR_CLASS_SELECT)
      {
         pBot->start_action = MSG_OPFOR_IDLE;  // switch back to idle
      
         if ((pBot->bot_class < 0) || (pBot->bot_class > 10))
            pBot->bot_class = -1;
         if (pBot->bot_class == -1)
            pBot->bot_class = randomint(1, 10);

	switch( pBot->bot_class )
        {
        case 1:
                c_class = "1";
                break;
        case 2:
                c_class = "2";
                break;
        case 3:
                c_class = "3";
                break;
        case 4:
                c_class = "4";
                break;
	case 5:
                c_class = "5";
                break;
	case 6:
                c_class = "6";
                break;
        default:
                c_class = "7";
                break;
        }

         FakeClientCommand(pEdict, "selectchar", c_class, NULL);

         // bot has now joined the game (doesn't need to be started)
         pBot->not_started = 0;

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
#if 0
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
   TRACERESULT tr;

   assert( pBot != 0 );
   // trace a line from bot's eyes to destination...
   UTIL_TraceLine( pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs,
                   dest, ignore_monsters,
                   pBot->pEdict->v.pContainingEntity, &tr );

   // check if line of sight to object is not blocked (i.e. visible)
   if (tr.fraction >= 1.0)
      return true;
   else
      return false;
}

void BotFixIdealYaw(EDICT *pEdict)
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
   EDICT *pEdict = pBot->pEdict;
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
   diff = fabs(current - ideal);

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
#endif
bool pb_pause = false;


// return team number 0 through 3 based what MOD uses for team numbers
int getteam(EDICT *player)
{
	const char *infobuffer;
	char teamName[32];
	char modelName[32];
	int i;

	switch( mod_id ) 
	{
	case DMC_DLL:
	case VALVE_DLL:
	case HUNGER_DLL:
	case GEARBOX_DLL:
		infobuffer = getinfokeybuffer(player);

		if(com.gamedll_flags & GAMEDLL_CTF) {
			strcpy(modelName, getinfokeyvalue(infobuffer, "model"));
			if (Q_STREQ(modelName, "ctf_barney")
			    || Q_STREQ(modelName, "cl_suit")
			    || Q_STREQ(modelName, "ctf_gina")
			    || Q_STREQ(modelName, "ctf_gordon")
			    || Q_STREQ(modelName, "otis")
			    || Q_STREQ(modelName, "ctf_scientist")) {
				return 0;
			}
			return 1;
		} else {
			strcpy(teamName, getinfokeyvalue(infobuffer, "team"));
			for (i = 0; i < valveTeamNumber; i++) {
				if (Q_STRIEQ(teamName, valveTeamList[i])) {
					return i;
				}
			}
		}
		DEBUG_MSG("ERROR: Team not found!\n");
		return 0;
	case AG_DLL:
	//case FRONTLINE_DLL:
	case TFC_DLL:
		return player->v.team - 1;  // teams are 1-4 based

	case CSTRIKE_DLL:
		infobuffer = getinfokeybuffer(player);
		strcpy(modelName, getinfokeyvalue(infobuffer, "model"));

		if (Q_STREQ(modelName, "terror")	// Phoenix Connektion
		    || Q_STREQ(modelName, "arab")	// Old L337 Krew
		    || Q_STREQ(modelName, "leet")	// L337 Krew
		    || Q_STREQ(modelName, "arctic")	// Artic Avenger
		    || Q_STREQ(modelName, "guerilla")) {	// Gorilla Warfare
			return 0;
		}
		return 1;
	default:
		int team = player->v.team;  // assume all others are 0-3 based

		if ((team < 0) || (team > 3)) {
			DEBUG_MSG( "getteam: Unknown team code = %i\n", team ); 
			team = -1;
		}			
		return team;
	}
}

int
getbotindex(EDICT *e)
{
	for (int i = 0; i < 32; i++) {
		if (bots[i].is_used && bots[i].e == e)
			return i;
	}

	return -1;
}

bot_t *
getbotpointer(EDICT *e)
{
	for (int i = 0; i < 32; i++) {
		if (bots[i].is_used && bots[i].e == e)
			return (&bots[i]);
	}

	return 0;
}

void BotThink( bot_t *pBot )
{
	Vec3D v_diff;             // vector from previous to current location
	TRACERESULT tr;

	assert( pBot != 0 );
	EDICT *pEdict = pBot->e;
	assert( pEdict != 0 );

	pEdict->v.flags |= FL_FAKECLIENT;

	if (pBot->name[0] == 0)  // name filled in yet?
		strcpy(pBot->name, STRING(pBot->e->v.netname));

	pEdict->v.button = 0;
	pBot->f_move_speed = 0.0;

	// if the bot hasn't selected stuff to start the game yet, go do that...
	if (pBot->not_started) {
		BotStartGame( pBot );
		fixangle( &pEdict->v.v_angle );
		runplayermove( pEdict, &pEdict->v.v_angle, 0.0, 0, 0, pEdict->v.button, 0, getframerateinterval());
		pEdict->v.flags |= FL_FAKECLIENT;
		return;
	}

	// if the bot is dead, randomly press fire to respawn...
	if ((pEdict->v.health < 1) || (pEdict->v.deadflag != DEAD_NO))
	{
		debugFile( "X" );
		if (pBot->need_to_initialize)
			BotSpawnInit(pBot);

		if (randomint(1, 100) > 50)
			pEdict->v.button = ACTION_ATTACK1;

		fixangle(&pEdict->v.v_angle);
		runplayermove( pEdict, &pEdict->v.v_angle, 0.0, 0, 0, pEdict->v.button, 0, getframerateinterval());
		pEdict->v.flags |= FL_FAKECLIENT;
		return;
	}

	// set this for the next time the bot dies so it will initialize stuff
	if (!pBot->need_to_initialize) {
		pBot->need_to_initialize = true;
		pBot->parabot->initAfterRespawn();
	}
  
	if ((worldtime() > roundStartTime) && !pb_pause) {	// don't call this before round has
		pBot->parabot->botThink();						// started
	} else {	// prevent crashes...???
		runplayermove( pEdict, &zerovector, 0, 0, 0, 0, 0, getframerateinterval());
	}

   pEdict->v.flags |= FL_FAKECLIENT;
   return;
}

void checkForMapChange()
{
	static float previous_time = 1000000; 
	static bool roundNotStarted = false;

	// if a new map has started then...
	if( previous_time > com.globals->time + 0.1 )
	{
#if _DEBUG
		glMarker.deleteAll();
#endif
		fatalParabotError = !loadLevelData();
		if (fatalParabotError) {
			ERROR_MSG( "The map %s is corrupt and cannot be played with bots!\nPlease exit and pick another one.", STRING(com.globals->mapname) );
		}
		observer_init();
		observerUpdate = worldtime() + 0.5;
		// 4..14 minutes
		nextAirstrikeTime = worldtime() + 240.0 + randomfloat( 0.0f, 600.0f );
		
		roundStartTime = worldtime();
		client_update_time = 0.0;  // start updating client data again
		
		// if haven't started respawning yet then set the respawn time
		if (respawn_time < 1.0) respawn_time = com.globals->time + 5.0;

		// set the time to check for automatically adding bots to dedicated servers
		bot_check_time = com.globals->time + 10.0;

		// if "map" command was used, set bots in use to respawn state...
		for (int i = 0; i < 32; i++)
		{
			if ((bots[i].is_used) && (bots[i].respawn_state != RESPAWN_NEED_TO_RESPAWN))
			{
				// bot has already been "kicked" by server so just set flag
				bots[i].respawn_state = RESPAWN_NEED_TO_RESPAWN;
			}
		}
	}
	previous_time = com.globals->time;

	if (mod_id == CSTRIKE_DLL) {
		// detect CS-roundstart
		if (worldtime() < roundStartTime) {
			roundNotStarted = true;
			return;
		}
		if (roundNotStarted) {	// things to do 1x at roundTime=0
			roundNotStarted = false;
			//observer.init();
			//observer.registerClients();
			for (int i = 0; i < com.globals->maxclients; i++) {
				if (bots[i].is_used && bots[i].respawn_state==RESPAWN_IDLE)
					bots[i].parabot->initAfterRespawn();	
			}
			DEBUG_MSG( "CS ROUND STARTED!\n" );
		}
	}
}

int gmsgHudText = 0;

void sendWelcomeToNewClients()
{
	if (welcome_index != -1) {
		EDICT *pPlayer;
		if (welcome_time == 0) {
			pPlayer = edictofindex(welcome_index);

			// are they out of observer mode yet?
			if (is_alive( pPlayer )) {
				welcome_time = worldtime() + 5.0;  // welcome in 5 seconds
			}
		} else if (worldtime() > welcome_time) {
			pPlayer = edictofindex(welcome_index);
			if (worldtime() > 30.0)		// if game has already started
				chat_parsemsg( pPlayer, " Hi " );	// make bots chat
			if (!configuration_ontouringmode()) {
				if (gmsgHudText == 0)
					gmsgHudText = RegUserMSG("HudText", -1);
				MSG_Begin(MSG_ONE, gmsgHudText, NULL, pPlayer);
				MSG_WriteString(welcome_msg);
				MSG_End();
			}
			welcome_time = 0;
			welcome_index = -1;
		}
	}
}

void checkForBotRespawn()
// are we currently respawning bots and is it time to spawn one yet?
{
	if ((respawn_time >= 1.0) && (respawn_time <= com.globals->time)) {
		int index = 0;

		// set the time to check for automatically adding bots to dedicated servers
		// bot_check_time = com.globals->time + 5.0;

		// find bot needing to be respawned...
		while ( (index < 32) && (bots[index].respawn_state != RESPAWN_NEED_TO_RESPAWN) )
			index++;

		if (index < 32) {
			bots[index].respawn_state = RESPAWN_IS_RESPAWNING;
			bots[index].is_used = false;      // free up this slot
			
			// respawn 1 bot then wait a while (otherwise engine crashes)
			BotCreate( bots[index].personality );
		
			bot_check_time = com.globals->time + 10.0;	// don't add new bots
			respawn_time = com.globals->time + 1.0;  // set next respawn time
		} else {
			respawn_time = 0.0;
		}
	}
}

void addRandomBot()
{
	BotCreate();
}

void kickRandomBot()
{
	int cnt = 0;
	while (++cnt < 500) {
		int i = randomint( 0, 32-1 );
		if (bots[i].is_used) {
			char cmd[64];
			sprintf(cmd, "kick \"%s\"\n", bots[i].name);
			servercommand( cmd );  // kick the bot using (kick "name")
			bots[i].is_used = false;
			break;
		}
	}
}

void checkForBotCreation()
// check if time to see if a bot needs to be created...
{
	if ( !configuration_onservermode() ) {
		// standard mode with fixed number of bots:
		if (bot_check_time < com.globals->time) {
			bot_check_time = com.globals->time + 5.0;		// add/kick bots with 5 sec. delay

			int numBots = 0;
			for (int b = 0; b < 32; b++)
				if (bots[b].is_used)
					numBots++;

			if (numBots < configuration_numbots() && numberOfClients < com.globals->maxclients)
				addRandomBot();
			else if (numBots > configuration_numbots())
				kickRandomBot();
		}
	} else {
		// server mode
		if (bot_check_time < com.globals->time) {
			int numBots = 0;
			for (int b = 0; b < 32; b++)
				if (bots[b].is_used)
					numBots++;

			float avgTime = 2.0f * configuration_staytime() / ( configuration_minbots() + configuration_maxbots() );
			bot_check_time = com.globals->time + randomfloat( 0.5f * avgTime, 1.5f * avgTime );
			float rnd = randomfloat(0.0f, 100.0f);

			if ( numberOfClients == com.globals->maxclients ) {
				// hold one slot free for human players
				if ( numBots > 0 ) kickRandomBot();
			} else if ( numBots < configuration_minbots() ) {
				if ( numberOfClients < (com.globals->maxclients - 1) ) {
					addRandomBot();
					if ( numBots < (configuration_minbots() - 1) )
						bot_check_time = com.globals->time + 5.0;		// add bots faster
				}
			} else if ( numBots == configuration_minbots() ) {
				if ( rnd < 50 && numberOfClients < (com.globals->maxclients - 1) ) addRandomBot(); 
			} else if ( numBots < configuration_maxbots() ) {
				if ( rnd < 30 && numberOfClients < (com.globals->maxclients - 1) ) addRandomBot();
				else if ( rnd > 70 && numBots > 0 ) kickRandomBot();
			} else if ( numBots == configuration_maxbots() ) {
				if ( rnd < 50 && numBots > 0 ) kickRandomBot();
			} else if ( numBots > configuration_maxbots() ) {
				kickRandomBot();
				if ( numBots > (configuration_maxbots() + 1) )
					bot_check_time = com.globals->time + 5.0;		// kick bots faster
			}
		}
	}
}

void checkForAirStrike()
{
	Vec3D dir;
	float dist;
	EDICT *pPlayer = 0;

	if ((airStrikeTime == 0)
	    || (worldtime() < airStrikeTime))
		return;

	for (int i = 1; i <= com.globals->maxclients; i++) {
		pPlayer = edictofindex(i);
		if (!is_alive((pPlayer))) continue;	// skip player if not alive
		if (pPlayer->v.solid == SOLID_NOT) continue;	

		bot_t *bot = getbotpointer(pPlayer);
		if (bot == 0) continue;
		if ((worldtime() - bot->parabot->lastRespawn) < 1.0) continue;

		DEBUG_MSG( "%s was save at airstrike!\n", STRING( pPlayer->v.netname ) );

		Vec3D pos;
		vcopy(&pPlayer->v.origin, &pos);
		PB_Navpoint *nearest = mapGraph.getNearestNavpoint(&pos, NAV_S_AIRSTRIKE_COVER);
		if (nearest) {
#if _DEBUG
			vsub(nearest->pos(), &pos, &dir);
			dist = vlen(&dir);
			if (dist < 256) {
				DEBUG_MSG("Airstrike cover stored nearby!\n");
			}
#endif
		} else {
			nearest = mapGraph.getNearestNavpoint(&pos, NAV_INFO_PLAYER_DM);
			if(nearest) {
				vsub(nearest->pos(), &pos, &dir);
				dist = vlen(&dir);
			}
			if (!nearest || dist > 64) {
				DEBUG_MSG("Adding airstrike cover!\n");
				PB_Navpoint coverNav;
				coverNav.init(&pos, NAV_S_AIRSTRIKE_COVER, 0);
				mapGraph.addNavpoint(coverNav);
			}
		}
	}

	airStrikeTime = 0;
	headToBunker = false;
}

void adjustAimSkills()
{
	int minAimSkill = configuration_minskill();
	int maxAimSkill = configuration_maxskill();

	for (int i=0; i<32; i++) if (bots[i].is_used) {
		int aimSkill = personalities_get( bots[i].personality ).aimskill;
		action_setaimskill(&bots[i].parabot->action,  clamp( aimSkill, maxAimSkill, minAimSkill ) );
	}
}

void DSaddbot() 
{    
	BotCreate();
}

void BotPause()
{
	pb_pause = true;
}

void BotUnPause()
{
	pb_pause = false;

	// call resetStuck() for all bots
	for( int i = 0; i < com.globals->maxclients; i++ )
	{
		if( bots[i].is_used
		    && bots[i].respawn_state == RESPAWN_IDLE ) {
			assert( bots[i].parabot != 0 );
			action_resetstuck(&bots[i].parabot->action);
		}
	}
}
