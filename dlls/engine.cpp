// Based on:
//
// HPB bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// engine.cpp
//

#include "extdll.h"
//#include "util.h"
#include "pb_global.h"
#include "sounds.h"

#include "bot.h"
#include "bot_client.h"
#include "engine.h"

// HolyWars: gets modified in writeString
bool haloOnBase = true;

extern enginefuncs_t g_engfuncs;
extern bot_t bots[32];
extern int mod_id;
extern float roundStartTime;
extern Sounds playerSounds;

bool valveTeamPlayMode = false;
char valveTeamList[MAX_TEAMS][32] = { "","","","","","","","","","","","","","","","" };
int  valveTeamNumber = 0;



int debug_engine = 0;

void (*botMsgFunction)(void *, int) = NULL;
int botMsgIndex;

// messages created in RegUserMsg which will be "caught"
int message_VGUI = 0;
int message_ShowMenu = 0;
int message_WeaponList = 0;
int message_CurWeapon = 0;
int message_AmmoX = 0;
int message_WeapPickup = 0;
int message_AmmoPickup = 0;
int message_ItemPickup = 0;
int message_Health = 0;
int message_Battery = 0;  // Armor
int message_Damage = 0;
int message_Death = 0;
int message_Money = 0;  // for Counter-Strike

static FILE *fp;



void pfnChangeLevel(char* s1, char* s2)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnChangeLevel:\n"); fclose(fp); }

   // kick any bot off of the server after time/frag limit...
   for (int index = 0; index < 32; index++)
   {
      if (bots[index].is_used)  // is this slot used?
      {
         char cmd[40];

         sprintf(cmd, "kick \"%s\"\n", bots[index].name);

         bots[index].respawn_state = RESPAWN_NEED_TO_RESPAWN;
		 bots[index].is_used = false;

         SERVER_COMMAND(cmd);  // kick the bot using (kick "name")
      }
   }

   (*g_engfuncs.pfnChangeLevel)(s1, s2);
}


void pfnMessageBegin(int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
   if (gpGlobals->deathmatch)
   {
      int index = -1;

      if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnMessageBegin: edict=%x dest=%d type=%d\n",ed,msg_dest,msg_type); fclose(fp); }

	  if (msg_type == message_Death) {
		  botMsgFunction = Client_Valve_DeathMsg;
	  }

      if (ed)
      {
		  
         index = UTIL_GetBotIndex(ed);

         // is this message for a bot?
         if (index != -1)
         {
            botMsgFunction = NULL;  // no msg function until known otherwise
            botMsgIndex = index;    // index of bot receiving message

            switch( mod_id ) {
			case VALVE_DLL:
				if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_Valve_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_Valve_CurrentWeapon;
				else if (msg_type == message_AmmoX)
					botMsgFunction = BotClient_Valve_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_Valve_AmmoPickup;
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_Valve_WeaponPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_Valve_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_Valve_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_Valve_Battery;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_Valve_Damage;
				break;
            case HOLYWARS_DLL:
				if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_Holywars_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_Holywars_CurrentWeapon;
				else if (msg_type == message_AmmoX)
					botMsgFunction = BotClient_Holywars_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_Holywars_AmmoPickup;
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_Holywars_WeaponPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_Holywars_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_Holywars_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_Holywars_Battery;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_Holywars_Damage;
				break;
            case DMC_DLL:
				if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_DMC_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_DMC_CurrentWeapon;
				else if (msg_type == message_AmmoX)
					botMsgFunction = BotClient_DMC_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_DMC_AmmoPickup;
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_DMC_WeaponPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_DMC_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_DMC_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_DMC_Battery;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_DMC_Damage;
				break;
            case TFC_DLL:
				if (msg_type == message_VGUI)
					botMsgFunction = BotClient_TFC_VGUI;
				else if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_TFC_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_TFC_CurrentWeapon;
				else if (msg_type == message_AmmoX)
					botMsgFunction = BotClient_TFC_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_TFC_AmmoPickup;
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_TFC_WeaponPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_TFC_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_TFC_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_TFC_Battery;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_TFC_Damage;
				break;
            case CSTRIKE_DLL:
				if (msg_type == message_VGUI)
					botMsgFunction = BotClient_CS_VGUI;
				else if (msg_type == message_ShowMenu)
					botMsgFunction = BotClient_CS_ShowMenu;
				else if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_CS_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_CS_CurrentWeapon;
				else if (msg_type == message_AmmoX) 
					botMsgFunction = BotClient_CS_AmmoX;
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_CS_WeaponPickup;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_CS_AmmoPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_CS_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_CS_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_CS_Battery;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_CS_Damage;
				else if (msg_type == message_Money)
					botMsgFunction = BotClient_CS_Money;
				break;
			case GEARBOX_DLL:
				if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_Gearbox_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_Gearbox_CurrentWeapon;
				else if (msg_type == message_AmmoX)
					botMsgFunction = BotClient_Gearbox_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_Gearbox_AmmoPickup;
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_Gearbox_WeaponPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_Gearbox_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_Gearbox_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_Gearbox_Battery;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_Gearbox_Damage;
				break;
			}
         }
		 else {	// message for a human client
			if (msg_type == message_CurWeapon) {
				 botMsgIndex = ENTINDEX( ed );
				 botMsgFunction = HumanClient_CurrentWeapon;
			 }
		 }
      }
   }

   (*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ed);
}


int pfnRegUserMsg(const char *pszName, int iSize)
{
	int msg = (*g_engfuncs.pfnRegUserMsg)(pszName, iSize);
	
	if (gpGlobals->deathmatch)
	{
		//fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnRegUserMsg: pszName=%s msg=%d\n",pszName,msg); fclose(fp);
				
			 if (strcmp( pszName, "WeaponList"	) == 0)	message_WeaponList = msg;
		else if (strcmp( pszName, "CurWeapon"	) == 0)	message_CurWeapon = msg;
		else if (strcmp( pszName, "AmmoX"       ) == 0)	message_AmmoX = msg;
		else if (strcmp( pszName, "AmmoPickup"  ) == 0)	message_AmmoPickup = msg;
		else if (strcmp( pszName, "WeapPickup"  ) == 0)	message_WeapPickup = msg;
		else if (strcmp( pszName, "ItemPickup"  ) == 0)	message_ItemPickup = msg;
		else if (strcmp( pszName, "Health"      ) == 0)	message_Health = msg;
		else if (strcmp( pszName, "Battery"     ) == 0)	message_Battery = msg;
		else if (strcmp( pszName, "Damage"      ) == 0)	message_Damage = msg;
		else if (strcmp( pszName, "DeathMsg"    ) == 0)	message_Death = msg;
		// TFC / CS
		else if (strcmp(pszName, "VGUIMenu"		) == 0) message_VGUI = msg;
		// CS only			
		else if (strcmp(pszName, "ShowMenu"		) == 0) message_ShowMenu = msg;
		else if (strcmp(pszName, "Money"		) == 0) message_Money = msg;         		
	}
	
	return msg;
}


void pfnMessageEnd(void)
{
   if (gpGlobals->deathmatch)
   {
      if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnMessageEnd:\n"); fclose(fp); }

      // clear out the bot message function pointer...
      botMsgFunction = NULL;
   }

   (*g_engfuncs.pfnMessageEnd)();
}



///////////////////////////////////////////////////////////////////////////////////
//
//  FORWARD ENGINE FUNCTIONS...
//
///////////////////////////////////////////////////////////////////////////////////


int pfnPrecacheModel(char* s)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnPrecacheModel: %s\n",s); fclose(fp); }
   return (*g_engfuncs.pfnPrecacheModel)(s);
}

int pfnPrecacheSound(char* s)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnPrecacheSound: %s\n",s); fclose(fp); }
   return (*g_engfuncs.pfnPrecacheSound)(s);
}

void pfnSetModel(edict_t *e, const char *m)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSetModel: edict=%x %s\n",e,m); fclose(fp); }
   (*g_engfuncs.pfnSetModel)(e, m);
}

int pfnModelIndex(const char *m)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnModelIndex: %s\n",m); fclose(fp); }
   return (*g_engfuncs.pfnModelIndex)(m);
}

int pfnModelFrames(int modelIndex)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnModelFrames:\n"); fclose(fp); }
   return (*g_engfuncs.pfnModelFrames)(modelIndex);
}

void pfnSetSize(edict_t *e, const float *rgflMin, const float *rgflMax)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSetSize: %x\n",e); fclose(fp); }
   (*g_engfuncs.pfnSetSize)(e, rgflMin, rgflMax);
}

void pfnGetSpawnParms(edict_t *ent)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetSpawnParms:\n"); fclose(fp); }
   (*g_engfuncs.pfnGetSpawnParms)(ent);
}

void pfnSaveSpawnParms(edict_t *ent)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSaveSpawnParms:\n"); fclose(fp); }
   (*g_engfuncs.pfnSaveSpawnParms)(ent);
}

float pfnVecToYaw(const float *rgflVector)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnVecToYaw:\n"); fclose(fp); }
   return (*g_engfuncs.pfnVecToYaw)(rgflVector);
}

void pfnVecToAngles(const float *rgflVectorIn, float *rgflVectorOut)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnVecToAngles:\n"); fclose(fp); }
   (*g_engfuncs.pfnVecToAngles)(rgflVectorIn, rgflVectorOut);
}

void pfnMoveToOrigin(edict_t *ent, const float *pflGoal, float dist, int iMoveType)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnMoveToOrigin:\n"); fclose(fp); }
   (*g_engfuncs.pfnMoveToOrigin)(ent, pflGoal, dist, iMoveType);
}

void pfnChangeYaw(edict_t* ent)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnChangeYaw:\n"); fclose(fp); }
   (*g_engfuncs.pfnChangeYaw)(ent);
}

void pfnChangePitch(edict_t* ent)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnChangePitch:\n"); fclose(fp); }
   (*g_engfuncs.pfnChangePitch)(ent);
}

edict_t* pfnFindEntityByString(edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue)
{
	//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnFindEntityByString: %s\n",pszValue); fclose(fp); }
	if (( mod_id == CSTRIKE_DLL ) &&
		( strcmp( pszField, "classname" ) == 0 ) && 
		( strcmp( pszValue, "info_map_parameters" ) == 0 ) ) {
		//debugMsg( "NEW CS-ROUND!\n" );
		roundStartTime = worldTime() + CVAR_GET_FLOAT("mp_freezetime");	// 5 seconds until round starts
	}
	return (*g_engfuncs.pfnFindEntityByString)(pEdictStartSearchAfter, pszField, pszValue);
}

int pfnGetEntityIllum(edict_t* pEnt)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetEntityIllum:\n"); fclose(fp); }
   return (*g_engfuncs.pfnGetEntityIllum)(pEnt);
}

edict_t* pfnFindEntityInSphere(edict_t *pEdictStartSearchAfter, const float *org, float rad)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnFindEntityInSphere:\n"); fclose(fp); }
   return (*g_engfuncs.pfnFindEntityInSphere)(pEdictStartSearchAfter, org, rad);
}

edict_t* pfnFindClientInPVS(edict_t *pEdict)
{
   //if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnFindClientInPVS:\n"); fclose(fp); }
   return (*g_engfuncs.pfnFindClientInPVS)(pEdict);
}

edict_t* pfnEntitiesInPVS(edict_t *pplayer)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnEntitiesInPVS:\n"); fclose(fp); }
   return (*g_engfuncs.pfnEntitiesInPVS)(pplayer);
}

void pfnMakeVectors(const float *rgflVector)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnMakeVectors:\n"); fclose(fp); }
   (*g_engfuncs.pfnMakeVectors)(rgflVector);
}

void pfnAngleVectors(const float *rgflVector, float *forward, float *right, float *up)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnAngleVectors:\n"); fclose(fp); }
   (*g_engfuncs.pfnAngleVectors)(rgflVector, forward, right, up);
}

edict_t* pfnCreateEntity(void)
{
   edict_t *pent = (*g_engfuncs.pfnCreateEntity)();
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCreateEntity: %x\n",pent); fclose(fp); }
   return pent;
}

void pfnRemoveEntity(edict_t* e)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnRemoveEntity: %x\n",e); fclose(fp); }
   if (debug_engine)
   {
      fp=fopen("parabot\\debug.txt", "a");
      fprintf(fp,"pfnRemoveEntity: %x\n",e);
      if (e->v.model != 0)
         fprintf(fp," model=%s\n", STRING(e->v.model));
      fclose(fp);
   }

   (*g_engfuncs.pfnRemoveEntity)(e);
}

edict_t* pfnCreateNamedEntity(int className)
{
   edict_t *pent = (*g_engfuncs.pfnCreateNamedEntity)(className);
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCreateNamedEntity: edict=%x name=%s\n",pent,STRING(className)); fclose(fp); }
   return pent;
}

void pfnMakeStatic(edict_t *ent)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnMakeStatic:\n"); fclose(fp); }
   (*g_engfuncs.pfnMakeStatic)(ent);
}

int pfnEntIsOnFloor(edict_t *e)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnEntIsOnFloor:\n"); fclose(fp); }
   return (*g_engfuncs.pfnEntIsOnFloor)(e);
}

int pfnDropToFloor(edict_t* e)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnDropToFloor:\n"); fclose(fp); }
   return (*g_engfuncs.pfnDropToFloor)(e);
}

int pfnWalkMove(edict_t *ent, float yaw, float dist, int iMode)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnWalkMove:\n"); fclose(fp); }
   return (*g_engfuncs.pfnWalkMove)(ent, yaw, dist, iMode);
}

void pfnSetOrigin(edict_t *e, const float *rgflOrigin)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSetOrigin:\n"); fclose(fp); }
   (*g_engfuncs.pfnSetOrigin)(e, rgflOrigin);
}

void pfnEmitSound(edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch)
{
	playerSounds.parseSound( entity, sample, volume );
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnEmitSound:\n"); fclose(fp); }
   (*g_engfuncs.pfnEmitSound)(entity, channel, sample, volume, attenuation, fFlags, pitch);
}

void pfnEmitAmbientSound(edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch)
{
	playerSounds.parseAmbientSound( entity, samp, vol );
	if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnEmitAmbientSound:\n"); fclose(fp); }
	(*g_engfuncs.pfnEmitAmbientSound)(entity, pos, samp, vol, attenuation, fFlags, pitch);
}

void pfnTraceLine(const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnTraceLine:\n"); fclose(fp); }
   (*g_engfuncs.pfnTraceLine)(v1, v2, fNoMonsters, pentToSkip, ptr);
}

void pfnTraceToss(edict_t* pent, edict_t* pentToIgnore, TraceResult *ptr)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnTraceToss:\n"); fclose(fp); }
   (*g_engfuncs.pfnTraceToss)(pent, pentToIgnore, ptr);
}

int pfnTraceMonsterHull(edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnTraceMonsterHull:\n"); fclose(fp); }
   return (*g_engfuncs.pfnTraceMonsterHull)(pEdict, v1, v2, fNoMonsters, pentToSkip, ptr);
}

void pfnTraceHull(const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnTraceHull:\n"); fclose(fp); }
   (*g_engfuncs.pfnTraceHull)(v1, v2, fNoMonsters, hullNumber, pentToSkip, ptr);
}

void pfnTraceModel(const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnTraceModel:\n"); fclose(fp); }
   (*g_engfuncs.pfnTraceModel)(v1, v2, hullNumber, pent, ptr);
}

const char *pfnTraceTexture(edict_t *pTextureEntity, const float *v1, const float *v2 )
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnTraceTexture:\n"); fclose(fp); }
   return (*g_engfuncs.pfnTraceTexture)(pTextureEntity, v1, v2);
}

void pfnTraceSphere(const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnTraceSphere:\n"); fclose(fp); }
   (*g_engfuncs.pfnTraceSphere)(v1, v2, fNoMonsters, radius, pentToSkip, ptr);
}

void pfnGetAimVector(edict_t* ent, float speed, float *rgflReturn)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetAimVector:\n"); fclose(fp); }
   (*g_engfuncs.pfnGetAimVector)(ent, speed, rgflReturn);
}

//void BotCreate( edict_t *pPlayer, const char *botTeam, const char *botClass, const char *arg3, const char *arg4 );

void pfnServerCommand(char* str)
{
	infoMsg( "ServerCommand: ", str, "\n" );
    if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnServerCommand: %s\n",str); fclose(fp); }
/*    if (FStrEq(str, "addbot")) {	// we've got this is DSaddbot
		BotCreate();
		return;
	}*/
    (*g_engfuncs.pfnServerCommand)(str);
}

void pfnServerExecute(void)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnServerExecute:\n"); fclose(fp); }
   (*g_engfuncs.pfnServerExecute)();
}

void pfnClientCommand(edict_t* pEdict, char* szFmt, ...)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnClientCommand=%s\n",szFmt); fclose(fp); }
   if (!(pEdict->v.flags & FL_FAKECLIENT))
   {
      char tempFmt[256];
      va_list argp;
      va_start(argp, szFmt);
      vsprintf(tempFmt, szFmt, argp);
      (*g_engfuncs.pfnClientCommand)(pEdict, tempFmt);
      va_end(argp);
   }
   return;
}

void pfnParticleEffect(const float *org, const float *dir, float color, float count)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnParticleEffect:\n"); fclose(fp); }
   (*g_engfuncs.pfnParticleEffect)(org, dir, color, count);
}

void pfnLightStyle(int style, char* val)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnLightStyle:\n"); fclose(fp); }
   (*g_engfuncs.pfnLightStyle)(style, val);
}

int pfnDecalIndex(const char *name)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnDecalIndex:\n"); fclose(fp); }
   return (*g_engfuncs.pfnDecalIndex)(name);
}

int pfnPointContents(const float *rgflVector)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnPointContents:\n"); fclose(fp); }
   return (*g_engfuncs.pfnPointContents)(rgflVector);
}

void pfnWriteByte(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnWriteByte: %d\n",iValue); fclose(fp); }

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }

   (*g_engfuncs.pfnWriteByte)(iValue);
}

void pfnWriteChar(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnWriteChar: %d\n",iValue); fclose(fp); }

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }

   (*g_engfuncs.pfnWriteChar)(iValue);
}

void pfnWriteShort(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"prnWriteShort: %d\n",iValue); fclose(fp); }

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }

   (*g_engfuncs.pfnWriteShort)(iValue);
}

void pfnWriteLong(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnWriteLong: %d\n",iValue); fclose(fp); }

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }

   (*g_engfuncs.pfnWriteLong)(iValue);
}

void pfnWriteAngle(float flValue)
{
   if (gpGlobals->deathmatch)
   {
      if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnWriteAngle: %f\n",flValue); fclose(fp); }

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&flValue, botMsgIndex);
   }

   (*g_engfuncs.pfnWriteAngle)(flValue);
}

void pfnWriteCoord(float flValue)
{
   if (gpGlobals->deathmatch)
   {
      if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnWriteCoord: %f\n",flValue); fclose(fp); }

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&flValue, botMsgIndex);
   }

   (*g_engfuncs.pfnWriteCoord)(flValue);
}

void pfnWriteString(const char *sz)
{
   if (gpGlobals->deathmatch)
   {
	   if (mod_id==HOLYWARS_DLL) {
		   if ( strncmp(sz, "The halo disappeared", 10)==0 ) {
			   //debugMsg( "HALO ON BASE!\n" );
			   haloOnBase = true;
		   }
		   else if ( strncmp(sz, "We've got a new saint", 3)==0 ) {
			   //debugMsg( "HALO NOT ON BASE!\n" );
			   haloOnBase = false;
		   }
	   }
	   //debugMsg( "MSG: ", sz, "\n" );
      if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnWriteString: %s\n",sz); fclose(fp); }

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)sz, botMsgIndex);
   }

   (*g_engfuncs.pfnWriteString)(sz);
}

void pfnWriteEntity(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnWriteEntity: %d\n",iValue); fclose(fp); }

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }

   (*g_engfuncs.pfnWriteEntity)(iValue);
}

void pfnCVarRegister(cvar_t *pCvar)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCVarRegister:\n"); fclose(fp); }
   (*g_engfuncs.pfnCVarRegister)(pCvar);
}

float pfnCVarGetFloat(const char *szVarName)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCVarGetFloat: %s\n",szVarName); fclose(fp); }
   return (*g_engfuncs.pfnCVarGetFloat)(szVarName);
}

const char* pfnCVarGetString(const char *szVarName)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCVarGetString:\n"); fclose(fp); }
   return (*g_engfuncs.pfnCVarGetString)(szVarName);
}

void pfnCVarSetFloat(const char *szVarName, float flValue)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCVarSetFloat:\n"); fclose(fp); }
   (*g_engfuncs.pfnCVarSetFloat)(szVarName, flValue);
}

void pfnCVarSetString(const char *szVarName, const char *szValue)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCVarSetString:\n"); fclose(fp); }
   (*g_engfuncs.pfnCVarSetString)(szVarName, szValue);
}

void* pfnPvAllocEntPrivateData(edict_t *pEdict, long cb)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnPvAllocEntPrivateData:\n"); fclose(fp); }
   return (*g_engfuncs.pfnPvAllocEntPrivateData)(pEdict, cb);
}

void* pfnPvEntPrivateData(edict_t *pEdict)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnPvEntPrivateData:\n"); fclose(fp); }
   return (*g_engfuncs.pfnPvEntPrivateData)(pEdict);
}

void pfnFreeEntPrivateData(edict_t *pEdict)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnFreeEntPrivateData:\n"); fclose(fp); }
   (*g_engfuncs.pfnFreeEntPrivateData)(pEdict);
}

const char* pfnSzFromIndex(int iString)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSzFromIndex:\n"); fclose(fp); }
   return (*g_engfuncs.pfnSzFromIndex)(iString);
}

int pfnAllocString(const char *szValue)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnAllocString:\n"); fclose(fp); }
   return (*g_engfuncs.pfnAllocString)(szValue);
}

entvars_t* pfnGetVarsOfEnt(edict_t *pEdict)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetVarsOfEnt:\n"); fclose(fp); }
   return (*g_engfuncs.pfnGetVarsOfEnt)(pEdict);
}

edict_t* pfnPEntityOfEntOffset(int iEntOffset)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnPEntityOfEntOffset:\n"); fclose(fp); }
   return (*g_engfuncs.pfnPEntityOfEntOffset)(iEntOffset);
}

int pfnEntOffsetOfPEntity(const edict_t *pEdict)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnEntOffsetOfPEntity: %x\n",pEdict); fclose(fp); }
   return (*g_engfuncs.pfnEntOffsetOfPEntity)(pEdict);
}

int pfnIndexOfEdict(const edict_t *pEdict)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnIndexOfEdict: %x\n",pEdict); fclose(fp); }
   return (*g_engfuncs.pfnIndexOfEdict)(pEdict);
}

edict_t* pfnPEntityOfEntIndex(int iEntIndex)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnPEntityOfEntIndex:\n"); fclose(fp); }
   return (*g_engfuncs.pfnPEntityOfEntIndex)(iEntIndex);
}

edict_t* pfnFindEntityByVars(entvars_t* pvars)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnFindEntityByVars:\n"); fclose(fp); }
   return (*g_engfuncs.pfnFindEntityByVars)(pvars);
}

void* pfnGetModelPtr(edict_t* pEdict)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetModelPtr: %x\n",pEdict); fclose(fp); }
   return (*g_engfuncs.pfnGetModelPtr)(pEdict);
}

void pfnAnimationAutomove(const edict_t* pEdict, float flTime)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnAnimationAutomove:\n"); fclose(fp); }
   (*g_engfuncs.pfnAnimationAutomove)(pEdict, flTime);
}

void pfnGetBonePosition(const edict_t* pEdict, int iBone, float *rgflOrigin, float *rgflAngles )
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetBonePosition:\n"); fclose(fp); }
   (*g_engfuncs.pfnGetBonePosition)(pEdict, iBone, rgflOrigin, rgflAngles);
}

unsigned long pfnFunctionFromName( const char *pName )
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnFunctionFromName:\n"); fclose(fp); }
   return (*g_engfuncs.pfnFunctionFromName)(pName);
}

const char *pfnNameForFunction( unsigned long function )
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnNameForFunction:\n"); fclose(fp); }
   return (*g_engfuncs.pfnNameForFunction)(function);
}

void pfnClientPrintf( edict_t* pEdict, PRINT_TYPE ptype, const char *szMsg )
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnClientPrintf:\n"); fclose(fp); }
   if (!(pEdict->v.flags & FL_FAKECLIENT)) (*g_engfuncs.pfnClientPrintf)(pEdict, ptype, szMsg);
}

void pfnServerPrint( const char *szMsg )
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnServerPrint:\n"); fclose(fp); }
   (*g_engfuncs.pfnServerPrint)(szMsg);
}

void pfnGetAttachment(const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles )
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetAttachment:\n"); fclose(fp); }
   (*g_engfuncs.pfnGetAttachment)(pEdict, iAttachment, rgflOrigin, rgflAngles);
}

void pfnCRC32_Init(CRC32_t *pulCRC)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCRC32_Init:\n"); fclose(fp); }
   (*g_engfuncs.pfnCRC32_Init)(pulCRC);
}

void pfnCRC32_ProcessBuffer(CRC32_t *pulCRC, void *p, int len)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCRC32_ProcessBuffer:\n"); fclose(fp); }
   (*g_engfuncs.pfnCRC32_ProcessBuffer)(pulCRC, p, len);
}

void pfnCRC32_ProcessByte(CRC32_t *pulCRC, unsigned char ch)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCRC32_ProcessByte:\n"); fclose(fp); }
   (*g_engfuncs.pfnCRC32_ProcessByte)(pulCRC, ch);
}

CRC32_t pfnCRC32_Final(CRC32_t pulCRC)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCRC32_Final:\n"); fclose(fp); }
   return (*g_engfuncs.pfnCRC32_Final)(pulCRC);
}

long pfnRandomLong(long lLow, long lHigh)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnRandomLong: lLow=%d lHigh=%d\n",lLow,lHigh); fclose(fp); }
   return (*g_engfuncs.pfnRandomLong)(lLow, lHigh);
}

float pfnRandomFloat(float flLow, float flHigh)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnRandomFloat:\n"); fclose(fp); }
   return (*g_engfuncs.pfnRandomFloat)(flLow, flHigh);
}

void pfnSetView(const edict_t *pClient, const edict_t *pViewent )
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSetView:\n"); fclose(fp); }
   (*g_engfuncs.pfnSetView)(pClient, pViewent);
}

float pfnTime( void )
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnTime:\n"); fclose(fp); }
   return (*g_engfuncs.pfnTime)();
}

void pfnCrosshairAngle(const edict_t *pClient, float pitch, float yaw)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCrosshairAngle:\n"); fclose(fp); }
   (*g_engfuncs.pfnCrosshairAngle)(pClient, pitch, yaw);
}

byte *pfnLoadFileForMe(char *filename, int *pLength)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnLoadFileForMe: filename=%s\n",filename); fclose(fp); }
   return (*g_engfuncs.pfnLoadFileForMe)(filename, pLength);
}

void pfnFreeFile(void *buffer)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnFreeFile:\n"); fclose(fp); }
   (*g_engfuncs.pfnFreeFile)(buffer);
}

void pfnEndSection(const char *pszSectionName)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnEndSection:\n"); fclose(fp); }
   (*g_engfuncs.pfnEndSection)(pszSectionName);
}

int pfnCompareFileTime(char *filename1, char *filename2, int *iCompare)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCompareFileTime:\n"); fclose(fp); }
   return (*g_engfuncs.pfnCompareFileTime)(filename1, filename2, iCompare);
}

void pfnGetGameDir(char *szGetGameDir)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetGameDir:\n"); fclose(fp); }
   (*g_engfuncs.pfnGetGameDir)(szGetGameDir);
}

void pfnCvar_RegisterVariable(cvar_t *variable)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCvar_RegisterVariable:\n"); fclose(fp); }
   (*g_engfuncs.pfnCvar_RegisterVariable)(variable);
}

void pfnFadeClientVolume(const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnFadeClientVolume:\n"); fclose(fp); }
   (*g_engfuncs.pfnFadeClientVolume)(pEdict, fadePercent, fadeOutSeconds, holdTime, fadeInSeconds);
}

void pfnSetClientMaxspeed(const edict_t *pEdict, float fNewMaxspeed)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSetClientMaxspeed: edict=%x %f\n",pEdict,fNewMaxspeed); fclose(fp); }
   (*g_engfuncs.pfnSetClientMaxspeed)(pEdict, fNewMaxspeed);
}

edict_t * pfnCreateFakeClient(const char *netname)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCreateFakeClient:\n"); fclose(fp); }
   return (*g_engfuncs.pfnCreateFakeClient)(netname);
}

void pfnRunPlayerMove(edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec )
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnRunPlayerMove: impulse=%i\n", impulse); fclose(fp); }
   (*g_engfuncs.pfnRunPlayerMove)(fakeclient, viewangles, forwardmove, sidemove, upmove, buttons, impulse, msec);
}

int pfnNumberOfEntities(void)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnNumberOfEntities:\n"); fclose(fp); }
   return (*g_engfuncs.pfnNumberOfEntities)();
}

char* pfnGetInfoKeyBuffer(edict_t *e)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetInfoKeyBuffer:\n"); fclose(fp); }
   return (*g_engfuncs.pfnGetInfoKeyBuffer)(e);
}

char* pfnInfoKeyValue(char *infobuffer, char *key)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnInfoKeyValue: %s %s\n",infobuffer,key); fclose(fp); }
   return (*g_engfuncs.pfnInfoKeyValue)(infobuffer, key);
}

void pfnSetKeyValue(char *infobuffer, char *key, char *value)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSetKeyValue: %s %s\n",key,value); fclose(fp); }
   (*g_engfuncs.pfnSetKeyValue)(infobuffer, key, value);
}


void pfnSetClientKeyValue( int clientIndex, char *infobuffer, char *key, char *value )
{
	if ((mod_id == VALVE_DLL || mod_id == GEARBOX_DLL) && (strcmp( key, "team" ) == 0)) {	// init teamlist
		valveTeamPlayMode = true;

		bool teamKnown = false;
		for (int team=0; team<valveTeamNumber; team++) 
			if (strcmp( value, valveTeamList[team] ) == 0) {
				teamKnown = true;
				break;
			}
		if (!teamKnown && valveTeamNumber<MAX_TEAMS) {
			strcpy( valveTeamList[valveTeamNumber], value );
			debugMsg( "Registered team ", value, "\n" );
			valveTeamNumber++;
		}
	}
	if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSetClientKeyValue: %s %s\n",key,value); fclose(fp); }
	(*g_engfuncs.pfnSetClientKeyValue)(clientIndex, infobuffer, key, value);
}


int pfnIsMapValid(char *filename)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnIsMapValid:\n"); fclose(fp); }
   return (*g_engfuncs.pfnIsMapValid)(filename);
}

void pfnStaticDecal( const float *origin, int decalIndex, int entityIndex, int modelIndex )
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnStaticDecal:\n"); fclose(fp); }
   (*g_engfuncs.pfnStaticDecal)(origin, decalIndex, entityIndex, modelIndex);
}

int pfnPrecacheGeneric(char* s)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnPrecacheGeneric: %s\n",s); fclose(fp); }
   return (*g_engfuncs.pfnPrecacheGeneric)(s);
}

int pfnGetPlayerUserId(edict_t *e )
{
   if (gpGlobals->deathmatch)
   {
      if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetPlayerUserId: %x\n",e); fclose(fp); }
   }

   return (*g_engfuncs.pfnGetPlayerUserId)(e);
}

void pfnBuildSoundMsg(edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnBuildSoundMsg:\n"); fclose(fp); }
   (*g_engfuncs.pfnBuildSoundMsg)(entity, channel, sample, volume, attenuation, fFlags, pitch, msg_dest, msg_type, pOrigin, ed);
}

int pfnIsDedicatedServer(void)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnIsDedicatedServer:\n"); fclose(fp); }
   return (*g_engfuncs.pfnIsDedicatedServer)();
}

cvar_t* pfnCVarGetPointer(const char *szVarName)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCVarGetPointer: %s\n",szVarName); fclose(fp); }
   return (*g_engfuncs.pfnCVarGetPointer)(szVarName);
}

unsigned int pfnGetPlayerWONId(edict_t *e)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetPlayerWONId: %x\n",e); fclose(fp); }
   return (*g_engfuncs.pfnGetPlayerWONId)(e);
}


// new stuff for SDK 2.0

void pfnInfo_RemoveKey(char *s, const char *key)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnInfo_RemoveKey:\n"); fclose(fp); }
   (*g_engfuncs.pfnInfo_RemoveKey)(s, key);
}

const char *pfnGetPhysicsKeyValue(const edict_t *pClient, const char *key)
{
	const char *res = (*g_engfuncs.pfnGetPhysicsKeyValue)(pClient, key);
	if (debug_engine) { 
		fp=fopen("parabot\\debug.txt", "a"); 
		fprintf(fp,"pfnGetPhysicsKeyValue: key=%s, result=%s\n", key, res); 
		fclose(fp); 
	}
	int ir = (int) res[0];
	//debugMsg( "PK=%i\n", ir );
	return res;
}

void pfnSetPhysicsKeyValue(const edict_t *pClient, const char *key, const char *value)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSetPhysicsKeyValue:\n"); fclose(fp); }
   (*g_engfuncs.pfnSetPhysicsKeyValue)(pClient, key, value);
}

const char *pfnGetPhysicsInfoString(const edict_t *pClient)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetPhysicsInfoString:\n"); fclose(fp); }
   return (*g_engfuncs.pfnGetPhysicsInfoString)(pClient);
}

unsigned short pfnPrecacheEvent(int type, const char *psz)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnPrecacheEvent:\n"); fclose(fp); }
   return (*g_engfuncs.pfnPrecacheEvent)(type, psz);
}

void pfnPlaybackEvent(int flags, const edict_t *pInvoker, unsigned short eventindex, float delay,
   float *origin, float *angles, float fparam1,float fparam2, int iparam1, int iparam2, int bparam1, int bparam2)
{
	if (debug_engine) { 
		fp=fopen("parabot\\debug.txt", "a"); 
		fprintf(fp,"pfnPlaybackEvent(flags=%i,index=%i, delay=%.2f)\n", flags, eventindex, delay); 
		fclose(fp); 
	}
   (*g_engfuncs.pfnPlaybackEvent)(flags, pInvoker, eventindex, delay, origin, angles, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2);
}

unsigned char *pfnSetFatPVS(float *org)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSetFatPVS:\n"); fclose(fp); }
   return (*g_engfuncs.pfnSetFatPVS)(org);
}

unsigned char *pfnSetFatPAS(float *org)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSetFatPAS:\n"); fclose(fp); }
   return (*g_engfuncs.pfnSetFatPAS)(org);
}

int pfnCheckVisibility(const edict_t *entity, unsigned char *pset)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCheckVisibility:\n"); fclose(fp); }
   return (*g_engfuncs.pfnCheckVisibility)(entity, pset);
}

void pfnDeltaSetField(struct delta_s *pFields, const char *fieldname)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnDeltaSetField:\n"); fclose(fp); }
   (*g_engfuncs.pfnDeltaSetField)(pFields, fieldname);
}

void pfnDeltaUnsetField(struct delta_s *pFields, const char *fieldname)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnDeltaUnsetField:\n"); fclose(fp); }
   (*g_engfuncs.pfnDeltaUnsetField)(pFields, fieldname);
}

void pfnDeltaAddEncoder(char *name, void (*conditionalencode)( struct delta_s *pFields, const unsigned char *from, const unsigned char *to))
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnDeltaAddEncoder:\n"); fclose(fp); }
   (*g_engfuncs.pfnDeltaAddEncoder)(name, conditionalencode);
}

int pfnGetCurrentPlayer(void)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetCurrentPlayer: "); fclose(fp); }
   return (*g_engfuncs.pfnGetCurrentPlayer)();
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"ok\n"); fclose(fp); }
}

int pfnCanSkipPlayer(const edict_t *player)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCanSkipPlayer:\n"); fclose(fp); }
   return (*g_engfuncs.pfnCanSkipPlayer)(player);
}

int pfnDeltaFindField(struct delta_s *pFields, const char *fieldname)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnDeltaFindField:\n"); fclose(fp); }
   return (*g_engfuncs.pfnDeltaFindField)(pFields, fieldname);
}

void pfnDeltaSetFieldByIndex(struct delta_s *pFields, int fieldNumber)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnDeltaSetFieldByIndex:\n"); fclose(fp); }
   (*g_engfuncs.pfnDeltaSetFieldByIndex)(pFields, fieldNumber);
}

void pfnDeltaUnsetFieldByIndex(struct delta_s *pFields, int fieldNumber)
{
//   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnDeltaUnsetFieldByIndex:\n"); fclose(fp); }
   (*g_engfuncs.pfnDeltaUnsetFieldByIndex)(pFields, fieldNumber);
}

void pfnSetGroupMask(int mask, int op)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnSetGroupMask:\n"); fclose(fp); }
   (*g_engfuncs.pfnSetGroupMask)(mask, op);
}

int pfnCreateInstancedBaseline(int classname, struct entity_state_s *baseline)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCreateInstancedBaseline:\n"); fclose(fp); }
   return (*g_engfuncs.pfnCreateInstancedBaseline)(classname, baseline);
}

void pfnCvar_DirectSet(struct cvar_s *var, char *value)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnCvar_DirectSet:\n"); fclose(fp); }
   (*g_engfuncs.pfnCvar_DirectSet)(var, value);
}

void pfnForceUnmodified(FORCE_TYPE type, float *mins, float *maxs, const char *filename)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnForceUnmodified:\n"); fclose(fp); }
   (*g_engfuncs.pfnForceUnmodified)(type, mins, maxs, filename);
}

void pfnGetPlayerStats(const edict_t *pClient, int *ping, int *packet_loss)
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnGetPlayerStats:\n"); fclose(fp); }
   (*g_engfuncs.pfnGetPlayerStats)(pClient, ping, packet_loss);
}

void pfnAddServerCommand( char *cmd_name, void (*function) (void) )
{
   if (debug_engine) { fp=fopen("parabot\\debug.txt", "a"); fprintf(fp,"pfnAddServerCommand:\n"); fclose(fp); }
   (*g_engfuncs.pfnAddServerCommand)( cmd_name, function );
}
